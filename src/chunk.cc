//
// Created by Yue Xue  on 7/30/25.
//

#include "chunk.h"
#include <unordered_map>

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

OperandSize Chunk::add_identifier(const std::string &str) {

    auto found = std::find(identifiers.begin(), identifiers.end(), str);
    if (found != identifiers.end()) {
        // 如果存在，则返回键
        size_t index = std::distance(identifiers.begin(), found);
        return index;
    }

    // 如果不存在，则新增
    size_t key = identifiers.size();
    identifiers.push_back(str);
    if (!within<uint16_t>(key)) {
        throw IdentifierPoolOverFlowError("identifier pool overflow");
    }
    return key;
}
