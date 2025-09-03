find_package(Catch2 3 REQUIRED)

add_executable(typechecker
        ../include/error.h
        ../scanner.cc
        ../include/scanner.h
        ../typechecker/typechecker.cc
        ../include/typechecker/typechecker.h
        main_tc.cc
)

target_include_directories(typechecker PRIVATE ../include)

target_link_libraries(typechecker fmt Catch2::Catch2WithMain)
