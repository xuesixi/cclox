#ifndef CCLOX_DISASSEMBLER_H
#define CCLOX_DISASSEMBLER_H

#include "chunk.h"
#include <memory>
class Disassembler {
public:

    Disassembler(std::shared_ptr<Chunk> the_chunk): chunk(the_chunk) {}

    void disassemble(const char *name);
    size_t disassemble_instruction(size_t offset);
    size_t instruction_operand_0(OpCode instruction, size_t offset);
    size_t instruction_operand_1(OpCode instruction, size_t offset);
    size_t instruction_operand_2(OpCode instruction, size_t offset);
private:
    std::shared_ptr<Chunk> chunk;
};

#endif