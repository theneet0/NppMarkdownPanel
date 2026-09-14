#include "../include/OutlineView.h"
#include <commctrl.h>

namespace {
const wchar_t* s_outlineClassName = L"NppMarkdownOutlineViewClass";
}

OutlineView::OutlineView() = default;

OutlineView::~OutlineView() {
    if (m_hBrushBg) {
        DeleteObject(m_hBrushBg);
        m_hBrushBg = nullptr;
    }
    if (m_hwnd && IsWindow(m_hwnd)) {
        DestroyWindow(m_hwnd);
    }
}

bool OutlineView::Create(HWND hParent, HINSTANCE hInst) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = s_outlineClassName;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassEx(&wc);

    m_hwnd = CreateWindowEx(
        WS_EX_TOOLWINDOW,
        s_outlineClassName,
        L"Document Outline",
        WS_CHILD | WS_CLIPSIBLINGS,
        0, 0, 200, 300,
        hParent,
        nullptr,
        hInst,
        this
    );

    if (!m_hwnd) return false;

    m_hListBox = CreateWindowEx(
        0, L"LISTBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        0, 0, 200, 300,
        m_hwnd,
        (HMENU)1001,
        hInst,
        nullptr
    );

    SetDarkMode(m_isDark);
    return true;
}

void OutlineView::SetHeadings(const std::vector<OutlineNode>& nodes) {
    m_nodes = nodes;
    if (!m_hListBox) return;

    SendMessage(m_hListBox, LB_RESETCONTENT, 0, 0);
    for (size_t i = 0; i < m_nodes.size(); ++i) {
        std::wstring itemStr;
        // Indentation according to heading level (H1 = 0, H2 = 2 spaces, etc.)
        int indent = (m_nodes[i].level - 1) * 2;
        if (indent > 0) itemStr.append(indent, L' ');

        itemStr += L"• " + m_nodes[i].title;
        SendMessage(m_hListBox, LB_ADDSTRING, 0, (LPARAM)itemStr.c_str());
        SendMessage(m_hListBox, LB_SETITEMDATA, i, (LPARAM)m_nodes[i].sourceLine);
    }
}

void OutlineView::Show(bool show) {
    if (m_hwnd && IsWindow(m_hwnd)) {
        ShowWindow(m_hwnd, show ? SW_SHOW : SW_HIDE);
    }
}

bool OutlineView::IsVisible() const {
    return m_hwnd && IsWindow(m_hwnd) && IsWindowVisible(m_hwnd);
}

void OutlineView::SetDarkMode(bool isDark) {
    m_isDark = isDark;
    if (m_hBrushBg) {
        DeleteObject(m_hBrushBg);
    }
    m_hBrushBg = CreateSolidBrush(m_isDark ? RGB(24, 24, 28) : RGB(245, 245, 247));

    if (m_hListBox && IsWindow(m_hListBox)) {
        InvalidateRect(m_hListBox, nullptr, TRUE);
    }
}

void OutlineView::Resize(int x, int y, int width, int height) {
    if (m_hwnd && IsWindow(m_hwnd)) {
        MoveWindow(m_hwnd, x, y, width, height, TRUE);
        if (m_hListBox && IsWindow(m_hListBox)) {
            MoveWindow(m_hListBox, 0, 0, width, height, TRUE);
        }
    }
}

LRESULT CALLBACK OutlineView::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    OutlineView* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        self = reinterpret_cast<OutlineView*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<OutlineView*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (!self) return DefWindowProc(hwnd, msg, wParam, lParam);

    switch (msg) {
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1001 && (HIWORD(wParam) == LBN_SELCHANGE || HIWORD(wParam) == LBN_DBLCLK)) {
                int sel = static_cast<int>(SendMessage(self->m_hListBox, LB_GETCURSEL, 0, 0));
                if (sel >= 0 && sel < static_cast<int>(self->m_nodes.size())) {
                    int line = self->m_nodes[sel].sourceLine;
                    if (self->m_callback) {
                        self->m_callback(line, self->m_userData);
                    }
                }
            }
            break;
        }

        case WM_CTLCOLORLISTBOX: {
            HDC hdc = (HDC)wParam;
            if (self->m_isDark) {
                SetTextColor(hdc, RGB(220, 220, 230));
                SetBkColor(hdc, RGB(24, 24, 28));
            } else {
                SetTextColor(hdc, RGB(36, 41, 47));
                SetBkColor(hdc, RGB(245, 245, 247));
            }
            return (LRESULT)self->m_hBrushBg;
        }

        case WM_DESTROY:
            return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
