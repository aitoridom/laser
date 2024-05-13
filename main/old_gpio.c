#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define TX_PIN GPIO_NUM_32
#define RX_PIN GPIO_NUM_25


void sender_task(void *pvParameter) {
    gpio_set_direction(TX_PIN, GPIO_MODE_OUTPUT);
    
    
    int data_to_send = 0;
    while (1) {
        // Alternar entre enviar 0 y 1 desde el emisor
        gpio_set_level(TX_PIN, data_to_send);
        printf("-----\n");
        printf("Tx (>>>): %d\n", data_to_send);
        printf("-----\n");
        
        data_to_send = !data_to_send; // Alternar entre 0 y 1
        vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar 1 segundo antes de enviar el próximo dato
    }
}

void receiver_task(void *pvParameter) {
    gpio_set_direction(RX_PIN, GPIO_MODE_INPUT);
    
    while (1) {
        // Leer el dato digital recibido en el pin RX_PIN
        int received_data = gpio_get_level(RX_PIN);
        
        // Imprimir el dato digital recibido
        printf("Rx (<<<): %d\n", received_data);
        
        vTaskDelay(pdMS_TO_TICKS(100)); // Esperar 1 segundo antes de leer el próximo dato
    }
}

void app_main() {
    xTaskCreate(sender_task, "sender_task", 2048, NULL, 5, NULL);
    xTaskCreate(receiver_task, "receiver_task", 2048, NULL, 5, NULL);
}
