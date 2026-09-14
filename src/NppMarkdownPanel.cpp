#include "../include/NppMarkdownPanel.h"
#include "../include/BiDiEngine.h"
#include "../include/HtmlExporter.h"
#include "../include/Resource.h"
#include <commctrl.h>
#include <shlwapi.h>
#include <commdlg.h>
#include <algorithm>

namespace {
const wchar_t* s_panelClassName = L"NppMarkdownPanelWindowClass";
const wchar_t* s_renderCanvasClassName = L"NppMarkdownCanvasWindowClass";
}

NppMarkdownPanel& NppMarkdownPanel::Instance() {
    static NppMarkdownPanel s_inst;
    return s_inst;
}

void NppMarkdownPanel::Init(HINSTANCE hInst, NppData nppData) {
    m_hInst = hInst;
    m_nppData = nppData;

    // Load config path from Notepad++ plugin config directory
    wchar_t configDir[MAX_PATH] = { 0 };
    SendMessage(m_nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)configDir);
    m_configPath = std::wstring(configDir) + L"\\NppMarkdownPanel.ini";
    m_config.Load(m_configPath);

    CreatePanelWindow();
}

void NppMarkdownPanel::Cleanup() {
    m_config.Save(m_configPath);
    if (m_hPanel && IsWindow(m_hPanel)) {
        DestroyWindow(m_hPanel);
    }
}

bool NppMarkdownPanel::CreatePanelWindow() {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
    wc.lpfnWndProc = PanelWndProc;
    wc.hInstance = m_hInst;
    wc.lpszClassName = s_panelClassName;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassEx(&wc);

    m_hPanel = CreateWindowEx(
        0,
        s_panelClassName,
        L"Markdown Panel",
        WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, 450, 700,
        m_nppData._nppHandle,
        nullptr,
        m_hInst,
        this
    );

    if (!m_hPanel) return false;

    // Initialize Direct2D Renderer
    m_renderer.Initialize(m_hPanel);
    m_renderer.SetZoom(m_config.zoomLevel);

    // Dark mode check from Notepad++
    bool isNppDark = SendMessage(m_nppData._nppHandle, NPPM_ISDARKMODEENABLED, 0, 0) != 0;
    if (m_config.darkModeOverride != -1) {
        m_renderer.SetDarkMode(m_config.darkModeOverride == 1);
    } else {
        m_renderer.SetDarkMode(isNppDark);
    }

    // Initialize Outline View
    m_outlineView.Create(m_hPanel, m_hInst);
    m_outlineView.SetDarkMode(m_renderer.IsDarkMode());
    m_outlineView.SetCallback([](int line, void* userData) {
        auto* panel = reinterpret_cast<NppMarkdownPanel*>(userData);
        if (panel) {
            panel->m_renderer.ScrollToSourceLine(line);
        }
    }, this);

    // Setup docking data
    m_tbData.hClient = m_hPanel;
    m_tbData.pszName = L"Markdown Panel";
    m_tbData.dlgID = CMD_TOGGLE_PANEL;
    m_tbData.uMask = DWS_DF_CONT_RIGHT | DWS_ICONTAB | DWS_ADDINFO;
    m_tbData.hIconTab = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_ICON_PANEL));
    m_tbData.pszAddInfo = L"";
    m_tbData.rcFloat = { 0, 0, 450, 700 };
    m_tbData.iPrevCont = CONT_RIGHT;
    m_tbData.pszModuleName = L"NppMarkdownPanel.dll";

    // Register docking dialog with Notepad++
    SendMessage(m_nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&m_tbData);
    m_isPanelRegistered = true;

    // Create modern toolbar controls inside panel
    int btnX = 6;
    int btnY = 6;
    int btnH = 26;

    CreateWindowEx(0, L"BUTTON", L"⟳", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, btnX, btnY, 32, btnH, m_hPanel, (HMENU)IDC_BTN_REFRESH, m_hInst, nullptr);
    btnX += 36;
    CreateWindowEx(0, L"BUTTON", L"☰ Outline", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, btnX, btnY, 70, btnH, m_hPanel, (HMENU)IDC_BTN_OUTLINE, m_hInst, nullptr);
    btnX += 74;
    CreateWindowEx(0, L"BUTTON", L"+", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, btnX, btnY, 30, btnH, m_hPanel, (HMENU)IDC_BTN_ZOOM_IN, m_hInst, nullptr);
    btnX += 34;
    CreateWindowEx(0, L"BUTTON", L"-", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, btnX, btnY, 30, btnH, m_hPanel, (HMENU)IDC_BTN_ZOOM_OUT, m_hInst, nullptr);
    btnX += 34;
    CreateWindowEx(0, L"BUTTON", L"📋 HTML", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, btnX, btnY, 65, btnH, m_hPanel, (HMENU)IDC_BTN_COPY_HTML, m_hInst, nullptr);
    btnX += 69;
    CreateWindowEx(0, L"BUTTON", L"💾 Export", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, btnX, btnY, 65, btnH, m_hPanel, (HMENU)IDC_BTN_EXPORT_HTML, m_hInst, nullptr);
    btnX += 69;
    CreateWindowEx(0, L"BUTTON", L"🔄 Sync", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_PUSHLIKE, btnX, btnY, 60, btnH, m_hPanel, (HMENU)IDC_BTN_SYNC_TOGGLE, m_hInst, nullptr);
    SendMessage(GetDlgItem(m_hPanel, IDC_BTN_SYNC_TOGGLE), BM_SETCHECK, m_config.syncWithCaret ? BST_CHECKED : BST_UNCHECKED, 0);

    return true;
}

