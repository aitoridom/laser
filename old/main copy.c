#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"
#include "stdio.h"

#include "driver/dac.h"
#include "driver/adc.h"

#define TIMER_DIVIDER 80 // Ajustar la división del reloj para reducir la velocidad
#define TIMER_SCALE (80 / TIMER_DIVIDER) // Ajustar la división del reloj para reducir la velocidad

#define TX_PIN GPIO_NUM_25
#define RX_PIN GPIO_NUM_32
#define DAC_CHANNEL DAC_CHAN_0
#define ADC_CHANNEL ADC1_CHANNEL_4  // Canal ADC para el pin 32

int count = 0;
int data_to_send = 0;
int data_sent = 0;
int data_received = 0;
bool tx_flag = false;
bool rx_flag = false;

int times_sent = 0;
int times_received = 0;

static void IRAM_ATTR timer_callback(void* args) {
    count++;

    data_to_send = (data_to_send == 0) ? 255 : 0;
    dac_output_voltage(DAC_CHANNEL, data_to_send);

    data_sent = data_to_send;
    times_sent++;

    data_received = adc1_get_raw(ADC_CHANNEL);
    times_received++;
}

void TIMER_INIT2(int timer_idx) {
    timer_config_t config = 
    {
  		  .alarm_en = true,
  		  .counter_en = false,
  		  .intr_type = TIMER_INTR_LEVEL,
  		  .counter_dir = TIMER_COUNT_UP,
  		  .auto_reload = true,
  		  .divider = TIMER_DIVIDER
            
    };

    timer_init(TIMER_GROUP_0, timer_idx, &config);
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0);
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, 50 * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_callback_add(TIMER_GROUP_0, timer_idx, timer_callback, NULL, 0);
    timer_start(TIMER_GROUP_0, timer_idx);
    printf("timer initialzied!!!!\n");
}

void printer_task(void *pvParameter) {
    while (1) {
        printf("Count: %d\n", count);
        printf("Tx: %d\n", data_sent);
        printf("Rx: %d\n", data_received);
        printf("# Tx: %d\n", times_sent);
        printf("# Rx: %d\n", times_received);
        printf("---\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main() {
    gpio_set_direction(TX_PIN, GPIO_MODE_OUTPUT);
    dac_output_enable(DAC_CHANNEL);

    gpio_set_direction(RX_PIN, GPIO_MODE_INPUT);
    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_0);

    TIMER_INIT2(TIMER_0);
    xTaskCreate(printer_task, "printer_task", 2048, NULL, 5, NULL);
}