#include <windows.h>
#include <stdio.h>

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ShowNotImplemented();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "HotelMgmtWindow";

    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Hotel Management",
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
                              NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void AddMenus(HWND hwnd) {
    HMENU hMenubar = CreateMenu();
    HMENU hMenu = CreateMenu();

    AppendMenu(hMenu, MF_STRING, 1, "Book A Room");
    AppendMenu(hMenu, MF_STRING, 2, "Display Room Record");
    AppendMenu(hMenu, MF_STRING, 3, "Rooms Status");
    AppendMenu(hMenu, MF_STRING, 4, "Room Features");
    AppendMenu(hMenu, MF_STRING, 5, "Modify Room Record");
    AppendMenu(hMenu, MF_STRING, 6, "Check-Out");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, 7, "Exit");

    AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hMenu, "Menu");
    SetMenu(hwnd, hMenubar);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        AddMenus(hwnd);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1: ShowNotImplemented(); break;
        case 2: ShowNotImplemented(); break;
        case 3: ShowNotImplemented(); break;
        case 4: ShowNotImplemented(); break;
        case 5: ShowNotImplemented(); break;
        case 6: ShowNotImplemented(); break;
        case 7: PostQuitMessage(0); break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void ShowNotImplemented() {
    MessageBox(NULL, "This feature is not yet implemented in the GUI.",
               "Info", MB_OK | MB_ICONINFORMATION);
}
