#pragma once
#include <cstdint>
#include <string>
#include <iostream>

#define STYLE_BOLD                "\e[1m"
#define STYLE_ITALIC              "\e[3m"
#define STYLE_UNDERLINE           "\e[4m"
#define STYLE_STRIKETHROUGH       "\e[9m"

#define STYLE_FOREGROUND_BLACK    "\e[30m"
#define STYLE_FOREGROUND_RED      "\e[31m"
#define STYLE_FOREGROUND_GREEN    "\e[32m"
#define STYLE_FOREGROUND_YELLOW   "\e[33m"
#define STYLE_FOREGROUND_BLUE     "\e[34m"
#define STYLE_FOREGROUND_PURPLE   "\e[35m"
#define STYLE_FOREGROUND_CYAN     "\e[36m"
#define STYLE_FOREGROUND_WHITE    "\e[37m"

#define STYLE_BACKGROUND_BLACK    "\e[40m"
#define STYLE_BACKGROUND_RED      "\e[41m"
#define STYLE_BACKGROUND_GREEN    "\e[42m"
#define STYLE_BACKGROUND_YELLOW   "\e[43m"
#define STYLE_BACKGROUND_BLUE     "\e[44m"
#define STYLE_BACKGROUND_PURPLE   "\e[45m"
#define STYLE_BACKGROUND_CYAN     "\e[46m"
#define STYLE_BACKGROUND_WHITE    "\e[47m"

#define STYLE_RESET               "\e[0m"

std::string FormatToStyle(int8_t format);
std::string HandleFormattingCodes(const std::string& input);