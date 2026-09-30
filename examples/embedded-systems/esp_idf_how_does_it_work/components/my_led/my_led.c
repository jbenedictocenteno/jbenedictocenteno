/* my_led — a minimal component, written to expose ESP-IDF's plumbing.
 *
 * Three things to notice:
 *   1. sdkconfig.h is a *generated* header. It exists only in build/config/,
 *      and it is on the include path because kconfig.cmake put it there.
 *   2. driver/gpio.h comes from the esp_driver_gpio component, which we can
 *      include only because CMakeLists.txt lists it under REQUIRES.
 *   3. my_led_toggle_fast() carries no IRAM_ATTR. It ends up in IRAM anyway,
 *      because linker.lf says so. Same result, decided at link time instead
 *      of in the source.
 */

#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "my_led.h"

static const char *TAG = "my_led";

/* CONFIG_MY_LED_GPIO is a #define generated from Kconfig. To the compiler
 * this line is just `static const gpio_num_t s_pin = 27;`. */
static const gpio_num_t s_pin = (gpio_num_t)CONFIG_MY_LED_GPIO;

#if CONFIG_MY_LED_ACTIVE_LOW
#define LEVEL_FOR(on) ((on) ? 0 : 1)
#else
#define LEVEL_FOR(on) ((on) ? 1 : 0)
#endif

/* Zero-initialized global -> .bss. Nobody assigns it before app_main runs,
 * so if it reads back as 0 there, something zeroed it for us. */
static bool s_state;

esp_err_t my_led_init(void)
{
    ESP_LOGI(TAG, "init on GPIO %d", (int)s_pin);

    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << s_pin,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    return gpio_config(&cfg);
}

esp_err_t my_led_set(bool on)
{
    s_state = on;
    return gpio_set_level(s_pin, LEVEL_FOR(on));
}

void my_led_toggle_fast(void)
{
    s_state = !s_state;
    gpio_set_level(s_pin, LEVEL_FOR(s_state));
}

gpio_num_t my_led_pin(void)
{
    return s_pin;
}
