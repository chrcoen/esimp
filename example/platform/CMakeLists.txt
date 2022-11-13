add_executable(platform
    src/main.cpp
)


target_include_directories(platform PRIVATE
    ../../include
    ${SYSTEMC_INCLUDE}
)

target_link_libraries(platform esimp systemc pthread dl rt)


