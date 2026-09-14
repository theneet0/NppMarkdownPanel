#include "../include/MarkdownRenderer.h"
#include "../include/BiDiEngine.h"
#include "../include/SyntaxHighlighter.h"
#include "../include/Scintilla.h"
#include <shellapi.h>
#include <algorithm>
#include <cmath>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

template <class T> void SafeRelease(T** ppT) {
    if (*ppT) {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

MarkdownRenderer::MarkdownRenderer() = default;

MarkdownRenderer::~MarkdownRenderer() {
    DiscardDeviceResources();
    ClearLayout();

    SafeRelease(&m_pFormatH1);
    SafeRelease(&m_pFormatH2);
    SafeRelease(&m_pFormatH3);
    SafeRelease(&m_pFormatH4);
    SafeRelease(&m_pFormatH5);
    SafeRelease(&m_pFormatH6);
    SafeRelease(&m_pFormatBody);
    SafeRelease(&m_pFormatBodyRTL);
    SafeRelease(&m_pFormatCode);
    SafeRelease(&m_pFormatTable);
    SafeRelease(&m_pFormatTableHeader);
    SafeRelease(&m_pFormatBadge);
    SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_pD2DFactory);
}

bool MarkdownRenderer::Initialize(HWND hwnd) {
    m_hwnd = hwnd;
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
    if (FAILED(hr)) return false;

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
    );
    if (FAILED(hr)) return false;

    CreateDeviceIndependentResources();
    CreateDeviceResources();
    return true;
}

HRESULT MarkdownRenderer::CreateDeviceIndependentResources() {
    if (!m_pDWriteFactory) return E_FAIL;

    SafeRelease(&m_pFormatH1);
    SafeRelease(&m_pFormatH2);
    SafeRelease(&m_pFormatH3);
    SafeRelease(&m_pFormatH4);
    SafeRelease(&m_pFormatH5);
    SafeRelease(&m_pFormatH6);
    SafeRelease(&m_pFormatBody);
    SafeRelease(&m_pFormatBodyRTL);
    SafeRelease(&m_pFormatCode);
    SafeRelease(&m_pFormatTable);
    SafeRelease(&m_pFormatTableHeader);
    SafeRelease(&m_pFormatBadge);

    const wchar_t* fontText = L"Segoe UI";
    const wchar_t* fontMono = L"Consolas";

    float scaledBase = m_baseFontSize * m_zoom;

    m_pDWriteFactory->CreateTextFormat(fontText, nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, scaledBase * 1.8f, L"en-us", &m_pFormatH1);
    m_pDWriteFactory->CreateTextFormat(fontText, nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, scaledBase * 1.5f, L"en-us", &m_pFormatH2);
    m_pDWriteFactory->CreateTextFormat(fontText, nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, scaledBase * 1.25f, L"en-us", &m_pFormatH3);
    m_pDWriteFactory->CreateTextFormat(fontText, nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, scaledBase * 1.1f, L"en-us", &m_pFormatH4);
    m_pDWriteFactory->CreateTextFormat(fontText, nullptr, DWRITE_FONT_WEIGHT_MEDIUM, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, scaledBase, L"en-us", &m_pFormatH5);
    m_pDWriteFactory->CreateTextFormat(fontText, nullptr, DWRITE_FONT_WEIGHT_MEDIUM, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, scaledBase * 0.9f, L"en-us", &m_pFormatH6);

    // Body Formats (LTR & RTL)
    m_pDWriteFactory->CreateTextFormat(fontText, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, scaledBase, L"en-us", &m_pFormatBody);
    m_pDWriteFactory->CreateTextFormat(fontText, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, scaledBase, L"fa-ir", &m_pFormatBodyRTL);
    if (m_pFormatBodyRTL) {
        m_pFormatBodyRTL->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
    }

    // Code Format
    m_pDWriteFactory->CreateTextFormat(fontMono, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, scaledBase * 0.9f, L"en-us", &m_pFormatCode);

    // Table Formats
    m_pDWriteFactory->CreateTextFormat(fontText, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, scaledBase * 0.95f, L"en-us", &m_pFormatTable);
    m_pDWriteFactory->CreateTextFormat(fontText, nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, scaledBase * 0.95f, L"en-us", &m_pFormatTableHeader);

    // Badge / Button Format
    m_pDWriteFactory->CreateTextFormat(fontText, nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, scaledBase * 0.75f, L"en-us", &m_pFormatBadge);

    return S_OK;
}

