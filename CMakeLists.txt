cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Pico SDK importieren (Pfad muss passen!)
include(pico_sdk_import.cmake)

project(MPU-Raspberry-Pi-Pico C CXX ASM)

set(PROJECT_VERSION "1.3.0-beta")
set(PROJECT_AUTHOR "Robin Gerhartz")
# SDK initialisieren
pico_sdk_init()

# Den C-Compiler anweisen, diese Makros global verfügbar zu machen
add_compile_definitions(
    APP_NAME="${PROJECT_NAME}"
    APP_VERSION="${PROJECT_VERSION}"
    APP_AUTHOR="${PROJECT_AUTHOR}"
)

add_executable(${PROJECT_NAME}
    lib/mpu/mpu.c
    lib/i2c.c
    src/main.c
    lib/rp_pico.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/lib/mpu
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
