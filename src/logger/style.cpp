#include "style.h"

std::string FormatToStyle(int8_t format) {
    switch(format) {
        // Colors
        case '0': 
            return "\e[30m";
        case '1':
            return "\e[34m";
        case '2':
            return "\e[32m";
        case '3':
            return "\e[36m";
        case '4':
            return "\e[31m";
        case '5':
            return "\e[35m";
        case '6':
            return "\e[33m";
        case '7':
            return "\e[37m";
        case '8':
            return "\e[90m";
        case '9':
            return "\e[94m";
        case 'a':
            return "\e[92m";
        case 'b':
            return "\e[96m";
        case 'c':
            return "\e[91m";
        case 'd':
            return "\e[95m";
        case 'e':
            return "\e[93m";
        case 'f':
            return "\e[97m";
        // Bold
        case 'l':
            return "\e[1m";
        // Strikethrough
        case 'm':
            return "\e[9m";
        // Underlined
        case 'n':
            return "\e[4m";
        // Italic
        case 'o':
            return "\e[3m";
        // Obfuscated
        case 'k':
            return "\e[37;105m";
        // Reset
        default:
            return STYLE_RESET;
    }
}

std::string HandleFormattingCodes(const std::string& input) {
    std::string output;
    for (size_t i = 0; i < input.size(); ++i) {
        // Check if first character is §
        if (input[i] == '\xC2' && input[i + 1] == '\xA7' && i + 2 < input.size() ) {
            output += FormatToStyle(input[i+2]); // Replace § and the next character
            ++i; // Skip the next character
            ++i; // Skip the next character
        } else {
            output += input[i];
        }
    }
    return output + std::string(STYLE_RESET);
}