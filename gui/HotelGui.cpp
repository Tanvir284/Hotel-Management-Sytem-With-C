#define UNICODE
#define _UNICODE
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#define _WIN32_IE 0x0600
#include "GuiTheme.hpp"
#include "../cpp/HotelService.hpp"
#include "../cpp/DateUtil.hpp"
#include "../cpp/RoomCatalog.hpp"

#include <windows.h>
#include <commctrl.h>
#include <algorithm>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {
constexpr int kSidebarWidth = 230;
constexpr int kNavBase = 2000;
constexpr int kViewDashboard = 0;
constexpr int kViewBook = 1;
constexpr int kViewRecord = 2;
constexpr int kViewCheckout = 3;
constexpr int kViewSearch = 4;
constexpr int kViewHistory = 5;
constexpr int kViewCoupon = 6;

HotelService* g_service = nullptr;
HWND g_main = nullptr;
HWND g_content = nullptr;
HWND g_status = nullptr;
int g_view = kViewDashboard;

std::wstring itow(int value) {
    wchar_t buf[32];
    wsprintfW(buf, L"%d", value);
    return buf;
}

std::wstring toWide(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::vector<wchar_t> out(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, out.data(), len);
    if (!out.empty() && out.back() == L'\0') out.pop_back();
    return std::wstring(out.begin(), out.end());
}

