#include "visual.h"
#include "fmt/core.h"
#include "value.h"
#include "objects/loxstring.h"
#include <iostream>

// 为字符串前后加上引号
std::string Visual::string_wrapper(std::string &&str) {
    return "\"" + str + "\"";
}

std::string Visual::to_visual_string(Value value) {
    std::string regular_string = LoxValue::to_string(value);
    if (std::holds_alternative<LoxReference>(value) && LoxValue::to_reference<LoxString>(value)) {
        return string_wrapper(std::move(regular_string));
    }
    return regular_string;
}

void Visual::print_with_color(const std::string &content, Color color) {
    std::string ansi_code;
    switch (color) {
        case Color::BLACK: ansi_code = "\033[30m";
            break;
        case Color::RED: ansi_code = "\033[31m";
            break;
        case Color::GREEN: ansi_code = "\033[32m";
            break;
        case Color::YELLOW: ansi_code = "\033[33m";
            break;
        case Color::BLUE: ansi_code = "\033[34m";
            break;
        case Color::MAGENTA: ansi_code = "\033[35m";
            break;
        case Color::CYAN: ansi_code = "\033[36m";
            break;
        case Color::WHITE: ansi_code = "\033[37m";
            break;
        case Color::BRIGHT_BLACK: ansi_code = "\033[90m";
            break;
        case Color::BRIGHT_RED: ansi_code = "\033[91m";
            break;
        case Color::BRIGHT_GREEN: ansi_code = "\033[92m";
            break;
        case Color::BRIGHT_YELLOW: ansi_code = "\033[93m";
            break;
        case Color::BRIGHT_BLUE: ansi_code = "\033[94m";
            break;
        case Color::BRIGHT_MAGENTA: ansi_code = "\033[95m";
            break;
        case Color::BRIGHT_CYAN: ansi_code = "\033[96m";
            break;
        case Color::BRIGHT_WHITE: ansi_code = "\033[97m";
            break;
    }

    // reset color
    const std::string reset_code = "\033[0m";

    std::cout << ansi_code << content << reset_code;
}
