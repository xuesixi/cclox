//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_ERROR_H
#define CCLOX_ERROR_H

#include "common.h"
#include <iostream>
#include <stdexcept>

using std::runtime_error;

#define declare_error(error, super) \
class error: public super {\
public:\
    using super::super;\
};\

/**
 * 调用该函数来表示某处出现了预想之外的情况（实现错误）。例如，本不应该到达的语句被运行了。该函数会输出错误信息，然后abort
 */
[[noreturn]] inline void IMPL_ERROR(const std::string &message) {
    std::cerr << fmt::format("there is an implementation error at file {}:{}: {}", __FILE__, __LINE__, message) << std::endl;
    std::abort();
}

[[noreturn]] inline void NOT_IMPLEMENTED() {
    std::cerr << fmt::format("not implemented at file {}:{}", __FILE__, __LINE__) << std::endl;
    std::abort();
}

[[noreturn]] inline void ASSERT_UNREACHABLE() {
    IMPL_ERROR("reach a unreachable code");
}

/**
 * 在scan_token中可能会被抛出。只在interpret()中被捕获。
 */
declare_error(ScannerError, runtime_error);

/**
 * lox语言内部的运行错误
 */
declare_error(LoxError, runtime_error);

/**
 * repl模式下的编译器consume失败会抛出该异常，允许用户继续输入
 */
declare_error(ConsumePending, runtime_error);

/**
 * 其他解释器级别的错误
 */
declare_error(InterpreterError, runtime_error);

/**
 * 编译错误
 */
declare_error(CompilerError, runtime_error);

declare_error(LoxTypeError, LoxError);
declare_error(LoxNameError, LoxError);
declare_error(LoxArgError, LoxError);
declare_error(LoxStackOverflowError, LoxError);

declare_error(FileOpenFailureError, InterpreterError);

declare_error(ConstantPoolOverflowError, CompilerError);
declare_error(StringInternOverflowError, CompilerError);
declare_error(DuplicateNameVariableError, CompilerError);
declare_error(UsingUninitializedLocalError, CompilerError);
declare_error(Uint8OperandOverflowError, CompilerError);
declare_error(JumpDistanceOverflowError, CompilerError);
declare_error(FmtStringUnbalancedError, CompilerError);

declare_error(ExpectedTokenNotFoundError, CompilerError);
declare_error(EmptyTupleError, CompilerError);
declare_error(MismatchedTypeError, CompilerError);
declare_error(ClassNotFoundError, CompilerError);
declare_error(ClassMemberNotFoundError, CompilerError);

/**
 * var、fun、class, method, field 出现在不合适的位置
 */
declare_error(DefinitionPositionError, CompilerError);

declare_error(InvalidAssignmentTargetError, CompilerError);
declare_error(NoInitializationError, CompilerError);
declare_error(VariableNotFoundError, CompilerError);


#undef declare_error

#endif //CCLOX_ERROR_H