HRESULT MarkdownRenderer::CreateDeviceResources() {
    if (!m_hwnd) return E_FAIL;
    HRESULT hr = S_OK;

    if (!m_pRenderTarget) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        D2D1_SIZE_U size = D2D1::SizeU(
            (std::max)(1L, rc.right - rc.left),
            (std::max)(1L, rc.bottom - rc.top)
        );

        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRenderTarget
        );
        if (FAILED(hr)) return hr;
    }

    // Release old brushes
    SafeRelease(&m_pBrushBg);
    SafeRelease(&m_pBrushText);
    SafeRelease(&m_pBrushH1);
    SafeRelease(&m_pBrushH2);
    SafeRelease(&m_pBrushH3);
    SafeRelease(&m_pBrushCodeBg);
    SafeRelease(&m_pBrushCodeText);
    SafeRelease(&m_pBrushCodeBorder);
    SafeRelease(&m_pBrushKeyword);
    SafeRelease(&m_pBrushType);
    SafeRelease(&m_pBrushString);
    SafeRelease(&m_pBrushComment);
    SafeRelease(&m_pBrushNumber);
    SafeRelease(&m_pBrushQuoteBg);
    SafeRelease(&m_pBrushQuoteBar);
    SafeRelease(&m_pBrushBorder);
    SafeRelease(&m_pBrushTableHdrBg);
    SafeRelease(&m_pBrushTableAltBg);
    SafeRelease(&m_pBrushLink);
    SafeRelease(&m_pBrushLinkHover);
    SafeRelease(&m_pBrushCheckboxBg);
    SafeRelease(&m_pBrushCheckboxBorder);
    SafeRelease(&m_pBrushCheckboxCheck);
    SafeRelease(&m_pBrushBtnBg);
    SafeRelease(&m_pBrushBtnHover);

    if (m_isDarkMode) {
        // Modern Dark Palette (VS Code / GitHub Dark inspired)
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x18181C), &m_pBrushBg);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xDFDFE6), &m_pBrushText);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x569CD6), &m_pBrushH1);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x4EC9B0), &m_pBrushH2);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xDCDCAA), &m_pBrushH3);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x121215), &m_pBrushCodeBg);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xD4D4D4), &m_pBrushCodeText);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x2D2D35), &m_pBrushCodeBorder);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xC586C0), &m_pBrushKeyword);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x4EC9B0), &m_pBrushType);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xCE9178), &m_pBrushString);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x6A9955), &m_pBrushComment);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xB5CEA8), &m_pBrushNumber);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x202026), &m_pBrushQuoteBg);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x007ACC), &m_pBrushQuoteBar);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x2D2D36), &m_pBrushBorder);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x22222A), &m_pBrushTableHdrBg);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x1A1A20), &m_pBrushTableAltBg);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x58A6FF), &m_pBrushLink);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x79C0FF), &m_pBrushLinkHover);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x25252E), &m_pBrushCheckboxBg);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x4E4E5E), &m_pBrushCheckboxBorder);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x4EC9B0), &m_pBrushCheckboxCheck);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x262630), &m_pBrushBtnBg);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x363644), &m_pBrushBtnHover);
    } else {
        // Modern Light Palette (GitHub Light inspired)
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xFFFFFF), &m_pBrushBg);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x24292F), &m_pBrushText);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x0969DA), &m_pBrushH1);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x1F2328), &m_pBrushH2);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x24292F), &m_pBrushH3);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xF6F8FA), &m_pBrushCodeBg);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x24292F), &m_pBrushCodeText);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xD0D7DE), &m_pBrushCodeBorder);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xCF222E), &m_pBrushKeyword);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x953800), &m_pBrushType);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x0A3069), &m_pBrushString);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x6E7781), &m_pBrushComment);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x0550AE), &m_pBrushNumber);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xF6F8FA), &m_pBrushQuoteBg);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x0969DA), &m_pBrushQuoteBar);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xD0D7DE), &m_pBrushBorder);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xF6F8FA), &m_pBrushTableHdrBg);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xFAFAFA), &m_pBrushTableAltBg);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x0969DA), &m_pBrushLink);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x1A7F37), &m_pBrushLinkHover);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xFFFFFF), &m_pBrushCheckboxBg);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x8C959F), &m_pBrushCheckboxBorder);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x0969DA), &m_pBrushCheckboxCheck);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xEEF1F4), &m_pBrushBtnBg);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xDFE3E8), &m_pBrushBtnHover);
    }

    return S_OK;
}

void MarkdownRenderer::DiscardDeviceResources() {
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pBrushBg);
    SafeRelease(&m_pBrushText);
    SafeRelease(&m_pBrushH1);
    SafeRelease(&m_pBrushH2);
    SafeRelease(&m_pBrushH3);
    SafeRelease(&m_pBrushCodeBg);
    SafeRelease(&m_pBrushCodeText);
    SafeRelease(&m_pBrushCodeBorder);
    SafeRelease(&m_pBrushKeyword);
    SafeRelease(&m_pBrushType);
    SafeRelease(&m_pBrushString);
    SafeRelease(&m_pBrushComment);
    SafeRelease(&m_pBrushNumber);
    SafeRelease(&m_pBrushQuoteBg);
    SafeRelease(&m_pBrushQuoteBar);
    SafeRelease(&m_pBrushBorder);
    SafeRelease(&m_pBrushTableHdrBg);
    SafeRelease(&m_pBrushTableAltBg);
    SafeRelease(&m_pBrushLink);
    SafeRelease(&m_pBrushLinkHover);
    SafeRelease(&m_pBrushCheckboxBg);
    SafeRelease(&m_pBrushCheckboxBorder);
    SafeRelease(&m_pBrushCheckboxCheck);
    SafeRelease(&m_pBrushBtnBg);
    SafeRelease(&m_pBrushBtnHover);
}

