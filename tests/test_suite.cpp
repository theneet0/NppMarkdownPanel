#include "../include/MarkdownParser.h"
#include "../include/BiDiEngine.h"
#include "../include/SyntaxHighlighter.h"
#include "../include/HtmlExporter.h"
#include <iostream>
#include <cassert>
#include <string>

void TestBiDiEngine() {
    std::cout << "[TEST] BiDiEngine..." << std::endl;

    // 1. RTL char detection
    assert(BiDiEngine::IsRTLChar(0x06AF)); // گ
    assert(BiDiEngine::IsRTLChar(0x0686)); // چ
    assert(BiDiEngine::IsRTLChar(0x067E)); // پ
    assert(BiDiEngine::IsRTLChar(0x0698)); // ژ
    assert(BiDiEngine::IsRTLChar(0x06CC)); // ی
    assert(!BiDiEngine::IsRTLChar(L'A'));
    assert(!BiDiEngine::IsRTLChar(L'1'));

    // 2. Strong direction detection
    std::wstring persianSentence = L"این یک متن فارسی است";
    assert(BiDiEngine::IsParagraphRTL(persianSentence));

    std::wstring englishSentence = L"This is an English sentence.";
    assert(!BiDiEngine::IsParagraphRTL(englishSentence));

    // 3. Mixed direction - first strong character
    std::wstring mixedRtl = L"  سلام Hello";
    assert(BiDiEngine::IsParagraphRTL(mixedRtl));

    std::wstring mixedLtr = L"  Hello سلام";
    assert(!BiDiEngine::IsParagraphRTL(mixedLtr));

    // 4. Persian normalization
    std::wstring arabicText = L"يك كمك"; // With Arabic Kaf (0x0643) and Arabic Yeh (0x064A)
    std::wstring normalized = BiDiEngine::NormalizePersianGlyphs(arabicText);
    assert(normalized.find(0x064A) == std::wstring::npos);
    assert(normalized.find(0x0643) == std::wstring::npos);
    assert(normalized.find(0x06CC) != std::wstring::npos); // Persian Yeh
    assert(normalized.find(0x06A9) != std::wstring::npos); // Persian Kaf

    // 5. UTF conversions
    std::string utf8Str = "سلام دنیا Hello World";
    std::wstring wideStr = BiDiEngine::Utf8ToWide(utf8Str);
    assert(!wideStr.empty());
    std::string backToUtf8 = BiDiEngine::WideToUtf8(wideStr);
    assert(backToUtf8 == utf8Str);

    // 6. Persian digit conversion & number parsing
    assert(BiDiEngine::ToPersianDigits(123) == L"۱۲۳");
    assert(BiDiEngine::ToPersianDigits(0) == L"۰");
    int parsedNum = 0;
    size_t consumed = 0;
    assert(BiDiEngine::ParseNumber(L"۴۵۶", 0, parsedNum, consumed));
    assert(parsedNum == 456 && consumed == 3);

    std::cout << "  -> BiDiEngine PASS" << std::endl;
}

void TestMarkdownParser() {
    std::cout << "[TEST] MarkdownParser..." << std::endl;

    std::wstring md =
        L"# Heading 1\n"
        L"## Heading 2\n"
        L"A simple paragraph with **bold** and *italic* and ==highlight== and `code`.\n"
        L"\n"
        L"- Item 1\n"
        L"- [ ] Incomplete task\n"
        L"- [x] Completed task\n"
        L"\n"
        L"۱. مورد اول فارسی\n"
        L"۲. مورد دوم فارسی\n"
        L"\n"
        L"```cpp\n"
        L"int main() {\n"
        L"    return 0;\n"
        L"}\n"
        L"```\n"
        L"\n"
        L"| Col 1 | Col 2 |\n"
        L"|:------|:-----:|\n"
        L"| Val A | Val B |\n"
        L"\n"
        L"> [!NOTE]\n"
        L"> This is an English note alert.\n"
        L"\n"
        L"> [!هشدار]\n"
        L"> این یک هشدار مهم به زبان فارسی است.\n"
        L"\n"
        L"> This is a classic blockquote\n"
        L"\n"
        L"---\n";

    MarkdownDocument doc = MarkdownParser::Parse(md);
    assert(!doc.blocks.empty());

    // Check headings
    assert(doc.blocks[0].type == MarkdownBlockType::Header1);
    assert(doc.blocks[0].rawText == L"Heading 1");
    assert(doc.blocks[1].type == MarkdownBlockType::Header2);
    assert(doc.blocks[1].rawText == L"Heading 2");

    // Check paragraph inlines
    assert(doc.blocks[2].type == MarkdownBlockType::Paragraph);
    bool hasBold = false, hasItalic = false, hasCode = false, hasHighlight = false;
    for (const auto& span : doc.blocks[2].inlines) {
        if (span.type == InlineStyleType::Bold && span.text == L"bold") hasBold = true;
        if (span.type == InlineStyleType::Italic && span.text == L"italic") hasItalic = true;
        if (span.type == InlineStyleType::Highlight && span.text == L"highlight") hasHighlight = true;
        if (span.type == InlineStyleType::InlineCode && span.text == L"code") hasCode = true;
    }
    assert(hasBold);
    assert(hasItalic);
    assert(hasHighlight);
    assert(hasCode);

    // Check list items
    assert(doc.blocks[3].type == MarkdownBlockType::UnorderedListItem);
    assert(doc.blocks[4].type == MarkdownBlockType::TaskListItem && !doc.blocks[4].isTaskChecked);
    assert(doc.blocks[5].type == MarkdownBlockType::TaskListItem && doc.blocks[5].isTaskChecked);

    // Check Persian ordered list
    assert(doc.blocks[6].type == MarkdownBlockType::OrderedListItem);
    assert(doc.blocks[6].listIndex == 1);
    assert(doc.blocks[6].isRTL);
    assert(doc.blocks[7].type == MarkdownBlockType::OrderedListItem);
    assert(doc.blocks[7].listIndex == 2);
    assert(doc.blocks[7].isRTL);

    // Check code block
    assert(doc.blocks[8].type == MarkdownBlockType::CodeBlock);
    assert(doc.blocks[8].codeLanguage == L"cpp");
    assert(doc.blocks[8].codeLines.size() == 3);

    // Check table
    assert(doc.blocks[9].type == MarkdownBlockType::Table);
    assert(doc.blocks[9].table.alignments.size() == 2);
    assert(doc.blocks[9].table.rows.size() == 2);

    // Check Alerts (English & Persian)
    assert(doc.blocks[10].type == MarkdownBlockType::AlertCallout);
    assert(doc.blocks[10].alertType == AlertType::Note);

    assert(doc.blocks[11].type == MarkdownBlockType::AlertCallout);
    assert(doc.blocks[11].alertType == AlertType::Warning);
    assert(doc.blocks[11].isRTL);

    // Check regular blockquote & HR
    assert(doc.blocks[12].type == MarkdownBlockType::Blockquote);
    assert(doc.blocks[13].type == MarkdownBlockType::HorizontalRule);

    std::cout << "  -> MarkdownParser PASS" << std::endl;
}

