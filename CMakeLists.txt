cmake_minimum_required(VERSION 3.5)
project(cclox)


set(CMAKE_CXX_STANDARD 20)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)

add_executable(cclox ${SRC})
target_include_directories(cclox PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/include")
target_link_libraries(cclox m readline)