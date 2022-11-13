cmake_minimum_required(VERSION 3.11...3.16)

set(CMAKE_C_COMPILER clang-11)
set(CMAKE_CXX_COMPILER clang++-11)

project(platform
    VERSION 0.1
    DESCRIPTION "Embedded software simulation platform (esimp)"
    LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 14)


# SystemC
include(cmake/systemc.cmake)


set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=readability-*,-readability-magic-numbers,-readability-make-member-function-const,-readability-convert-member-functions-to-static)

add_subdirectory(instr)
set(INSTRUMENTATION_FLAGS "SHELL:-Xclang -load" "SHELL:-Xclang $<TARGET_FILE:siminstr>")

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(example)

target_compile_options(esimp PRIVATE -g -O0)
target_compile_options(utests PRIVATE -g -O0)
target_compile_options(utest_platform PRIVATE -g -O0)
target_compile_options(platform PRIVATE -g -O0)
target_compile_options(mcu0 PRIVATE -g -O0)
target_compile_options(boot PRIVATE -g -O0)

include(cmake/clang-format.cmake)

