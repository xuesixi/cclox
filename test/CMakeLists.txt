
add_executable(scanner_test scanner_test.cc )

find_package(Catch2 3 REQUIRED)

target_link_libraries(scanner_test PRIVATE Catch2::Catch2WithMain fmt::fmt cclox_lib)

include(CTest)
include(Catch)