std::string toUtf8(const std::wstring& s) {
    if (s.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::vector<char> out(static_cast<size_t>(len), '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, out.data(), len, nullptr, nullptr);
    if (!out.empty() && out.back() == '\0') out.pop_back();
    return std::string(out.begin(), out.end());
}

std::wstring getText(HWND hwnd) {
    int len = GetWindowTextLengthW(hwnd);
    if (len <= 0) return L"";
    std::vector<wchar_t> buf(static_cast<size_t>(len) + 1, L'\0');
    GetWindowTextW(hwnd, buf.data(), len + 1);
    return std::wstring(buf.data());
}

int getInt(HWND hwnd, int fallback = 0) {
    try {
        return std::stoi(toUtf8(getText(hwnd)));
    } catch (...) {
        return fallback;
    }
}

void setStatus(const std::wstring& text) {
    if (g_status) SetWindowTextW(g_status, text.c_str());
}

void showInfo(HWND owner, const std::wstring& title, const std::wstring& msg) {
    MessageBoxW(owner, msg.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION);
}

void showError(HWND owner, const std::wstring& title, const std::wstring& msg) {
    MessageBoxW(owner, msg.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
}

std::wstring bookingDetails(const Booking& b) {
    std::wstring out = L"Room " + itow(b.roomNo()) + L"\r\n";
    out += L"Guests: " + itow(b.capacity()) + L"\r\n";
    for (int i = 0; i < b.capacity(); ++i) {
        const Guest& g = b.guests()[i];
        out += L"  " + itow(i + 1) + L". " + toWide(g.name()) + L" (" +
               toWide(g.gender()) + L", age " + itow(g.age()) + L")\r\n";
    }
    if (!b.address().empty()) {
        out += L"Address: " + toWide(b.address()) + L"\r\n";
        out += L"Phone: " + toWide(b.phone()) + L"\r\n";
    }
    out += L"Stay: " + itow(b.period()) + L" days\r\n";
    out += L"Room: " + itow(b.roomPrice()) + L"  Laundry: " + itow(b.laundryCost()) +
           L"  Gym: " + itow(b.gymCost()) + L"  Food: " + itow(b.foodPrice()) + L"\r\n";
    if (b.discount() > 0) out += L"Discount: " + itow(b.discount()) + L"\r\n";
    out += L"Total: " + itow(b.total()) + L"  Paid: " + itow(b.payment()) + L"  Due: " +
           itow(b.balanceDue()) + L"\r\n";
    out += L"Check-in: " + itow(b.checkInDay()) + L"/" + itow(b.checkInMonth()) + L"/" +
           itow(b.checkInYear()) + L" at " + itow(b.checkInHour()) + L":" +
           itow(b.checkInMinute());
    return out;
}

HWND makeLabel(HWND parent, const wchar_t* text, int x, int y, int w, int h) {
    HWND lbl = CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE, x, y, w, h, parent,
                             nullptr, nullptr, nullptr);
    GuiTheme::useFont(lbl, 15);
    return lbl;
}

HWND makeEdit(HWND parent, int id, int x, int y, int w, int h, bool number = false) {
    DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
    if (number) style |= ES_NUMBER;
    HWND edit = CreateWindowW(L"EDIT", L"", style, x, y, w, h, parent,
                              reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr,
                              nullptr);
    GuiTheme::useFont(edit, 15);
    return edit;
}

HWND makeButton(HWND parent, const wchar_t* text, int id, int x, int y, int w, int h) {
    HWND btn = CreateWindowW(L"BUTTON", text,
                             WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x, y, w, h, parent,
                             reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr,
                             nullptr);
    GuiTheme::useFont(btn, 15, true);
    return btn;
}

void clearContent() {
    if (!g_content) return;
    HWND child = GetWindow(g_content, GW_CHILD);
    while (child) {
        HWND next = GetWindow(child, GW_HWNDNEXT);
        DestroyWindow(child);
        child = next;
    }
}

void populateRoomList(HWND list) {
    ListView_DeleteAllItems(list);
    auto booked = g_service->bookedRooms();
    for (int r = RoomCatalog::kMinRoom; r <= RoomCatalog::kMaxRoom; ++r) {
        auto type = RoomCatalog::typeOf(r);
        bool occupied = std::binary_search(booked.begin(), booked.end(), r);
        LVITEMW item{};
        item.mask = LVIF_TEXT;
        item.iItem = r - 1;
        std::wstring room = itow(r);
        item.pszText = const_cast<LPWSTR>(room.c_str());
        ListView_InsertItem(list, &item);
        ListView_SetItemText(list, r - 1, 1, const_cast<LPWSTR>(toWide(type->name()).c_str()));
        ListView_SetItemText(list, r - 1, 2,
                             const_cast<LPWSTR>(type->isAC() ? L"AC" : L"Non-AC"));
        ListView_SetItemText(list, r - 1, 3, const_cast<LPWSTR>(occupied ? L"Booked" : L"Vacant"));
    }
}

void showDashboard() {
    clearContent();
    auto booked = g_service->bookedRooms();
    auto empty = g_service->emptyRooms();
    std::wstring stats = L"Booked: " + itow(static_cast<int>(booked.size())) +
                         L"    Vacant: " + itow(static_cast<int>(empty.size())) +
                         L"    Total Rooms: 100";
    makeLabel(g_content, stats.c_str(), 20, 15, 700, 28);

    HWND list = CreateWindowW(WC_LISTVIEWW, L"",
                              WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
                              20, 55, 760, 430, g_content, reinterpret_cast<HMENU>(3001),
                              nullptr, nullptr);
    GuiTheme::useFont(list, 14);
    ListView_SetExtendedListViewStyle(list, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    const wchar_t* cols[] = {L"Room", L"Type", L"Cooling", L"Status"};
    int widths[] = {70, 180, 90, 100};
    for (int i = 0; i < 4; ++i) {
        LVCOLUMNW col{};
        col.mask = LVCF_TEXT | LVCF_WIDTH;
        col.pszText = const_cast<LPWSTR>(cols[i]);
        col.cx = widths[i];
        ListView_InsertColumn(list, i, &col);
    }
    populateRoomList(list);
    makeButton(g_content, L"Refresh", 3002, 20, 500, 120, 36);
    setStatus(L"Dashboard loaded.");
}

void showBookForm() {
    clearContent();
    makeLabel(g_content, L"Book A Room", 20, 15, 300, 30);
    makeLabel(g_content, L"Adults", 20, 60, 120, 24);
    makeEdit(g_content, 3101, 150, 56, 120, 28, true);
    makeLabel(g_content, L"Children", 20, 100, 120, 24);
    makeEdit(g_content, 3102, 150, 96, 120, 28, true);
    makeLabel(g_content, L"Stay (days)", 20, 140, 120, 24);
    makeEdit(g_content, 3103, 150, 136, 120, 28, true);
    makeLabel(g_content, L"Room type", 20, 180, 120, 24);
    HWND combo = CreateWindowW(L"COMBOBOX", nullptr,
                               WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 150, 176, 220, 200,
                               g_content, reinterpret_cast<HMENU>(3104), nullptr, nullptr);
    GuiTheme::useFont(combo, 15);
    SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"A.C."));
    SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Non A.C."));
    SendMessageW(combo, CB_SETCURSEL, 0, 0);

    makeLabel(g_content, L"Primary guest name", 20, 230, 180, 24);
    makeEdit(g_content, 3105, 210, 226, 260, 28);
    makeLabel(g_content, L"Phone", 20, 270, 120, 24);
    makeEdit(g_content, 3106, 150, 266, 220, 28);
    makeLabel(g_content, L"Address", 20, 310, 120, 24);
    makeEdit(g_content, 3107, 150, 306, 320, 28);
    makeLabel(g_content, L"Advance payment", 20, 350, 140, 24);
    makeEdit(g_content, 3108, 170, 346, 120, 28, true);

    makeButton(g_content, L"Create Booking", 3200, 20, 410, 180, 40);
    setStatus(L"Fill in booking details and click Create Booking.");
}

void showRecordForm() {
    clearContent();
    makeLabel(g_content, L"View Room Record", 20, 15, 300, 30);
    makeLabel(g_content, L"Room number", 20, 60, 120, 24);
    makeEdit(g_content, 3301, 150, 56, 120, 28, true);
    makeButton(g_content, L"Load Record", 3302, 20, 100, 140, 36);
    HWND details = CreateWindowW(L"EDIT", L"",
                                 WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE |
                                     ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
                                 20, 150, 760, 340, g_content,
                                 reinterpret_cast<HMENU>(3303), nullptr, nullptr);
    GuiTheme::useFont(details, 14);
    SetWindowLongPtrW(details, GWLP_USERDATA, 1); // marker
    setStatus(L"Enter a room number to view its booking.");
}

void showCheckoutForm() {
    clearContent();
    makeLabel(g_content, L"Check Out Guest", 20, 15, 300, 30);
    makeLabel(g_content, L"Room number", 20, 60, 120, 24);
    makeEdit(g_content, 3401, 150, 56, 120, 28, true);
    makeButton(g_content, L"Load", 3402, 20, 100, 90, 36);
    makeButton(g_content, L"Confirm Check-Out", 3403, 130, 100, 190, 36);
    CreateWindowW(L"EDIT", L"",
                  WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY |
                      WS_VSCROLL | ES_AUTOVSCROLL,
                  20, 150, 760, 340, g_content, reinterpret_cast<HMENU>(3404), nullptr,
                  nullptr);
    setStatus(L"Load a room, review the bill, then confirm check-out.");
}

void showSearchForm() {
    clearContent();
    makeLabel(g_content, L"Search Guest", 20, 15, 300, 30);
    makeLabel(g_content, L"Name contains", 20, 60, 140, 24);
    makeEdit(g_content, 3501, 170, 56, 260, 28);
    makeButton(g_content, L"Search", 3502, 20, 100, 120, 36);
    CreateWindowW(L"EDIT", L"",
                  WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY |
                      WS_VSCROLL | ES_AUTOVSCROLL,
                  20, 150, 760, 340, g_content, reinterpret_cast<HMENU>(3503), nullptr,
                  nullptr);
    setStatus(L"Search active bookings by guest name.");
}

void showHistoryForm() {
    clearContent();
    makeLabel(g_content, L"Checkout History", 20, 15, 300, 30);
    makeButton(g_content, L"Refresh", 3601, 20, 55, 120, 36);
    HWND list = CreateWindowW(WC_LISTVIEWW, L"",
                              WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
                              20, 100, 760, 390, g_content, reinterpret_cast<HMENU>(3602),
                              nullptr, nullptr);
    GuiTheme::useFont(list, 14);
    ListView_SetExtendedListViewStyle(list, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    const wchar_t* cols[] = {L"Room", L"Guest", L"Days", L"Total"};
    int widths[] = {70, 260, 70, 100};
    for (int i = 0; i < 4; ++i) {
        LVCOLUMNW col{};
        col.mask = LVCF_TEXT | LVCF_WIDTH;
        col.pszText = const_cast<LPWSTR>(cols[i]);
        col.cx = widths[i];
        ListView_InsertColumn(list, i, &col);
    }
    int row = 0;
    for (const auto& b : g_service->checkoutHistory()) {
        LVITEMW item{};
        item.mask = LVIF_TEXT;
        item.iItem = row;
        std::wstring room = itow(b.roomNo());
        item.pszText = const_cast<LPWSTR>(room.c_str());
        ListView_InsertItem(list, &item);
        std::wstring guest = b.capacity() > 0 ? toWide(b.guests()[0].name()) : L"-";
        ListView_SetItemText(list, row, 1, const_cast<LPWSTR>(guest.c_str()));
        std::wstring days = itow(b.period());
        std::wstring total = itow(b.total());
        ListView_SetItemText(list, row, 2, const_cast<LPWSTR>(days.c_str()));
        ListView_SetItemText(list, row, 3, const_cast<LPWSTR>(total.c_str()));
        ++row;
    }
    setStatus(L"Showing past checkouts.");
}

void showCouponForm() {
    clearContent();
    makeLabel(g_content, L"Apply Discount Coupon", 20, 15, 320, 30);
    makeLabel(g_content, L"Coupon code", 20, 60, 120, 24);
    makeEdit(g_content, 3701, 150, 56, 180, 28);
    makeLabel(g_content, L"Room number", 20, 100, 120, 24);
    makeEdit(g_content, 3702, 150, 96, 120, 28, true);
    makeButton(g_content, L"Apply Coupon", 3703, 20, 145, 160, 36);
    makeLabel(g_content, L"Available: WELCOME10 (10%), STAY20 (20%)", 20, 200, 500, 24);
    setStatus(L"Apply a coupon to an occupied room.");
}

void switchView(int view) {
    g_view = view;
    switch (view) {
        case kViewDashboard: showDashboard(); break;
        case kViewBook: showBookForm(); break;
        case kViewRecord: showRecordForm(); break;
        case kViewCheckout: showCheckoutForm(); break;
        case kViewSearch: showSearchForm(); break;
        case kViewHistory: showHistoryForm(); break;
        case kViewCoupon: showCouponForm(); break;
    }
    InvalidateRect(g_main, nullptr, TRUE);
}

void handleBookSubmit() {
    int adults = getInt(GetDlgItem(g_content, 3101));
    int children = getInt(GetDlgItem(g_content, 3102));
    int period = getInt(GetDlgItem(g_content, 3103));
    int totalGuests = adults + children;
    if (totalGuests < 1 || totalGuests > 10 || period < 1 || period > 60) {
        showError(g_main, L"Booking", L"Enter 1-10 guests and a stay of 1-60 days.");
        return;
    }
    bool wantAC = SendMessageW(GetDlgItem(g_content, 3104), CB_GETCURSEL, 0, 0) == 0;
    std::vector<int> rooms;
    if (!g_service->assignRooms(totalGuests, wantAC, period, rooms)) {
        showError(g_main, L"Booking", L"Not enough free rooms for this party.");
        return;
    }
    std::wstring guestName = getText(GetDlgItem(g_content, 3105));
    if (guestName.empty()) {
        showError(g_main, L"Booking", L"Primary guest name is required.");
        return;
    }
    std::string address = toUtf8(getText(GetDlgItem(g_content, 3107)));
    std::string phone = toUtf8(getText(GetDlgItem(g_content, 3106)));
    int advance = getInt(GetDlgItem(g_content, 3108));

    int guestsLeft = totalGuests;
    bool primary = true;
    for (int room : rooms) {
        int cap = RoomCatalog::typeOf(room)->capacity();
        int here = std::min(cap, guestsLeft);
        Booking b;
        b.setRoomNo(room);
        b.setPeriod(period);
        b.setLaundry(2);
        b.addGuest(Guest(toUtf8(guestName), "Male", 30));
        for (int i = 1; i < here; ++i)
            b.addGuest(Guest("Guest " + std::to_string(i + 1), "Male", 25));
        if (primary) {
            b.setAddress(address);
            b.setPhone(phone);
            int pay = std::min(advance, b.total());
            b.setPayment(pay);
            primary = false;
        }
        b.recomputeRoomPrice();
        Date d = DateUtil::now();
        auto t = DateUtil::nowTime();
        b.setCheckIn(d.day, d.month, d.year, t[0], t[1]);
        g_service->saveBooking(b);
        guestsLeft -= here;
    }
    showInfo(g_main, L"Success", L"Booking created for room(s) assigned.");
    switchView(kViewDashboard);
}

void handleRecordLoad() {
    int room = getInt(GetDlgItem(g_content, 3301));
    Booking b;
    if (!g_service->findBooking(room, b)) {
        SetWindowTextW(GetDlgItem(g_content, 3303), L"Room is vacant.");
        return;
    }
    SetWindowTextW(GetDlgItem(g_content, 3303), bookingDetails(b).c_str());
}

void handleCheckoutLoad() {
    int room = getInt(GetDlgItem(g_content, 3401));
    Booking b;
    if (!g_service->findBooking(room, b)) {
        SetWindowTextW(GetDlgItem(g_content, 3404), L"Room is vacant.");
        return;
    }
    SetWindowTextW(GetDlgItem(g_content, 3404), bookingDetails(b).c_str());
}

void handleCheckoutConfirm() {
    int room = getInt(GetDlgItem(g_content, 3401));
    Booking b;
    if (!g_service->checkout(room, b)) {
        showError(g_main, L"Checkout", L"Room is vacant or invalid.");
        return;
    }
    std::wstring receipt = L"Receipt_Room" + itow(room) + L".txt";
    showInfo(g_main, L"Checked Out",
             L"Guest checked out successfully.\nReceipt: " + receipt);
    switchView(kViewDashboard);
}

void handleSearch() {
    auto matches = g_service->searchGuests(toUtf8(getText(GetDlgItem(g_content, 3501))));
    std::wstring out;
    if (matches.empty()) {
        out = L"No matches found.";
    } else {
        for (const auto& b : matches) {
            out += L"Room " + itow(b.roomNo()) + L" - " +
                   (b.capacity() > 0 ? toWide(b.guests()[0].name()) : L"-") + L"\r\n";
        }
    }
    SetWindowTextW(GetDlgItem(g_content, 3503), out.c_str());
}

void handleCouponApply() {
    int room = getInt(GetDlgItem(g_content, 3702));
    std::string code = toUtf8(getText(GetDlgItem(g_content, 3701)));
    int discount = 0;
    if (!g_service->applyCoupon(room, code, discount)) {
        showError(g_main, L"Coupon", L"Invalid coupon or vacant room.");
        return;
    }
    showInfo(g_main, L"Coupon Applied",
             L"Discount applied: " + itow(discount) + L"/-");
}

bool showLogin(HINSTANCE inst) {
    const wchar_t* cls = L"HotelLoginDlg";
    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = inst;
    wc.lpszClassName = cls;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    HWND dlg = CreateWindowExW(WS_EX_DLGMODALFRAME, cls, L"Hotel Admin Login",
                               WS_POPUP | WS_CAPTION | WS_SYSMENU,
                               CW_USEDEFAULT, CW_USEDEFAULT, 420, 280, nullptr, nullptr,
                               inst, nullptr);
    GuiTheme::useFont(dlg, 16, true);

    CreateWindowW(L"STATIC", L"THE RUHIT'S PALACE", WS_CHILD | WS_VISIBLE, 30, 20, 360, 28,
                  dlg, nullptr, inst, nullptr);
    CreateWindowW(L"STATIC", L"Username", WS_CHILD | WS_VISIBLE, 30, 70, 100, 24, dlg,
                  nullptr, inst, nullptr);
    HWND user = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                              140, 66, 230, 28, dlg, reinterpret_cast<HMENU>(101), inst,
                              nullptr);
    CreateWindowW(L"STATIC", L"Password", WS_CHILD | WS_VISIBLE, 30, 115, 100, 24, dlg,
                  nullptr, inst, nullptr);
    HWND pass = CreateWindowW(L"EDIT", L"",
                              WS_CHILD | WS_VISIBLE | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL,
                              140, 111, 230, 28, dlg, reinterpret_cast<HMENU>(102), inst,
                              nullptr);
    HWND loginBtn = CreateWindowW(L"BUTTON", L"Sign In", WS_CHILD | WS_VISIBLE, 140, 170, 110,
                                  36, dlg, reinterpret_cast<HMENU>(IDOK), inst, nullptr);
    HWND createBtn = CreateWindowW(L"BUTTON", L"Create Admin", WS_CHILD | WS_VISIBLE, 260, 170,
                                   110, 36, dlg, reinterpret_cast<HMENU>(1001), inst, nullptr);
    GuiTheme::useFont(user, 15);
    GuiTheme::useFont(pass, 15);
    GuiTheme::useFont(loginBtn, 15, true);
    GuiTheme::useFont(createBtn, 15);

    ShowWindow(dlg, SW_SHOW);
    UpdateWindow(dlg);
    bool ok = false;
    bool running = true;
    while (running && IsWindow(dlg)) {
        MSG msg;
        if (!GetMessageW(&msg, nullptr, 0, 0)) break;
        if (msg.message == WM_COMMAND && msg.hwnd == dlg) {
            if (LOWORD(msg.wParam) == IDOK) {
                if (g_service->auth().checkCredentials(toUtf8(getText(user)),
                                                         toUtf8(getText(pass)))) {
                    ok = true;
                    running = false;
                } else if (!g_service->auth().hasStoredAccount()) {
                    showError(dlg, L"Login", L"No admin account. Click Create Admin first.");
                } else {
                    showError(dlg, L"Login", L"Invalid username or password.");
                }
            } else if (LOWORD(msg.wParam) == 1001) {
                if (g_service->auth().createAccount(toUtf8(getText(user)),
                                                    toUtf8(getText(pass)))) {
                    showInfo(dlg, L"Account", L"Admin account created. You can sign in now.");
                }
            }
        } else if (msg.message == WM_CLOSE && msg.hwnd == dlg) {
            running = false;
        } else {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    DestroyWindow(dlg);
    return ok;
}

LRESULT CALLBACK ContentProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_COMMAND) {
        switch (LOWORD(wParam)) {
            case 3002: populateRoomList(GetDlgItem(hwnd, 3001)); break;
            case 3200: handleBookSubmit(); break;
            case 3302: handleRecordLoad(); break;
            case 3402: handleCheckoutLoad(); break;
            case 3403: handleCheckoutConfirm(); break;
            case 3502: handleSearch(); break;
            case 3601: switchView(kViewHistory); break;
            case 3703: handleCouponApply(); break;
        }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_main = hwnd;
            CreateWindowW(L"STATIC", L"THE RUHIT'S PALACE", WS_CHILD | WS_VISIBLE, 20, 18, 190,
                          30, hwnd, nullptr, nullptr, nullptr);
            const wchar_t* labels[] = {L"Dashboard", L"Book Room", L"View Record",
                                       L"Check Out", L"Search Guest", L"History",
                                       L"Apply Coupon"};
            for (int i = 0; i < 7; ++i) {
                HWND btn = CreateWindowW(
                    L"BUTTON", labels[i],
                    WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 15, 70 + i * 48, 200, 40, hwnd,
                    reinterpret_cast<HMENU>(static_cast<INT_PTR>(kNavBase + i)), nullptr,
                    nullptr);
                GuiTheme::useFont(btn, 14, true);
            }
            g_content = CreateWindowW(L"HotelContent", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
                                        kSidebarWidth, 0, 800, 560, hwnd, nullptr, nullptr,
                                        nullptr);
            g_status = CreateWindowW(L"STATIC", L"Ready", WS_CHILD | WS_VISIBLE, kSidebarWidth,
                                     560, 800, 28, hwnd, nullptr, nullptr, nullptr);
            GuiTheme::useFont(g_status, 13);
            switchView(kViewDashboard);
            return 0;
        }
        case WM_SIZE: {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            MoveWindow(g_content, kSidebarWidth, 0, w - kSidebarWidth, h - 30, TRUE);
            MoveWindow(g_status, kSidebarWidth, h - 30, w - kSidebarWidth, 30, TRUE);
            return 0;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, GuiTheme::kText);
            return reinterpret_cast<LRESULT>(GetStockObject(WHITE_BRUSH));
        }
        case WM_ERASEBKGND: {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            RECT rc;
            GetClientRect(hwnd, &rc);
            HBRUSH sidebar = CreateSolidBrush(GuiTheme::kSidebar);
            RECT left{0, 0, kSidebarWidth, rc.bottom};
            FillRect(hdc, &left, sidebar);
            DeleteObject(sidebar);
            HBRUSH bg = CreateSolidBrush(GuiTheme::kBackground);
            RECT right{kSidebarWidth, 0, rc.right, rc.bottom};
            FillRect(hdc, &right, bg);
            DeleteObject(bg);
            return 1;
        }
        case WM_DRAWITEM: {
            auto* dis = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
            if (dis->CtlType == ODT_BUTTON && dis->CtlID >= kNavBase &&
                dis->CtlID < kNavBase + 7) {
                bool selected = (dis->CtlID - kNavBase) == g_view;
                COLORREF bg = selected ? GuiTheme::kAccent : GuiTheme::kSidebar;
                if (dis->itemState & ODS_SELECTED) bg = GuiTheme::kAccentHover;
                HBRUSH brush = CreateSolidBrush(bg);
                FillRect(dis->hDC, &dis->rcItem, brush);
                DeleteObject(brush);
                SetBkMode(dis->hDC, TRANSPARENT);
                SetTextColor(dis->hDC, RGB(255, 255, 255));
                wchar_t text[64];
                GetWindowTextW(dis->hwndItem, text, 64);
                DrawTextW(dis->hDC, text, -1, &dis->rcItem,
                          DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                return TRUE;
            }
            break;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) >= kNavBase && LOWORD(wParam) < kNavBase + 7) {
                switchView(static_cast<int>(LOWORD(wParam) - kNavBase));
                for (int i = 0; i < 7; ++i)
                    InvalidateRect(GetDlgItem(hwnd, kNavBase + i), nullptr, TRUE);
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
} // namespace

int WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPSTR, int show) {
    INITCOMMONCONTROLSEX icc{sizeof(icc), ICC_LISTVIEW_CLASSES};
    InitCommonControlsEx(&icc);

    HotelService service;
    g_service = &service;

    WNDCLASSW contentClass{};
    contentClass.lpfnWndProc = ContentProc;
    contentClass.hInstance = inst;
    contentClass.lpszClassName = L"HotelContent";
    contentClass.hbrBackground = CreateSolidBrush(GuiTheme::kBackground);
    RegisterClassW(&contentClass);

    WNDCLASSW mainClass{};
    mainClass.lpfnWndProc = MainProc;
    mainClass.hInstance = inst;
    mainClass.lpszClassName = L"HotelGuiMain";
    mainClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    mainClass.hbrBackground = CreateSolidBrush(GuiTheme::kBackground);
    RegisterClassW(&mainClass);

    if (!showLogin(inst)) return 0;

    HWND hwnd = CreateWindowW(L"HotelGuiMain", L"Ruhit's Palace - Hotel Management",
                              WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1080, 680,
                              nullptr, nullptr, inst, nullptr);
    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
