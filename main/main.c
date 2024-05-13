#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/timer.h"

#define TX_PIN GPIO_NUM_32
#define RX_PIN GPIO_NUM_25

timer_config_t timerConfig{
    .divider = TIMER_BASE_CLK / TIMER_TIMER_CLK
    
}

void sender_task(void *pvParameter) {
    gpio_set_direction(TX_PIN, GPIO_MODE_OUTPUT);
    

    int data_to_send = 0;
    while (1) {
        gpio_set_level(TX_PIN, data_to_send);
        printf("Tx (>>>): %d\n", data_to_send);
        printf("-----\n");

        data_to_send = !data_to_send;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void receiver_task(void *pvParameter) {
    gpio_set_direction(RX_PIN, GPIO_MODE_INPUT);
    
    int received_data = 0;
    while (1) {
        received_data = gpio_get_level(RX_PIN);
        printf("Rx (<<<): %d\n", received_data);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main() {
    xTaskCreate(sender_task, "sender_task", 2048, NULL, 5, NULL);
    xTaskCreate(receiver_task, "receiver_task", 2048, NULL, 5, NULL);
}

