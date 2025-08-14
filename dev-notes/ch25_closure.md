# 闭包

闭包的意思是，如果我们在函数A内部定义函数B，那么B可以访问A内部的**本地变量**，即使A允许结束后，B仍然可以访问它们。

我将分多个部份来解释闭包的实现：
* 解析
* 捕获
* 访问
* 逃逸

## 解析 upvalue

在解析变量的时，如果该变量是外部函数的本地变量，那么我们需要将其解析为一个Upvalue。
Upvalue是解析时的行为，因此是Scope的成员之一。我们用一个vector来储存。
```c++
class Scope {
    std::vector<Local> locals;
    std::vector<Upvalue> upvalues; // 本层级捕获的外层变量
    int depth = 0;
    FunctionType function_type;
    std::shared_ptr<LoxFunction> function;
    std::shared_ptr<Scope> outer;
};

class Upvalue {
    bool is_local; // 这个值是来自外层的本地变量，还是外层的upvalue
    uint8_t index;
    std::string name;
};    
```

值得注意的是，任何函数都只能捕获来自**直接外层**的本地变量或者Upvalue。假如我们有下面这样的例子
```lox
fun A() {
    var num = 10;
    fun B() {
        fun C() {
            print num;
        }
    }
}
```
这里，C的`print num`使用了A中的变量num。它的解析流程是这样的：
* C检查自己的locals，查看num是否是自己的本地变量。发现不是
* C检查自己的upvalues，检查num是否已经被捕获了，发现没有
* 检查B的locals，查看num是否是B的本地变量，发现不是
* 对B进行upvalue解析。B捕获了A中的本地变量num，将其添加到B的upvalues中，由于是A的本地变量，is_local为true，index是num在A中的本地索引（locals的索引）
* C从B的upvalues中找到了num，将其添加到自己的upvalues中。由于是B的upvalue，is_local为false。index是num在B的upvalues中的索引


这里num成功被解析为了一个upvalue，因此num变量的访问会生成LoadUpvalue/SetUpvalue指令。

当一个函数解析完毕后，检查对应的scope中的upvalues，生成一个MakeClosure指令，
其格式：`MakeClosure, num_upvalues [, is_local, index...] `

## 捕获 capture

LoxFunction最主要的成员就是Chunk。Chunk内部有code、constants、identifiers等运行时的最重要的数据。
而LoxClosure是对LoxFunction的再一层的包装：多了一个vector来储存被捕获的值。

```c++
class LoxClosure: public LoxObject {
    std::shared_ptr<LoxFunction> function;
    std::vector<std::shared_ptr<Captured>> captureds;
};

class Captured {
public:

private:
    using OpenRef = std::pair<std::vector<Value> *, size_t>; // <stack, index>
    std::variant<OpenRef, Value> data;
}
```

Captured有两种状态：目标值处于栈上，或者储存在自身内部。这里用一个variant来表示。`value`函数可以根据variant
的具体情况来提供合适的访问。
```c++
Value &Captured::value() {
    if (std::holds_alternative<OpenRef>(data)) {
        auto [stack, index] = std::get<OpenRef>(data);
        return stack->at(index);
    } else {
        return std::get<Value>(data);
    }
}
```

在运行时，我们从常数池中加载LoxFunction对象，下一个指令肯定就是MakeClosure。我们根据其后的num_upvalue, 
is_local，index属性， 对外部的变量进行捕获。仍然以上面A、B、C嵌套的三层函数定义为例子。
* 当加载A的时候，A没有任何捕获值，因此MakeClosure什么事都不做。
* 当调用A的时候，B被加载。MakeClosure的参数告诉虚拟机：B会捕获外层函数中的一个本地索引为1的本地变量（num）。此时，
这个被捕获值肯定处在栈上（因为A正在被调用），我们用它的信息生成一个Captured对象，添加到B的captureds列表中。
此外，我们还储存了一个全局的`open_captured`列表，用来记录那些还存在于栈上的被捕获值（之后进行逃逸的时候会用到）
* 当B被调用时，C被加载。MakeClosure的参数告诉虚拟机：C回捕获外层函数中的一个captureds中索引为0的捕获值（num）。
由于这个值最初是由B从A中捕获的，此时，C只需要将其复制到自己的captured列表中即可。

值得注意的是，captured列表的定义是`std::vector<std::shared_ptr<Captured>> captureds`.
这里储存的是指针，因为我们希望B和C访问的是同一个东西（而不是互相独立的复制体）

## 访问

LoadCaptured/SetCaptured的机制非常简单，从closure的captured中获取对应的捕获值进行访问。

```c++
case OpCode::LoadCaptured: {
    auto index = read_operand_1();
    Value v = closure()->captureds_.at(index)->value();
    push(v);
    break;
}
case OpCode::SetCaptured: {
    auto index = read_operand_1();
    closure()->captureds_.at(index)->value() = stack.back();
    break;
}
```

## 逃逸 escape

当捕获值被创建的时候，被捕获的对象肯定处在栈上，此时我们储存栈上的索引就可以提供访问。但当该函数的栈帧被弹出后，
这个本地变量就不再存在。此时，我们就需要将那个变量的值直接转移到Captured内部（我称之为逃逸escape，原书称之为close）。
如此一来，各个函数存有Captured的指针，就可以持续进行访问。

从上面这段话中，可以看出，进行逃逸的最佳时机就是函数栈帧被弹出时，也就是return的时机。
因此，在return指令被执行时，我们额外调用`escape_above(fp)`，将该栈帧中所有的被捕获值进行逃逸。
```c++
/**
 * 将位处于index及以上的所有处在栈上（open）的捕获值进行逃逸
 * @param index 该栈索引以及其上的所有捕获值将会被逃逸
 */
void escape_above(size_t index) {
    while (open_captured.empty() == false) {
        auto curr = open_captured.back();
        if (curr->index() >= index) {
            open_captured.pop_back();
            curr->escape();
        } else {
            break;
        }
    }
}

/**
 * 将捕获的值从栈上转移到该对象内部。只能调用一次。
 */
void Captured::escape() {
    auto [stack, index] = std::get<OpenRef>(data);
    data = stack->at(index);
}
```
在调用escape之后，被捕获值直接储存在Captured内部，各个函数可以通过指针访问同一个值。