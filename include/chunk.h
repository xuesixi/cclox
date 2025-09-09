//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_CHUNK_H
#define CCLOX_CHUNK_H

#include "common.h"
#include "value.h"
#include "cclox_util.h"
#include "error.h"

class Disassembler;
class LoxClass;
class LoxClosure;

enum class Opcode: uint8_t {
    Return, // ; 弹出当前栈帧，将原本栈顶的值作为返回值，置于原本的fp的位置（也就是新的栈顶）
    LoadConstant, // index: operand1 ；将当前栈帧的function的constants[index]的值置入栈顶
    LoadConstant2, // index: operand2 ；将当前栈帧的function的constants[index]的值置入栈顶
    LoadImmediate, // index: operand1 ；将index所代表的立即数置入栈顶
    Negate, // ；弹出栈顶的值，取其相反数，置入栈顶
    Add, // ；弹出栈顶的两个值 a, b，将 a + b 的结果置入栈顶。如果两者都是字符串，可以进行字符串拼接
    Subtract, // ；弹出栈顶的两个值 a, b，将 a - b 的结果置入栈顶
    Multipy, // ；弹出栈顶的两个值 a, b，将 a * b 的结果置入栈顶
    Divide, // ；弹出栈顶的两个值 a, b，将 a / b 的结果置入栈顶
    Power, // ；弹出栈顶的两个值 a, b，将 a ** b 的结果置入栈顶
    LoadNil, // ；将nil置入栈顶
    LoadTrue, // ； 将true置入栈顶
    LoadFalse, // ；将false置入栈顶
    LoadEmptyString, // ；将""置入栈顶
    Not, // ；弹出栈顶的值，取其逻辑not，置入栈顶
    Greater, // ；弹出栈顶的两个值 a, b，将 a < b 的结果置入栈顶
    Less, // ；弹出栈顶的两个值 a, b，将 a < b 的结果置入栈顶
    Equal, // ；弹出栈顶的两个值 a, b，将 a == b 的结果置入栈顶
    Print, // ；弹出并打印栈顶的值，自带换行符。如果设置了Flag::print_color，则打印绿色
    Pop, // ； 弹出栈顶的值
    ClearN, // n: operand1 ；弹出栈顶的n个值，并进行捕获值逃逸
    DefineGlobal, // todo 已弃用。 string_id: operand2 ；弹出栈顶的值，创建一个名为string_id所对应的标识符的全局变量，将其值设置为刚才弹出的那个值
    LoadGlobal, // index: operand2 ；将 index 对应的全局变量置入栈顶
    SetGlobal, // todo 已弃用. string_id: operand2 ； 将名为string_id所对应的标识符的全局变量设置为栈顶的值
    LoadLocal, // index: operand1 ；将帧栈中本地索引为index的那个本地变量的值置入栈顶
    SetLocal, // index: operand1 ； 将栈帧中本地索引为index的那个本地变量设置为栈顶的值
    SetCaptured, // index: operand1 ；将当前栈帧closure的captured[index]的值设置为当前栈顶的值
    LoadCaptured, // index: operand1 ；将当前栈帧的closure的captureds[index]的值置入栈顶
    Jump, // distance: operand2 ； pc += distance
    JumpIfPopFalse, // distance: operand2 ；弹出栈顶的值，如果其为false，则pc += distance
    JumpIfFalse, // distance: operand2 ；如果栈顶的值为false，则pc += distance
    JumpIfTrue, // distance: operand2 ; 如果栈顶的值为true, 则pc += distance
    JumpBack, // distance: operand2 ；pc -= distance
    Call, // arg_count: operand1 ；以栈顶的arg_count个值作为参数，以再前的一个值为函数，生成新的栈帧
    MakeClosure, // captured_count: operand1, [is_local: operand1, index: operand1]... ； captured_count标识后面有多少个捕获值。进行捕获
    Recur, // arg_count: operand1 ； 将栈顶的arg_count个值移动到当前栈帧的前arg_count个本地参数的位置，弹出此后的其他值。重置pc为0
    StringConcat, // str_count: operand1 ； 弹出栈顶的str_count个值，将它们合成一个字符串，置于栈顶。
    MakeClass, // string_id: operand2, num_field: operand1, num_method: operand1 ; 此时栈顶的num_method个值都是该类的method，将它们全部弹出，生成一个class，置于栈顶
    LoadField, // index: operand1 ；以stack[fp]为instance，将其索引为index的字段置入栈顶
    SetField, // index: operand1 ；以stack[fp]为instance，设置instance的索引为index的字段
    MethodBind, // string_id: operand2 ； 此时，栈顶是方法的接受者。在其类中查找对应的方法closure。将获取到的closure绑定到接受者上，弹出接受者，置入method
    MethodInvoke, // string_id: operand2, arg_count: operand1； 此时，栈顶是方法的接受者, 在其类中查找对应的方法closure。用查询到的closure创建新的栈帧。
    As, // index: operand2 ；如果constants[index]的所代表的类型接受栈顶的表达式，则无事发生。否则抛出异常
    Is, // index: operand2 ；弹出栈顶的值，如果constants[index]的所代表的类型接受原本栈顶的值，置入 true，否则置入 false
};