HWND NppMarkdownPanel::GetCurrentScintilla() const {
    int which = -1;
    SendMessage(m_nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    return (which == 0) ? m_nppData._scintillaMainHandle : m_nppData._scintillaSecondHandle;
}

std::string NppMarkdownPanel::GetScintillaText(HWND hSci) const {
    if (!hSci) return "";
    int length = static_cast<int>(SendMessage(hSci, SCI_GETLENGTH, 0, 0));
    if (length <= 0) return "";

    std::string buffer(length + 1, '\0');
    SendMessage(hSci, SCI_GETTEXT, length + 1, (LPARAM)&buffer[0]);
    buffer.resize(length);
    return buffer;
}

int NppMarkdownPanel::GetScintillaCaretLine(HWND hSci) const {
    if (!hSci) return 0;
    int pos = static_cast<int>(SendMessage(hSci, SCI_GETCURRENTPOS, 0, 0));
    return static_cast<int>(SendMessage(hSci, SCI_LINEFROMPOSITION, pos, 0));
}

int NppMarkdownPanel::GetScintillaFirstVisibleLine(HWND hSci) const {
    if (!hSci) return 0;
    return static_cast<int>(SendMessage(hSci, SCI_GETFIRSTVISIBLELINE, 0, 0));
}

std::wstring NppMarkdownPanel::GetCurrentBufferPath() const {
    wchar_t path[MAX_PATH] = { 0 };
    SendMessage(m_nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, (WPARAM)-1, (LPARAM)path);
    return path;
}

bool NppMarkdownPanel::IsCurrentBufferMarkdown() const {
    if (m_config.allowAllExtensions) return true;
    std::wstring path = GetCurrentBufferPath();
    if (path.empty()) return true; // New empty file

    size_t dotPos = path.find_last_of(L'.');
    if (dotPos == std::wstring::npos) return false;
    std::wstring ext = path.substr(dotPos);
    return m_config.IsExtensionSupported(ext);
}

void NppMarkdownPanel::TogglePanel() {
    m_isPanelVisible = !m_isPanelVisible;
    SendMessage(m_nppData._nppHandle, m_isPanelVisible ? NPPM_DMMSHOW : NPPM_DMMHIDE, 0, (LPARAM)m_hPanel);
    SendMessage(m_nppData._nppHandle, NPPM_SETMENUITEMCHECK, CMD_TOGGLE_PANEL, m_isPanelVisible ? TRUE : FALSE);

    if (m_isPanelVisible) {
        ExecuteRender();
    }
}

void NppMarkdownPanel::ToggleSyncWithCaret() {
    m_config.syncWithCaret = !m_config.syncWithCaret;
    if (m_config.syncWithCaret) m_config.syncWithFirstLine = false;
    SendMessage(m_nppData._nppHandle, NPPM_SETMENUITEMCHECK, CMD_SYNC_CARET, m_config.syncWithCaret ? TRUE : FALSE);
    SendMessage(m_nppData._nppHandle, NPPM_SETMENUITEMCHECK, CMD_SYNC_FIRST_LINE, FALSE);
    SendMessage(GetDlgItem(m_hPanel, IDC_BTN_SYNC_TOGGLE), BM_SETCHECK, m_config.syncWithCaret ? BST_CHECKED : BST_UNCHECKED, 0);
}

void NppMarkdownPanel::ToggleSyncWithFirstLine() {
    m_config.syncWithFirstLine = !m_config.syncWithFirstLine;
    if (m_config.syncWithFirstLine) m_config.syncWithCaret = false;
    SendMessage(m_nppData._nppHandle, NPPM_SETMENUITEMCHECK, CMD_SYNC_FIRST_LINE, m_config.syncWithFirstLine ? TRUE : FALSE);
    SendMessage(m_nppData._nppHandle, NPPM_SETMENUITEMCHECK, CMD_SYNC_CARET, FALSE);
}

void NppMarkdownPanel::ToggleOutline() {
    m_config.showOutline = !m_config.showOutline;
    m_outlineView.Show(m_config.showOutline);
    SendMessage(m_nppData._nppHandle, NPPM_SETMENUITEMCHECK, CMD_TOGGLE_OUTLINE, m_config.showOutline ? TRUE : FALSE);

    // Trigger window resize to arrange canvas and outline
    RECT rc;
    GetClientRect(m_hPanel, &rc);
    SendMessage(m_hPanel, WM_SIZE, 0, MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));
}

