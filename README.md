# NppMarkdownPanel (Native C++ Modern Edition) 🚀

[![CI_build](https://github.com/theneet0/NppMarkdownPanel/actions/workflows/CI_build.yml/badge.svg)](https://github.com/theneet0/NppMarkdownPanel/actions/workflows/CI_build.yml)
[![Release](https://img.shields.io/github/v/release/theneet0/NppMarkdownPanel?color=brightgreen)](https://github.com/theneet0/NppMarkdownPanel/releases)
[![Standard](https://img.shields.io/badge/C%2B%2B-26%20%2F%2020-blue.svg)](https://en.cppreference.com/)
[![Rendering](https://img.shields.io/badge/Engine-WebView2%20%2B%20Direct2D-purple.svg)](https://docs.microsoft.com/en-us/microsoft-edge/webview2/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](License.txt)

> **افزونه‌ی فوق‌العاده مدرن، زیبا و قدرتمند پیش‌نمایش مارک‌داون برای Notepad++ با موتور دوگانه WebView2 و Direct2D، رابط کاربری شیشه‌ای (Glassmorphism)، فرمول‌های ریاضی KaTeX، دیاگرام‌های Mermaid و پشتیبانی عالی از زبان فارسی.**

---

## ✨ قابلیت‌های برجسته نسخه مدرن (v1.0.2 Features)

- 🌐 **موتور رندرینگ دوگانه نسل جدید (Dual-Engine Architecture)**:
  - **موتور پیش‌فرض WebView2 (Evergreen Chromium)**: رندرینگ وب فوق‌العاده با HTML5/CSS3، پشتیبانی از فونت‌های وب، استایل‌های مدرن و عدم برش لبه‌های متن.
  - **موتور Direct2D / DirectWrite نیتیو (Fallback)**: تضمین کارکرد حتی در سیستم‌های فاقد WebView2 بدون وابستگی خارجی.
- 💎 **تولبار شیشه‌ای شناور (Floating Glassmorphic Toolbar)**:
  - حذف نوار ابزار خاکستری و سنتی Win32 و جایگزینی با پنل کنترلی شیشه‌ای شناور با افکت بلور (Backdrop Blur).
  - دکمه‌های جستجوی زنده، فهرست سرفصل‌ها (TOC Drawer)، تغییر پوسته، آمار مطالعه، خروجی و پرینت.
- 🔍 **جستجوی درون‌متنی زنده (In-Page Search & Highlight)**:
  - جستجوی بلادرنگ با کلید میانبر `Ctrl+F`، شمارنده نتایج (`[1/5]`) و هایلایت زرد نتایج منطبق.
- 📐 **پشتیبانی کامل از فرمول‌های ریاضی (KaTeX LaTeX Math)**:
  - رندرینگ سریع فرمول‌های ریاضی درون‌خطی (`$E=mc^2$`) و بلوکی (`$$\sum_{i=1}^n i = \frac{n(n+1)}{2}$$`).
- 📊 **دیاگرام‌ها و نمودارهای تعاملی (Mermaid.js)**:
  - پشتیبانی مستقیم از فلوچارت‌ها، نمودارهای توالی (Sequence Diagrams)، نقشه ذهنی (Mindmaps) و گانت چارت در بلوک‌های ````mermaid.
- 🇮🇷 **پشتیبانی درجه‌یک از زبان فارسی و راست‌به‌چپ (BiDi Engine & Typography)**:
  - تشخیص خودکار جهت پاراگراف‌ها (RTL / LTR) و بدون افتادگی کلمات در انتهای خطوط.
  - تایپوگرافی چشم‌نواز با فونت ملی **وزیرمتن (Vazirmatn)** و فال‌بک تمیز به Segoe UI / Tahoma.
  - اعداد فارسی هوشمند در لیست‌ها و نشانگر زمان تخمینی مطالعه.
- 💻 **بلوک‌های کد پیشرفته استایل macOS**:
  - نوارهای بالایی به سبک macOS با دکمه‌های کنترلی سه‌رنگ، نشانگر نام زبان برنامه‌نویسی و دکمه کپی فوری با انیمیشن `✓ کپی شد!`.
- ☑️ **همگام‌سازی دوطرفه تسک‌لیست (Interactive 2-Way Checkbox)**:
  - کلیک روی چک‌باکس‌های `[ ]` در پنل پیش‌نمایش بلافاصله متن متناظر را در فایل فعال Notepad++ به `[x]` تبدیل می‌کند.
- ⏱️ **نشانگر آمار و زمان تخمینی مطالعه**:
  - شمارش هوشمند واژه‌ها و تخمین زمان مطالعه به زبان فارسی و انگلیسی.
- 🖨️ **خروجی مستقل HTML و چاپ به PDF**:
  - امکان ذخیره سند با فرمت HTML مستقل و ارسال مستقیم به دیالوگ پرینت ویندوز (Save as PDF).

---

## 📥 نحوه نصب (Installation)

### نصب دستی (Manual)
1. آخرین نسخه‌ی فشرده متناسب با معماری نوت‌پد‌پلاس‌پلاس خود (`x64` یا `x86`) را از بخش [Releases](https://github.com/theneet0/NppMarkdownPanel/releases) دانلود کنید.
2. پوشه‌ای با نام `NppMarkdownPanel` در مسیر افزونه‌های Notepad++ ایجاد کنید:
   - برای نسخه ۶۴ بیتی: `C:\Program Files\Notepad++\plugins\NppMarkdownPanel\`
   - برای نسخه پرتابل: `<Npp_Folder>\plugins\NppMarkdownPanel\`
3. فایل `NppMarkdownPanel.dll` را درون این پوشه قرار دهید و Notepad++ را ری‌استارت کنید.

---

## ⌨️ کلیدهای میانبر پیش‌فرض (Shortcuts)

| دستور | کلید میانبر | عملکرد |
| :--- | :---: | :--- |
| **Toggle Markdown Panel** | `Ctrl + Shift + M` | نمایش / پنهان‌سازی پنل پیش‌نمایش |
| **Outline / TOC** | نوار ابزار پنل | باز و بسته کردن فهرست درختی سرفصل‌ها |
| **Zoom In / Out** | `Ctrl + Wheel` یا دکمه‌های `+` / `-` | بزرگ‌نمایی و کوچک‌نمایی متن پیش‌نمایش |
| **Copy HTML** | دکمه `📋 HTML` | کپی کدهای HTML رندرشده به کلیپ‌بورد ویندوز |
| **Export HTML** | دکمه `💾 Export` | ذخیره خروجی HTML در یک فایل مستقل |

---

## 🛠️ کامپایل از سورس‌کد (Building from Source)

برای بیلد محلی، تنها به یک کامپایلر Clang/LLVM مدرن با پشتیبانی از C++23/C++26 نیاز دارید:

```powershell
# اجرای تست‌های واحد و کامپایل DLL نسخه x64 و x86
./build.ps1

# ایجاد بسته‌های فشرده ریلیز
./makerelease.ps1
```

---

## 📄 مجوز (License)

این پروژه تحت مجوز **MIT License** منتشر شده است.
Copyright (c) 2026 [theneet0](https://github.com/theneet0)
