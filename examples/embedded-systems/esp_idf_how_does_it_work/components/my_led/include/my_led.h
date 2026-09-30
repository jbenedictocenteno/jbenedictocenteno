/* Public header of the my_led component.
 *
 * It lives in the directory listed as INCLUDE_DIRS, so any component that
 * declares `REQUIRES my_led` can #include "my_led.h" with no extra -I flags.
 */
#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Configure the LED pin as an output. Call once before the others. */
esp_err_t my_led_init(void);

/* Drive the LED on or off. Lives in flash, like most code. */
esp_err_t my_led_set(bool on);

/* Same idea, but this one is placed in IRAM by linker.lf, so it is safe to
 * call from an interrupt handler that runs while the flash cache is disabled.
 */
void my_led_toggle_fast(void);

/* The pin this component drives, taken from CONFIG_MY_LED_GPIO.
 *
 * The return type is gpio_num_t, which comes from driver/gpio.h. That is what
 * makes esp_driver_gpio a *public* dependency: this header cannot even be
 * parsed without it, so everyone who includes us needs it on their include
 * path too. Hence REQUIRES and not PRIV_REQUIRES in CMakeLists.txt.
 */
gpio_num_t my_led_pin(void);

#ifdef __cplusplus
}
#endif
