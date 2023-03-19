/**
 *
 * This is the C++ adaptation and derivative work from:
 *
 * Respect Windows Dark/Light themes for QT windows
 * Naveen K <prime@envyen.com>
 *
 * Based on https://github.com/statiolake/neovim-qt/commit/da8eaba7f0e38b6b51f3bacd02a8cc2d1f7a34d8
 * Warning: this is not a proper solution, and Windows api can change !!
 * Stylesheet based on https://forum.qt.io/post/523388
 *
 **/

#ifndef WINTHEME_H
#define WINTHEME_H

#include <QApplication>
#include <QPalette>
#include <QSettings>
#include <QStyleFactory>
#include <QWidget>

#ifdef Q_OS_WIN

#include <Windows.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

namespace WinTheme {
void SetDark_qApp();
void SetDarkTitleBar(HWND hwnd);
bool IsDarkTheme();

enum PreferredAppMode { Default, AllowDark, ForceDark, ForceLight, Max };

enum WINDOWCOMPOSITIONATTRIB {
    WCA_UNDEFINED = 0,
    WCA_NCRENDERING_ENABLED = 1,
    WCA_NCRENDERING_POLICY = 2,
    WCA_TRANSITIONS_FORCEDISABLED = 3,
    WCA_ALLOW_NCPAINT = 4,
    WCA_CAPTION_BUTTON_BOUNDS = 5,
    WCA_NONCLIENT_RTL_LAYOUT = 6,
    WCA_FORCE_ICONIC_REPRESENTATION = 7,
    WCA_EXTENDED_FRAME_BOUNDS = 8,
    WCA_HAS_ICONIC_BITMAP = 9,
    WCA_THEME_ATTRIBUTES = 10,
    WCA_NCRENDERING_EXILED = 11,
    WCA_NCADORNMENTINFO = 12,
    WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
    WCA_VIDEO_OVERLAY_ACTIVE = 14,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
    WCA_DISALLOW_PEEK = 16,
    WCA_CLOAK = 17,
    WCA_CLOAKED = 18,
    WCA_ACCENT_POLICY = 19,
    WCA_FREEZE_REPRESENTATION = 20,
    WCA_EVER_UNCLOAKED = 21,
    WCA_VISUAL_OWNER = 22,
    WCA_HOLOGRAPHIC = 23,
    WCA_EXCLUDED_FROM_DDA = 24,
    WCA_PASSIVEUPDATEMODE = 25,
    WCA_USEDARKMODECOLORS = 26,
    WCA_LAST = 27
};

struct WINDOWCOMPOSITIONATTRIBDATA {
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
};

using fnAllowDarkModeForWindow = BOOL(WINAPI*)(HWND hWnd, BOOL allow);
using fnSetPreferredAppMode = PreferredAppMode(WINAPI*)(PreferredAppMode appMode);
using fnSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND hwnd, WINDOWCOMPOSITIONATTRIBDATA*);

bool IsDarkTheme() {
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                       QSettings::NativeFormat);
    if (settings.value("AppsUseLightTheme") == 0) {
        return true;
    }
    return false;
}

void SetDark_qApp() {
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;
    QColor darkColor = QColor(25, 25, 25);
    QColor disabledColor = QColor(127, 127, 127);
    darkPalette.setColor(QPalette::Window, darkColor);
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(18, 18, 18));
    darkPalette.setColor(QPalette::AlternateBase, darkColor);
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
    darkPalette.setColor(QPalette::Button, darkColor);
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);
    qApp->setPalette(darkPalette);
    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
}

void SetDarkTitleBar(HWND hwnd) {
    HMODULE hUxtheme = LoadLibraryExW(L"uxtheme.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    fnAllowDarkModeForWindow AllowDarkModeForWindow =
        reinterpret_cast<fnAllowDarkModeForWindow>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(133)));
    fnSetPreferredAppMode SetPreferredAppMode =
        reinterpret_cast<fnSetPreferredAppMode>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135)));
    fnSetWindowCompositionAttribute SetWindowCompositionAttribute =
        reinterpret_cast<fnSetWindowCompositionAttribute>(GetProcAddress(hUser32, "SetWindowCompositionAttribute"));

    SetPreferredAppMode(AllowDark);
    BOOL dark = TRUE;
    AllowDarkModeForWindow(hwnd, dark);
    WINDOWCOMPOSITIONATTRIBDATA data = {WCA_USEDARKMODECOLORS, &dark, sizeof(dark)};
    SetWindowCompositionAttribute(hwnd, &data);
}
} // namespace WinTheme
#endif

#endif // WINTHEME_H
