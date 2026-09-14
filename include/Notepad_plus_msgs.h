#pragma once
#include <windows.h>

#define NPPMSG (WM_USER + 1000)

#define NPPM_GETCURRENTSCINTILLA       (NPPMSG + 4)
#define NPPM_GETCURRENTBUFFERID        (NPPMSG + 60)
#define NPPM_GETBUFFERFILENAME         (NPPMSG + 61)
#define NPPM_GETFULLPATHFROMBUFFERID   (NPPMSG + 58)
#define NPPM_MENUCOMMAND               (NPPMSG + 48)
#define NPPM_MODELESSDIALOG            (NPPMSG + 12)
#define NPPM_GETMENUHANDLE             (NPPMSG + 39)
#define NPPM_SETMENUITEMCHECK          (NPPMSG + 27)
#define NPPM_ADDTOOLBARICON            (NPPMSG + 32)
#define NPPM_GETPLUGINHOMEPATH         (NPPMSG + 94)
#define NPPM_GETPLUGINSCONFIGDIR       (NPPMSG + 46)

// Docking Manager Messages
#define NPPM_DMMREGASDCKDLG            (NPPMSG + 20)
#define NPPM_DMMSHOW                   (NPPMSG + 21)
#define NPPM_DMMHIDE                   (NPPMSG + 22)
#define NPPM_DMMUPDATETITLE            (NPPMSG + 23)

// Dark Mode Messages (Notepad++ v8.4.1+)
#define NPPM_ISDARKMODEENABLED         (NPPMSG + 107)
#define NPPM_GETDARKMODECOLORS         (NPPMSG + 108)

// Notifications
#define NPPN_FIRST                     1000
#define NPPN_READY                     (NPPN_FIRST + 1)
#define NPPN_TBMODIFICATION            (NPPN_FIRST + 2)
#define NPPN_FILEBEFORECLOSE           (NPPN_FIRST + 3)
#define NPPN_FILEOPENED                (NPPN_FIRST + 4)
#define NPPN_FILECLOSED                (NPPN_FIRST + 5)
#define NPPN_FILESAVED                 (NPPN_FIRST + 6)
#define NPPN_SHUTDOWN                  (NPPN_FIRST + 7)
#define NPPN_BUFFERACTIVATED           (NPPN_FIRST + 8)
#define NPPN_LANGCHANGED               (NPPN_FIRST + 9)
#define NPPN_WORDSTYLESUPDATED         (NPPN_FIRST + 10)
#define NPPN_SHORTCUTREMAPPED          (NPPN_FIRST + 11)
#define NPPN_DARKMODECHANGED           (NPPN_FIRST + 27)

// Standard View IDs
#define IDM_VIEW_RTL                   44072
#define IDM_VIEW_LTR                   44073

struct NppData {
    HWND _nppHandle;
    HWND _scintillaMainHandle;
    HWND _scintillaSecondHandle;
};

struct NppDarkModeColors {
    COLORREF background;
    COLORREF softerBackground;
    COLORREF hotBackground;
    COLORREF pureBackground;
    COLORREF errorBackground;
    COLORREF text;
    COLORREF darkerText;
    COLORREF disabledText;
    COLORREF linkText;
    COLORREF edge;
    COLORREF hotEdge;
    COLORREF disabledEdge;
};
