idf_component_register(
    SRCS main.c         # list the source files of this component
    SRCS leds_gpio/gpio.c
    SRCS gsm/gsm_sim800l.c
    INCLUDE_DIRS        # optional, add here public include directories
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)