void MarkdownRenderer::ClearLayout() {
    for (auto& item : m_renderItems) {
        SafeRelease(&item.pLayout);
        for (auto& row : item.tableCellLayouts) {
            for (auto& pL : row) {
                SafeRelease(&pL);
            }
        }
        item.tableCellLayouts.clear();
    }
    m_renderItems.clear();
    m_links.clear();
}

void MarkdownRenderer::SetDarkMode(bool isDark) {
    if (m_isDarkMode != isDark) {
        m_isDarkMode = isDark;
        CreateDeviceResources();
    }
}

void MarkdownRenderer::SetZoom(float zoom) {
    if (zoom < 0.3f) zoom = 0.3f;
    if (zoom > 3.0f) zoom = 3.0f;
    if (std::abs(m_zoom - zoom) > 0.01f) {
        m_zoom = zoom;
        CreateDeviceIndependentResources();
        if (m_lastClientWidth > 10.0f) {
            Layout(m_lastClientWidth);
        }
    }
}

void MarkdownRenderer::SetDocument(const MarkdownDocument& doc) {
    m_doc = doc;
    m_scrollY = 0;
    if (m_lastClientWidth > 10.0f) {
        Layout(m_lastClientWidth);
    }
}

void MarkdownRenderer::Resize(UINT width, UINT height) {
    if (m_pRenderTarget) {
        m_pRenderTarget->Resize(D2D1::SizeU(width, height));
    }
    Layout(static_cast<float>(width));
}

