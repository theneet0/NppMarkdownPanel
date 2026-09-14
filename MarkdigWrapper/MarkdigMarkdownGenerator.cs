using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using Markdig;
using Markdig.Renderers;
using Markdig.Renderers.Html;
using Markdig.Syntax;
using Markdig.Syntax.Inlines;
using Markdig.SyntaxHighlighting;
using MarkdigWrapper.Markdig.YamlFrontMatter;
using Ganss.Xss;

namespace MarkdigWrapper
{
    public class MarkdigMarkdownGenerator
    {
        private static readonly HtmlSanitizer htmlSanitizer = new HtmlSanitizer();

        public MarkdigMarkdownGenerator()
        {
            htmlSanitizer.AllowedAttributes.Add("data-line");
            htmlSanitizer.AllowedAttributes.Add("class");
            htmlSanitizer.AllowedAttributes.Add("id");
            htmlSanitizer.AllowedAttributes.Add("dir");
            htmlSanitizer.AllowedSchemes.Add("file");
        }

        public string ConvertToHtml(string markDownText, string filepath, bool supportEscapeCharsInUris)
        {
            var sb = new StringBuilder();
            var htmlWriter = new StringWriter(sb);
            var htmlRenderer = new HtmlRenderer(htmlWriter);

            var pipeline = new MarkdownPipelineBuilder()
                .UseAdvancedExtensions()
                // YamlFrontMatter block needs to be parsed with UseYamlFrontMatter()
                // and is then rendered as code block with UseRenderYamlFrontMatterAsCodeBlock()
                .UseYamlFrontMatter()
                .UseRenderYamlFrontMatterAsCodeBlock()
                // Syntax Highlighting
                .UseSyntaxHighlighting()
                .UsePreciseSourceLocation()
                .Build();
            try
            {
                if (filepath != null)
                {
                    htmlRenderer.BaseUrl = new Uri(filepath);
                }
                else
                {
                    htmlRenderer.BaseUrl = null;
                }
            }
            catch (Exception e)
            {
                if (e != null) { }
            }
            sb.Clear();

            var result = "";

            try
            {
                var document = Markdown.Parse(markDownText, pipeline, null);

                SetLineNoAttributeOnAllBlocks(document);

                pipeline.Setup(htmlRenderer);
                htmlRenderer.Render(document);

                htmlWriter.Flush();
                result = sb.ToString();
                result = FixFragmentLinks(result);
                if (filepath != null) result = ResolveRelativePaths(result, filepath);
            }
            catch (Exception e)
            {
                result = e.Message;
            }

            if (supportEscapeCharsInUris) result = UnescapeImageUris(result);
            if (supportEscapeCharsInUris) result = UnescapeAnchorUris(result);
            result = htmlSanitizer.Sanitize(result);
            return result;
        }

        private static bool IsRtlChar(char c)
        {
            return (c >= 0x0600 && c <= 0x06FF) || // Arabic / Persian / Urdu
                   (c >= 0x0750 && c <= 0x077F) || // Arabic Supplement
                   (c >= 0x08A0 && c <= 0x08FF) || // Arabic Extended-A
                   (c >= 0xFB50 && c <= 0xFDFF) || // Arabic Presentation Forms-A
                   (c >= 0xFE70 && c <= 0xFEFF) || // Arabic Presentation Forms-B
                   (c >= 0x0590 && c <= 0x05FF);   // Hebrew
        }

        private static bool IsLtrChar(char c)
        {
            return (c >= 'A' && c <= 'Z') ||
                   (c >= 'a' && c <= 'z') ||
                   (c >= 0x00C0 && c <= 0x024F); // Latin
        }

        public static string DetectDirection(string text)
        {
            if (string.IsNullOrWhiteSpace(text)) return "ltr";

            int rtlCount = 0;
            int ltrCount = 0;
            int firstStrong = 0; // 1 for RTL, -1 for LTR

            for (int i = 0; i < text.Length; i++)
            {
                char c = text[i];
                if (IsRtlChar(c))
                {
                    rtlCount++;
                    if (firstStrong == 0) firstStrong = 1;
                }
                else if (IsLtrChar(c))
                {
                    ltrCount++;
                    if (firstStrong == 0) firstStrong = -1;
                }
            }

            if (rtlCount == 0 && ltrCount == 0) return "ltr";
            if (firstStrong == 1) return "rtl";
            // If it starts with Latin/English words (e.g. "Notepad++ یک ویرایشگر عالی است")
            // but the content is predominantly RTL:
            if (rtlCount > ltrCount) return "rtl";

            return "ltr";
        }

