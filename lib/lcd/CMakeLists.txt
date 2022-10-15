aux_source_directory(. DIR_LCD_SRCS)
add_library(lcd ${DIR_LCD_SRCS})
target_link_libraries(lcd PUBLIC pico_stdlib hardware_spi hardware_i2c hardware_pwm hardware_adc)
