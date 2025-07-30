cmake_minimum_required(VERSION 3.5)
project(cclox)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 10.2.1  # Use latest stable version
)

FetchContent_MakeAvailable(fmt)

add_executable(cclox
        src/include/chunk.h
        src/chunk.cc
        src/value.cc
        src/include/value.h
        src/include/bytes.h
        src/include/error.h
        src/main.cc)
target_include_directories(cclox PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/include")
target_link_libraries(cclox m readline fmt)