using OperandSize = uint16_t;

class Chunk {
public:
    friend class Disassembler;
    friend class LoxFunction;

    struct MethodCache {
        // 弱指针以避免本该被回收的对象因为缓存而不被回收
        std::weak_ptr<LoxClass> cached_class;
        std::weak_ptr<LoxClosure> cached_closure;
    };

    size_t code_size() {
        return code.size();
    }

    uint8_t &code_at(size_t index) {
        return code.at(index);
    }

    Value constant_at(size_t index) {
        return constants.at(index);
    }

    /**
     * 向code中写入一个新的指令，并记录其所在的行数
     */
    void write_opcode(Opcode opcode, int line);

    /**
     * 向code中写入operand所代表的字节。如果在uint8范围内，写入一个字节，如果超出此范围，但处在uint16范围内，写入两个字节。
     * @pre operand 处在uint16范围内
     */
    void write_operand(size_t operand, int line);

    /**
     * 向code中写入operand。只会写入一个字节。如果超出 u8 范围，实现错误
     * @pre operand 在uint8范围内
     */
    void write_operand_1(size_t operand, int line);

    /**
     * 向code中写入operand。无论operand的值是否超出uint8的范围，都会写入两个字节。
     * @pre operand 在uint16范围内
     */
    void write_operand_2(size_t operand, int line);

    /**
     * 向常数池中增加一个值，并返回其索引。如果索引在uint16范围内，返回之。
     * @throws ConstantPoolOverflowError 如果常数池的元素数量超出uint16
     */
    OperandSize add_constant(Value &&value);

    /**
     * 向常数池中增加一个值，并返回其索引。如果索引在uint16范围内，返回之。
     * @throws ConstantPoolOverflowError 如果常数池的元素数量超出uint16
     */
    OperandSize add_constant(const Value &value);

    /**
     * 对于一个Value，如果它属于立即数，返回其立即数索引，否则返回nullopt
     */
    static std::optional<uint8_t> to_immediate(Value value);

    /**
     * 给定一个立即数索引，返回其对应的Value
     */
    static inline Value read_immediate(uint8_t index) {
        if (index <= 240) {
            return static_cast<int64_t>(index);
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
            case 254: return 8.0;
            case 255: return 16.0;

            default:
                ASSERT_UNREACHABLE();
        }
    }

    int get_line_num(size_t offset) {
        return lines.at(offset);
    }

    /**
     * 尝试将chunk的各个vector的capacity缩减为size。然后估测本chunk的容器的所占用的内存。但不包括chunk自身的内存。
     * @return 估算的内存占用
     */
    size_t estimate_memory_size() {
        code.shrink_to_fit();
        constants.shrink_to_fit();
        lines.shrink_to_fit();
        const size_t sum = sizeof(uint8_t) * code.capacity() + sizeof(Value) * constants.capacity()
        + sizeof(int) * lines.capacity() ;
        return sum;
    }

private:

    // 字节码
    std::vector<uint8_t> code;
    // 常数池
    std::vector<Value> constants;
    // 与字节码一一对应的行数记录
    std::vector<int> lines;
};

#endif //CCLOX_CHUNK_H
