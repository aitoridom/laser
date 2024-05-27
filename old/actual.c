#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/dac.h"
#include "driver/adc.h"

#define DAC_PIN GPIO_NUM_25
#define ADC_PIN GPIO_NUM_32

void sender_task(void *pvParameter) {
    // Configurar el pin DAC
    dac_output_enable(DAC_CHANNEL_1);

    int data_to_send = 0;
    while (1) {
        // Enviar un bit digital como señal analógica
        if (data_to_send == 0) {
            dac_output_voltage(DAC_CHANNEL_1, 0.5 * 3.3); // 0.5V representa un bit '0'
        } else {
            dac_output_voltage(DAC_CHANNEL_1, 2.5 * 3.3); // 2.5V representa un bit '1'
        }

        // Esperar un tiempo antes de enviar el siguiente bit
        vTaskDelay(pdMS_TO_TICKS(100));

        // Cambiar el bit a enviar
        data_to_send = !data_to_send;
    }
}

void receiver_task(void *pvParameter) {
    // Configurar el pin ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);

    int received_data = 0;
    int bit_count = 0;

    while (1) {
        // Leer el valor del ADC
        int adc_value = adc1_get_raw(ADC1_CHANNEL_0);

        // Determinar el bit recibido
        if (adc_value < (3.3 / 2)) {
            received_data |= (1 << bit_count); // Bit '1'
        } else {
            received_data &= ~(1 << bit_count); // Bit '0'
        }

        // Incrementar el contador de bits
        bit_count++;

        // Si se han recibido 8 bits, imprimir el byte recibido y reiniciar el contador
        if (bit_count == 8) {
            printf("Byte recibido: %d\n", received_data);
            bit_count = 0;
            received_data = 0;
        }

        // Esperar antes de leer el siguiente bit
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main() {
    // Crear la tarea de envío
    xTaskCreate(sender_task, "sender_task", 2048, NULL, 5, NULL);

    // Crear la tarea de recepción
    xTaskCreate(receiver_task, "receiver_task", 2048, NULL, 5, NULL);
}