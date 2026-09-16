# NppMarkdownPanel (Native C++ Modern Edition) 🚀

[![CI_build](https://github.com/theneet0/NppMarkdownPanel/actions/workflows/CI_build.yml/badge.svg)](https://github.com/theneet0/NppMarkdownPanel/actions/workflows/CI_build.yml)
[![Release](https://img.shields.io/github/v/release/theneet0/NppMarkdownPanel?color=brightgreen)](https://github.com/theneet0/NppMarkdownPanel/releases)
[![Standard](https://img.shields.io/badge/C%2B%2B-26%20%2F%2023-blue.svg)](https://en.cppreference.com/)
[![Rendering](https://img.shields.io/badge/Engine-WebView2%20%2B%20Direct2D-purple.svg)](https://docs.microsoft.com/en-us/microsoft-edge/webview2/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](License.txt)

> **High-performance, ultra-modern native Markdown preview panel plugin for Notepad++ powered by dual-engine architecture (Evergreen Chromium WebView2 + GPU-accelerated Direct2D fallback), 100% offline assets, intelligent BiDi RTL Persian/Arabic typography, KaTeX math rendering, interactive Mermaid diagrams, responsive non-overlapping TOC sidebar, and custom right-click context menu with zoom controls.**

---

## ✨ Key Features & Highlights

- 🌐 **Dual-Engine Architecture**:
  - **Default: Microsoft WebView2 (Evergreen Chromium)**: Modern HTML5/CSS3 rendering, responsive layouts, smooth scrolling, sub-pixel typography, and zero edge-clipping.
  - **Fallback: Direct2D 1.1 / DirectWrite Native GPU Engine**: Hardware-accelerated native Win32 fallback guaranteeing functionality even on restricted or legacy systems without WebView2.
- ⚡ **100% Offline & Zero Network Latency**:
  - Built-in offline MathML engine compatible with KaTeX/LaTeX syntax.
  - Procedural offline SVG Mermaid diagram generation for flowcharts and sequence graphs.
  - Complete elimination of external Google Fonts, CDN scripts, and internet dependencies. Instant preview loading under all firewall and offline network conditions.
- 🔍 **Interactive Right-Click Context Menu & Zoom Controls**:
  - Clean context menu tailored specifically for markdown previewing:
    - 🔍 **Zoom In** (`Ctrl + +` / `Ctrl + =`)
    - 🔍 **Zoom Out** (`Ctrl + -`)
    - 🔍 **Reset Zoom 100%** (`Ctrl + 0`)
    - ✂️ **Copy Selection** (`Ctrl + C`)
    - 🔍 **Find in Document** (`Ctrl + F`)
    - 📑 **Outline / Table of Contents**
    - 🌓 **Toggle Dark / Light Theme**
    - 🔄 **Toggle Caret Sync Scroll**
    - 🌐 **Smart BiDi (RTL / LTR)**
    - 📋 **Copy Full Rendered HTML**
    - 💾 **Save As HTML...**
    - 📄 **Print / Save as PDF**
- 📑 **Non-Overlapping Responsive TOC Sidebar**:
  - Fixed-layout sliding Table of Contents drawer with smooth anchor navigation, active section indicators, and semi-transparent backdrop overlay.
  - Dynamically adapts to panel width to prevent content occlusion.
- 🔎 **Real-Time In-Page Search**:
  - Interactive search overlay (`Ctrl+F`) with match counter (`[1/5]`), cycle navigation (`Enter` / `Shift+Enter`), smooth scrolling, and keyword highlights.
- 📐 **KaTeX / LaTeX Mathematical Expressions**:
  - Fast inline math expressions (`$E=mc^2$`) and display equation blocks (`$$\sum_{i=1}^n i = \frac{n(n+1)}{2}$$`).
- 📊 **Interactive Mermaid.js Diagram Support**:
  - Native offline procedural SVG rendering for flowcharts, sequence diagrams, mindmaps, and block graphs inside ````mermaid code fences.
- 🌍 **Intelligent BiDi RTL & Persian/Arabic Typography**:
  - Automatic paragraph direction detection (RTL / LTR) via custom native `BiDiEngine`.
  - Inline code isolation prevents bidirectional punctuation corruption and text reversal.
  - Optimized typography with font fallbacks (`Vazirmatn`, `Segoe UI`, `Tahoma`).
- 💻 **macOS-Style Code Blocks with 1-Click Copy**:
  - Clean cards featuring macOS traffic light controls, language badges, and one-click copy with feedback notification.
- ☑️ **Bidirectional 2-Way Task List Sync**:
  - Clicking `[ ]` task checkboxes in the preview window immediately toggles the corresponding `[x]` markdown text in the active Notepad++ Scintilla document.
- 🖨️ **HTML Export & PDF Printing**:
  - One-click clipboard copy of complete styled HTML, standalone HTML export, and native print dialog integration (Save to PDF).

---

## 🏛️ Architecture Overview

```
 ┌─────────────────────────────────────────────────────────────┐
 │                      Notepad++ Host                         │
 └──────────────────────────────┬──────────────────────────────┘
                                │ Win32 / Scintilla Messages
                                ▼
 ┌─────────────────────────────────────────────────────────────┐
 │              NppMarkdownPanel Plugin Core (C++)             │
 │                                                             │
 │  ┌───────────────────────┐       ┌───────────────────────┐  │
 │  │    MarkdownParser     │       │      BiDiEngine       │  │
 │  │  GFM, Tables, KaTeX   │       │   RTL / Inline Code   │  │
 │  └───────────┬───────────┘       └───────────┬───────────┘  │
 │              └───────────────┬───────────────┘              │
 │                              ▼                              │
 │              ┌───────────────────────────────┐              │
 │              │         HtmlExporter          │              │
 │              │  100% Offline Template + CSS  │              │
 │              └───────────────┬───────────────┘              │
 └──────────────────────────────┼──────────────────────────────┘
                                │
        ┌───────────────────────┴───────────────────────┐
        ▼                                               ▼
 ┌─────────────────────────────┐         ┌─────────────────────────────┐
 │    WebView2Viewer Engine    │         │  Direct2D Fallback Engine   │
 │  Chromium Core + IPC Sync   │         │ DirectWrite Hardware Accel  │
 └─────────────────────────────┘         └─────────────────────────────┘
```

---

## 📥 Installation

### Manual Installation

1. Download the latest release package matching your Notepad++ architecture (`x64` or `x86`) from the [Releases](https://github.com/theneet0/NppMarkdownPanel/releases) page.
2. In Notepad++, open `Settings` -> `Import` or navigate to your plugins folder:
   - **64-bit Notepad++**: `C:\Program Files\Notepad++\plugins\`
   - **32-bit Notepad++**: `C:\Program Files (x86)\Notepad++\plugins\`
   - **Portable Notepad++**: `<Notepad++_Folder>\plugins\`
3. Create a folder named `NppMarkdownPanel`.
4. Extract `NppMarkdownPanel.dll` and `WebView2Loader.dll` into that folder:
   ```
   <Notepad++>\plugins\NppMarkdownPanel\NppMarkdownPanel.dll
   <Notepad++>\plugins\NppMarkdownPanel\WebView2Loader.dll
   ```
5. Restart Notepad++.

---

## ⌨️ Default Shortcuts

| Action | Shortcut | Description |
| :--- | :---: | :--- |
| **Toggle Markdown Panel** | `Ctrl + Shift + M` | Toggle preview panel visibility |
| **Find in Document** | `Ctrl + F` | Open realtime search bar in preview |
| **Zoom In** | `Ctrl + +` / `Ctrl + =` | Increase preview zoom factor |
| **Zoom Out** | `Ctrl + -` | Decrease preview zoom factor |
| **Reset Zoom** | `Ctrl + 0` | Reset preview zoom to 100% |
| **Context Menu** | `Right-Click` | Open full preview action menu |
| **Close Overlay / Menu** | `Escape` | Close search overlay, TOC, or context menu |

---

## 🛠️ Building from Source

### Prerequisites
- Clang / LLVM toolchain with C++23/C++26 support (`clang++`, `windres`)
- Microsoft WebView2 SDK (automatically cached and restored during build)

### Build Commands
Run from PowerShell in the repository root:

```powershell
# 1. Run unit test suite and compile both x64 and x86 DLLs
.\build.ps1

# 2. Package release zip archives
.\makerelease.ps1
```

Compiled binaries will be generated in `bin/` (`x64`) and `bin/x86/` (`x86`), and packaged into `Release/`.

---

## 📄 License

This project is licensed under the **MIT License**. See [License.txt](License.txt) for details.

Copyright (c) 2026 [theneet0](https://github.com/theneet0).
