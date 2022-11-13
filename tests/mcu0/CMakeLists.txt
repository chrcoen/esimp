
add_library(utests MODULE
    src/esimp.cpp
    src/testfct.cpp
)

target_include_directories(utests PRIVATE
    ../../include
)

target_compile_options(utests PRIVATE
    ${INSTRUMENTATION_FLAGS}
)

add_dependencies(utests siminstr)

