# Hotel Management System

This project provides a hotel management application written in C.
Originally it was a console application. A minimal GUI prototype using
**Win32 API** is now included in `gui-win32.c`.

## Building (Console Version)
Use any C compiler on Windows. For example using GCC:

```bash
gcc hotel-management-project-with-C.c -o hotel.exe
```

## Building (GUI Prototype)
The GUI version requires the Windows SDK. Compile with a command similar to:

```bash
gcc gui-win32.c -o gui.exe -lgdi32 -mwindows
```

This creates a simple window with menu items that mirror the console menu.
The features currently display a "not implemented" message.

Further work is needed to migrate the booking, display and checkout
functions into GUI dialogs.
