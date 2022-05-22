
add_library(utests MODULE
    src/main.cpp
)

target_include_directories(utests PRIVATE
    ../../include
)

