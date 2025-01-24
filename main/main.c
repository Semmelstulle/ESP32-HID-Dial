#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"

#define ENCODER_PIN_CLK     4
#define ENCODER_PIN_DT      5
#define ENCODER_PIN_BUTTON  6
#define DEVICE_NAME        "ESP32-Encoder"

uint8_t ble_addr_type;
static bool is_connected = false;

// Forward declarations
static void ble_app_advertise(void);
void ble_app_on_sync(void);
void host_task(void *param);

// Implementation of the gap event handler
static int ble_gap_event(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status == 0) {
                is_connected = true;
                printf("Connected\n");
            } else {
                ble_app_advertise();
            }
            break;
        case BLE_GAP_EVENT_DISCONNECT:
            is_connected = false;
            printf("Disconnected\n");
            ble_app_advertise();
            break;
    }
    return 0;
}

static void ble_app_advertise(void) {
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    
    memset(&fields, 0, sizeof(fields));
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name = (uint8_t *)DEVICE_NAME;
    fields.name_len = strlen(DEVICE_NAME);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);
    
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER,
                      &adv_params, ble_gap_event, NULL);
}

void ble_app_on_sync(void) {
    ble_hs_id_infer_auto(0, &ble_addr_type);
    ble_app_advertise();
}

void host_task(void *param) {
    nimble_port_run();
}

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Configure GPIO pins
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL<<ENCODER_PIN_CLK) | (1ULL<<ENCODER_PIN_DT) | (1ULL<<ENCODER_PIN_BUTTON),
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };
    gpio_config(&io_conf);

    // Initialize NimBLE
    nimble_port_init();
    ble_svc_gap_device_name_set(DEVICE_NAME);
    ble_svc_gap_init();
    ble_hs_cfg.sync_cb = ble_app_on_sync;

    // Start the NimBLE host task
    nimble_port_freertos_init(host_task);

    // Main loop to read encoder values
    while(1) {
        int pin_clk_value = gpio_get_level(ENCODER_PIN_CLK);
        int pin_dt_value = gpio_get_level(ENCODER_PIN_DT);
        int button_value = gpio_get_level(ENCODER_PIN_BUTTON);
        
        if (is_connected) {
            printf("Encoder A: %d, Encoder B: %d, Button: %d\n", 
                   pin_clk_value, pin_dt_value, button_value);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
