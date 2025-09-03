find_package(Catch2 3 REQUIRED)

add_executable(tc_test
        ../include/error.h
        ../scanner.cc
        ../include/scanner.h
        typeparser.cc
        ../include/typechecker/typeparser.h
        tc_test.cc
        expression.cc
)

target_include_directories(tc_test PRIVATE ../include)

target_link_libraries(tc_test fmt Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tc_test)
