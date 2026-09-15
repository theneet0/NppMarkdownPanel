#include "../include/HtmlExporter.h"
#include "../include/BiDiEngine.h"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace {

const wchar_t* s_htmlStyles = LR"(
:root {
    --bg-color: #ffffff;
    --text-color: #24292f;
    --border-color: #d0d7de;
    --code-bg: #f6f8fa;
    --quote-border: #0969da;
    --table-alt: #f6f8fa;
    --link-color: #0969da;
    --alert-note-bg: #edf6fd;
    --alert-note-bar: #0969da;
    --alert-tip-bg: #edf9ef;
    --alert-tip-bar: #1a7f37;
    --alert-important-bg: #f7f2fa;
    --alert-important-bar: #8250df;
    --alert-warning-bg: #fffbea;
    --alert-warning-bar: #bf8700;
    --alert-caution-bg: #fff0ed;
    --alert-caution-bar: #cf222e;
}
@media (prefers-color-scheme: dark) {
    :root {
        --bg-color: #0d1117;
        --text-color: #e6edf3;
        --border-color: #30363d;
        --code-bg: #161b22;
        --quote-border: #1f6feb;
        --table-alt: #161b22;
        --link-color: #58a6ff;
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
    }
}
body.dark {
    --bg-color: #0d1117;
    --text-color: #e6edf3;
    --border-color: #30363d;
    --code-bg: #161b22;
    --quote-border: #1f6feb;
    --table-alt: #161b22;
    --link-color: #58a6ff;
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
}
body {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", "Vazirmatn", Roboto, Helvetica, Arial, sans-serif;
    font-size: 16px;
    line-height: 1.6;
    background-color: var(--bg-color);
    color: var(--text-color);
    padding: 32px;
    max-width: 900px;
    margin: 0 auto;
}
h1, h2, h3, h4, h5, h6 { margin-top: 24px; margin-bottom: 16px; font-weight: 600; line-height: 1.25; }
h1 { font-size: 2em; border-bottom: 1px solid var(--border-color); padding-bottom: .3em; }
h2 { font-size: 1.5em; border-bottom: 1px solid var(--border-color); padding-bottom: .3em; }
h3 { font-size: 1.25em; }
hr { height: .25em; padding: 0; margin: 24px 0; background-color: var(--border-color); border: 0; }
blockquote { margin: 16px 0; padding: 8px 16px; color: var(--text-color); border-left: 4px solid var(--quote-border); background-color: var(--table-alt); border-radius: 4px; }
blockquote[dir="rtl"] { border-left: none; border-right: 4px solid var(--quote-border); }
pre { background-color: var(--code-bg); padding: 16px; border-radius: 8px; overflow: auto; border: 1px solid var(--border-color); font-size: 90%; }
code { font-family: ui-monospace, SFMono-Regular, "SF Mono", Menlo, Consolas, monospace; font-size: 85%; padding: .2em .4em; background-color: var(--code-bg); border-radius: 4px; }
pre code { padding: 0; background: transparent; font-size: 100%; }
table { border-collapse: collapse; width: 100%; margin: 16px 0; }
th, td { border: 1px solid var(--border-color); padding: 8px 13px; }
tr:nth-child(2n) { background-color: var(--table-alt); }
th { font-weight: 600; background-color: var(--table-alt); }
ul, ol { padding-left: 2em; margin-bottom: 16px; }
ul[dir="rtl"], ol[dir="rtl"] { padding-left: 0; padding-right: 2em; }
a { color: var(--link-color); text-decoration: none; }
a:hover { text-decoration: underline; }
mark { background-color: #fff8c5; color: #24292f; padding: 0.1em 0.3em; border-radius: 3px; }
body.dark mark { background-color: #3e3816; color: #e6edf3; }
.task-list-item { list-style-type: none; margin-left: -1.5em; }
.alert-callout { border-radius: 6px; padding: 12px 16px; margin: 16px 0; border-left: 4px solid var(--quote-border); }
.alert-callout[dir="rtl"] { border-left: none; border-right: 4px solid var(--quote-border); }
.alert-callout-title { font-weight: 600; margin-bottom: 6px; }
.alert-note { background-color: var(--alert-note-bg); border-color: var(--alert-note-bar); }
.alert-tip { background-color: var(--alert-tip-bg); border-color: var(--alert-tip-bar); }
.alert-important { background-color: var(--alert-important-bg); border-color: var(--alert-important-bar); }
.alert-warning { background-color: var(--alert-warning-bg); border-color: var(--alert-warning-bar); }
.alert-caution { background-color: var(--alert-caution-bg); border-color: var(--alert-caution-bar); }
)";

} // namespace

