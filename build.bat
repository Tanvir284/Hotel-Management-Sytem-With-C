@echo off
setlocal
cd /d "%~dp0"

set CXX=g++
set CXXFLAGS=-std=c++17 -I cpp -Wall -Wextra -O2
set CORE=cpp/Booking.cpp cpp/BookingRepository.cpp cpp/Auth.cpp cpp/HotelService.cpp

echo Building console app...
%CXX% %CXXFLAGS% cpp/main.cpp cpp/HotelApp.cpp cpp/HotelApp_features.cpp cpp/HotelApp_modify.cpp %CORE% cpp/Terminal.cpp -o hotel-cpp.exe
if errorlevel 1 exit /b 1

echo Building tests...
%CXX% %CXXFLAGS% cpp/tests.cpp cpp/Booking.cpp cpp/BookingRepository.cpp -o test-hotel.exe
if errorlevel 1 exit /b 1

echo Building GUI...
%CXX% %CXXFLAGS% gui/HotelGui.cpp %CORE% cpp/Terminal.cpp -o hotel-gui.exe -mwindows -lcomctl32 -lgdi32 -luser32
if errorlevel 1 exit /b 1

echo.
echo Build complete:
echo   hotel-cpp.exe  - console application
echo   hotel-gui.exe  - modern desktop GUI
echo   test-hotel.exe - automated tests
endlocal
