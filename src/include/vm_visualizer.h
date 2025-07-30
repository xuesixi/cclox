#ifndef CCLOX_VM_VISUALIZER_H
#define CCLOX_VM_VISUALIZER_H

#include "vm.h"

enum class Color {
    BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE,
    BRIGHT_BLACK, BRIGHT_RED, BRIGHT_GREEN, BRIGHT_YELLOW,
    BRIGHT_BLUE, BRIGHT_MAGENTA, BRIGHT_CYAN, BRIGHT_WHITE
};

namespace Visual {
    void show_stack(VM &vm);
    void print_with_color(const std::string &content, Color color);
}

#endif