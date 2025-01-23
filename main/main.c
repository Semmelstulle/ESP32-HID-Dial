#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ENCODER_PIN_CLK     4
#define ENCODER_PIN_DT      5
#define ENCODER_BUTTON      6

void app_main(void)
{
    // Configure GPIO pins
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL<<ENCODER_PIN_CLK) | (1ULL<<ENCODER_PIN_DT) | (1ULL<<ENCODER_BUTTON),
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };
    gpio_config(&io_conf);

    // Main loop to read and print values
    while(1) {
        int pin_clk_value = gpio_get_level(ENCODER_PIN_CLK);
        int pin_dt_value = gpio_get_level(ENCODER_PIN_DT);
        int button_value = gpio_get_level(ENCODER_BUTTON);
        
        printf("Encoder A: %d, Encoder B: %d, Button: %d\n", 
               pin_clk_value, pin_dt_value, button_value);
        
        vTaskDelay(pdMS_TO_TICKS(100));  // Read every 100ms
    }
}
