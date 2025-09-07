
find_package(Catch2 3 REQUIRED)
include(CTest)
include(Catch)

set(src ../src)
set(incl ../include)
set(tc typechecker)

file(GLOB type_cc ${src}/${tc}/types/*.cc)
file(GLOB type_h ${incl}/${tc}/types/*.h)

add_executable(scanner_test
        scanner_test.cc
        ${src}/scanner.cc
        ${incl}/scanner.h
        ${incl}/error.h
)
target_link_libraries(scanner_test PRIVATE Catch2::Catch2WithMain fmt::fmt)
target_include_directories(scanner_test PRIVATE ${incl})
catch_discover_tests(scanner_test)

add_executable(type_test
        type_parsing_test.cc
        type_accept_test.cc
        ${type_cc}
        ${type_h}
        ${src}/${tc}/typeparser.cc
        ${incl}/${tc}/typeparser.h
        ${src}/${tc}/lox_type.cc
        ${incl}/${tc}/lox_type.h
        ${src}/scanner.cc
        ${incl}/scanner.h
        ${incl}/common.h
        ${incl}/error.h
)
target_link_libraries(type_test PRIVATE Catch2::Catch2WithMain fmt::fmt)
target_include_directories(type_test PRIVATE ${incl})
catch_discover_tests(type_test)

