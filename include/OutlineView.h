#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include "MarkdownParser.h"

struct OutlineNode {
    int level = 1;
    std::wstring title;
    int sourceLine = 0;
};

class OutlineView {
public:
    OutlineView();
    ~OutlineView();

    bool Create(HWND hParent, HINSTANCE hInst);
    void SetHeadings(const std::vector<OutlineNode>& nodes);
    void Show(bool show);
    bool IsVisible() const;
    void SetDarkMode(bool isDark);
    void Resize(int x, int y, int width, int height);
    HWND GetHwnd() const noexcept { return m_hwnd; }

    // Callback on heading select
    typedef void (*OnHeadingSelectedCallback)(int sourceLine, void* userData);
    void SetCallback(OnHeadingSelectedCallback cb, void* userData) {
        m_callback = cb;
        m_userData = userData;
    }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd = nullptr;
    HWND m_hListBox = nullptr;
    std::vector<OutlineNode> m_nodes;
    OnHeadingSelectedCallback m_callback = nullptr;
    void* m_userData = nullptr;
    bool m_isDark = true;
    HBRUSH m_hBrushBg = nullptr;
};
