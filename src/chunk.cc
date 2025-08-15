//
// Created by Yue Xue  on 7/30/25.
//

#include "chunk.h"

std::optional<uint8_t> Chunk::to_immediate(Value value) {
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
        if (double_equal(d, 8.0)) return 254;
        if (double_equal(d, 16.0)) return 255;
    }
    return std::nullopt;

}

void Chunk::write_opcode(Opcode opcode, int line) {
    code.push_back(static_cast<uint8_t>(opcode));
    lines.push_back(line);
}

void Chunk::write_operand(size_t operand, int line) {
    if (within<uint8_t>(operand)) {
        code.push_back(static_cast<uint8_t>(operand));
        lines.push_back(line);
    } else if (within<uint16_t>(operand)) {
        write_operand_2(operand, line);
    } else {
        implementation_error("not within uint16 limits");
    }
}

void Chunk::write_operand_2(size_t operand, int line) {
    auto [high, low] = u16_to_u8(operand);
    code.push_back(low);
    code.push_back(high);
    lines.push_back(line);
    lines.push_back(line);
}

OperandSize Chunk::add_constant(Value &&value) {
    constants.push_back(std::move(value));
    size_t index = constants.size() - 1;
    if (!within<OperandSize>(index)) {
        throw ConstantPoolOverflowError("constant pool overflow");
    } else {
        return index;
    }
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
