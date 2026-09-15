# NppMarkdownPanel (Native C++26 Edition) 🚀

[![CI_build](https://github.com/theneet0/NppMarkdownPanel/actions/workflows/CI_build.yml/badge.svg)](https://github.com/theneet0/NppMarkdownPanel/actions/workflows/CI_build.yml)
[![Release](https://img.shields.io/github/v/release/theneet0/NppMarkdownPanel?color=brightgreen)](https://github.com/theneet0/NppMarkdownPanel/releases)
[![Standard](https://img.shields.io/badge/C%2B%2B-26%20%2F%2023-blue.svg)](https://en.cppreference.com/)
[![Rendering](https://img.shields.io/badge/Engine-Direct2D%20%2B%20DirectWrite-purple.svg)](https://docs.microsoft.com/en-us/windows/win32/direct2d/direct2d-portal)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](License.txt)

> **افزونه‌ی فوق‌سبک، فوق‌سریع و کاملاً نیتیو پیش‌نمایش مارک‌داون برای Notepad++ بازسازی‌شده با استاندارد مدرن C++26 و شتاب‌دهی سخت‌افزاری Direct2D.**

---

## ⚡ چرا بازسازی به معماری Pure Native C++26؟ (Performance & Architecture)

نسخه‌های قبلی وابسته به رانتایم سنگین C# .NET 4.7.2 و موتورهای وب کرومیوم/IE11 بودند. در نسخه **1.0.0 Native** کل افزونه از صفر با زبان سی‌پلاس‌پلاس مدرن بازنویسی شد:

| شاخص عملکردی | نسخه سنتی (.NET + WebView2) | نسخه مدرن (Native C++26 + Direct2D) | بهبود |
| :--- | :---: | :---: | :---: |
| **وابستگی‌های خارجی (DLLs)** | ۱۷ فایل مجزا در پوشه `lib/` | **۰ (تک‌فایل مستقل `NppMarkdownPanel.dll`)** | **۱۰۰٪ حذف** |
| **وابستگی به رانتایم** | .NET Framework 4.7.2 + WebView2 Runtime | **هیچ (Native Win32)** | **مستقل** |
| **پروسه‌های جانبی** | ۴ تا ۶ پروسه `msedgewebview2.exe` | **۰ پروسه (کاملاً In-Process)** | **۱۰۰٪ حذف** |
| **مصرف حافظه رم (RAM)** | ~۱۸۰ الی ۲۲۰ مگابایت | **تنها ۵ الی ۸ مگابایت** | **~۹۶٪ کاهش** |
| **تاخیر آغاز به کار (Startup)** | ~۱.۲ ثانیه (سربار لود CLR) | **کمتر از ۱ میلی‌ثانیه (آنی)** | **> ۱۰۰۰ برابر سریع‌تر** |
| **حجم بسته فشرده (Release Zip)** | ~۱۸ مگابایت | **تنها ~۳۴۰ کیلوبایت** | **~۹۸٪ سبک‌تر** |

---

## ✨ قابلیت‌های کلیدی (Key Features)

- 🏎️ **شتاب‌دهی سخت‌افزاری Direct2D و DirectWrite**: رندرینگ GPU با وضوح فوق‌العاده‌ی ClearType و اسکرول ۱۲۰/۶۰ هرتز نرم و بدون لگ حتی روی اسناد چندهزار خطی.
- 🇮🇷 **پشتیبانی درجه‌یک از زبان فارسی و راست‌به‌چپ (RTL BiDi Engine)**:
  - تشخیص هوشمند جهت پاراگراف و خطوط (RTL / LTR).
  - **ایزولاسیون هوشمند کدهای اینلاین (BiDi Isolation)**: عدم به‌هم‌ریختگی براکت‌ها، پرانتزها و کلمات انگلیسی میان جملات فارسی.
  - پشتیبانی کامل از اعداد فارسی در لیست‌های ترتیبی (مانند `۱.`، `۲.`، `۳)`).
  - پشتیبانی از نیم‌فاصله (ZWNJ) و استانداردسازی خودکار حروف (کاف و یای فارسی).
  - انتخاب خودکار زیباترین تایپوگرافی: تشخیص فونت‌های ملی مدرن مانند `Vazirmatn` و fallback به `Segoe UI` و `Tahoma`.
- 🔔 **پشتیبانی از کادرهای اعلانات مدرن (Alert Callouts)**:
  - جعبه‌های مدرن رنگی با آیکون و حاشیه متناسب برای `[!NOTE]`، `[!TIP]`، `[!IMPORTANT]`، `[!WARNING]` و `[!CAUTION]`.
  - پشتیبانی بومی از برچسب‌های فارسی: `[!نکته]`، `[!راهنما]`، `[!مهم]`، `[!هشدار]`، `[!احتیاط]` و `[!خطر]`.
- 📊 **پشتیبانی کامل از مشخصات GFM (GitHub Flavored Markdown)**:
  - **جدول‌ها (Tables)**: رسم کادرها، ردیف‌های متناوب، هدر متمایز و چینش ستون‌ها (چپ، وسط، راست).
  - **چک‌باکس تسک‌لیست (`[ ]` و `[x]`)**: با قابلیت کلیک مستقیم در پنل برای تیک زدن خودکار در کد و پشتیبانی راست‌چین.
  - **بلوک‌های کد پیشرفته با Syntax Highlighting**: استایل مدرن شبیه پنجره‌های macOS با دکمه‌های کنترلی سه‌رنگ، برچسب زبان، رنگ‌آمیزی کلمات کلیدی، رشته‌ها و کامنت‌ها، و دکمه کپی تعاملی با فیدبک `✓ Copied`.
  - **هایلایت متن و فرمول**: پشتیبانی از هایلایت متن (`==متن==`) و نمایش فرمول‌های درون‌خطی (`$math$`).
  - سرفصل‌های H1 تا H6، لیست‌های تو در تو، خطوط نقل‌قول (Blockquote) و خط‌کش افقی.
- 🧭 **پنل ناوبری سرفصل‌ها (Document Outline / TOC)**: فهرست درختی سرفصل‌های فایل با کلیک برای پرش آنی به بخش مربوطه.
- 🔄 **هماهنگی هوشمند اسکرول (Sync Scroll)**: همگام‌سازی بلادرنگ موقعیت پنل با مکان‌نما (Caret) یا خط اول ویرایشگر Scintilla.
- 🎨 **سازگاری خودکار با تم Dark و Light**: هماهنگی خودکار با تم تیره و روشن نوت‌پد‌پلاس‌پلاس بدون نیاز به بارگذاری مجدد.
- 💾 **استخراج تمیز HTML و کپی به کلیپ‌بورد**: خروجی HTML ریسپانسیو با پشتیبانی از وب‌فونت Vazirmatn، کادرهای هشدار و جداول بدون نیاز به اینترنت.

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
