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

int last_5_sent[5];
int last_5_received[5];
int times_sent = 0;
int times_received = 0;

QueueHandle_t TxQueueHandle;
QueueHandle_t RxQueueHandle;
const int QueueElementSize = 10;
typedef struct {
  char line[64];
  uint8_t line_length;
} message_t;
message_t message;

static void IRAM_ATTR timer_callback(void* args) {
    count++;
    data_to_send = (data_to_send == 0) ? 255 : 0;

    xQueueSend(TxQueueHandle, (void *)&message, 0);
    xQueueSend(RxQueueHandle, (void *)&message, 0);
}

void insert_and_rotate(int arr[], int num) {
    for (int i = 3; i >= 0; i--) {
        arr[i+1] = arr[i];
    }
    arr[0] = num;
}

void print_array(int arr[]) {
    for (int i = 0; i<5; i++) {
        printf("%d ,", arr[i]);
    }
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
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, 1000 * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_callback_add(TIMER_GROUP_0, timer_idx, timer_callback, NULL, 0);
    timer_start(TIMER_GROUP_0, timer_idx);
    printf("timer initialzied!!!!\n");
}

void printer_task(void *pvParameter) {
    while (1) {
        printf("Count: %d\n", count);
        printf("Last 5 Tx:");
        print_array(last_5_sent);
        printf("\n");
        printf("Last 5 Rx:");
        print_array(last_5_received);
        printf("\n");
        printf("# Tx: %d\n", times_sent);
        printf("# Rx: %d\n", times_received);
        printf("---\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void sender_task(void *pvParameter) {
    gpio_set_direction(TX_PIN, GPIO_MODE_OUTPUT);
    dac_output_enable(DAC_CHANNEL);

    printf("Tx loop initialized\n");
    while (1) {
        int ret = xQueueReceive(TxQueueHandle, &message, portMAX_DELAY);
        dac_output_voltage(DAC_CHANNEL, data_to_send);
        data_sent = data_to_send;
        insert_and_rotate(last_5_sent, data_sent);
        times_sent++;
    }
}

void receiver_task(void *pvParameter) {
    gpio_set_direction(RX_PIN, GPIO_MODE_INPUT);
    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_0);

    printf("Rx loop initialized\n");
    while (1) {
        int ret = xQueueReceive(RxQueueHandle, &message, portMAX_DELAY);
        data_received = adc1_get_raw(ADC_CHANNEL);
        insert_and_rotate(last_5_received, data_received);
        times_received++;
    }
}

void app_main() {
    TxQueueHandle = xQueueCreate(QueueElementSize, sizeof(message_t));
    RxQueueHandle = xQueueCreate(QueueElementSize, sizeof(message_t));

    xTaskCreate(receiver_task, "receiver_task", 2048, NULL, 5, NULL);
    xTaskCreate(sender_task, "sender_task", 2048, NULL, 5, NULL);
    xTaskCreate(printer_task, "printer_task", 2048, NULL, 5, NULL);
    TIMER_INIT2(TIMER_0);
}