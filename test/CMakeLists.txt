
find_package(Catch2 3 REQUIRED)
include(CTest)
include(Catch)

set(src_dir ../src)
set(include_dir ../include)

file(GLOB type_cc ${src_dir}/typechecker/types/*.cc)
file(GLOB type_h ${src_dir}/typechecker/types/*.h)

add_executable(scanner_test
        scanner_test.cc
        ${src_dir}/scanner.cc
        ${include_dir}/scanner.h
        ${include_dir}/error.h
)
target_link_libraries(scanner_test PRIVATE Catch2::Catch2WithMain fmt::fmt)
target_include_directories(scanner_test PRIVATE ../include)
catch_discover_tests(scanner_test)

add_executable(type_test
        type_parsing_test.cc
        type_accept_test.cc
        ${type_cc}
        ${type_h}
        ${src_dir}/typechecker/typeparser.cc
        ${include_dir}/typechecker/typeparser.h
        ${src_dir}/scanner.cc
        ${include_dir}/scanner.h
        ${include_dir}/common.h
        ${include_dir}/error.h
        type_accept_test.cc
)
target_link_libraries(type_test PRIVATE Catch2::Catch2WithMain fmt::fmt)
target_include_directories(type_test PRIVATE ${include_dir})
catch_discover_tests(type_test)