void NppMarkdownPanel::CopyRenderedHtml() {
    std::wstring title = L"Markdown Preview";
    HtmlExporter::CopyToClipboard(m_hPanel, m_currentDoc, title, m_renderer.IsDarkMode());
}

void NppMarkdownPanel::SaveAsHtml() {
    wchar_t fileName[MAX_PATH] = L"export.html";
    OPENFILENAMEW ofn = { sizeof(OPENFILENAMEW) };
    ofn.hwndOwner = m_hPanel;
    ofn.lpstrFilter = L"HTML Files (*.html;*.htm)\0*.html;*.htm\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = L"html";

    if (GetSaveFileNameW(&ofn)) {
        HtmlExporter::SaveToFile(fileName, m_currentDoc, L"Markdown Export", m_renderer.IsDarkMode());
    }
}

void NppMarkdownPanel::ZoomIn() {
    m_config.zoomLevel += 0.1f;
    if (m_config.zoomLevel > 3.0f) m_config.zoomLevel = 3.0f;
    m_renderer.SetZoom(m_config.zoomLevel);
    InvalidateRect(m_hPanel, nullptr, FALSE);
}

void NppMarkdownPanel::ZoomOut() {
    m_config.zoomLevel -= 0.1f;
    if (m_config.zoomLevel < 0.4f) m_config.zoomLevel = 0.4f;
    m_renderer.SetZoom(m_config.zoomLevel);
    InvalidateRect(m_hPanel, nullptr, FALSE);
}

void NppMarkdownPanel::ZoomReset() {
    m_config.zoomLevel = 1.0f;
    m_renderer.SetZoom(m_config.zoomLevel);
    InvalidateRect(m_hPanel, nullptr, FALSE);
}

void NppMarkdownPanel::ToggleBiDi() {
    m_config.isSmartBiDiEnabled = !m_config.isSmartBiDiEnabled;
    SendMessage(m_nppData._nppHandle, NPPM_SETMENUITEMCHECK, CMD_TOGGLE_BIDI, m_config.isSmartBiDiEnabled ? TRUE : FALSE);
    ExecuteRender();
}

