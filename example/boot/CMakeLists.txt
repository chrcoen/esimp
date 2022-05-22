add_library(boot SHARED
    src/esimp.cpp
)

target_include_directories(boot PRIVATE
    ../../include
)


target_compile_options(boot PRIVATE
    ${INSTRUMENTATION_FLAGS}
)

add_dependencies(boot siminstr)

