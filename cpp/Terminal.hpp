#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <string>

// Encapsulates all console I/O concerns that were spread across the original
// terminal_util.c and inline conio/windows calls. Static because there is a
// single physical console; grouping keeps the responsibility in one place.
class Terminal {
public:
    // Clear the screen (ANSI) and flush.
    static void clear();

    // Set colour from a 2-char "BF" code (Background, Foreground) as used by
    // the original set_color("E1") style calls.
    static void setColor(const std::string& code);

    // Reset colour attributes to the terminal default.
    static void resetColor();

    // Block until a single key is pressed (mirrors conio getch()).
    static int getKey();

    // Busy-wait style pause used by the original animated screens (ms).
    static void delayMs(int ms);

    // Read a full line of text (trimmed of trailing newline).
    static std::string readLine();

    // Read a validated integer, re-prompting on invalid input. Empty prompt
    // means "no prompt". Never loops forever on EOF.
    static int readInt(const std::string& prompt = "");

    // Read a masked password from the console, echoing '*' per character.
    static std::string readPassword();

    // Wait for any key with the classic "Press any key to continue" message.
    static void pause(const std::string& msg = "\n\n Press any key to continue!!");

private:
    static int hexToInt(char ch);
};

#endif // TERMINAL_HPP
