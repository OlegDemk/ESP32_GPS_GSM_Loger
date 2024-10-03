idf_component_register(
    SRCS main.c         # list the source files of this component
    SRCS leds_gpio/leds_gpio.c
    SRCS bme280/bme280.c
    SRCS bme280/main_bme280.c
    SRCS microsd/mount.c
    SRCS gps/main_gps.c
    SRCS gps/nmea_parser.c
    SRCS gsm/gsm_sim800l.c
    SRCS wifi/wifi.c
    SRCS wifi/http.c
    INCLUDE_DIRS        # optional, add here public include directories
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)
