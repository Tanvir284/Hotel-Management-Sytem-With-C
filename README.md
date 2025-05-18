# Hotel Management System (C)

This project is a console-based hotel management application written in C. It stores guest details in `Record.txt` and provides a simple text interface for booking and managing hotel rooms.

## Features

- **Room Booking** – create new reservations, specifying guest details and stay information.
- **Display Room Record** – view existing bookings saved in `Record.txt`.
- **Rooms Status** – list vacant and occupied rooms.
- **Room Features** – check available amenities based on room number.
- **Modify Room Record** – update guest information or services.
- **Check-Out** – finalize a stay and remove the room record.

## Build Instructions

1. Ensure you have a C compiler installed. The project was originally written for Windows and uses `conio.h` and `windows.h`. Using a compiler like MinGW is recommended on Windows.
2. Open a terminal in the repository root.
3. Compile the source file:
   ```sh
   gcc hotel-management-project-with-C.c -o hotel.exe
   ```

   On non‑Windows platforms you may need to remove or replace platform-specific headers.

## Running the Program

After a successful build, run the executable from the terminal:

```sh
./hotel.exe   # or `hotel.exe` on Windows
```

The application will create and update `Record.txt` in the current directory to store guest information. A temporary file `Temp.txt` is also used during modifications.

Follow the on-screen menu to book rooms, view records, and manage hotel operations.
