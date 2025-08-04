//
// Created by Yue Xue  on 7/30/25.
//

#include "chunk.h"
#include "value.h"
#include <cstddef>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <unordered_map>

Value Chunk::read_immediate(uint8_t index) {
    if (index <= 240) {
        return static_cast<long>(index);
    }

    switch (index) {
        case 241: return 0.0;
        case 242: return 1.0;
        case 243: return 0.5;
        case 244: return 0.25;
        case 245: return 0.125;
        case 246: return 0.1;
        case 247: return 2.0;
        case 248: return 4.0;

        case 249: return 5.0;
        case 250: return 10.0;
        case 251: return 100.0;
        case 252: return 1000.0;
        case 253: return 10000.0;

        default:
            implementation_error("unknown immediate index");
            return 0L;
    }
}

uint8_t Chunk::to_immediate(Value value) {
    if (std::holds_alternative<long>(value)) {
        long l = std::get<long>(value);
        if (l <= 240) {
            return static_cast<int>(l);
        }
    } else if (std::holds_alternative<double>(value)) {
        double d = std::get<double>(value);

        if (double_equal(d, 0.0)) return 241;
        if (double_equal(d, 1.0)) return 242;
        if (double_equal(d, 0.5)) return 243;
        if (double_equal(d, 0.25)) return 244;
        if (double_equal(d, 0.125)) return 245;
        if (double_equal(d, 0.1)) return 246;
        if (double_equal(d, 2.0)) return 247;
        if (double_equal(d, 4.0)) return 248;

        if (double_equal(d, 5.0)) return 249;
        if (double_equal(d, 10.0)) return 250;
        if (double_equal(d, 100.0)) return 251;
        if (double_equal(d, 1000.0)) return 252;
        if (double_equal(d, 10000.0)) return 253;

        return 255;
    }

    return 255;
}
