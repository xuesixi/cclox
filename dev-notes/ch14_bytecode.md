这是第一份笔记。内容是关于书中的第14章：Chunks of Bytecode。本文的代码是基于tag:note14的，后续可能发生巨大修改。
2025/08/01

## chunk
chunk是一个编译的单元，截止目前，大致的结构如下
```c++
class Chunk {
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<int> lines;
};
```
原书是用c语言编写的，因此耗费了不少功夫来编写动态扩容的容器，但我们用的是c++，直接用`vector`即可，又快又安全。
* 这里的`code`就是“字节码”，它们代表着指令。每一个`uint8`元素要么是一个指令的opcode，要么是指令的操作数（operand）。下面是一些示例的opcode
    ```c++
    enum class OpCode: uint8_t {
        Return,
        LoadConstant8,
        LoadConstant16,
        Negate,
        Add,
        Subtract,
        Multipy,
        Divide,
    };
    ```
* `constants`字段则是常数池。
* 字节码中的每一个字节都有一个对应的行号， 储存在`lines`字段中（当然，这么做浪费了很多的资源，一个`uint8`对应一个`int`的数据，完全可以优化）。

上面出现了`Value`类型，我们用它来表示lox中的值。clox的实现中用的是经典的tagged union模式，出于练习c++的目的，我特意换成了c++17标准中的`variant`，目前有三种变体，分别代表整数、小数、布尔值，以后还会增加指针。
```c++
using Value = std::variant<long, double, bool>;
```
chunk类提供了一些方法来向其中写入数据，主要是
```c++
void write_opcode(OpCode opcode, int line); // 向code中写入一个opcode
size_t add_constant(Value value); // 向常数池中添加一个新的常数，并返回其索引
void write_operand(size_t index, int line); // 向code中写入一个索引。会自动处理uint8和uint16
```
原书中`uint8`的索引，也就是说，常数池中最多只能存放256个常数，我觉得太少了，因此允许`uint16`的索引。`write_operand()`函数会自动根据传入索引的大小，向`code`中写入一个字节（`uint8`）或者两个字节（`uint16`）来代表索引。如果超出`uint16`则出现编译错误（不太可能发生）。实际上我们还写了一些辅助函数来进行`uint8`和`uint16`之间的互相转化，具体细节可以自己去看。

## disassembler

意思是反汇编器。上面生成的`chunk`类内含难以阅读的数据，为了方便debug和理解虚拟机，我们编写了一个反汇编器来将字节码的信息格式化输出为字符串。

原书中没有专门的`dissassembler`类和文件，而是写在chunk文件中，但我觉得分离开比较好。

目前，一个`disassembler`会绑定一个`chunk`。它会用`disassembe()`方法来反汇编整个`chunk`，该函数内部其实是一个循环，它维护了一个`offset`变量，代表`code`中的索引。然后循环调用`disassemble_instruction()`方法。
```c++
void Disassembler::disassemble(const char *name) {
    // offset代表当前指令在chunk的字节码中的索引。有的指令有额外的参数（占用超过一个字节），
    // 因此下一个指令的索引将由disassemble_instruction函数判断
    for (size_t offset = 0; offset < chunk->code.size();) {
        offset = disassemble_instruction(offset);
    }
}
```
`disassemble_instruction(offset)`会反汇编起始于`code[offset]`处的指令。由于不同指令占用的字节数不同，所以该方法内部会通过opcode来判断其适用的具体反汇编逻辑，并返回下一个指令的起始索引作为`offset`。

下面是两个实例
```c++
// 对于无操作数的指令，disassemble_instruction()实际上会调用该函数
size_t Disassembler::instruction_operand_0(OpCode instruction, size_t offset) {
    cout << fmt::format("{:12}\n", opcode_names.at(instruction));
    return offset + 1; // 由于没有操作数，下一个指令的位置就是offset+1
}

// 对于只有一个常数池索引作为操作数的的指令，disassemble_instruction()实际上会调用该函数
size_t Disassembler::instruction_constant_operand_1(OpCode instruction, size_t offset) {
    size_t index = chunk->code.at(offset + 1);
    Value value = chunk->constants.at(index);
    std::string value_str = LoxValue::to_string(value);
    cout << fmt::format("{:12} {}index {}, value {}\n", opcode_names.at(instruction), spaces_4, index, value_str);
    return offset + 2; // 由于有1个字节的操作数，下一个指令的位置是offset+2
}
```
`disassembler`这个东西完全不影响虚拟机自身的运行，主要就是输出一个相对美观的字节码解析结果。