std::wstring HtmlExporter::EscapeHtml(const std::wstring& str) {
    std::wstring out;
    out.reserve(str.size());
    for (wchar_t ch : str) {
        switch (ch) {
            case L'&': out += L"&amp;"; break;
            case L'<': out += L"&lt;"; break;
            case L'>': out += L"&gt;"; break;
            case L'\"': out += L"&quot;"; break;
            case L'\'': out += L"&#39;"; break;
            default: out += ch; break;
        }
    }
    return out;
}

std::wstring HtmlExporter::InlinesToHtml(const std::vector<MarkdownSpan>& inlines) {
    std::wstring out;
    for (const auto& span : inlines) {
        std::wstring escaped = EscapeHtml(span.text);
        switch (span.type) {
            case InlineStyleType::Bold:
                out += L"<strong>" + escaped + L"</strong>";
                break;
            case InlineStyleType::Italic:
                out += L"<em>" + escaped + L"</em>";
                break;
            case InlineStyleType::BoldItalic:
                out += L"<strong><em>" + escaped + L"</em></strong>";
                break;
            case InlineStyleType::Strikethrough:
                out += L"<del>" + escaped + L"</del>";
                break;
            case InlineStyleType::Underline:
                out += L"<u>" + escaped + L"</u>";
                break;
            case InlineStyleType::Highlight:
                out += L"<mark>" + escaped + L"</mark>";
                break;
            case InlineStyleType::InlineCode:
                out += L"<code>" + escaped + L"</code>";
                break;
            case InlineStyleType::InlineMath:
                out += L"<code class=\"math\">" + escaped + L"</code>";
                break;
            case InlineStyleType::Link:
                out += L"<a href=\"" + EscapeHtml(span.extra) + L"\" target=\"_blank\">" + escaped + L"</a>";
                break;
            case InlineStyleType::Image:
                out += L"<img src=\"" + EscapeHtml(span.extra) + L"\" alt=\"" + escaped + L"\" style=\"max-width:100%;\" />";
                break;
            default:
                out += escaped;
                break;
        }
    }
    return out;
}