        private static string GetInlineText(ContainerInline container)
        {
            if (container == null) return string.Empty;
            var sb = new StringBuilder();
            var inline = container.FirstChild;
            while (inline != null)
            {
                if (inline is LiteralInline literal)
                {
                    sb.Append(literal.Content.ToString());
                }
                else if (inline is CodeInline code)
                {
                    sb.Append(code.Content);
                    var codeAttrs = code.GetAttributes();
                    codeAttrs.AddPropertyIfNotExist("dir", "ltr");
                    code.SetAttributes(codeAttrs);
                }
                else if (inline is ContainerInline childContainer)
                {
                    sb.Append(GetInlineText(childContainer));
                }
                inline = inline.NextSibling;
            }
            return sb.ToString();
        }

        private static string GetBlockText(Block block)
        {
            if (block == null || block is CodeBlock) return string.Empty;

            if (block is LeafBlock leaf)
            {
                if (leaf.Inline != null)
                {
                    return GetInlineText(leaf.Inline);
                }
                return string.Empty;
            }

            if (block is ContainerBlock container)
            {
                var sb = new StringBuilder();
                foreach (var child in container)
                {
                    sb.Append(GetBlockText(child));
                    sb.Append(' ');
                }
                return sb.ToString();
            }

            return string.Empty;
        }

        private void SetLineNoAttributeOnAllBlocks(ContainerBlock rootBlock)
        {
            foreach (var childBlock in rootBlock)
            {
                if (childBlock is ContainerBlock containerBlock)
                {
                    SetLineNoAttributeOnAllBlocks(containerBlock);
                }

                var attributes = childBlock.GetAttributes();
                attributes.AddProperty("data-line", childBlock.Line.ToString());

                if (childBlock is CodeBlock || childBlock.GetType().Name.Contains("Math"))
                {
                    attributes.AddPropertyIfNotExist("dir", "ltr");
                }
                else
                {
                    string text = GetBlockText(childBlock);
                    string dir = DetectDirection(text);
                    attributes.AddPropertyIfNotExist("dir", dir);
                }
                childBlock.SetAttributes(attributes);
            }
        }

        private string UnescapeImageUris(string html)
        {
            // Unescape URI with % characters for non - US - ASCII characters in order to workaround
            // a bug under IE/Edge with local file links containing non US-ASCII chars.
            //               using System.Text.RegularExpressions;
            //string inp = " * %25%20x : `<img src=\"file:///C:/tmp/test%20nonAscii%20path/A%C4%85C%C4%87E/A%C4%84%2520(2).png\" />`";
            //outp:          * %25%20x : `<img src="file:///C:/tmp/test nonAscii path/AąCćE/AĄ%20(2).png" />`
            Regex regex = new Regex("src=\"file:///[^\"]+");
            return regex.Replace(html, m =>
            {
                return Uri.UnescapeDataString(m.Value);
            });
        }

        private string UnescapeAnchorUris(string html)
        {
            // Unescape URI with % characters for non - US - ASCII characters in order to workaround
            // a bug under IE/Edge with local file links containing non US-ASCII chars.
            //               using System.Text.RegularExpressions;
            Regex regex = new Regex("href=\"file:///[^\"]+");
            return regex.Replace(html, m =>
            {
                return Uri.UnescapeDataString(m.Value);
            });
        }

        /// <summary>
        /// Markdig resolves fragment-only links (#anchor) against BaseUrl,
        /// producing file:///path/%23anchor. Convert them back to #anchor.
        /// </summary>
        private string FixFragmentLinks(string html)
        {
            var regex = new Regex("href=\"file:///[^\"]*%23([^\"]+)\"");
            return regex.Replace(html, m =>
            {
                return "href=\"#" + m.Groups[1].Value + "\"";
            });
        }

        private string ResolveRelativePaths(string html, string baseFilePath)
        {
            if (string.IsNullOrEmpty(baseFilePath)) return html;

            string baseDir;
            try
            {
                baseDir = Path.GetDirectoryName(baseFilePath);
                if (string.IsNullOrEmpty(baseDir)) return html;
            }
            catch
            {
                return html;
            }

            var regex = new Regex(
                @"(src|href)\s*=\s*[""']((?!\s*(?:https?|ftp|file|data|about|mailto|javascript):|//)[^""']+)[""']",
                RegexOptions.IgnoreCase
            );

            return regex.Replace(html, match =>
            {
                var attribute = match.Groups[1].Value;
                var relativePath = match.Groups[2].Value;

                if (string.IsNullOrWhiteSpace(relativePath) || relativePath.StartsWith("#"))
                    return match.Value;

                try
                {
                    var absolutePath = Path.GetFullPath(Path.Combine(baseDir, relativePath));
                    return attribute + "=\"file:///" + absolutePath.Replace('\\', '/') + "\"";
                }
                catch
                {
                    return match.Value;
                }
            });
        }
    }
}
