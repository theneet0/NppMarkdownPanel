#include "../include/HtmlExporter.h"
#include "../include/BiDiEngine.h"
#include "../include/SyntaxHighlighter.h"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace {

std::string WideToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return "";
    std::string str(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], len, nullptr, nullptr);
    return str;
}

std::wstring Utf8ToWide(const std::string& str) {
    if (str.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (len <= 0) return L"";
    std::wstring wstr(len - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);
    return wstr;
}

const char* s_modernPreviewCss = R"CSS(
:root {
    --bg-page: #ffffff;
    --text-primary: #1f2328;
    --text-secondary: #656d76;
    --border-color: #d0d7de;
    --code-bg: #f6f8fa;
    --code-header-bg: #eaeff4;
    --code-border: #d0d7de;
    --quote-bg: #f6f8fa;
    --quote-bar: #0969da;
    --table-alt: #f6f8fa;
    --table-header: #f0f3f6;
    --link-color: #0969da;
    --toolbar-bg: rgba(255, 255, 255, 0.85);
    --toolbar-border: rgba(208, 215, 222, 0.8);
    --toolbar-shadow: 0 8px 32px rgba(0, 0, 0, 0.12);
    --search-match: #fff3a8;
    --search-active: #ff9632;
    --alert-note-bg: #edf6fd;
    --alert-note-bar: #0969da;
    --alert-tip-bg: #edf9ef;
    --alert-tip-bar: #1a7f37;
    --alert-important-bg: #f7f2fa;
    --alert-important-bar: #8250df;
    --alert-warning-bg: #fffbea;
    --alert-warning-bar: #d29922;
    --alert-caution-bg: #fff0ed;
    --alert-caution-bar: #cf222e;
    --drawer-bg: #ffffff;
}

body.dark {
    --bg-page: #0d1117;
    --text-primary: #e6edf3;
    --text-secondary: #8b949e;
    --border-color: #30363d;
    --code-bg: #161b22;
    --code-header-bg: #21262d;
    --code-border: #30363d;
    --quote-bg: #161b22;
    --quote-bar: #2f81f7;
    --table-alt: #161b22;
    --table-header: #1c2128;
    --link-color: #58a6ff;
    --toolbar-bg: rgba(22, 27, 34, 0.85);
    --toolbar-border: rgba(48, 54, 61, 0.8);
    --toolbar-shadow: 0 8px 32px rgba(0, 0, 0, 0.4);
    --search-match: #53450e;
    --search-active: #9e6a03;
    --alert-note-bg: #1c2836;
    --alert-note-bar: #2f81f7;
    --alert-tip-bg: #162d20;
    --alert-tip-bar: #2ea043;
    --alert-important-bg: #261e33;
    --alert-important-bar: #a371f7;
    --alert-warning-bg: #2e2214;
    --alert-warning-bar: #d29922;
    --alert-caution-bg: #31191b;
    --alert-caution-bar: #f85149;
    --drawer-bg: #161b22;
}

* { box-sizing: border-box; }

body {
    margin: 0;
    padding: 0;
    background-color: var(--bg-page);
    color: var(--text-primary);
    font-family: -apple-system, BlinkMacSystemFont, "Vazirmatn", "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
    font-size: 15px;
    line-height: 1.68;
    transition: background-color 0.25s ease, color 0.25s ease;
    overflow-x: hidden;
}

/* Main Content Container */
#content-container {
    max-width: 900px;
    margin: 0 auto;
    padding: 56px 36px 120px 36px;
    word-wrap: break-word;
    overflow-wrap: break-word;
}

/* Floating Glassmorphic Control Bar */
#floating-toolbar {
    position: fixed;
    top: 12px;
    left: 50%;
    transform: translateX(-50%);
    z-index: 1000;
    display: flex;
    align-items: center;
    gap: 6px;
    padding: 6px 14px;
    background: var(--toolbar-bg);
    backdrop-filter: blur(16px);
    -webkit-backdrop-filter: blur(16px);
    border: 1px solid var(--toolbar-border);
    border-radius: 30px;
    box-shadow: var(--toolbar-shadow);
    transition: all 0.2s ease;
}

#floating-toolbar:hover {
    box-shadow: 0 12px 40px rgba(0, 0, 0, 0.18);
}

.tool-btn {
    background: transparent;
    border: none;
    color: var(--text-primary);
    width: 32px;
    height: 32px;
    border-radius: 50%;
    cursor: pointer;
    display: flex;
    align-items: center;
    justify-content: center;
    transition: background-color 0.15s ease, transform 0.1s ease;
    font-size: 14px;
    position: relative;
}

.tool-btn:hover {
    background: rgba(128, 128, 128, 0.18);
    transform: scale(1.08);
}

.tool-btn:active {
    transform: scale(0.95);
}

.tool-btn.active {
    background: var(--link-color);
    color: #ffffff;
}

.tool-sep {
    width: 1px;
    height: 18px;
    background: var(--border-color);
    margin: 0 4px;
}

.stats-pill {
    font-size: 11px;
    color: var(--text-secondary);
    padding: 2px 8px;
    background: rgba(128, 128, 128, 0.12);
    border-radius: 12px;
    white-space: nowrap;
    user-select: none;
}

/* Search Bar (Expandable) */
#search-bar {
    display: none;
    align-items: center;
    gap: 4px;
    margin-left: 4px;
}

