#ifndef COMMON_H
#define COMMON_H

namespace Flag {
    extern bool trace;
    extern bool disassembly;
    extern bool repl;
}

#define DEBUG_TRACE_EXECUTION
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
