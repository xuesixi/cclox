#include "cclox_util.h"
#include "error.h"
#include <fmt/core.h>

std::string read_file(const std::string &path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        throw FileOpenFailureError(fmt::format("the file: {} cannot be opened", path));
    }
    return std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}

void print_with_color(const std::string &content, Color color) {
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