std::wstring HtmlExporter::ExportToHtml(const MarkdownDocument& doc, const std::wstring& title, bool isDarkMode) {
    std::wstringstream ss;
    ss << L"<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n";
    ss << L"<meta charset=\"UTF-8\">\n";
    ss << L"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    ss << L"<title>" << EscapeHtml(title) << L"</title>\n";
    ss << L"<link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">\n";
    ss << L"<link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" crossorigin>\n";
    ss << L"<link href=\"https://fonts.googleapis.com/css2?family=Vazirmatn:wght@300;400;500;600;700&display=swap\" rel=\"stylesheet\">\n";
    ss << L"<style>\n" << s_htmlStyles << L"</style>\n";
    ss << L"</head>\n<body class=\"" << (isDarkMode ? L"dark" : L"light") << L"\">\n";

    bool inUl = false;
    bool inOl = false;
    auto closeLists = [&]() {
        if (inUl) { ss << L"</ul>\n"; inUl = false; }
        if (inOl) { ss << L"</ol>\n"; inOl = false; }
    };

    for (const auto& block : doc.blocks) {
        std::wstring dirAttr = block.isRTL ? L" dir=\"rtl\"" : L" dir=\"ltr\"";
        if (block.type != MarkdownBlockType::UnorderedListItem &&
            block.type != MarkdownBlockType::OrderedListItem &&
            block.type != MarkdownBlockType::TaskListItem) {
            closeLists();
        }

        switch (block.type) {
            case MarkdownBlockType::Header1:
                ss << L"<h1" << dirAttr << L">" << InlinesToHtml(block.inlines) << L"</h1>\n";
                break;
            case MarkdownBlockType::Header2:
                ss << L"<h2" << dirAttr << L">" << InlinesToHtml(block.inlines) << L"</h2>\n";
                break;
            case MarkdownBlockType::Header3:
                ss << L"<h3" << dirAttr << L">" << InlinesToHtml(block.inlines) << L"</h3>\n";
                break;
            case MarkdownBlockType::Header4:
                ss << L"<h4" << dirAttr << L">" << InlinesToHtml(block.inlines) << L"</h4>\n";
                break;
            case MarkdownBlockType::Header5:
                ss << L"<h5" << dirAttr << L">" << InlinesToHtml(block.inlines) << L"</h5>\n";
                break;
            case MarkdownBlockType::Header6:
                ss << L"<h6" << dirAttr << L">" << InlinesToHtml(block.inlines) << L"</h6>\n";
                break;
            case MarkdownBlockType::HorizontalRule:
                ss << L"<hr/>\n";
                break;
            case MarkdownBlockType::Blockquote:
                ss << L"<blockquote" << dirAttr << L"><p>" << InlinesToHtml(block.inlines) << L"</p></blockquote>\n";
                break;
            case MarkdownBlockType::AlertCallout: {
                std::wstring alertClass = L"alert-note";
                std::wstring alertTitle = block.isRTL ? L"ℹ️ نکته" : L"ℹ️ Note";
                switch (block.alertType) {
                    case AlertType::Tip:
                        alertClass = L"alert-tip";
                        alertTitle = block.isRTL ? L"💡 راهنما / ترفند" : L"💡 Tip";
                        break;
                    case AlertType::Important:
                        alertClass = L"alert-important";
                        alertTitle = block.isRTL ? L"📌 مهم" : L"📌 Important";
                        break;
                    case AlertType::Warning:
                        alertClass = L"alert-warning";
                        alertTitle = block.isRTL ? L"⚠️ هشدار" : L"⚠️ Warning";
                        break;
                    case AlertType::Caution:
                        alertClass = L"alert-caution";
                        alertTitle = block.isRTL ? L"🛑 احتیاط" : L"🛑 Caution";
                        break;
                    default:
                        break;
                }
                ss << L"<div class=\"alert-callout " << alertClass << L"\"" << dirAttr << L">\n";
                ss << L"  <div class=\"alert-callout-title\">" << alertTitle << L"</div>\n";
                ss << L"  <div>" << InlinesToHtml(block.inlines) << L"</div>\n";
                ss << L"</div>\n";
                break;
            }
            case MarkdownBlockType::CodeBlock: {
                std::wstring lang = block.codeLanguage.empty() ? L"" : L" class=\"language-" + EscapeHtml(block.codeLanguage) + L"\"";
                ss << L"<pre><code" << lang << L">";
                for (size_t l = 0; l < block.codeLines.size(); ++l) {
                    ss << EscapeHtml(block.codeLines[l]);
                    if (l + 1 < block.codeLines.size()) ss << L"\n";
                }
                ss << L"</code></pre>\n";
                break;
            }
            case MarkdownBlockType::Table: {
                ss << L"<table" << dirAttr << L">\n";
                for (const auto& row : block.table.rows) {
                    ss << L"  <tr>\n";
                    for (size_t c = 0; c < row.cells.size(); ++c) {
                        std::wstring tag = row.isHeader ? L"th" : L"td";
                        std::wstring alignAttr;
                        if (c < block.table.alignments.size()) {
                            if (block.table.alignments[c] == TableColumnAlign::Center) alignAttr = L" align=\"center\"";
                            else if (block.table.alignments[c] == TableColumnAlign::Right) alignAttr = L" align=\"right\"";
                            else alignAttr = L" align=\"left\"";
                        }
                        ss << L"    <" << tag << alignAttr << L">" << InlinesToHtml(row.cells[c].spans) << L"</" << tag << L">\n";
                    }
                    ss << L"  </tr>\n";
                }
                ss << L"</table>\n";
                break;
            }
            case MarkdownBlockType::TaskListItem: {
                if (inOl) { ss << L"</ol>\n"; inOl = false; }
                if (!inUl) { ss << (block.isRTL ? L"<ul dir=\"rtl\">\n" : L"<ul>\n"); inUl = true; }
                std::wstring checked = block.isTaskChecked ? L"checked " : L"";
                ss << L"<li class=\"task-list-item\"" << dirAttr << L"><input type=\"checkbox\" " << checked << L"disabled/> " << InlinesToHtml(block.inlines) << L"</li>\n";
                break;
            }
            case MarkdownBlockType::UnorderedListItem: {
                if (inOl) { ss << L"</ol>\n"; inOl = false; }
                if (!inUl) { ss << (block.isRTL ? L"<ul dir=\"rtl\">\n" : L"<ul>\n"); inUl = true; }
                ss << L"<li" << dirAttr << L">" << InlinesToHtml(block.inlines) << L"</li>\n";
                break;
            }
            case MarkdownBlockType::OrderedListItem: {
                if (inUl) { ss << L"</ul>\n"; inUl = false; }
                if (!inOl) { ss << (block.isRTL ? L"<ol dir=\"rtl\">\n" : L"<ol>\n"); inOl = true; }
                ss << L"<li" << dirAttr << L">" << InlinesToHtml(block.inlines) << L"</li>\n";
                break;
            }
            case MarkdownBlockType::Paragraph:
            default:
                ss << L"<p" << dirAttr << L">" << InlinesToHtml(block.inlines) << L"</p>\n";
                break;
        }
    }

    closeLists();
    ss << L"</body>\n</html>\n";
    return ss.str();
}