void NppMarkdownPanel::ShowSettings() {
    MessageBox(m_hPanel,
        L"NppMarkdownPanel C++26 Native Edition\n\n"
        L"Settings are loaded and saved automatically in NppMarkdownPanel.ini\n"
        L"- Zero .NET dependencies\n"
        L"- Pure Direct2D / DirectWrite GPU hardware acceleration\n"
        L"- Seamless Auto Dark Mode and Persian BiDi Support",
        L"Markdown Panel Settings", MB_OK | MB_ICONINFORMATION);
}

void NppMarkdownPanel::ShowAbout() {
    const wchar_t* aboutMsg =
        L"NppMarkdownPanel - Modern Native Edition (2026)\n"
        L"Version 1.0.0 (x64 / x86 Pure C++26)\n\n"
        L"Ultra-lightweight Notepad++ Markdown preview panel.\n"
        L"Engine: Direct2D 1.1 + DirectWrite GPU Hardware Accelerated\n"
        L"Features:\n"
        L"  • Zero .NET / Zero Chromium Child Processes (RAM < 8 MB)\n"
        L"  • First-class Persian / RTL BiDi & Inline Code Isolation\n"
        L"  • GFM Tables, Task Lists, Syntax Highlighting & Copy Button\n"
        L"  • Live Caret & Scrollbar Synchronization\n"
        L"  • Document Map / Outline (TOC) Navigation\n"
        L"  • Self-Contained HTML Export & Clipboard Copy\n"
        L"  • Native Dark / Light Mode Synchronization\n\n"
        L"Original concept by theneet0 & mohzy83\n"
        L"Modernized into C++26 by Google DeepMind Antigravity.";

    MessageBox(m_nppData._nppHandle, aboutMsg, L"About Markdown Panel", MB_OK | MB_ICONINFORMATION);
}

void NppMarkdownPanel::RequestRenderDebounced() {
    if (!m_isPanelVisible) return;
    SetTimer(m_hPanel, IDT_RENDER_DEBOUNCE, RENDER_DEBOUNCE_DELAY_MS, nullptr);
}

void NppMarkdownPanel::ExecuteRender() {
    HWND hSci = GetCurrentScintilla();
    if (!hSci) return;

    std::string utf8Content = GetScintillaText(hSci);
    std::wstring wideContent = BiDiEngine::Utf8ToWide(utf8Content);

    if (wideContent == m_lastRawContent) {
        // Only sync caret position if text didn't change
        if (m_config.syncWithCaret) {
            int line = GetScintillaCaretLine(hSci);
            m_renderer.ScrollToSourceLine(line);
        } else if (m_config.syncWithFirstLine) {
            int line = GetScintillaFirstVisibleLine(hSci);
            m_renderer.ScrollToSourceLine(line);
        }
        return;
    }

    m_lastRawContent = wideContent;

    // Parse Document
    m_currentDoc = MarkdownParser::Parse(wideContent);

    // Update Outline View
    std::vector<OutlineNode> outlineNodes;
    for (const auto& block : m_currentDoc.blocks) {
        if (block.type >= MarkdownBlockType::Header1 && block.type <= MarkdownBlockType::Header6) {
            OutlineNode node;
            node.level = block.level;
            node.title = block.rawText;
            node.sourceLine = block.sourceLineStart;
            outlineNodes.push_back(node);
        }
    }
    m_outlineView.SetHeadings(outlineNodes);

    // Layout in Renderer
    RECT rc;
    GetClientRect(m_hPanel, &rc);
    float outlineW = (m_config.showOutline && m_outlineView.IsVisible()) ? 180.0f : 0.0f;
    float clientW = static_cast<float>(rc.right - rc.left) - outlineW;
    m_renderer.SetDocument(m_currentDoc);
    m_renderer.Layout(clientW);

    // Sync scroll with current position
    if (m_config.syncWithCaret) {
        int line = GetScintillaCaretLine(hSci);
        m_renderer.ScrollToSourceLine(line);
    }

    InvalidateRect(m_hPanel, nullptr, FALSE);
}

