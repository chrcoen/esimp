add_library(esimp STATIC
    mcu.cpp
    irq.cpp
    isr.cpp
    firmware.cpp
    nvic.cpp
    rtos_thread.cpp
    systemc_thread.cpp
    context.cpp
    timer.cpp
    trace.cpp
)


target_include_directories(esimp PRIVATE
    ../include
    ${SYSTEMC_INCLUDE}
)

target_link_directories(esimp PUBLIC ${SYSTEMC_LIB})
target_link_libraries(esimp systemc pthread dl rt)