#search-input {
    background: rgba(128, 128, 128, 0.12);
    border: 1px solid var(--border-color);
    border-radius: 14px;
    padding: 3px 10px;
    font-size: 12px;
    color: var(--text-primary);
    width: 120px;
    outline: none;
    transition: width 0.2s ease;
}

#search-input:focus {
    width: 170px;
    border-color: var(--link-color);
}

#search-count {
    font-size: 11px;
    color: var(--text-secondary);
    min-width: 38px;
    text-align: center;
}

mark.search-match {
    background-color: var(--search-match);
    color: inherit;
    border-radius: 2px;
    padding: 1px 2px;
}

mark.search-match.active {
    background-color: var(--search-active);
    color: #ffffff;
    font-weight: bold;
}

/* Table of Contents Drawer */
#toc-drawer {
    position: fixed;
    top: 0;
    right: -280px;
    width: 270px;
    height: 100vh;
    background: var(--drawer-bg);
    border-left: 1px solid var(--border-color);
    box-shadow: -8px 0 32px rgba(0, 0, 0, 0.2);
    z-index: 999;
    padding: 20px 16px;
    overflow-y: auto;
    transition: right 0.3s cubic-bezier(0.16, 1, 0.3, 1);
}

#toc-drawer.open {
    right: 0;
}

.toc-header {
    font-size: 14px;
    font-weight: 700;
    margin-bottom: 16px;
    padding-bottom: 8px;
    border-bottom: 1px solid var(--border-color);
    display: flex;
    justify-content: space-between;
    align-items: center;
}

.toc-close {
    cursor: pointer;
    background: none;
    border: none;
    font-size: 16px;
    color: var(--text-secondary);
}

.toc-item {
    display: block;
    color: var(--text-secondary);
    text-decoration: none;
    font-size: 12px;
    padding: 4px 8px;
    border-radius: 6px;
    margin-bottom: 2px;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
    transition: all 0.15s ease;
}

.toc-item:hover {
    color: var(--link-color);
    background: rgba(128, 128, 128, 0.1);
}

.toc-item.active {
    color: var(--link-color);
    font-weight: 600;
    background: rgba(9, 105, 218, 0.1);
}

.toc-l1 { padding-left: 8px; font-weight: 600; }
.toc-l2 { padding-left: 18px; }
.toc-l3 { padding-left: 28px; }
.toc-l4 { padding-left: 38px; }

/* Markdown Typography */
h1, h2, h3, h4, h5, h6 {
    margin-top: 28px;
    margin-bottom: 16px;
    font-weight: 700;
    line-height: 1.3;
    scroll-margin-top: 60px;
}

h1 { font-size: 2em; border-bottom: 1px solid var(--border-color); padding-bottom: 0.3em; }
h2 { font-size: 1.5em; border-bottom: 1px solid var(--border-color); padding-bottom: 0.3em; }
h3 { font-size: 1.25em; }
h4 { font-size: 1.05em; }

p { margin-top: 0; margin-bottom: 16px; }

a { color: var(--link-color); text-decoration: none; }
a:hover { text-decoration: underline; }

hr {
    height: 2px;
    background-color: var(--border-color);
    border: none;
    margin: 28px 0;
}

blockquote {
    margin: 16px 0;
    padding: 10px 18px;
    background: var(--quote-bg);
    border-left: 4px solid var(--quote-bar);
    border-radius: 6px;
    color: var(--text-primary);
}

blockquote[dir="rtl"] {
    border-left: none;
    border-right: 4px solid var(--quote-bar);
}

/* Modern Code Blocks with macOS styling */
.code-block-card {
    margin: 20px 0;
    background: var(--code-bg);
    border: 1px solid var(--code-border);
    border-radius: 10px;
    overflow: hidden;
    box-shadow: 0 4px 16px rgba(0, 0, 0, 0.04);
}

.code-block-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 8px 14px;
    background: var(--code-header-bg);
    border-bottom: 1px solid var(--code-border);
    user-select: none;
}

.mac-controls {
    display: flex;
    gap: 7px;
    align-items: center;
}

.mac-dot {
    width: 11px;
    height: 11px;
    border-radius: 50%;
}