void NppMarkdownPanel::OnNotification(SCNotification* notifyCode) {
    if (!notifyCode) return;

    if (notifyCode->nmhdr.code == SCN_MODIFIED) {
        if (notifyCode->modificationType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT)) {
            RequestRenderDebounced();
        }
    } else if (notifyCode->nmhdr.code == SCN_UPDATEUI) {
        if (m_config.syncWithCaret || m_config.syncWithFirstLine) {
            HWND hSci = GetCurrentScintilla();
            if (hSci) {
                int line = m_config.syncWithCaret ? GetScintillaCaretLine(hSci) : GetScintillaFirstVisibleLine(hSci);
                m_renderer.ScrollToSourceLine(line);
            }
        }
    }
}

void NppMarkdownPanel::OnDarkModeChanged() {
    bool isDark = SendMessage(m_nppData._nppHandle, NPPM_ISDARKMODEENABLED, 0, 0) != 0;
    if (m_config.darkModeOverride != -1) {
        isDark = (m_config.darkModeOverride == 1);
    }
    m_renderer.SetDarkMode(isDark);
    m_outlineView.SetDarkMode(isDark);
    InvalidateRect(m_hPanel, nullptr, TRUE);
}

void NppMarkdownPanel::OnBufferActivated() {
    if (m_config.autoShowForMarkdown) {
        bool isMd = IsCurrentBufferMarkdown();
        if (isMd && !m_isPanelVisible) {
            TogglePanel();
        } else if (!isMd && m_isPanelVisible && !m_config.allowAllExtensions) {
            TogglePanel();
        }
    }

    if (m_isPanelVisible) {
        m_lastRawContent.clear();
        ExecuteRender();
    }
}

LRESULT CALLBACK NppMarkdownPanel::PanelWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    NppMarkdownPanel* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        self = reinterpret_cast<NppMarkdownPanel*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<NppMarkdownPanel*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (!self) return DefWindowProc(hwnd, msg, wParam, lParam);

    switch (msg) {
        case WM_TIMER: {
            if (wParam == IDT_RENDER_DEBOUNCE) {
                KillTimer(hwnd, IDT_RENDER_DEBOUNCE);
                self->ExecuteRender();
                return 0;
            }
            break;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDC_BTN_REFRESH:
                    self->m_lastRawContent.clear();
                    self->ExecuteRender();
                    break;
                case IDC_BTN_OUTLINE:
                    self->ToggleOutline();
                    break;
                case IDC_BTN_ZOOM_IN:
                    self->ZoomIn();
                    break;
                case IDC_BTN_ZOOM_OUT:
                    self->ZoomOut();
                    break;
                case IDC_BTN_COPY_HTML:
                    self->CopyRenderedHtml();
                    break;
                case IDC_BTN_EXPORT_HTML:
                    self->SaveAsHtml();
                    break;
                case IDC_BTN_SYNC_TOGGLE:
                    self->ToggleSyncWithCaret();
                    break;
            }
            return 0;
        }

        case WM_SIZE: {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);

            int toolbarH = 38;
            float outlineW = (self->m_config.showOutline && self->m_outlineView.IsVisible()) ? 180.0f : 0.0f;

            if (self->m_outlineView.IsVisible()) {
                self->m_outlineView.Resize(static_cast<int>(width - outlineW), toolbarH, static_cast<int>(outlineW), height - toolbarH);
            }

            self->m_renderer.Resize(static_cast<UINT>(width - outlineW), height - toolbarH);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            self->m_renderer.Render();
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            bool isCtrl = (LOWORD(wParam) & MK_CONTROL) != 0;
            if (isCtrl) {
                if (delta > 0) self->ZoomIn();
                else self->ZoomOut();
            } else {
                self->m_renderer.ScrollBy(-delta / 2);
            }
            return 0;
        }

        case WM_MOUSEMOVE: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam) - 38; // Subtract toolbar height
            if (y >= 0 && self->m_renderer.OnMouseMove(x, y)) {
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam) - 38; // Subtract toolbar height
            if (y >= 0) {
                HWND hSci = self->GetCurrentScintilla();
                if (self->m_renderer.OnLButtonDown(x, y, hSci)) {
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
            }
            return 0;
        }

        case WM_ERASEBKGND:
            return 1; // Prevent flicker
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
