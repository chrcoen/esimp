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
    uart.cpp
    gpio.cpp
)


target_include_directories(esimp PRIVATE
    ../include
)

target_link_libraries(esimp systemc)

