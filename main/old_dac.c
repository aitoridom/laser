#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/dac.h"
#include "driver/adc.h"

#define TX_PIN GPIO_NUM_25
#define RX_PIN GPIO_NUM_32

#define DAC_CHANNEL DAC_CHAN_0
#define ADC_CHANNEL ADC1_CHANNEL_6  // Canal ADC para el pin 32

void sender_task(void *pvParameter) {
    gpio_set_direction(TX_PIN, GPIO_MODE_OUTPUT);

    // Configurar el DAC
    dac_output_enable(DAC_CHANNEL);

    int data_to_send = 0;
    data_to_send = 255;
    while (1) {
        // Enviar datos analógicos alternando entre 0 y 4095 (12 bits)
        dac_output_voltage(DAC_CHANNEL, data_to_send);
        // printf("-----\n");
        // printf("Tx (>>>): %d\n", data_to_send);
        // printf("-----\n");

        // data_to_send = (data_to_send == 0) ? 255 : 0; // Alternar entre 0 y 4095
        vTaskDelay(pdMS_TO_TICKS(10)); // Esperar 1 segundo antes de enviar el próximo dato
    }
}

void receiver_task(void *pvParameter) {
    gpio_set_direction(RX_PIN, GPIO_MODE_INPUT);

    // Configurar el ADC
    adc1_config_width(ADC_WIDTH_BIT_DEFAULT); // Usar 12 bits para una mejor precisión
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_0);

    while (1) {
        // Leer el dato analógico recibido del pin RX_PIN
        int received_data = adc1_get_raw(ADC_CHANNEL);

        // Imprimir el dato analógico recibido
        printf("Rx (<<<): %d\n", received_data);

        vTaskDelay(pdMS_TO_TICKS(100)); // Esperar 1 segundo antes de leer el próximo dato
    }
}

void app_main() {
    xTaskCreate(sender_task, "sender_task", 2048, NULL, 5, NULL);
    xTaskCreate(receiver_task, "receiver_task", 2048, NULL, 5, NULL);
}