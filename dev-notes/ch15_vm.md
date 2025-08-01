第15章，虚拟机。
## VM
目前`vm`的结构大约是这样的：
```c++
class VM {
    std::vector<Value> stack; // 栈。暂时用vector来表示
    std::shared_ptr<Chunk> chunk; // 目前的虚拟机暂时绑定一个chunk，以后可能会修改
    size_t pc; // Program Counter，chunk中下一个要执行的字节码的索引。
};
```
由于我使用`vector`来表示栈，因此相比起原书的实现省了一些字段。

如上面注释所说，目前的虚拟机绑定的是一个`chunk`，运行的规则很简单：从当前`chunk`中读取下一个指令，根据指令的`opcode`来执行不同的逻辑
```c++

// InterpeterResult 代表的是运行的结果, 这里可以暂时不管
InterpreterResult VM::interpret(std::shared_ptr<Chunk> chunk) {
    while (true) {
        OpCode instruction = read_opcode(); // 读取chunk中下一个指令
        switch (instruction) {
            // 对不同的opcode执行不同的具体动作
        }
    }
}
```

虚拟机中比较重要的几个方法
```c++
OpCode VM::read_opcode(); // chunk中读取下一个指令（并移动pc）
Value VM::read_constant_1(); // 读取下一个操作数作为索引，从常数池中读取对应的值。这个函数有双字节版本（uint16）版本的
void VM::push(Value value);
Value VM::pop();
```
执行加法计算的逻辑是这样的：
```c++
case OpCode::Add: {
    Value b = pop();
    Value a = pop();
    Value result = a + b; // 我们为Vkjue重载了各种运算符
    push(result); 
    break;
}
```

## Value

```c++
using Value = std::variant<long, double, bool>;
```
目前的`Value`还非常简单，只包括三种值：`long`，`double`，`bool`。之后会增加引用类型。`variant`和c中的tagged union非常相似，但它会自动帮我们进行类型管理。建议去看一下此时的`Value`的运算符重载。