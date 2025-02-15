#pragma once
#include <cstdint>
#include <string>
#include <iostream>

#define STYLE_START "\e["
#define STYLE_END "m"

#define STYLE_BOLD 1
#define STYLE_ITALIC 3
#define STYLE_UNDERLINE 4
#define STYLE_STRIKETHROUGH 9

#define STYLE_FOREGROUND_BLACK    30
#define STYLE_FOREGROUND_RED      31
#define STYLE_FOREGROUND_GREEN    32
#define STYLE_FOREGROUND_YELLOW   33
#define STYLE_FOREGROUND_BLUE     34
#define STYLE_FOREGROUND_PURPLE   35
#define STYLE_FOREGROUND_CYAN     36
#define STYLE_FOREGROUND_WHITE    37

#define STYLE_BACKGROUND_BLACK    40
#define STYLE_BACKGROUND_RED      41
#define STYLE_BACKGROUND_GREEN    42
#define STYLE_BACKGROUND_YELLOW   43
#define STYLE_BACKGROUND_BLUE     44
#define STYLE_BACKGROUND_PURPLE   45
#define STYLE_BACKGROUND_CYAN     46
#define STYLE_BACKGROUND_WHITE    47

#define STYLE_RESET "0"

void SetStyle(int8_t fg = 0, int8_t bg = 0, int8_t styling = 0);
void ResetStyle();