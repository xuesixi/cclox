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


# 原版的cclox，不包括任何typechecker的源代码
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
)

target_include_directories(cclox PRIVATE "include")
target_link_libraries(cclox m readline fmt)

enable_testing()
add_subdirectory(test)
add_subdirectory(src/typechecker)
