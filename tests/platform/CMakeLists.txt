add_executable(utest_platform
    src/main.cpp
)


target_include_directories(utest_platform PRIVATE
    ../../include
    ${SYSTEMC_INCLUDE}
)

target_link_libraries(utest_platform esimp)

