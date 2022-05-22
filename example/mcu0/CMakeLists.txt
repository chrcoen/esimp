add_library(mcu0 SHARED
    src/esimp.cpp
)

target_include_directories(mcu0 PRIVATE
    ../../include
)


target_compile_options(mcu0 PRIVATE
    ${INSTRUMENTATION_FLAGS}
)

add_dependencies(mcu0 siminstr)

