set(src ../../src)
set(incl ../../include)
set(tc typechecker)

file(GLOB_RECURSE tc_cc ${src}/{tc}/*.cc)
file(GLOB_RECURSE tc_h ${incl}/{tc}/*.h)
file(GLOB clox_cc ${src}/*.cc)
file(GLOB clox_h ${incl}/*.h)

add_executable(stlox
        ${tc_cc}
        ${tc_h}
        ${clox_cc}
        ${clox_h}
)

target_include_directories(stlox PRIVATE ${incl})
target_link_libraries(stlox fmt::fmt)
