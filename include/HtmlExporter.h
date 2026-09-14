#pragma once
#include <windows.h>
#include <string>
#include "MarkdownParser.h"

class HtmlExporter {
public:
    static std::wstring ExportToHtml(const MarkdownDocument& doc, const std::wstring& title, bool isDarkMode);
    static bool SaveToFile(const std::wstring& filePath, const MarkdownDocument& doc, const std::wstring& title, bool isDarkMode);
    static bool CopyToClipboard(HWND hwndOwner, const MarkdownDocument& doc, const std::wstring& title, bool isDarkMode);

private:
    static std::wstring EscapeHtml(const std::wstring& str);
    static std::wstring InlinesToHtml(const std::vector<MarkdownSpan>& inlines);
    static std::string GenerateCfHtml(const std::string& htmlFragment);
};