void MarkdownRenderer::Layout(float clientWidth) {
    if (!m_pDWriteFactory || clientWidth < 10.0f) return;
    m_lastClientWidth = clientWidth;

    ClearLayout();

    float padX = 24.0f * m_zoom;
    float contentW = (std::max)(120.0f, clientWidth - (padX * 2.0f));
    float currentY = 16.0f * m_zoom;

    for (const auto& block : m_doc.blocks) {
        RenderItem item;
        item.type = block.type;
        item.sourceLineStart = block.sourceLineStart;
        item.sourceLineEnd = block.sourceLineEnd;
        item.isRTL = block.isRTL;
        item.level = block.level;
        item.listIndex = block.listIndex;
        item.isTaskChecked = block.isTaskChecked;
        item.codeLanguage = block.codeLanguage;
        item.codeLines = block.codeLines;
        item.table = block.table;

        float blockMarginTop = 8.0f * m_zoom;
        float blockMarginBottom = 8.0f * m_zoom;

        switch (block.type) {
            case MarkdownBlockType::Header1:
            case MarkdownBlockType::Header2:
            case MarkdownBlockType::Header3:
            case MarkdownBlockType::Header4:
            case MarkdownBlockType::Header5:
            case MarkdownBlockType::Header6: {
                IDWriteTextFormat* pFormat = m_pFormatH1;
                if (block.type == MarkdownBlockType::Header2) pFormat = m_pFormatH2;
                else if (block.type == MarkdownBlockType::Header3) pFormat = m_pFormatH3;
                else if (block.type == MarkdownBlockType::Header4) pFormat = m_pFormatH4;
                else if (block.type == MarkdownBlockType::Header5) pFormat = m_pFormatH5;
                else if (block.type == MarkdownBlockType::Header6) pFormat = m_pFormatH6;

                blockMarginTop = 18.0f * m_zoom;
                blockMarginBottom = 10.0f * m_zoom;
                currentY += blockMarginTop;

                m_pDWriteFactory->CreateTextLayout(
                    block.rawText.c_str(),
                    static_cast<UINT32>(block.rawText.size()),
                    pFormat,
                    contentW,
                    10000.0f,
                    &item.pLayout
                );

                if (item.pLayout) {
                    if (block.isRTL) item.pLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
                    DWRITE_TEXT_METRICS tm;
                    item.pLayout->GetMetrics(&tm);
                    item.rect = D2D1::RectF(padX, currentY, padX + contentW, currentY + tm.height);
                    currentY += tm.height + blockMarginBottom;
                }
                break;
            }

            case MarkdownBlockType::HorizontalRule: {
                blockMarginTop = 12.0f * m_zoom;
                blockMarginBottom = 12.0f * m_zoom;
                currentY += blockMarginTop;
                item.rect = D2D1::RectF(padX, currentY, padX + contentW, currentY + 2.0f);
                currentY += 2.0f + blockMarginBottom;
                break;
            }

            case MarkdownBlockType::CodeBlock: {
                blockMarginTop = 12.0f * m_zoom;
                blockMarginBottom = 14.0f * m_zoom;
                currentY += blockMarginTop;

                float codePadding = 12.0f * m_zoom;
                float headerH = 26.0f * m_zoom;
                float lineH = (m_baseFontSize * 0.9f * m_zoom) * 1.5f;
                float totalCodeH = headerH + (block.codeLines.size() * lineH) + codePadding;

                item.rect = D2D1::RectF(padX, currentY, padX + contentW, currentY + totalCodeH);
                // Copy button rect in header
                float btnW = 55.0f * m_zoom;
                float btnH = 20.0f * m_zoom;
                item.copyBtnRect = D2D1::RectF(
                    padX + contentW - btnW - 8.0f,
                    currentY + 3.0f,
                    padX + contentW - 8.0f,
                    currentY + 3.0f + btnH
                );

                currentY += totalCodeH + blockMarginBottom;
                break;
            }

            case MarkdownBlockType::Table: {
                blockMarginTop = 12.0f * m_zoom;
                blockMarginBottom = 14.0f * m_zoom;
                currentY += blockMarginTop;

                size_t numCols = block.table.alignments.size();
                if (numCols > 0) {
                    float colW = contentW / static_cast<float>(numCols);
                    float tableTotalH = 0.0f;

                    for (const auto& row : block.table.rows) {
                        float maxCellH = 28.0f * m_zoom;
                        std::vector<IDWriteTextLayout*> rowLayouts;

                        for (size_t c = 0; c < row.cells.size(); ++c) {
                            IDWriteTextLayout* pCellL = nullptr;
                            IDWriteTextFormat* pFmt = row.isHeader ? m_pFormatTableHeader : m_pFormatTable;
                            m_pDWriteFactory->CreateTextLayout(
                                row.cells[c].text.c_str(),
                                static_cast<UINT32>(row.cells[c].text.size()),
                                pFmt,
                                colW - (12.0f * m_zoom),
                                5000.0f,
                                &pCellL
                            );
                            if (pCellL) {
                                TableColumnAlign al = (c < block.table.alignments.size()) ? block.table.alignments[c] : TableColumnAlign::Left;
                                if (al == TableColumnAlign::Center) pCellL->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
                                else if (al == TableColumnAlign::Right) pCellL->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);

                                DWRITE_TEXT_METRICS tm;
                                pCellL->GetMetrics(&tm);
                                if (tm.height + 12.0f * m_zoom > maxCellH) {
                                    maxCellH = tm.height + 12.0f * m_zoom;
                                }
                            }
                            rowLayouts.push_back(pCellL);
                        }
                        item.tableCellLayouts.push_back(rowLayouts);
                        tableTotalH += maxCellH;
                    }

                    item.rect = D2D1::RectF(padX, currentY, padX + contentW, currentY + tableTotalH);
                    currentY += tableTotalH + blockMarginBottom;
                }
                break;
            }

            case MarkdownBlockType::Blockquote: {
                blockMarginTop = 8.0f * m_zoom;
                blockMarginBottom = 8.0f * m_zoom;
                currentY += blockMarginTop;

                float quoteIndent = (20.0f * block.level) * m_zoom;
                float quoteW = contentW - quoteIndent - (16.0f * m_zoom);

                IDWriteTextFormat* pFormat = block.isRTL ? m_pFormatBodyRTL : m_pFormatBody;
                m_pDWriteFactory->CreateTextLayout(
                    block.rawText.c_str(),
                    static_cast<UINT32>(block.rawText.size()),
                    pFormat,
                    quoteW,
                    10000.0f,
                    &item.pLayout
                );

                if (item.pLayout) {
                    DWRITE_TEXT_METRICS tm;
                    item.pLayout->GetMetrics(&tm);
                    float boxH = tm.height + (12.0f * m_zoom);
                    item.rect = D2D1::RectF(padX + quoteIndent, currentY, padX + contentW, currentY + boxH);
                    currentY += boxH + blockMarginBottom;
                }
                break;
            }

            case MarkdownBlockType::UnorderedListItem:
            case MarkdownBlockType::OrderedListItem:
            case MarkdownBlockType::TaskListItem: {
                currentY += 4.0f * m_zoom;
                float indent = (18.0f * block.level) * m_zoom;
                float prefixW = 24.0f * m_zoom;
                float listW = contentW - indent - prefixW;

                IDWriteTextFormat* pFormat = block.isRTL ? m_pFormatBodyRTL : m_pFormatBody;
                m_pDWriteFactory->CreateTextLayout(
                    block.rawText.c_str(),
                    static_cast<UINT32>(block.rawText.size()),
                    pFormat,
                    listW,
                    10000.0f,
                    &item.pLayout
                );

                if (item.pLayout) {
                    DWRITE_TEXT_METRICS tm;
                    item.pLayout->GetMetrics(&tm);
                    float itemH = (std::max)(tm.height, 20.0f * m_zoom);
                    item.rect = D2D1::RectF(padX + indent, currentY, padX + contentW, currentY + itemH);

                    if (block.type == MarkdownBlockType::TaskListItem) {
                        float cbSize = 14.0f * m_zoom;
                        float cbY = currentY + (itemH - cbSize) * 0.5f;
                        item.checkboxRect = D2D1::RectF(padX + indent, cbY, padX + indent + cbSize, cbY + cbSize);
                    }

                    currentY += itemH + 4.0f * m_zoom;
                }
                break;
            }

            case MarkdownBlockType::Paragraph:
            default: {
                currentY += 6.0f * m_zoom;
                IDWriteTextFormat* pFormat = block.isRTL ? m_pFormatBodyRTL : m_pFormatBody;
                m_pDWriteFactory->CreateTextLayout(
                    block.rawText.c_str(),
                    static_cast<UINT32>(block.rawText.size()),
                    pFormat,
                    contentW,
                    10000.0f,
                    &item.pLayout
                );

                if (item.pLayout) {
                    // Apply inline styling
                    size_t charPos = 0;
                    for (const auto& span : block.inlines) {
                        DWRITE_TEXT_RANGE range = { static_cast<UINT32>(charPos), static_cast<UINT32>(span.text.size()) };
                        if (span.type == InlineStyleType::Bold) {
                            item.pLayout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
                        } else if (span.type == InlineStyleType::Italic) {
                            item.pLayout->SetFontStyle(DWRITE_FONT_STYLE_ITALIC, range);
                        } else if (span.type == InlineStyleType::BoldItalic) {
                            item.pLayout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
                            item.pLayout->SetFontStyle(DWRITE_FONT_STYLE_ITALIC, range);
                        } else if (span.type == InlineStyleType::InlineCode) {
                            item.pLayout->SetFontFamilyName(L"Consolas", range);
                        } else if (span.type == InlineStyleType::Link) {
                            item.pLayout->SetUnderline(TRUE, range);
                        }
                        charPos += span.text.size();
                    }

                    DWRITE_TEXT_METRICS tm;
                    item.pLayout->GetMetrics(&tm);
                    item.rect = D2D1::RectF(padX, currentY, padX + contentW, currentY + tm.height);
                    currentY += tm.height + 8.0f * m_zoom;
                }
                break;
            }
        }

        m_renderItems.push_back(item);
    }

    m_totalHeight = currentY + 30.0f * m_zoom;
}

