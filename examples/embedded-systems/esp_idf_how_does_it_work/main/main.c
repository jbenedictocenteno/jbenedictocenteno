/* The application, for the note "How does ESP-IDF work?".
 *
 * The two globals are the same probes used in the bare-metal STM32 note: one
 * with a non-zero initializer (.data) and one without (.bss). On the STM32 we
 * had to copy and zero them ourselves in Reset_Handler. Here nobody wrote that
 * code — but the values still come out right, and this program prints them to
 * prove it.
 */

#include <stdint.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_system.h"
#include "esp_app_desc.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"

#include "my_led.h"

static const char *TAG = "app";

volatile uint32_t counter = 7;   /* initialized  -> lands in .data */
volatile uint32_t flag;          /* zero-init    -> lands in .bss  */

/* A function pinned to internal RAM by an attribute in the source, as opposed
 * to my_led_toggle_fast(), which gets there via a linker fragment instead. */
IRAM_ATTR void app_blink_isr_safe(void)
{
    my_led_toggle_fast();
}

void app_main(void)
{
    /* 1. The probes. Nothing in this program assigned these yet. */
    ESP_LOGI(TAG, "counter = %" PRIu32 " (.data, expected 7)", counter);
    ESP_LOGI(TAG, "flag    = %" PRIu32 " (.bss,  expected 0)", flag);
    counter++;

    /* 2. Where the code we are running actually lives. Flash is mapped into
     *    the 0x4xxxxxxx window; internal RAM is up at 0x4FFxxxxx. */
    ESP_LOGI(TAG, "app_main            @ %p (flash, via cache)", (void *)&app_main);
    ESP_LOGI(TAG, "app_blink_isr_safe  @ %p (IRAM, via IRAM_ATTR)", (void *)&app_blink_isr_safe);
    ESP_LOGI(TAG, "my_led_toggle_fast  @ %p (IRAM, via linker.lf)", (void *)&my_led_toggle_fast);
    ESP_LOGI(TAG, "counter             @ %p (DRAM)", (void *)&counter);

    /* 3. Who am I? Read back out of the image header the bootloader used. */
    const esp_app_desc_t *desc = esp_app_get_description();
    ESP_LOGI(TAG, "project '%s', IDF %s, built %s %s",
             desc->project_name, desc->idf_ver, desc->date, desc->time);

    /* 4. The partition table, read from flash at run time. */
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (running != NULL) {
        ESP_LOGI(TAG, "running from partition '%s' at 0x%" PRIx32 ", size 0x%" PRIx32,
                 running->label, (uint32_t)running->address, (uint32_t)running->size);
    }

    /* 5. Free heap, and the FreeRTOS task we are running inside. */
    ESP_LOGI(TAG, "task '%s', free heap %" PRIu32 " bytes",
             pcTaskGetName(NULL), esp_get_free_heap_size());

    /* 6. Actually use the component.
     *
     *    gpio_num_t is a type from esp_driver_gpio, a component main never
     *    names. We can see it only because my_led lists esp_driver_gpio under
     *    REQUIRES rather than PRIV_REQUIRES, so it propagates to us. Change
     *    that one word in components/my_led/CMakeLists.txt and the next line
     *    stops compiling. */
    const gpio_num_t pin = my_led_pin();
    ESP_LOGI(TAG, "my_led drives GPIO %d (gpio_num_t reached us via REQUIRES)", (int)pin);

    ESP_ERROR_CHECK(my_led_init());

    while (1) {
        my_led_set(true);
        vTaskDelay(pdMS_TO_TICKS(500));
        my_led_set(false);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
