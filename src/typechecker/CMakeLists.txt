find_package(Catch2 3 REQUIRED)

add_executable(tc_test
        ../../include/error.h
        ../scanner.cc
        ../../include/scanner.h
        typeparser.cc
        ../../include/typechecker/typeparser.h
        tc_test.cc
        expression.cc
        ../../include/typechecker/expression.h
        ../../include/typechecker/tokenholder.h
        statement.cc
        ../../include/typechecker/statement.h
        ../../include/typechecker/types/class_type.h
        ../../include/typechecker/types/primitive_type.h
        ../../include/typechecker/types/intersection_type.h
        ../../include/typechecker/types/union_type.h
        ../../include/typechecker/types/tuple_type.h
        ../../include/typechecker/types/array_type.h
        ../../include/typechecker/types/function_type.h
        ../../include/typechecker/lox_type.h
        ../../include/typechecker/ast.h
        ast.cc
        ../../include/typechecker/statements/var_statement.h
        ../../include/typechecker/statements/print_statement.h
        ../../include/typechecker/statements/fun_statement.h
        ../../include/typechecker/statements/expression_statement.h
        ../../include/typechecker/statements/block_statement.h
        statements/block_statement.cc
        statements/expression_statement.cc
        statements/fun_statement.cc
        statements/print_statement.cc
        statements/var_statement.cc
        expressions/assignment_expression.cc
        ../../include/typechecker/expressions/assignment_expression.h
        expressions/or_expression.cc
        ../../include/typechecker/expressions/or_expression.h
        expressions/and_expression.cc
        ../../include/typechecker/expressions/and_expression.h
        expressions/binary_expression.cc
        ../../include/typechecker/expressions/binary_expression.h
        expressions/unary_expression.cc
        ../../include/typechecker/expressions/unary_expression.h
        expressions/call_expression.cc
        ../../include/typechecker/expressions/call_expression.h
        expressions/dot_expression.cc
        ../../include/typechecker/expressions/dot_expression.h
        expressions/primary_expression.cc
        ../../include/typechecker/expressions/primary_expression.h
)

target_include_directories(tc_test PRIVATE ../../include)

target_link_libraries(tc_test fmt Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tc_test)
