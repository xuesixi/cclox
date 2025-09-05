find_package(Catch2 3 REQUIRED)

add_executable(tc_test
        ../include/error.h
        ../scanner.cc
        ../include/scanner.h
        typeparser.cc
        ../include/typechecker/typeparser.h
        tc_test.cc
        expression.cc
        ../include/typechecker/expression.h
        ../include/typechecker/tokenholder.h
        statement.cc
        ../include/typechecker/statement.h
        ../include/typechecker/types/class_type.h
        ../include/typechecker/types/primitive_type.h
        ../include/typechecker/types/intersection_type.h
        ../include/typechecker/types/union_type.h
        ../include/typechecker/types/tuple_type.h
        ../include/typechecker/types/array_type.h
        ../include/typechecker/types/function_type.h
        ../include/typechecker/types/lox_type.h
)

target_include_directories(tc_test PRIVATE ../include)

target_link_libraries(tc_test fmt Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tc_test)
