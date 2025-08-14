#ifndef CCLOX_DISASSEMBLER_H
#define CCLOX_DISASSEMBLER_H

#include "chunk.h"

/**
 * 反汇编器。非必要组件。
 * 负责将给定的chunk以文本的形式进行格式化打印输出，主要是直观地展示字节码的内容，方便debug和理解字节码
 */
class Disassembler {
public:

    /**
     * 反汇编整个chunk
     * @param name
     */
    void disassemble(const std::string &name);

    /**
     * 格式化输出chunk.code[offset]所代表的指令，并返回下一个指令的索引。
     * 该函数会根据具体指令的类型，调用其他函数
     */
    size_t disassemble_instruction(size_t offset);

    /**
     * 单字节指令，没有额外的操作数
     */
    size_t instruction_operand_0(OpCode instruction, size_t offset);

    /**
     * 有一个额外的操作数代表常数池索引
     */
    size_t instruction_constant_operand_1(OpCode instruction, size_t offset);

    /**
     * 有一个用于标识本地变量索引的操作数
     */
    size_t instruction_local(OpCode instruction, size_t offset);

    /**
     * 通用的单操作数指令
     */
    size_t instruction_general(OpCode instruction, size_t offset);

    size_t instruction_jump(OpCode instruction, size_t offset);

    size_t instruction_jump_back(OpCode instruction, size_t offset);

    /**
     * 有两个额外的操作数代表常数池索引
     */
    size_t instruction_constant_operand_2(OpCode instruction, size_t offset);

    size_t instruction_identifier_operand_2(OpCode instruction, size_t offset);

    size_t instruction_load_immediate(OpCode instruction, size_t offset);

    size_t instruction_make_closure(OpCode instruction, size_t offset);

    void set_chunk(const Chunk *the_chunk) {
        this->chunk = the_chunk;
    }

private:
    const Chunk *chunk = nullptr;
};

#endif
