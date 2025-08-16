#ifndef CCLOX_VISUAL_H
#define CCLOX_VISUAL_H

// #include "vm.h"
#include <string>
#include "value.h"

enum class Color {
    BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE,
    BRIGHT_BLACK, BRIGHT_RED, BRIGHT_GREEN, BRIGHT_YELLOW,
    BRIGHT_BLUE, BRIGHT_MAGENTA, BRIGHT_CYAN, BRIGHT_WHITE
};

namespace Visual {
    // 为字符串前后加上引号
    std::string string_wrapper(std::string &&input);

    void print_with_color(const std::string &content, Color color);

    /**
     * 会根据值的具体类型来改变展示时的格式。例如，LoxString会具有前后引号
     */
    std::string to_visual_string(Value value);
}

#endif
