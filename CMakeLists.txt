cmake_minimum_required(VERSION 3.12)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(airquality C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(PICO_BOARD pico_w)

pico_sdk_init()

add_executable(${PROJECT_NAME}
    main.c
)

pico_add_extra_outputs(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_gpio
    hardware_i2c
)

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)