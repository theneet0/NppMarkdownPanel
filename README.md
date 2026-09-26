# NppMarkdownPanel

[![CI Build](https://github.com/theneet0/NppMarkdownPanel/actions/workflows/CI_build.yml/badge.svg)](https://github.com/theneet0/NppMarkdownPanel/actions/workflows/CI_build.yml)
[![Release](https://img.shields.io/github/v/release/theneet0/NppMarkdownPanel?color=brightgreen)](https://github.com/theneet0/NppMarkdownPanel/releases)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-23%20%2F%2026-blue.svg)](https://en.cppreference.com/)
[![Engines](https://img.shields.io/badge/Engine-WebView2%20%2B%20Direct2D-purple.svg)](https://docs.microsoft.com/en-us/microsoft-edge/webview2/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](License.txt)

A high-performance native Markdown preview plugin for Notepad++. Features a dual-engine architecture combining Evergreen Chromium (WebView2) with a GPU-accelerated Direct2D fallback, full bidirectional (RTL) typography for Persian and Arabic, offline LaTeX math and Mermaid diagrams, and Material Design 3 theming.

---

## Features

- **Dual-Engine Rendering**:
  - **Primary: Microsoft WebView2 (Chromium)**: Modern HTML5/CSS3 rendering with sub-pixel typography, smooth scrolling, and dynamic layout.
  - **Fallback: Direct2D 1.1 / DirectWrite**: Native GPU-accelerated hardware rendering fallback for environments without WebView2.
- **Material Design 3 Theming**:
  - Clean, minimal cards and tonal palettes in both Light and Dark modes.
  - Minimal code block cards with syntax language pill badges and 1-click clipboard copy.
  - Native M3 styling for alert callouts (`[!NOTE]`, `[!TIP]`, `[!WARNING]`), blockquotes, and tables.
- **Bidirectional (RTL) Typography & Persian/Arabic Support**:
  - Automatic paragraph direction detection via native `BiDiEngine`.
  - Line-by-line BiDi isolation (`dir="rtl"` / `dir="ltr"`) within code blocks to ensure mixed Persian and English commands render in correct reading order.
  - Inline code isolation prevents bidirectional punctuation corruption and text reversal.
- **LaTeX Math & Mermaid Diagrams**:
  - Built-in offline MathML engine for inline math (`$E=mc^2$`) and display equations (`$$\sum_{i=1}^n i$$`).
  - Offline SVG rendering for flowcharts and sequence graphs within ````mermaid code blocks.
  - 100% offline with zero CDN dependencies or external web requests.
- **Navigation & Search**:
  - Responsive, non-overlapping Table of Contents (TOC) drawer.
  - Real-time in-page search (`Ctrl + F`) with match counting, cycle navigation, and keyword highlights.
- **Editor Synchronization**:
  - Bidirectional caret and scroll tracking with active Scintilla document.
  - 2-way task list synchronization: clicking checkboxes in preview toggles `[x]` in the active editor.
- **Context Menu & Controls**:
  - Right-click context menu with zoom controls (`Ctrl++`, `Ctrl+-`, `Ctrl+0`), theme toggle, copy full HTML, standalone HTML export, and PDF printing.

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

1. Download the latest release package matching your Notepad++ architecture (`x64`, `x86`, or `arm64`) from the [Releases](https://github.com/theneet0/NppMarkdownPanel/releases) page.
2. In Notepad++, open `Settings` -> `Import` or navigate to your plugins folder:
   - **64-bit Notepad++ (x64)**: `C:\Program Files\Notepad++\plugins\`
   - **ARM64 Notepad++ (ARM64)**: `C:\Program Files\Notepad++\plugins\`
   - **32-bit Notepad++ (x86)**: `C:\Program Files (x86)\Notepad++\plugins\`
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
- Clang / LLVM toolchain with C++23/C++26 support (`clang++`, `windres`, plus optional `i686` and `aarch64` targets)
- Microsoft WebView2 SDK (automatically cached and restored during build)

### Build Commands
Run from PowerShell in the repository root:

```powershell
# 1. Run unit test suite and compile x64, x86, and arm64 DLLs
.\build.ps1

# 2. Package release zip archives
.\makerelease.ps1
```

Compiled binaries will be generated in `bin/` (`x64`), `bin/x86/` (`x86`), and `bin/arm64/` (`arm64`), and packaged into `Release/`.

---

## 📄 License

This project is licensed under the **MIT License**. See [License.txt](License.txt) for details.

Copyright (c) 2026 [theneet0](https://github.com/theneet0).
