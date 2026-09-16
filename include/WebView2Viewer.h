#pragma once

#include <windows.h>
#include <string>
#include <functional>
#include <memory>

// Forward declarations for WebView2 interfaces
struct ICoreWebView2Environment;
struct ICoreWebView2Controller;
struct ICoreWebView2;

class WebView2Viewer {
public:
    using CheckboxCallback = std::function<void(int lineNo)>;
    using NavigationCallback = std::function<void(const std::wstring& url)>;

    WebView2Viewer();
    ~WebView2Viewer();

    // Check if WebView2 runtime / loader is available
    static bool IsAvailable();

    // Initialize WebView2 inside the specified parent HWND
    bool Initialize(HWND hParentWnd, const std::wstring& userDataDir);

    // Window management
    void Resize(int x, int y, int width, int height);
    void SetVisible(bool visible);
    bool IsInitialized() const { return m_isInitialized; }
    bool IsPageReady() const { return m_isInitialized && m_pageReady; }

    // Content updates
    void SetHtmlContent(const std::string& htmlUtf8);
    bool UpdateContent(const std::string& bodyHtml, const std::string& tocHtml, const std::string& statsText);
    void ExecuteScript(const std::wstring& script);

    // Visual settings
    void SetDarkMode(bool isDark);
    void SetZoom(float zoomFactor);

    // Callbacks
    void SetCheckboxCallback(CheckboxCallback cb) { m_checkboxCallback = cb; }
    void SetNavigationCallback(NavigationCallback cb) { m_navCallback = cb; }

    // Actions
    void ScrollToLine(int line);
    void PrintToPdf(const std::wstring& pdfPath);

    // Close and release
    void Close();

private:
    HWND m_hParent = nullptr;
    HMODULE m_hLoaderModule = nullptr;
    bool m_isInitialized = false;
    bool m_isInitializing = false;
    bool m_pageReady = false;
    bool m_darkMode = false;
    float m_zoom = 1.0f;
    std::string m_pendingHtml;

    ICoreWebView2Environment* m_pEnvironment = nullptr;
    ICoreWebView2Controller* m_pController = nullptr;
    ICoreWebView2* m_pWebView = nullptr;

    CheckboxCallback m_checkboxCallback;
    NavigationCallback m_navCallback;

    // Internal initialization completion
    void OnEnvironmentCreated(HRESULT result, ICoreWebView2Environment* env);
    void OnControllerCreated(HRESULT result, ICoreWebView2Controller* controller);
    void OnWebMessageReceived(const std::wstring& message);
};
