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
        src/include/error.h
        src/include/common.h
        src/include/chunk.h
        src/chunk.cc
        src/include/value.h
        src/value.cc
        src/include/disassembler.h
        src/disassembler.cc
        src/include/vm.h
        src/vm.cc
        src/include/compiler.h
        src/compiler.cc
        src/include/scanner.h
        src/scanner.cc
        src/include/cclox_util.h
        src/cclox_util.cc
        src/object.cc
        src/include/object.h
        src/runtime.cc
        src/include/runtime.h
        src/include/objects/loxstring.h
        src/scope.cc
        src/include/scope.h
        src/include/objects/loxfunction.h
        src/include/objects/loxcaptured.h
        src/include/objects/loxclosure.h
        src/include/objects/loxclass.h
        src/include/objects/loxinstance.h
        src/include/objects/loxmethod.h
        src/native.cc
        src/include/native.h
        src/stringintern.cc
        src/include/stringintern.h
        src/include/nativeobjects/loxconcurrent.h
        src/typechecker/typeparser.cc
        src/include/typechecker/typeparser.h
        src/typechecker/expression.cc
        src/include/typechecker/expression.h
        src/include/typechecker/tokenholder.h
        src/typechecker/statement.cc
        src/include/typechecker/statement.h
        src/include/typechecker/types/class_type.h
        src/include/typechecker/types/primitive_type.h
        src/include/typechecker/types/intersection_type.h
        src/include/typechecker/types/union_type.h
        src/include/typechecker/types/tuple_type.h
        src/include/typechecker/types/array_type.h
        src/include/typechecker/types/function_type.h
        src/include/typechecker/lox_type.h
        src/include/typechecker/ast.h
        src/typechecker/ast.cc
        src/include/typechecker/statements/var_statement.h
        src/include/typechecker/statements/print_statement.h
        src/include/typechecker/statements/fun_statement.h
        src/include/typechecker/statements/expression_statement.h
)

target_include_directories(cclox PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/include")
target_link_libraries(cclox m readline fmt)

enable_testing()
add_subdirectory(src/typechecker)
add_subdirectory(test)
