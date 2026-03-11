cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Pico SDK importieren (Pfad muss passen!)
include(pico_sdk_import.cmake)

project(MPU60X0_RaspberryPi_Pico C CXX ASM)

# SDK initialisieren
pico_sdk_init()

add_executable(${PROJECT_NAME}
    src/main.c
    src/default.c
    src/mpu60x0.c
    lib/rp_pico.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/lib
)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_i2c
    pico_multicore
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})
