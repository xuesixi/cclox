cmake_minimum_required(VERSION 3.5)
project(cclox)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 10.2.1  # Use latest stable version
)

FetchContent_MakeAvailable(fmt)

add_executable(cclox
        src/main.cc
        include/error.h
        include/common.h
        include/chunk.h
        src/chunk.cc
        include/value.h
        src/value.cc
        include/disassembler.h
        src/disassembler.cc
        include/vm.h
        src/vm.cc
        include/compiler.h
        src/compiler.cc
        include/scanner.h
        src/scanner.cc
        include/cclox_util.h
        src/cclox_util.cc
        src/object.cc
        include/object.h
        src/runtime.cc
        include/runtime.h
        include/objects/loxstring.h
        src/scope.cc
        include/scope.h
        include/objects/loxfunction.h
        include/objects/loxcaptured.h
        include/objects/loxclosure.h
        include/objects/loxclass.h
        include/objects/loxinstance.h
        include/objects/loxmethod.h
        src/native.cc
        include/native.h
        src/stringintern.cc
        include/stringintern.h
        include/nativeobjects/loxconcurrent.h
        src/typechecker/typeparser.cc
        include/typechecker/typeparser.h
        src/typechecker/expression.cc
        include/typechecker/expression.h
        include/typechecker/tokenholder.h
        src/typechecker/statement.cc
        include/typechecker/statement.h
        include/typechecker/types/class_type.h
        include/typechecker/types/primitive_type.h
        include/typechecker/types/intersection_type.h
        include/typechecker/types/union_type.h
        include/typechecker/types/tuple_type.h
        include/typechecker/types/array_type.h
        include/typechecker/types/function_type.h
        include/typechecker/lox_type.h
        include/typechecker/ast.h
        src/typechecker/ast.cc
        include/typechecker/statements/var_statement.h
        include/typechecker/statements/print_statement.h
        include/typechecker/statements/fun_statement.h
        include/typechecker/statements/expression_statement.h
        include/typechecker/statements/block_statement.h
        src/typechecker/statements/block_statement.cc
        src/typechecker/statements/expression_statement.cc
        src/typechecker/statements/fun_statement.cc
        src/typechecker/statements/print_statement.cc
        src/typechecker/statements/var_statement.cc
        src/typechecker/expressions/assignment_expression.cc
        include/typechecker/expressions/assignment_expression.h
        src/typechecker/expressions/or_expression.cc
        include/typechecker/expressions/or_expression.h
        src/typechecker/expressions/and_expression.cc
        include/typechecker/expressions/and_expression.h
        src/typechecker/expressions/binary_expression.cc
        include/typechecker/expressions/binary_expression.h
        src/typechecker/expressions/unary_expression.cc
        include/typechecker/expressions/unary_expression.h
        src/typechecker/expressions/call_expression.cc
        include/typechecker/expressions/call_expression.h
        src/typechecker/expressions/dot_expression.cc
        include/typechecker/expressions/dot_expression.h
        src/typechecker/expressions/primary_expression.cc
        include/typechecker/expressions/primary_expression.h
        src/typechecker/types/array_type.cc
        src/typechecker/types/class_type.cc
        src/typechecker/types/function_type.cc
        src/typechecker/types/intersection_type.cc
        src/typechecker/types/primitive_type.cc
        src/typechecker/types/tuple_type.cc
        src/typechecker/types/union_type.cc
)

target_include_directories(cclox PRIVATE "include")
target_link_libraries(cclox m readline fmt)

enable_testing()
add_subdirectory(src/typechecker)
add_subdirectory(test)
