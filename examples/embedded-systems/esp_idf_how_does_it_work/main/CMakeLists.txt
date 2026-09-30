# `main` is an ordinary component that ESP-IDF treats specially: it is always
# included in the build, and it is where app_main() is expected to live.
#
# We name my_led under REQUIRES so that main.c can #include "my_led.h".
#
# Note what is *not* named here: esp_driver_gpio. main.c uses gpio_num_t all
# the same, because my_led declares esp_driver_gpio public and public
# dependencies propagate. That is the whole difference between the two lists.
#
# Leave any of these out and the build fails with a very specific error naming
# the header, the component that owns it, and the line to add here. Nothing is
# on the include path unless a component asked for it.

idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS ""
                       REQUIRES my_led
                       PRIV_REQUIRES esp_app_format app_update esp_partition)
