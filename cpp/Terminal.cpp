#include "Terminal.hpp"

#include <cctype>
#include <cstdio>
#include <iostream>
#include <string>

#if defined(_WIN32)
#include <conio.h>
#include <windows.h>
#else
#include <chrono>
#include <thread>
#endif

int Terminal::hexToInt(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    if (ch >= 'A' && ch <= 'F') return 10 + ch - 'A';
    return 0;
}

void Terminal::clear() {
    std::printf("\033[2J\033[H");
    std::fflush(stdout);
}

void Terminal::setColor(const std::string& code) {
    if (code.size() < 2) return;
    int bg = hexToInt(code[0]);
    int fg = hexToInt(code[1]);
    static const int fgColors[16] = {30, 34, 32, 36, 31, 35, 33, 37,
                                     90, 94, 92, 96, 91, 95, 93, 97};
    static const int bgColors[16] = {40,  44,  42,  46,  41,  45,  43,  47,
                                     100, 104, 102, 106, 101, 105, 103, 107};
    std::printf("\033[%d;%dm", fgColors[fg], bgColors[bg]);
    std::fflush(stdout);
}

void Terminal::resetColor() {
    std::printf("\033[0m");
    std::fflush(stdout);
}

int Terminal::getKey() {
#if defined(_WIN32)
    return _getch();
#else
    return std::getchar();
#endif
}

void Terminal::delayMs(int ms) {
#if defined(_WIN32)
    Sleep(static_cast<DWORD>(ms));
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}

std::string Terminal::readLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

int Terminal::readInt(const std::string& prompt) {
    std::string line;
    while (true) {
        if (!prompt.empty()) std::cout << prompt;
        if (!std::getline(std::cin, line)) {
            // On EOF, return 0 rather than spinning forever.
            std::cin.clear();
            return 0;
        }
        try {
            size_t idx = 0;
            int value = std::stoi(line, &idx);
            return value;
        } catch (const std::exception&) {
            std::cout << " Invalid input. Please enter a number.\n";
        }
    }
}

std::string Terminal::readPassword() {
    std::string pword;
#if defined(_WIN32)
    int c;
    while ((c = _getch()) != '\r' && c != '\n') {
        if (c == '\b') { // backspace
            if (!pword.empty()) {
                pword.pop_back();
                std::printf("\b \b");
            }
            continue;
        }
        pword.push_back(static_cast<char>(c));
        std::printf("*");
    }
    std::printf("\n");
#else
    std::getline(std::cin, pword);
#endif
    return pword;
}

void Terminal::pause(const std::string& msg) {
    std::cout << msg;
    getKey();
}
