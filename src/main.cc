//
// Created by Yue Xue  on 7/30/25.
//

#include "chunk.h"

int main() {
    Chunk chunk;
    auto index = chunk.add_constant(998);
    chunk.write_opcode(OpCode::LoadConstant8, 123);
    chunk.write_index(index, 123);
    chunk.write_opcode(OpCode::Return, 123);
    chunk.disassemble("test chunk");
}
