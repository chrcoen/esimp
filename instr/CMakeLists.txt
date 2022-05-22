add_library(siminstr SHARED instr.cpp)
set_property(TARGET siminstr PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(siminstr PRIVATE
    /usr/include/llvm-11
    /usr/include/llvm-c-11
)

set_target_properties(siminstr PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
)
add_compile_options(-fno-rtti)