bool HtmlExporter::SaveToFile(const std::wstring& filePath, const MarkdownDocument& doc, const std::wstring& title, bool isDarkMode) {
    std::wstring htmlContent = ExportToHtml(doc, title, isDarkMode);
    std::string utf8Content = BiDiEngine::WideToUtf8(htmlContent);

    std::ofstream file(filePath.c_str(), std::ios::binary);
    if (!file.is_open()) return false;

    file.write(utf8Content.data(), utf8Content.size());
    return file.good();
}

std::string HtmlExporter::GenerateCfHtml(const std::string& htmlFragment) {
    // Windows CF_HTML specification format
    const char* headerTemplate =
        "Version:0.9\r\n"
        "StartHTML:%010d\r\n"
        "EndHTML:%010d\r\n"
        "StartFragment:%010d\r\n"
        "EndFragment:%010d\r\n";

    std::string prefix = "<html><body><!--StartFragment-->";
    std::string postfix = "<!--EndFragment--></body></html>";

    int dummyLen = snprintf(nullptr, 0, headerTemplate, 0, 0, 0, 0);
    int startHTML = dummyLen;
    int startFragment = startHTML + static_cast<int>(prefix.size());
    int endFragment = startFragment + static_cast<int>(htmlFragment.size());
    int endHTML = endFragment + static_cast<int>(postfix.size());

    char headerBuf[256] = { 0 };
    snprintf(headerBuf, sizeof(headerBuf), headerTemplate, startHTML, endHTML, startFragment, endFragment);

    return std::string(headerBuf) + prefix + htmlFragment + postfix;
}

bool HtmlExporter::CopyToClipboard(HWND hwndOwner, const MarkdownDocument& doc, const std::wstring& title, bool isDarkMode) {
    std::wstring htmlContent = ExportToHtml(doc, title, isDarkMode);
    std::string utf8Html = BiDiEngine::WideToUtf8(htmlContent);
    std::string cfHtml = GenerateCfHtml(utf8Html);

    if (!OpenClipboard(hwndOwner)) return false;
    EmptyClipboard();

    // 1. Set CF_UNICODETEXT
    size_t unicodeBytes = (htmlContent.size() + 1) * sizeof(wchar_t);
    HGLOBAL hMemUnicode = GlobalAlloc(GMEM_MOVEABLE, unicodeBytes);
    if (hMemUnicode) {
        void* ptr = GlobalLock(hMemUnicode);
        memcpy(ptr, htmlContent.c_str(), unicodeBytes);
        GlobalUnlock(hMemUnicode);
        SetClipboardData(CF_UNICODETEXT, hMemUnicode);
    }

    // 2. Set CF_HTML
    UINT formatHtml = RegisterClipboardFormatA("HTML Format");
    if (formatHtml != 0) {
        size_t htmlBytes = cfHtml.size() + 1;
        HGLOBAL hMemHtml = GlobalAlloc(GMEM_MOVEABLE, htmlBytes);
        if (hMemHtml) {
            void* ptr = GlobalLock(hMemHtml);
            memcpy(ptr, cfHtml.c_str(), htmlBytes);
            GlobalUnlock(hMemHtml);
            SetClipboardData(formatHtml, hMemHtml);
        }
    }

    CloseClipboard();
    return true;
}
