#pragma once

#define IDI_ICON_PANEL                 101
#define IDB_TOOLBAR_BMP                102
#define IDI_ICON_PANEL_DARK            103

// Plugin Commands
#define CMD_TOGGLE_PANEL               0
#define CMD_SYNC_CARET                 1
#define CMD_SYNC_FIRST_LINE            2
#define CMD_TOGGLE_OUTLINE             3
#define CMD_COPY_HTML                  4
#define CMD_SAVE_HTML                  5
#define CMD_ZOOM_IN                    6
#define CMD_ZOOM_OUT                   7
#define CMD_ZOOM_RESET                 8
#define CMD_TOGGLE_BIDI                9
#define CMD_SETTINGS                   10
#define CMD_ABOUT                      11
#define NB_PLUGIN_COMMANDS             12

// Panel UI Child IDs
#define IDC_PANEL_TOOLBAR              2001
#define IDC_BTN_REFRESH                2002
#define IDC_BTN_OUTLINE                2003
#define IDC_BTN_ZOOM_IN                2004
#define IDC_BTN_ZOOM_OUT               2005
#define IDC_BTN_COPY_HTML              2006
#define IDC_BTN_EXPORT_HTML            2007
#define IDC_BTN_SYNC_TOGGLE            2008
#define IDC_BTN_BIDI_TOGGLE            2009
#define IDC_STATIC_STATS               2010

// Timer IDs
#define IDT_RENDER_DEBOUNCE            3001
#define RENDER_DEBOUNCE_DELAY_MS       120
