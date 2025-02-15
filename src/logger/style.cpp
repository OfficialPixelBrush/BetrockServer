#include "style.h"

void SetStyle(int8_t fg, int8_t bg, int8_t styling) {
    std::string style = std::string(STYLE_START) + STYLE_RESET;
    if (fg == 0 && bg == 0 && styling == 0) {
        std::cout << style << ";" << STYLE_END;
    }

}

void ResetStyle() {
    std::cout << STYLE_START << STYLE_RESET << STYLE_END;
}