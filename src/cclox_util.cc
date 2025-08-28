#include "cclox_util.h"
#include "error.h"
#include <fmt/core.h>

#include "common.h"

const auto program_start = std::chrono::high_resolution_clock::now();

std::string read_file(const std::string &path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        throw FileOpenFailureError(fmt::format("the file: {} cannot be opened", path));
    }
    return std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}

std::string timestamp_str() {
    auto time = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(time);

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&tt), "%Y%m%d_%H%M%S");
    return oss.str();
}

std::string nanos_str() {
    auto now = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now - program_start
    ).count();
    return fmt::format("{}", ns);
}

void print_to(std::ostream &out ,const std::string &content, Color color) {
     std::string ansi_code;
    if (Flag::print_color == false) {
        out << content;
        return;
    }
    switch (color) {
        case Color::None:
            out << content;
            return;
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

    out << ansi_code << content << reset_code;
}