void TestSyntaxHighlighter() {
    std::cout << "[TEST] SyntaxHighlighter..." << std::endl;

    std::wstring cppCode = L"const int count = 42; // Answer";
    auto tokens = SyntaxHighlighter::Tokenize(cppCode, L"cpp");
    assert(!tokens.empty());

    bool foundKeyword = false;
    bool foundType = false;
    bool foundNumber = false;
    bool foundComment = false;

    for (const auto& t : tokens) {
        if (t.type == HighlightTokenType::Keyword) foundKeyword = true;
        if (t.type == HighlightTokenType::Type) foundType = true;
        if (t.type == HighlightTokenType::Number) foundNumber = true;
        if (t.type == HighlightTokenType::Comment) foundComment = true;
    }
    assert(foundKeyword);
    assert(foundType);
    assert(foundNumber);
    assert(foundComment);

    std::cout << "  -> SyntaxHighlighter PASS" << std::endl;
}

void TestHtmlExporter() {
    std::cout << "[TEST] HtmlExporter..." << std::endl;

    std::wstring md = L"# Title\n\nPersian: سلام دنیا\n\n- Item 1\n- Item 2\n\n> [!NOTE]\n> Alert test\n\n$$E = mc^2$$\n\n```mermaid\ngraph TD\nA-->B\n```\n";
    MarkdownDocument doc = MarkdownParser::Parse(md);
    std::wstring html = HtmlExporter::ExportToHtml(doc, L"Test Title", true);

    assert(!html.empty());
    assert(html.find(L"<!DOCTYPE html>") != std::wstring::npos);
    assert(html.find(L"<title>Test Title</title>") != std::wstring::npos);
    assert(html.find(L"dir=\"rtl\"") != std::wstring::npos);
    assert(html.find(L"<ul") != std::wstring::npos);
    assert(html.find(L"</ul>") != std::wstring::npos);
    assert(html.find(L"alert-callout") != std::wstring::npos);

    // Test modern GeneratePreviewHtml (Glassmorphic Toolbar, KaTeX, Mermaid, Search, TOC)
    std::string previewHtml = HtmlExporter::GeneratePreviewHtml(doc, L"Preview Title", true, 1.0f, true);
    assert(!previewHtml.empty());
    assert(previewHtml.find("floating-toolbar") != std::string::npos);
    assert(previewHtml.find("search-bar") != std::string::npos);
    assert(previewHtml.find("toc-drawer") != std::string::npos);
    assert(previewHtml.find("katex") != std::string::npos);
    assert(previewHtml.find("mermaid") != std::string::npos);

    std::cout << "  -> HtmlExporter PASS" << std::endl;
}

int main() {
    std::cout << "=== Running NppMarkdownPanel Native C++ Unit Tests ===" << std::endl;
    TestBiDiEngine();
    TestMarkdownParser();
    TestSyntaxHighlighter();
    TestHtmlExporter();
    std::cout << "\n>>> ALL 4 TEST SUITES PASSED SUCCESSFULLY! <<<" << std::endl;
    return 0;
}
