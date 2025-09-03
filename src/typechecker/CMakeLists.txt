find_package(Catch2 3 REQUIRED)

add_executable(typechecker
        ../include/error.h
        ../scanner.cc
        ../include/scanner.h
        typeparser.cc
        ../include/typechecker/typeparser.h
        main_tc.cc
        expression.cc
)

target_include_directories(typechecker PRIVATE ../include)

target_link_libraries(typechecker fmt Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(typechecker)
