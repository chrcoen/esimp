cmake_minimum_required(VERSION 3.11...3.16)

set(CMAKE_C_COMPILER clang-11)
set(CMAKE_CXX_COMPILER clang++-11)

project(platform
    VERSION 0.1
    DESCRIPTION "Simulation Platform"
    LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 14)

# Configure cmake dependency
if(NOT DEFINED SYSTEMC_PATH)
    set(SYSTEMC_PATH /home/user/systemc-2.3.3)
endif()
set(SYSTEMC_INCLUDE "${SYSTEMC_PATH}/include")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
set(SYSTEMC_LIB "${SYSTEMC_PATH}/lib-linux64-debug")
else()
set(SYSTEMC_LIB "${SYSTEMC_PATH}/lib-linux64")
endif()


set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=readability-*,-readability-magic-numbers,-readability-make-member-function-const)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(instr)
set(INSTRUMENTATION_FLAGS "SHELL:-Xclang -load" "SHELL:-Xclang $<TARGET_FILE:siminstr>")



add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(example)

# Including custom cmake rules
include(cmake/clang-format.cmake)