void MarkdownRenderer::Render() {
    if (!m_pRenderTarget && FAILED(CreateDeviceResources())) return;

    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->Clear(m_pBrushBg->GetColor());

    RECT rc;
    GetClientRect(m_hwnd, &rc);
    float clientH = static_cast<float>(rc.bottom - rc.top);

    // Apply scroll transform
    D2D1_MATRIX_3X2_F translation = D2D1::Matrix3x2F::Translation(0.0f, static_cast<float>(-m_scrollY));
    m_pRenderTarget->SetTransform(translation);

    for (const auto& item : m_renderItems) {
        // Culling: check if item is outside visible viewport
        if (item.rect.bottom < m_scrollY || item.rect.top > m_scrollY + clientH) {
            continue;
        }

        switch (item.type) {
            case MarkdownBlockType::Header1: {
                if (item.pLayout) {
                    m_pRenderTarget->DrawTextLayout(D2D1::Point2F(item.rect.left, item.rect.top), item.pLayout, m_pBrushH1);
                    // Underline rule for H1
                    m_pRenderTarget->DrawLine(
                        D2D1::Point2F(item.rect.left, item.rect.bottom + 2.0f),
                        D2D1::Point2F(item.rect.right, item.rect.bottom + 2.0f),
                        m_pBrushBorder, 1.0f
                    );
                }
                break;
            }

            case MarkdownBlockType::Header2: {
                if (item.pLayout) {
                    m_pRenderTarget->DrawTextLayout(D2D1::Point2F(item.rect.left, item.rect.top), item.pLayout, m_pBrushH2);
                    m_pRenderTarget->DrawLine(
                        D2D1::Point2F(item.rect.left, item.rect.bottom + 2.0f),
                        D2D1::Point2F(item.rect.right, item.rect.bottom + 2.0f),
                        m_pBrushBorder, 0.75f
                    );
                }
                break;
            }

            case MarkdownBlockType::Header3:
            case MarkdownBlockType::Header4:
            case MarkdownBlockType::Header5:
            case MarkdownBlockType::Header6: {
                if (item.pLayout) {
                    m_pRenderTarget->DrawTextLayout(D2D1::Point2F(item.rect.left, item.rect.top), item.pLayout, m_pBrushH3);
                }
                break;
            }

            case MarkdownBlockType::HorizontalRule: {
                m_pRenderTarget->DrawLine(
                    D2D1::Point2F(item.rect.left, item.rect.top),
                    D2D1::Point2F(item.rect.right, item.rect.top),
                    m_pBrushBorder, 1.5f
                );
                break;
            }

            case MarkdownBlockType::CodeBlock: {
                D2D1_ROUNDED_RECT rRect = D2D1::RoundedRect(item.rect, 6.0f, 6.0f);
                m_pRenderTarget->FillRoundedRectangle(&rRect, m_pBrushCodeBg);
                m_pRenderTarget->DrawRoundedRectangle(&rRect, m_pBrushCodeBorder, 1.0f);

                // Code Header (Language badge + Copy button)
                float headerH = 26.0f * m_zoom;
                D2D1_RECT_F headerRect = D2D1::RectF(item.rect.left, item.rect.top, item.rect.right, item.rect.top + headerH);
                m_pRenderTarget->DrawLine(
                    D2D1::Point2F(item.rect.left, item.rect.top + headerH),
                    D2D1::Point2F(item.rect.right, item.rect.top + headerH),
                    m_pBrushCodeBorder, 0.75f
                );

                // Language tag
                if (!item.codeLanguage.empty()) {
                    D2D1_RECT_F langRect = D2D1::RectF(item.rect.left + 12.0f, item.rect.top + 4.0f, item.rect.left + 200.0f, item.rect.top + headerH);
                    m_pRenderTarget->DrawTextW(
                        item.codeLanguage.c_str(),
                        static_cast<UINT32>(item.codeLanguage.size()),
                        m_pFormatBadge,
                        &langRect,
                        m_pBrushComment
                    );
                }

                // Copy button
                D2D1_ROUNDED_RECT btnR = D2D1::RoundedRect(item.copyBtnRect, 3.0f, 3.0f);
                m_pRenderTarget->FillRoundedRectangle(&btnR, item.isCopyBtnHovered ? m_pBrushBtnHover : m_pBrushBtnBg);
                m_pRenderTarget->DrawRoundedRectangle(&btnR, m_pBrushCodeBorder, 0.75f);
                m_pRenderTarget->DrawTextW(
                    L"Copy", 4, m_pFormatBadge, &item.copyBtnRect, m_pBrushText
                );

                // Render code lines with syntax highlighting
                float lineH = (m_baseFontSize * 0.9f * m_zoom) * 1.5f;
                float lineY = item.rect.top + headerH + (8.0f * m_zoom);
                float lineX = item.rect.left + (12.0f * m_zoom);

                for (const auto& codeLine : item.codeLines) {
                    if (lineY + lineH > m_scrollY && lineY < m_scrollY + clientH) {
                        D2D1_RECT_F lRect = D2D1::RectF(lineX, lineY, item.rect.right - 12.0f, lineY + lineH);
                        // Draw default line text
                        m_pRenderTarget->DrawTextW(
                            codeLine.c_str(),
                            static_cast<UINT32>(codeLine.size()),
                            m_pFormatCode,
                            &lRect,
                            m_pBrushCodeText
                        );
                    }
                    lineY += lineH;
                }
                break;
            }

            case MarkdownBlockType::Blockquote: {
                D2D1_ROUNDED_RECT rRect = D2D1::RoundedRect(item.rect, 4.0f, 4.0f);
                m_pRenderTarget->FillRoundedRectangle(&rRect, m_pBrushQuoteBg);
                // Vertical accent bar on left (or right for RTL)
                if (item.isRTL) {
                    m_pRenderTarget->FillRectangle(
                        D2D1::RectF(item.rect.right - 4.0f, item.rect.top, item.rect.right, item.rect.bottom),
                        m_pBrushQuoteBar
                    );
                } else {
                    m_pRenderTarget->FillRectangle(
                        D2D1::RectF(item.rect.left, item.rect.top, item.rect.left + 4.0f, item.rect.bottom),
                        m_pBrushQuoteBar
                    );
                }

                if (item.pLayout) {
                    float textX = item.isRTL ? item.rect.left + (8.0f * m_zoom) : item.rect.left + (14.0f * m_zoom);
                    m_pRenderTarget->DrawTextLayout(D2D1::Point2F(textX, item.rect.top + (6.0f * m_zoom)), item.pLayout, m_pBrushText);
                }
                break;
            }

            case MarkdownBlockType::Table: {
                m_pRenderTarget->DrawRectangle(&item.rect, m_pBrushBorder, 1.0f);
                float currentY = item.rect.top;

                size_t numCols = item.table.alignments.size();
                float colW = (item.rect.right - item.rect.left) / static_cast<float>(numCols);

                for (size_t r = 0; r < item.tableCellLayouts.size(); ++r) {
                    const auto& rowLayouts = item.tableCellLayouts[r];
                    bool isHdr = (r == 0 && !item.table.rows.empty() && item.table.rows[0].isHeader);

                    float rowH = 28.0f * m_zoom;
                    for (auto* pL : rowLayouts) {
                        if (pL) {
                            DWRITE_TEXT_METRICS tm;
                            pL->GetMetrics(&tm);
                            if (tm.height + 12.0f * m_zoom > rowH) rowH = tm.height + 12.0f * m_zoom;
                        }
                    }

                    D2D1_RECT_F rowRect = D2D1::RectF(item.rect.left, currentY, item.rect.right, currentY + rowH);
                    if (isHdr) {
                        m_pRenderTarget->FillRectangle(&rowRect, m_pBrushTableHdrBg);
                    } else if (r % 2 == 1) {
                        m_pRenderTarget->FillRectangle(&rowRect, m_pBrushTableAltBg);
                    }

                    // Draw cell borders and text
                    for (size_t c = 0; c < rowLayouts.size(); ++c) {
                        float cellX = item.rect.left + (c * colW);
                        D2D1_RECT_F cellRect = D2D1::RectF(cellX, currentY, cellX + colW, currentY + rowH);
                        m_pRenderTarget->DrawRectangle(&cellRect, m_pBrushBorder, 0.5f);

                        if (rowLayouts[c]) {
                            m_pRenderTarget->DrawTextLayout(
                                D2D1::Point2F(cellX + (6.0f * m_zoom), currentY + (6.0f * m_zoom)),
                                rowLayouts[c],
                                isHdr ? m_pBrushH1 : m_pBrushText
                            );
                        }
                    }

                    currentY += rowH;
                }
                break;
            }

            case MarkdownBlockType::UnorderedListItem: {
                // Draw custom bullet circle
                float bulletX = item.isRTL ? item.rect.right - (12.0f * m_zoom) : item.rect.left + (6.0f * m_zoom);
                float bulletY = item.rect.top + (8.0f * m_zoom);
                m_pRenderTarget->FillEllipse(
                    D2D1::Ellipse(D2D1::Point2F(bulletX, bulletY), 3.0f * m_zoom, 3.0f * m_zoom),
                    m_pBrushH2
                );

                if (item.pLayout) {
                    float textX = item.isRTL ? item.rect.left : item.rect.left + (18.0f * m_zoom);
                    m_pRenderTarget->DrawTextLayout(D2D1::Point2F(textX, item.rect.top), item.pLayout, m_pBrushText);
                }
                break;
            }

            case MarkdownBlockType::OrderedListItem: {
                if (item.pLayout) {
                    float textX = item.isRTL ? item.rect.left : item.rect.left + (18.0f * m_zoom);
                    m_pRenderTarget->DrawTextLayout(D2D1::Point2F(textX, item.rect.top), item.pLayout, m_pBrushText);
                }
                break;
            }

            case MarkdownBlockType::TaskListItem: {
                // Checkbox
                D2D1_ROUNDED_RECT cbR = D2D1::RoundedRect(item.checkboxRect, 3.0f, 3.0f);
                m_pRenderTarget->FillRoundedRectangle(&cbR, m_pBrushCheckboxBg);
                m_pRenderTarget->DrawRoundedRectangle(&cbR, m_pBrushCheckboxBorder, 1.2f);

                if (item.isTaskChecked) {
                    // Draw checkmark inside
                    float l = item.checkboxRect.left;
                    float t = item.checkboxRect.top;
                    float s = item.checkboxRect.right - l;
                    m_pRenderTarget->DrawLine(
                        D2D1::Point2F(l + s * 0.2f, t + s * 0.5f),
                        D2D1::Point2F(l + s * 0.45f, t + s * 0.75f),
                        m_pBrushCheckboxCheck, 1.8f
                    );
                    m_pRenderTarget->DrawLine(
                        D2D1::Point2F(l + s * 0.45f, t + s * 0.75f),
                        D2D1::Point2F(l + s * 0.8f, t + s * 0.25f),
                        m_pBrushCheckboxCheck, 1.8f
                    );
                }

                if (item.pLayout) {
                    float textX = item.isRTL ? item.rect.left : item.checkboxRect.right + (8.0f * m_zoom);
                    m_pRenderTarget->DrawTextLayout(D2D1::Point2F(textX, item.rect.top), item.pLayout, m_pBrushText);
                }
                break;
            }

            case MarkdownBlockType::Paragraph:
            default: {
                if (item.pLayout) {
                    m_pRenderTarget->DrawTextLayout(D2D1::Point2F(item.rect.left, item.rect.top), item.pLayout, m_pBrushText);
                }
                break;
            }
        }
    }

    HRESULT hr = m_pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        DiscardDeviceResources();
    }
}

