set(src ../../src)
set(incl ../../include)
set(tc typechecker)


## 包含了 typechecker目录中的所有文件
file(GLOB_RECURSE tc_cc ${src}/${tc}/*.cc)
file(GLOB_RECURSE tc_h ${incl}/${tc}/*.h)

file(GLOB clox_cc ${src}/*.cc)
file(GLOB clox_h ${incl}/*.h)
file(GLOB clox_obj_h ${incl}/objects/*.h)
file(GLOB clox_native_h ${incl}/nativeobjects/*.h)

add_executable(stlox
        ${tc_cc}
        ${tc_h}
        ${clox_cc}
        ${clox_h}
        ${clox_obj_h}
        ${clox_native_h}
        ../objects/loxfunction.cc
        ../objects/loxclosure.cc
        ../objects/loxstring.cc
        expressions/lambda_expression.cc
        ../../include/typechecker/expressions/lambda_expression.h
        expressions/fmt_string_expression.cc
        ../../include/typechecker/expressions/fmt_string_expression.h
)

target_include_directories(stlox PRIVATE ${incl})
target_link_libraries(stlox fmt::fmt)
