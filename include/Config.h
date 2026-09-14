#pragma once
#include <windows.h>
#include <string>
#include <vector>

struct PluginConfig {
    bool isPanelVisible = false;
    bool syncWithCaret = true;
    bool syncWithFirstLine = false;
    bool showOutline = false;
    float zoomLevel = 1.0f;
    bool autoShowForMarkdown = true;
    bool allowAllExtensions = false;
    bool isSmartBiDiEnabled = true;
    int darkModeOverride = -1; // -1: auto NPP, 0: Light, 1: Dark
    float baseFontSize = 15.0f;
    std::wstring fontFamily = L"Segoe UI";
    std::wstring codeFontFamily = L"Consolas";
    std::wstring supportedExtensions = L".md,.markdown,.mdown,.mkd,.rst";

    bool IsExtensionSupported(const std::wstring& ext) const;
    void Load(const std::wstring& configPath);
    void Save(const std::wstring& configPath) const;
};
