cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
set(PICO_BOARD="waveshare_rp2040_lcd_1.28")
project(picoclock)
pico_sdk_init()

add_subdirectory(./lib/lcd)
include_directories(./lib/lcd)
add_subdirectory(./lib/QMI8658)
include_directories(./lib/QMI8658)

add_executable(main
  main.c
  lib/draw.c
  w.c
  CST816S.c
)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(main)

target_link_libraries(main
lcd QMI8658
pico_stdlib
hardware_spi
hardware_i2c
hardware_pwm
hardware_rtc
hardware_timer
hardware_adc
hardware_flash
hardware_sync
pico_bootsel_via_double_reset
)

add_custom_command(
        TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/main.uf2
                ${CMAKE_CURRENT_BINARY_DIR}/../uf2/main.uf2
)
