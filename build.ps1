# Build script for NppMarkdownPanel Native C++26 Edition
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $scriptDir

# 1. Locate toolchain (clang++ and windres)
$clang64 = "clang++"
$clang32 = "i686-w64-mingw32-clang++"
$windres64 = "windres"
$windres32 = "i686-w64-mingw32-windres"

if (-not (Get-Command $clang64 -ErrorAction SilentlyContinue)) {
    $wingetBin = Get-ChildItem -Path "$env:LOCALAPPDATA\Microsoft\WinGet\Packages" -Recurse -Filter "clang++.exe" -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty DirectoryName
    if ($wingetBin) {
        $env:PATH = "$wingetBin;$env:PATH"
    } else {
        Write-Error "clang++ compiler could not be found!"
        exit 1
    }
}

Write-Host ">>> Using Compiler: $(Get-Command $clang64 | Select-Object -ExpandProperty Source)" -ForegroundColor Cyan

# Ensure bin directories
$binDir = Join-Path $scriptDir "bin"
$bin32Dir = Join-Path $binDir "x86"
if (-not (Test-Path $binDir)) { New-Item -ItemType Directory -Path $binDir | Out-Null }
if (-not (Test-Path $bin32Dir)) { New-Item -ItemType Directory -Path $bin32Dir | Out-Null }

# Ensure WebView2 package is present from local nuget cache
$wv2PkgDir = Join-Path $scriptDir "packages\Microsoft.Web.WebView2.1.0.3650.58"
if (-not (Test-Path "$wv2PkgDir\build\native\include\WebView2.h")) {
    $nugetCache = Join-Path $env:USERPROFILE ".nuget\packages\microsoft.web.webview2\1.0.3650.58"
    if (Test-Path "$nugetCache\build\native\include\WebView2.h") {
        New-Item -ItemType Directory -Path (Split-Path -Parent $wv2PkgDir) -Force | Out-Null
        Copy-Item -Path $nugetCache -Destination $wv2PkgDir -Recurse -Force
    }
}

# 2. Compile and run unit tests
Write-Host "`n>>> [1/3] Building and running unit tests..." -ForegroundColor Yellow
$testExe = Join-Path $binDir "test_suite.exe"
$testSources = @(
    (Join-Path $scriptDir "tests\test_suite.cpp"),
    (Join-Path $scriptDir "src\MarkdownParser.cpp"),
    (Join-Path $scriptDir "src\BiDiEngine.cpp"),
    (Join-Path $scriptDir "src\SyntaxHighlighter.cpp"),
    (Join-Path $scriptDir "src\HtmlExporter.cpp")
)

& $clang64 -std=c++23 -O2 -I"$scriptDir\include" $testSources -luser32 -o "$testExe"
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to compile unit tests!"
    exit $LASTEXITCODE
}

& $testExe
if ($LASTEXITCODE -ne 0) {
    Write-Error "Unit tests failed!"
    exit $LASTEXITCODE
}

# 3. Build x64 DLL
Write-Host "`n>>> [2/3] Compiling NppMarkdownPanel.dll (x64 Native)..." -ForegroundColor Yellow
$resFile64 = Join-Path $binDir "resource.res"
& $windres64 (Join-Path $scriptDir "res\NppMarkdownPanel.rc") -O coff -o "$resFile64"

$dllPath64 = Join-Path $binDir "NppMarkdownPanel.dll"
$pluginSources = @(
    (Join-Path $scriptDir "src\Main.cpp"),
    (Join-Path $scriptDir "src\NppMarkdownPanel.cpp"),
    (Join-Path $scriptDir "src\WebView2Viewer.cpp"),
    (Join-Path $scriptDir "src\MarkdownRenderer.cpp"),
    (Join-Path $scriptDir "src\MarkdownParser.cpp"),
    (Join-Path $scriptDir "src\BiDiEngine.cpp"),
    (Join-Path $scriptDir "src\SyntaxHighlighter.cpp"),
    (Join-Path $scriptDir "src\OutlineView.cpp"),
    (Join-Path $scriptDir "src\HtmlExporter.cpp"),
    (Join-Path $scriptDir "src\Config.cpp"),
    $resFile64
)

$wv2Include = Join-Path $scriptDir "packages\Microsoft.Web.WebView2.1.0.3650.58\build\native\include"
$libs = @("-ld2d1", "-ldwrite", "-luser32", "-lgdi32", "-lcomctl32", "-lshlwapi", "-lole32", "-luuid", "-lcomdlg32")

& $clang64 -shared -std=c++23 -O3 -static -municode -I"$scriptDir\include" -I"$wv2Include" $pluginSources $libs -o "$dllPath64"
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to compile x64 NppMarkdownPanel.dll!"
    exit $LASTEXITCODE
}
strip --strip-all "$dllPath64"

# Copy x64 WebView2Loader.dll to bin
$wv2Loader64 = Join-Path $scriptDir "packages\Microsoft.Web.WebView2.1.0.3650.58\build\native\x64\WebView2Loader.dll"
if (Test-Path $wv2Loader64) {
    Copy-Item $wv2Loader64 -Destination (Join-Path $binDir "WebView2Loader.dll") -Force
}

Write-Host "Built x64 DLL: $dllPath64 ($( (Get-Item $dllPath64).Length / 1KB ) KB)" -ForegroundColor Green

# 4. Build x86 DLL (if 32-bit compiler is available)
if (Get-Command $clang32 -ErrorAction SilentlyContinue) {
    Write-Host "`n>>> [3/3] Compiling NppMarkdownPanel.dll (x86 Native)..." -ForegroundColor Yellow
    $resFile32 = Join-Path $bin32Dir "resource.res"
    & $windres32 (Join-Path $scriptDir "res\NppMarkdownPanel.rc") -O coff -o "$resFile32"

    $dllPath32 = Join-Path $bin32Dir "NppMarkdownPanel.dll"
    $pluginSources32 = @(
        (Join-Path $scriptDir "src\Main.cpp"),
        (Join-Path $scriptDir "src\NppMarkdownPanel.cpp"),
        (Join-Path $scriptDir "src\WebView2Viewer.cpp"),
        (Join-Path $scriptDir "src\MarkdownRenderer.cpp"),
        (Join-Path $scriptDir "src\MarkdownParser.cpp"),
        (Join-Path $scriptDir "src\BiDiEngine.cpp"),
        (Join-Path $scriptDir "src\SyntaxHighlighter.cpp"),
        (Join-Path $scriptDir "src\OutlineView.cpp"),
        (Join-Path $scriptDir "src\HtmlExporter.cpp"),
        (Join-Path $scriptDir "src\Config.cpp"),
        $resFile32
    )

    & $clang32 -shared -std=c++23 -O3 -static -municode -I"$scriptDir\include" -I"$wv2Include" $pluginSources32 $libs -o "$dllPath32"
    if ($LASTEXITCODE -eq 0) {
        strip --strip-all "$dllPath32"

        # Copy x86 WebView2Loader.dll to bin\x86
        $wv2Loader32 = Join-Path $scriptDir "packages\Microsoft.Web.WebView2.1.0.3650.58\build\native\x86\WebView2Loader.dll"
        if (Test-Path $wv2Loader32) {
            Copy-Item $wv2Loader32 -Destination (Join-Path $bin32Dir "WebView2Loader.dll") -Force
        }

        Write-Host "Built x86 DLL: $dllPath32 ($( (Get-Item $dllPath32).Length / 1KB ) KB)" -ForegroundColor Green
    }
}

Write-Host "`n[SUCCESS] Build completed successfully!" -ForegroundColor Green