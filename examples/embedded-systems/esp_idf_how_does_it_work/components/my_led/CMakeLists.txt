# A hand-written component. This one call is the whole component API.
#
#   SRCS          — what gets compiled into libmy_led.a
#   INCLUDE_DIRS  — the *public* include path; anything that REQUIRES us
#                   gets this directory on its -I list automatically
#   REQUIRES      — public dependencies: they appear in our public header,
#                   so whoever includes my_led.h needs them too
#   PRIV_REQUIRES — private dependencies: used only inside my_led.c
#   LDFRAGMENTS   — our linker fragment, fed to ldgen (see linker.lf)

idf_component_register(SRCS "my_led.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_driver_gpio
                       PRIV_REQUIRES log
                       LDFRAGMENTS "linker.lf")