.mac-dot.close { background: #ff5f56; }
.mac-dot.min { background: #ffbd2e; }
.mac-dot.max { background: #27c93f; }

.lang-badge {
    font-size: 11px;
    font-family: ui-monospace, "Cascadia Code", Consolas, monospace;
    font-weight: 600;
    color: var(--text-secondary);
    text-transform: uppercase;
    letter-spacing: 0.5px;
}

.copy-btn {
    background: rgba(128, 128, 128, 0.15);
    border: 1px solid var(--border-color);
    border-radius: 6px;
    color: var(--text-primary);
    font-size: 11px;
    padding: 3px 10px;
    cursor: pointer;
    display: flex;
    align-items: center;
    gap: 5px;
    transition: all 0.15s ease;
}

.copy-btn:hover {
    background: rgba(128, 128, 128, 0.28);
}

.copy-btn.copied {
    background: #2ea043;
    color: #ffffff;
    border-color: #2ea043;
}

pre {
    margin: 0;
    padding: 14px 16px;
    overflow-x: auto;
    font-family: ui-monospace, "Cascadia Code", "Fira Code", Consolas, monospace;
    font-size: 13.5px;
    line-height: 1.55;
    tab-size: 4;
}

code {
    font-family: ui-monospace, "Cascadia Code", "Fira Code", Consolas, monospace;
    font-size: 85%;
    padding: 0.2em 0.45em;
    background-color: var(--code-bg);
    border: 1px solid var(--border-color);
    border-radius: 5px;
}

pre code {
    background: transparent;
    border: none;
    padding: 0;
    font-size: 100%;
}

/* Syntax Highlighting Tokens */
.hl-keyword { color: #cf222e; font-weight: 600; }
.hl-type { color: #953800; }
.hl-string { color: #0a3069; }
.hl-comment { color: #6e7781; font-style: italic; }
.hl-number { color: #0550ae; }
.hl-preprocessor { color: #8250df; }
.hl-operator { color: #24292f; }

body.dark .hl-keyword { color: #ff7b72; font-weight: 600; }
body.dark .hl-type { color: #ffa657; }
body.dark .hl-string { color: #a5d6ff; }
body.dark .hl-comment { color: #8b949e; font-style: italic; }
body.dark .hl-number { color: #79c0ff; }
body.dark .hl-preprocessor { color: #d2a8ff; }
body.dark .hl-operator { color: #e6edf3; }

/* Alert Callouts */
.alert-callout {
    border-radius: 8px;
    padding: 14px 18px;
    margin: 18px 0;
    border-left: 4px solid var(--quote-bar);
    box-shadow: 0 2px 8px rgba(0, 0, 0, 0.03);
}

.alert-callout[dir="rtl"] {
    border-left: none;
    border-right: 4px solid var(--quote-bar);
}

.alert-callout.note { background: var(--alert-note-bg); border-color: var(--alert-note-bar); }
.alert-callout.tip { background: var(--alert-tip-bg); border-color: var(--alert-tip-bar); }
.alert-callout.important { background: var(--alert-important-bg); border-color: var(--alert-important-bar); }
.alert-callout.warning { background: var(--alert-warning-bg); border-color: var(--alert-warning-bar); }
.alert-callout.caution { background: var(--alert-caution-bg); border-color: var(--alert-caution-bar); }

.alert-header {
    display: flex;
    align-items: center;
    gap: 8px;
    font-weight: 700;
    margin-bottom: 6px;
}

.alert-callout.note .alert-header { color: var(--alert-note-bar); }
.alert-callout.tip .alert-header { color: var(--alert-tip-bar); }
.alert-callout.important .alert-header { color: var(--alert-important-bar); }
.alert-callout.warning .alert-header { color: var(--alert-warning-bar); }
.alert-callout.caution .alert-header { color: var(--alert-caution-bar); }

/* Tables */
table {
    border-collapse: collapse;
    width: 100%;
    margin: 20px 0;
    border-radius: 8px;
    overflow: hidden;
    border: 1px solid var(--border-color);
}

th, td {
    border: 1px solid var(--border-color);
    padding: 9px 15px;
}

th {
    background-color: var(--table-header);
    font-weight: 600;
}

tr:nth-child(2n) {
    background-color: var(--table-alt);
}

/* Task Lists */
.task-list {
    list-style-type: none;
    padding-left: 0;
}

.task-list[dir="rtl"] {
    padding-right: 0;
}

.task-item {
    display: flex;
    align-items: flex-start;
    gap: 8px;
    margin: 6px 0;
}

.task-checkbox {
    width: 16px;
    height: 16px;
    margin-top: 4px;
    cursor: pointer;
    accent-color: var(--link-color);
}

/* Highlight and Math */
mark {
    background-color: #fff8c5;
    color: #1f2328;
    padding: 0.15em 0.4em;
    border-radius: 4px;
}

body.dark mark {
    background-color: #3e3816;
    color: #e6edf3;
}

.math-inline {
    font-family: KaTeX_Main, "Times New Roman", serif;
    padding: 0 3px;
}

@media print {
    #floating-toolbar, #toc-drawer { display: none !important; }
    #content-container { padding: 0 !important; max-width: 100% !important; }
    body { background: #ffffff !important; color: #000000 !important; }
}
)CSS";

std::wstring TokenizeCodeHtml(const std::wstring& line, const std::wstring& language) {
    auto tokens = SyntaxHighlighter::Tokenize(line, language);
    if (tokens.empty()) {
        return HtmlExporter::ExportToHtml(MarkdownDocument{}, line, false); // Just escape
    }

    std::wstringstream ss;
    size_t cursor = 0;
    for (const auto& tok : tokens) {
        if (tok.start > cursor) {
            std::wstring plain = line.substr(cursor, tok.start - cursor);
            for (wchar_t ch : plain) {
                if (ch == L'&') ss << L"&amp;";
                else if (ch == L'<') ss << L"&lt;";
                else if (ch == L'>') ss << L"&gt;";
                else if (ch == L'\"') ss << L"&quot;";
                else ss << ch;
            }
        }

        std::wstring tokenText = line.substr(tok.start, tok.length);
        const wchar_t* clsName = L"hl-default";
        switch (tok.type) {
            case HighlightTokenType::Keyword: clsName = L"hl-keyword"; break;
            case HighlightTokenType::Type: clsName = L"hl-type"; break;
            case HighlightTokenType::String: clsName = L"hl-string"; break;
            case HighlightTokenType::Comment: clsName = L"hl-comment"; break;
            case HighlightTokenType::Number: clsName = L"hl-number"; break;
            case HighlightTokenType::Preprocessor: clsName = L"hl-preprocessor"; break;
            case HighlightTokenType::Operator: clsName = L"hl-operator"; break;
            default: break;
        }

        ss << L"<span class=\"" << clsName << L"\">";
        for (wchar_t ch : tokenText) {
            if (ch == L'&') ss << L"&amp;";
            else if (ch == L'<') ss << L"&lt;";
            else if (ch == L'>') ss << L"&gt;";
            else if (ch == L'\"') ss << L"&quot;";
            else ss << ch;
        }
        ss << L"</span>";
        cursor = tok.start + tok.length;
    }

    if (cursor < line.size()) {
        std::wstring plain = line.substr(cursor);
        for (wchar_t ch : plain) {
            if (ch == L'&') ss << L"&amp;";
            else if (ch == L'<') ss << L"&lt;";
            else if (ch == L'>') ss << L"&gt;";
            else if (ch == L'\"') ss << L"&quot;";
            else ss << ch;
        }
    }

    return ss.str();
}

} // namespace

std::string HtmlExporter::GeneratePreviewHtml(
    const MarkdownDocument& doc,
    const std::wstring& title,
    bool isDarkMode,
    float zoomLevel,
    bool isSyncEnabled
) {
    std::wstringstream bodyStream;
    std::wstringstream tocStream;
    int headingIndex = 0;

    bool inUnorderedList = false;
    bool inOrderedList = false;
    bool inTaskList = false;

    auto closeOpenLists = [&]() {
        if (inUnorderedList) { bodyStream << L"</ul>\n"; inUnorderedList = false; }
        if (inOrderedList) { bodyStream << L"</ol>\n"; inOrderedList = false; }
        if (inTaskList) { bodyStream << L"</div>\n"; inTaskList = false; }
    };

    for (const auto& block : doc.blocks) {
        if (block.type != MarkdownBlockType::UnorderedListItem) {
            if (inUnorderedList) { bodyStream << L"</ul>\n"; inUnorderedList = false; }
        }
        if (block.type != MarkdownBlockType::OrderedListItem) {
            if (inOrderedList) { bodyStream << L"</ol>\n"; inOrderedList = false; }
        }
        if (block.type != MarkdownBlockType::TaskListItem) {
            if (inTaskList) { bodyStream << L"</div>\n"; inTaskList = false; }
        }

        const wchar_t* dirAttr = block.isRTL ? L" dir=\"rtl\"" : L" dir=\"ltr\"";

        switch (block.type) {
            case MarkdownBlockType::Header1:
            case MarkdownBlockType::Header2:
            case MarkdownBlockType::Header3:
            case MarkdownBlockType::Header4:
            case MarkdownBlockType::Header5:
            case MarkdownBlockType::Header6: {
                int hNum = 1;
                if (block.type == MarkdownBlockType::Header2) hNum = 2;
                else if (block.type == MarkdownBlockType::Header3) hNum = 3;
                else if (block.type == MarkdownBlockType::Header4) hNum = 4;
                else if (block.type == MarkdownBlockType::Header5) hNum = 5;
                else if (block.type == MarkdownBlockType::Header6) hNum = 6;

                headingIndex++;
                std::wstring hId = L"h-" + std::to_wstring(headingIndex);
                std::wstring hText = InlinesToHtml(block.inlines);

                bodyStream << L"<h" << hNum << L" id=\"" << hId << L"\" data-source-line=\""
                           << block.sourceLineStart << L"\"" << dirAttr << L">"
                           << hText << L"</h" << hNum << L">\n";

                // Add to TOC
                tocStream << L"<a href=\"#" << hId << L"\" class=\"toc-item toc-l" << hNum << L"\""
                          << dirAttr << L" onclick=\"document.getElementById('" << hId << L"').scrollIntoView({behavior:'smooth'}); return false;\">"
                          << hText << L"</a>\n";
                break;
            }

            case MarkdownBlockType::HorizontalRule: {
                bodyStream << L"<hr data-source-line=\"" << block.sourceLineStart << L"\" />\n";
                break;
            }

            case MarkdownBlockType::Blockquote: {
                bodyStream << L"<blockquote data-source-line=\"" << block.sourceLineStart << L"\""
                           << dirAttr << L"><p>" << InlinesToHtml(block.inlines) << L"</p></blockquote>\n";
                break;
            }

            case MarkdownBlockType::AlertCallout: {
                const wchar_t* alertCls = L"note";
                const wchar_t* alertIcon = L"ℹ️";
                switch (block.alertType) {
                    case AlertType::Tip: alertCls = L"tip"; alertIcon = L"💡"; break;
                    case AlertType::Important: alertCls = L"important"; alertIcon = L"🟣"; break;
                    case AlertType::Warning: alertCls = L"warning"; alertIcon = L"⚠️"; break;
                    case AlertType::Caution: alertCls = L"caution"; alertIcon = L"🛑"; break;
                    default: break;
                }

                bodyStream << L"<div class=\"alert-callout " << alertCls << L"\" data-source-line=\""
                           << block.sourceLineStart << L"\"" << dirAttr << L">\n";
                bodyStream << L"  <div class=\"alert-header\">" << alertIcon << L" " << EscapeHtml(block.alertTitle) << L"</div>\n";
                bodyStream << L"  <div class=\"alert-body\">" << InlinesToHtml(block.inlines) << L"</div>\n";
                bodyStream << L"</div>\n";
                break;
            }

            case MarkdownBlockType::CodeBlock: {
                if (block.codeLanguage == L"mermaid") {
                    bodyStream << L"<div class=\"mermaid\" data-source-line=\"" << block.sourceLineStart << L"\">\n";
                    for (const auto& cl : block.codeLines) {
                        bodyStream << EscapeHtml(cl) << L"\n";
                    }
                    bodyStream << L"</div>\n";
                } else {
                    std::wstring langDisplay = block.codeLanguage.empty() ? L"TEXT" : block.codeLanguage;
                    bodyStream << L"<div class=\"code-block-card\" data-source-line=\"" << block.sourceLineStart << L"\">\n";
                    bodyStream << L"  <div class=\"code-block-header\">\n";
                    bodyStream << L"    <div class=\"mac-controls\"><div class=\"mac-dot close\"></div><div class=\"mac-dot min\"></div><div class=\"mac-dot max\"></div></div>\n";
                    bodyStream << L"    <div class=\"lang-badge\">" << EscapeHtml(langDisplay) << L"</div>\n";
                    bodyStream << L"    <button class=\"copy-btn\" onclick=\"copyCodeBlock(this)\">📋 Copy</button>\n";
                    bodyStream << L"  </div>\n";
                    bodyStream << L"  <pre><code>";
                    for (size_t i = 0; i < block.codeLines.size(); ++i) {
                        bodyStream << TokenizeCodeHtml(block.codeLines[i], block.codeLanguage);
                        if (i + 1 < block.codeLines.size()) bodyStream << L"\n";
                    }
                    bodyStream << L"</code></pre>\n";
                    bodyStream << L"</div>\n";
                }
                break;
            }

            case MarkdownBlockType::Table: {
                bodyStream << L"<table data-source-line=\"" << block.sourceLineStart << L"\"" << dirAttr << L">\n";
                for (size_t r = 0; r < block.table.rows.size(); ++r) {
                    const auto& row = block.table.rows[r];
                    bodyStream << L"  <tr>\n";
                    for (size_t c = 0; c < row.cells.size(); ++c) {
                        const wchar_t* tag = row.isHeader ? L"th" : L"td";
                        std::wstring alignStyle;
                        if (c < block.table.alignments.size()) {
                            if (block.table.alignments[c] == TableColumnAlign::Center) alignStyle = L" style=\"text-align:center;\"";
                            else if (block.table.alignments[c] == TableColumnAlign::Right) alignStyle = L" style=\"text-align:right;\"";
                            else if (block.table.alignments[c] == TableColumnAlign::Left) alignStyle = L" style=\"text-align:left;\"";
                        }
                        bodyStream << L"    <" << tag << alignStyle << L">" << InlinesToHtml(row.cells[c].spans) << L"</" << tag << L">\n";
                    }
                    bodyStream << L"  </tr>\n";
                }
                bodyStream << L"</table>\n";
                break;
            }

            case MarkdownBlockType::UnorderedListItem: {
                if (!inUnorderedList) {
                    bodyStream << L"<ul" << dirAttr << L" data-source-line=\"" << block.sourceLineStart << L"\">\n";
                    inUnorderedList = true;
                }
                bodyStream << L"  <li data-source-line=\"" << block.sourceLineStart << L"\">" << InlinesToHtml(block.inlines) << L"</li>\n";
                break;
            }

            case MarkdownBlockType::OrderedListItem: {
                if (!inOrderedList) {
                    bodyStream << L"<ol" << dirAttr << L" data-source-line=\"" << block.sourceLineStart << L"\">\n";
                    inOrderedList = true;
                }
                bodyStream << L"  <li data-source-line=\"" << block.sourceLineStart << L"\">" << InlinesToHtml(block.inlines) << L"</li>\n";
                break;
            }

            case MarkdownBlockType::TaskListItem: {
                if (!inTaskList) {
                    bodyStream << L"<div class=\"task-list\"" << dirAttr << L" data-source-line=\"" << block.sourceLineStart << L"\">\n";
                    inTaskList = true;
                }
                bodyStream << L"  <div class=\"task-item\" data-source-line=\"" << block.sourceLineStart << L"\">\n";
                bodyStream << L"    <input type=\"checkbox\" class=\"task-checkbox\" data-line=\""
                           << block.sourceLineStart << L"\" " << (block.isTaskChecked ? L"checked" : L"")
                           << L" onchange=\"handleTaskCheck(this)\">\n";
                bodyStream << L"    <span>" << InlinesToHtml(block.inlines) << L"</span>\n";
                bodyStream << L"  </div>\n";
                break;
            }

            case MarkdownBlockType::Paragraph:
            default: {
                bodyStream << L"<p data-source-line=\"" << block.sourceLineStart << L"\"" << dirAttr << L">"
                           << InlinesToHtml(block.inlines) << L"</p>\n";
                break;
            }
        }
    }
    closeOpenLists();

    // Reading statistics
    int estMinutes = static_cast<int>((std::max)(size_t(1), doc.wordCount / 200));
    std::wstring statsText = std::to_wstring(doc.wordCount) + L" words \u2022 " + std::to_wstring(estMinutes) + L" min read";
    if (doc.blocks.size() > 0 && doc.blocks[0].isRTL) {
        statsText = BiDiEngine::ToPersianDigits(static_cast<int>(doc.wordCount)) + L" \u0648\u0627\u0698\u0647 \u2022 " +
                    BiDiEngine::ToPersianDigits(estMinutes) + L" \u062F\u0642\u06CC\u0642\u0647 \u0645\u0637\u0627\u0644\u0639\u0647";
    }

    std::stringstream html;
    html << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n";
    html << "<meta charset=\"UTF-8\">\n";
    html << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    html << "<title>" << WideToUtf8(title) << "</title>\n";

    // Google Fonts for Vazirmatn and modern typography
    html << "<link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">\n";
    html << "<link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" crossorigin>\n";
    html << "<link href=\"https://fonts.googleapis.com/css2?family=Vazirmatn:wght@300;400;500;600;700;800&family=Cascadia+Code:wght@400;600&display=swap\" rel=\"stylesheet\">\n";

    // KaTeX CDN
    html << "<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/katex@0.16.11/dist/katex.min.css\" crossorigin=\"anonymous\">\n";
    html << "<script defer src=\"https://cdn.jsdelivr.net/npm/katex@0.16.11/dist/katex.min.js\" crossorigin=\"anonymous\"></script>\n";
    html << "<script defer src=\"https://cdn.jsdelivr.net/npm/katex@0.16.11/dist/contrib/auto-render.min.js\" crossorigin=\"anonymous\"></script>\n";

    // Mermaid.js CDN
    html << "<script src=\"https://cdn.jsdelivr.net/npm/mermaid@10.9.1/dist/mermaid.min.js\"></script>\n";

    // Embedded CSS
    html << "<style>\n" << s_modernPreviewCss << "\n</style>\n";
    html << "</head>\n<body class=\"" << (isDarkMode ? "dark" : "") << "\">\n";

    // Floating Glassmorphic Toolbar
    html << R"HTML(
<div id="floating-toolbar">
  <button class="tool-btn" id="btn-toc" title="Table of Contents (Outline)" onclick="toggleToc()">📑</button>
  <button class="tool-btn" id="btn-search" title="Find in Document (Ctrl+F)" onclick="toggleSearch()">🔍</button>
  <div id="search-bar">
    <input type="text" id="search-input" placeholder="Find..." oninput="doSearch()" onkeydown="onSearchKey(event)" />
    <span id="search-count">0/0</span>
    <button class="tool-btn" style="width:24px;height:24px;font-size:11px;" onclick="prevMatch()">▲</button>
    <button class="tool-btn" style="width:24px;height:24px;font-size:11px;" onclick="nextMatch()">▼</button>
  </div>
  <div class="tool-sep"></div>
  <button class="tool-btn" id="btn-theme" title="Toggle Dark / Light Theme" onclick="toggleTheme()">🌓</button>
  <button class="tool-btn" id="btn-sync" title="Sync with Notepad++ Caret" onclick="toggleSync(this)">🔄</button>
  <div class="tool-sep"></div>
  <button class="tool-btn" id="btn-copy-html" title="Copy Rendered HTML" onclick="copyFullHtml()">📋</button>
  <button class="tool-btn" id="btn-print" title="Print or Export to PDF" onclick="window.print()">📄</button>
  <div class="tool-sep"></div>
  <div class="stats-pill">)HTML";
    html << WideToUtf8(statsText);
    html << R"HTML(</div>
</div>

<div id="toc-drawer">
  <div class="toc-header">
    <span>Document Outline</span>
    <button class="toc-close" onclick="toggleToc()">✕</button>
  </div>
  <div class="toc-content">)HTML";
    html << WideToUtf8(tocStream.str());
    html << R"HTML(  </div>
</div>

<div id="content-container">)HTML";
    html << WideToUtf8(bodyStream.str());
    html << R"HTML(</div>

<script>
// KaTeX and Mermaid initialization
document.addEventListener("DOMContentLoaded", function() {
    if (typeof renderMathInElement !== 'undefined') {
        renderMathInElement(document.getElementById("content-container"), {
            delimiters: [
                {left: "$$", right: "$$", display: true},
                {left: "$", right: "$", display: false}
            ],
            throwOnError: false
        });
    }
    if (typeof mermaid !== 'undefined') {
        mermaid.initialize({ startOnLoad: true, theme: document.body.classList.contains('dark') ? 'dark' : 'default' });
    }
});

// Checkbox two-way sync to Notepad++ Scintilla
function handleTaskCheck(cb) {
    var line = cb.getAttribute("data-line");
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage("toggleCheckbox:" + line);
    }
}

// Copy single code block
function copyCodeBlock(btn) {
    var card = btn.closest(".code-block-card");
    if (!card) return;
    var pre = card.querySelector("pre");
    if (!pre) return;
    navigator.clipboard.writeText(pre.innerText).then(function() {
        var oldText = btn.innerHTML;
        btn.innerHTML = "✓ Copied!";
        btn.classList.add("copied");
        setTimeout(function() {
            btn.innerHTML = oldText;
            btn.classList.remove("copied");
        }, 1800);
    });
}

// Copy full HTML
function copyFullHtml() {
    var c = document.getElementById("content-container").innerHTML;
    navigator.clipboard.writeText(c).then(function() {
        var btn = document.getElementById("btn-copy-html");
        btn.innerHTML = "✓";
        setTimeout(function() { btn.innerHTML = "📋"; }, 1500);
    });
}

// Theme toggle
function toggleTheme() {
    document.body.classList.toggle("dark");
    if (typeof mermaid !== 'undefined') {
        mermaid.initialize({ theme: document.body.classList.contains('dark') ? 'dark' : 'default' });
    }
}

// TOC Drawer toggle
function toggleToc() {
    var drawer = document.getElementById("toc-drawer");
    drawer.classList.toggle("open");
}

// Scroll to Scintilla source line
window.scrollToSourceLine = function(targetLine) {
    var el = document.querySelector('[data-source-line="' + targetLine + '"]');
    if (!el) {
        // Find closest
        var all = document.querySelectorAll('[data-source-line]');
        var minDiff = 999999;
        for (var i = 0; i < all.length; i++) {
            var l = parseInt(all[i].getAttribute('data-source-line'));
            var diff = Math.abs(l - targetLine);
            if (diff < minDiff) {
                minDiff = diff;
                el = all[i];
            }
        }
    }
    if (el) {
        el.scrollIntoView({ behavior: 'smooth', block: 'center' });
    }
};

// In-Page Realtime Search
var searchMatches = [];
var currentMatchIndex = -1;

function toggleSearch() {
    var bar = document.getElementById("search-bar");
    if (bar.style.display === "flex") {
        bar.style.display = "none";
        clearSearch();
    } else {
        bar.style.display = "flex";
        document.getElementById("search-input").focus();
    }
}

function clearSearch() {
    var container = document.getElementById("content-container");
    var marks = container.querySelectorAll("mark.search-match");
    marks.forEach(function(m) {
        var parent = m.parentNode;
        parent.replaceChild(document.createTextNode(m.textContent), m);
        parent.normalize();
    });
    searchMatches = [];
    currentMatchIndex = -1;
    document.getElementById("search-count").innerText = "0/0";
}

function doSearch() {
    clearSearch();
    var query = document.getElementById("search-input").value.trim();
    if (!query) return;

    var container = document.getElementById("content-container");
    var walker = document.createTreeWalker(container, NodeFilter.SHOW_TEXT, null, false);
    var textNodes = [];
    while (walker.nextNode()) {
        if (walker.currentNode.parentElement.closest("pre, code, #floating-toolbar, #toc-drawer")) continue;
        textNodes.push(walker.currentNode);
    }

    var qLower = query.toLowerCase();
    textNodes.forEach(function(node) {
        var text = node.nodeValue;
        var tLower = text.toLowerCase();
        var idx = tLower.indexOf(qLower);
        if (idx !== -1) {
            var frag = document.createDocumentFragment();
            var lastIdx = 0;
            while (idx !== -1) {
                frag.appendChild(document.createTextNode(text.substring(lastIdx, idx)));
                var mark = document.createElement("mark");
                mark.className = "search-match";
                mark.textContent = text.substr(idx, query.length);
                frag.appendChild(mark);
                searchMatches.push(mark);
                lastIdx = idx + query.length;
                idx = tLower.indexOf(qLower, lastIdx);
            }
            frag.appendChild(document.createTextNode(text.substring(lastIdx)));
            node.parentNode.replaceChild(frag, node);
        }
    });

    if (searchMatches.length > 0) {
        currentMatchIndex = 0;
        updateActiveMatch();
    } else {
        document.getElementById("search-count").innerText = "0/0";
    }
}

function updateActiveMatch() {
    searchMatches.forEach(function(m) { m.classList.remove("active"); });
    if (currentMatchIndex >= 0 && currentMatchIndex < searchMatches.length) {
        var active = searchMatches[currentMatchIndex];
        active.classList.add("active");
        active.scrollIntoView({ behavior: 'smooth', block: 'center' });
        document.getElementById("search-count").innerText = (currentMatchIndex + 1) + "/" + searchMatches.length;
    }
}

function nextMatch() {
    if (searchMatches.length === 0) return;
    currentMatchIndex = (currentMatchIndex + 1) % searchMatches.length;
    updateActiveMatch();
}

function prevMatch() {
    if (searchMatches.length === 0) return;
    currentMatchIndex = (currentMatchIndex - 1 + searchMatches.length) % searchMatches.length;
    updateActiveMatch();
}

function onSearchKey(e) {
    if (e.key === "Enter") {
        if (e.shiftKey) prevMatch();
        else nextMatch();
    } else if (e.key === "Escape") {
        toggleSearch();
    }
}

// Global shortcut Ctrl+F
document.addEventListener("keydown", function(e) {
    if ((e.ctrlKey || e.metaKey) && e.key === "f") {
        e.preventDefault();
        var bar = document.getElementById("search-bar");
        if (bar.style.display !== "flex") {
            bar.style.display = "flex";
        }
        var input = document.getElementById("search-input");
        input.focus();
        input.select();
    }
});
</script>
</body>
</html>
)HTML";

    return html.str();
}

std::wstring HtmlExporter::ExportToHtml(const MarkdownDocument& doc, const std::wstring& title, bool isDarkMode) {
    std::string previewHtml = GeneratePreviewHtml(doc, title, isDarkMode, 1.0f, false);
    return Utf8ToWide(previewHtml);
}

bool HtmlExporter::SaveToFile(const std::wstring& filePath, const MarkdownDocument& doc, const std::wstring& title, bool isDarkMode) {
    std::string html = GeneratePreviewHtml(doc, title, isDarkMode, 1.0f, false);
    std::ofstream out(filePath.c_str(), std::ios::binary);
    if (!out.is_open()) return false;
    out.write(html.data(), html.size());
    return true;
}

bool HtmlExporter::CopyToClipboard(HWND hwndOwner, const MarkdownDocument& doc, const std::wstring& title, bool isDarkMode) {
    std::string html = GeneratePreviewHtml(doc, title, isDarkMode, 1.0f, false);
    std::string cfHtml = GenerateCfHtml(html);

    if (!OpenClipboard(hwndOwner)) return false;
    EmptyClipboard();

    UINT cfHtmlFormat = RegisterClipboardFormatA("HTML Format");
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, cfHtml.size() + 1);
    if (hMem) {
        void* pData = GlobalLock(hMem);
        if (pData) {
            memcpy(pData, cfHtml.c_str(), cfHtml.size() + 1);
            GlobalUnlock(hMem);
            SetClipboardData(cfHtmlFormat, hMem);
        }
    }

    CloseClipboard();
    return true;
}

std::string HtmlExporter::GenerateCfHtml(const std::string& htmlFragment) {
    std::string header =
        "Version:0.9\r\n"
        "StartHTML:00000000\r\n"
        "EndHTML:00000000\r\n"
        "StartFragment:00000000\r\n"
        "EndFragment:00000000\r\n";

    std::string startHtml = "<html><body>\r\n<!--StartFragment-->";
    std::string endHtml = "<!--EndFragment-->\r\n</body></html>";

    size_t startHtmlPos = header.length();
    size_t startFragPos = startHtmlPos + startHtml.length();
    size_t endFragPos = startFragPos + htmlFragment.length();
    size_t endHtmlPos = endFragPos + endHtml.length();

    char buf[128];
    snprintf(buf, sizeof(buf),
        "Version:0.9\r\n"
        "StartHTML:%08zu\r\n"
        "EndHTML:%08zu\r\n"
        "StartFragment:%08zu\r\n"
        "EndFragment:%08zu\r\n",
        startHtmlPos, endHtmlPos, startFragPos, endFragPos);

    return std::string(buf) + startHtml + htmlFragment + endHtml;
}

std::wstring HtmlExporter::EscapeHtml(const std::wstring& str) {
    std::wstringstream ss;
    for (wchar_t ch : str) {
        switch (ch) {
            case L'&': ss << L"&amp;"; break;
            case L'<': ss << L"&lt;"; break;
            case L'>': ss << L"&gt;"; break;
            case L'\"': ss << L"&quot;"; break;
            case L'\'': ss << L"&#39;"; break;
            default: ss << ch; break;
        }
    }
    return ss.str();
}

std::string HtmlExporter::EscapeHtmlUtf8(const std::string& str) {
    std::stringstream ss;
    for (char ch : str) {
        switch (ch) {
            case '&': ss << "&amp;"; break;
            case '<': ss << "&lt;"; break;
            case '>': ss << "&gt;"; break;
            case '\"': ss << "&quot;"; break;
            case '\'': ss << "&#39;"; break;
            default: ss << ch; break;
        }
    }
    return ss.str();
}

std::wstring HtmlExporter::InlinesToHtml(const std::vector<MarkdownSpan>& inlines) {
    std::wstringstream ss;
    for (const auto& span : inlines) {
        switch (span.type) {
            case InlineStyleType::Bold:
                ss << L"<strong>" << EscapeHtml(span.text) << L"</strong>";
                break;
            case InlineStyleType::Italic:
                ss << L"<em>" << EscapeHtml(span.text) << L"</em>";
                break;
            case InlineStyleType::BoldItalic:
                ss << L"<strong><em>" << EscapeHtml(span.text) << L"</em></strong>";
                break;
            case InlineStyleType::Strikethrough:
                ss << L"<del>" << EscapeHtml(span.text) << L"</del>";
                break;
            case InlineStyleType::Highlight:
                ss << L"<mark>" << EscapeHtml(span.text) << L"</mark>";
                break;
            case InlineStyleType::InlineCode:
                ss << L"<code>" << EscapeHtml(span.text) << L"</code>";
                break;
            case InlineStyleType::InlineMath:
                ss << L"$" << EscapeHtml(span.text) << L"$";
                break;
            case InlineStyleType::Link:
                ss << L"<a href=\"" << EscapeHtml(span.extra) << L"\">" << EscapeHtml(span.text) << L"</a>";
                break;
            case InlineStyleType::Image:
                ss << L"<img src=\"" << EscapeHtml(span.extra) << L"\" alt=\"" << EscapeHtml(span.text) << L"\" style=\"max-width:100%; border-radius:6px;\" />";
                break;
            case InlineStyleType::Normal:
            default:
                ss << EscapeHtml(span.text);
                break;
        }
    }
    return ss.str();
}
