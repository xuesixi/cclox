#ifndef COMMON_H
#define COMMON_H

namespace Flag {
    extern bool trace;// 是否一步步打印虚拟机的运行过程
    extern bool disassembly;// 是否在编译后对字节码进行反汇编输出
    extern bool repl;// 是否是repl模式
    extern bool show_heap; // 是否在分配和释放堆内存的时候进行日志输出
    extern bool print_color; // 打印时是否带颜色
    extern bool not_run; // 是否运行字节码（如果设置为false，仍然可以通过其他选项进行反汇编等非运行操作）
}

namespace Configuration {
    extern int frame_max; // 栈帧的数量上限
}

#define DEBUG_IMPLEMENTATION_CHECK

#define STRINGIFY_DETAIL(x) #x
#define STRINGIFY(x) STRINGIFY_DETAIL(x)

#ifdef DEBUG_IMPLEMENTATION_CHECK
    #define DEBUG_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            implementation_error(__FILE__ ":" STRINGIFY(__LINE__) " - " message);  \
        } \
    } while (0)
#else
    #define DEBUG_ASSERT(condition, message)
#endif
#endif // COMMON_H
