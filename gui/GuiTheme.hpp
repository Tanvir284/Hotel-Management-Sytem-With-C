#ifndef GUI_THEME_HPP
#define GUI_THEME_HPP

#include <windows.h>

namespace GuiTheme {
constexpr COLORREF kSidebar = RGB(15, 23, 42);
constexpr COLORREF kSidebarHover = RGB(30, 41, 59);
constexpr COLORREF kAccent = RGB(37, 99, 235);
constexpr COLORREF kAccentHover = RGB(29, 78, 216);
constexpr COLORREF kBackground = RGB(248, 250, 252);
constexpr COLORREF kCard = RGB(255, 255, 255);
constexpr COLORREF kText = RGB(15, 23, 42);
constexpr COLORREF kMuted = RGB(100, 116, 139);
constexpr COLORREF kSuccess = RGB(22, 163, 74);
constexpr COLORREF kDanger = RGB(220, 38, 38);

inline void useFont(HWND hwnd, int size = 16, bool bold = false) {
    HFONT font = CreateFontW(
        -size, 0, 0, 0, bold ? FW_SEMIBOLD : FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
}
} // namespace GuiTheme

#endif