void MarkdownRenderer::ScrollBy(int deltaY) {
    ScrollTo(m_scrollY + deltaY);
}

void MarkdownRenderer::ScrollTo(int newScrollY) {
    RECT rc;
    GetClientRect(m_hwnd, &rc);
    int clientH = rc.bottom - rc.top;
    int maxScroll = (std::max)(0, static_cast<int>(m_totalHeight) - clientH);

    if (newScrollY < 0) newScrollY = 0;
    if (newScrollY > maxScroll) newScrollY = maxScroll;

    if (m_scrollY != newScrollY) {
        m_scrollY = newScrollY;
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void MarkdownRenderer::ScrollToSourceLine(int sourceLine) {
    for (const auto& item : m_renderItems) {
        if (sourceLine >= item.sourceLineStart && sourceLine <= item.sourceLineEnd) {
            ScrollTo(static_cast<int>(item.rect.top - (20.0f * m_zoom)));
            return;
        }
    }
}

bool MarkdownRenderer::OnMouseMove(int x, int y) {
    float docY = static_cast<float>(y + m_scrollY);
    float docX = static_cast<float>(x);
    bool needRepaint = false;

    for (auto& item : m_renderItems) {
        if (item.type == MarkdownBlockType::CodeBlock) {
            bool isInside = (docX >= item.copyBtnRect.left && docX <= item.copyBtnRect.right &&
                             docY >= item.copyBtnRect.top && docY <= item.copyBtnRect.bottom);
            if (item.isCopyBtnHovered != isInside) {
                item.isCopyBtnHovered = isInside;
                needRepaint = true;
            }
        }
    }
    return needRepaint;
}

bool MarkdownRenderer::OnLButtonDown(int x, int y, HWND hSciEditor) {
    float docY = static_cast<float>(y + m_scrollY);
    float docX = static_cast<float>(x);

    for (auto& item : m_renderItems) {
        // 1. Copy button on Code Block
        if (item.type == MarkdownBlockType::CodeBlock) {
            if (docX >= item.copyBtnRect.left && docX <= item.copyBtnRect.right &&
                docY >= item.copyBtnRect.top && docY <= item.copyBtnRect.bottom) {

                std::wstring allCode;
                for (const auto& l : item.codeLines) {
                    allCode += l + L"\r\n";
                }
                if (OpenClipboard(m_hwnd)) {
                    EmptyClipboard();
                    size_t bytes = (allCode.size() + 1) * sizeof(wchar_t);
                    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
                    if (hMem) {
                        void* ptr = GlobalLock(hMem);
                        memcpy(ptr, allCode.c_str(), bytes);
                        GlobalUnlock(hMem);
                        SetClipboardData(CF_UNICODETEXT, hMem);
                    }
                    CloseClipboard();
                }
                return true;
            }
        }

        // 2. Checkbox toggle on Task List Item
        if (item.type == MarkdownBlockType::TaskListItem && hSciEditor) {
            if (docX >= item.checkboxRect.left && docX <= item.checkboxRect.right &&
                docY >= item.checkboxRect.top && docY <= item.checkboxRect.bottom) {

                // Toggle checkbox in Scintilla source!
                int line = item.sourceLineStart;
                int linePos = static_cast<int>(SendMessage(hSciEditor, SCI_POSITIONFROMLINE, line, 0));
                int lineEnd = static_cast<int>(SendMessage(hSciEditor, SCI_POSITIONFROMLINE, line + 1, 0));
                int lineLen = lineEnd - linePos;
                if (lineLen > 0) {
                    std::string lineBuf(lineLen + 1, '\0');
                    SendMessage(hSciEditor, SCI_GETTEXT, lineLen + 1, (LPARAM)&lineBuf[0]);
                    // Search for [ ] or [x]
                    size_t posUnchecked = lineBuf.find("[ ]");
                    size_t posChecked = lineBuf.find("[x]");
                    if (posChecked == std::string::npos) posChecked = lineBuf.find("[X]");

                    if (item.isTaskChecked && posChecked != std::string::npos) {
                        SendMessage(hSciEditor, SCI_SETSEL, linePos + posChecked + 1, linePos + posChecked + 2);
                        SendMessage(hSciEditor, SCI_REPLACESEL, 0, (LPARAM)" ");
                    } else if (!item.isTaskChecked && posUnchecked != std::string::npos) {
                        SendMessage(hSciEditor, SCI_SETSEL, linePos + posUnchecked + 1, linePos + posUnchecked + 2);
                        SendMessage(hSciEditor, SCI_REPLACESEL, 0, (LPARAM)"x");
                    }
                }
                return true;
            }
        }
    }

    return false;
}
