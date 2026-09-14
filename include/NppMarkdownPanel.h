#pragma once
#include <windows.h>
#include <string>
#include "PluginInterface.h"
#include "Docking.h"
#include "Config.h"
#include "MarkdownRenderer.h"
#include "OutlineView.h"

class NppMarkdownPanel {
public:
    static NppMarkdownPanel& Instance();

    void Init(HINSTANCE hInst, NppData nppData);
    void Cleanup();

    // Menu Command Handlers
    void TogglePanel();
    void ToggleSyncWithCaret();
    void ToggleSyncWithFirstLine();
    void ToggleOutline();
    void CopyRenderedHtml();
    void SaveAsHtml();
    void ZoomIn();
    void ZoomOut();
    void ZoomReset();
    void ToggleBiDi();
    void ShowSettings();
    void ShowAbout();

    // Notepad++ Notifications
    void OnNotification(SCNotification* notifyCode);
    void OnDarkModeChanged();
    void OnBufferActivated();
    void OnDocModified();

    HWND GetPanelHwnd() const noexcept { return m_hPanel; }
    tTbData* GetTbData() noexcept { return &m_tbData; }

    HWND GetCurrentScintilla() const;
    std::string GetScintillaText(HWND hSci) const;
    int GetScintillaCaretLine(HWND hSci) const;
    int GetScintillaFirstVisibleLine(HWND hSci) const;

private:
    NppMarkdownPanel() = default;
    ~NppMarkdownPanel() = default;

    bool CreatePanelWindow();
    void RequestRenderDebounced();
    void ExecuteRender();
    void UpdateScrollbars();
    void UpdateStatsUI();
    std::wstring GetCurrentBufferPath() const;
    bool IsCurrentBufferMarkdown() const;

    static LRESULT CALLBACK PanelWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HINSTANCE m_hInst = nullptr;
    NppData m_nppData = {};
    HWND m_hPanel = nullptr;
    HWND m_hToolbar = nullptr;
    HWND m_hStatsText = nullptr;
    tTbData m_tbData = {};

    PluginConfig m_config;
    std::wstring m_configPath;
    MarkdownRenderer m_renderer;
    OutlineView m_outlineView;
    MarkdownDocument m_currentDoc;

    bool m_isPanelRegistered = false;
    bool m_isPanelVisible = false;
    bool m_isRenderPending = false;
    std::wstring m_lastRawContent;
};
