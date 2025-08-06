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
        src/include/visual.h
        src/visual.cc
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
        src/include/scope.h)

target_include_directories(cclox PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/include")
target_link_libraries(cclox m readline fmt)