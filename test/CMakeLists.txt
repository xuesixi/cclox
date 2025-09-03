
add_executable(scanner_test
        scanner_test.cc
        ../src/scanner.cc
        ../src/include/scanner.h
        ../src/include/error.h
)

find_package(Catch2 3 REQUIRED)

target_link_libraries(scanner_test PRIVATE Catch2::Catch2WithMain fmt::fmt)
target_include_directories(scanner_test PRIVATE ../src/include)

include(CTest)
include(Catch)
catch_discover_tests(scanner_test)
