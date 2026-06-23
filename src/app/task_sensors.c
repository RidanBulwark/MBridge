#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <inttypes.h>

#include "FreeRTOSConfig.h"
#include "sys/data_types.h"
#include "drivers/app_hal.h"
#include "drivers/drv_uart.h"
#include "services/logger.h"

#define SENSOR_I2C_ADDR 0x44 // e.g. an SHT31 Temp Sensor

extern QueueHandle_t xSensorQueue; // Declared in main.c

void vTask_I2CSensorReader(void *pvParameters) {
    (void)pvParameters;

    uint8_t raw_data[4] = {0};;
    SensorSample_t sample;

    for(;;) {
        // Read 4 bytes starting at register 0x00
        if (APP_I2C_ReadRegister(SENSOR_I2C_ADDR, 0x00, raw_data, 4) == 0) {
            APP_LOG("[I2C_Task] Reading sensor: %d\r\n", SENSOR_I2C_ADDR);
            
            // "Process" the raw bytes into human values
            uint16_t raw_temp = (raw_data[0] << 8) | raw_data[1];
            uint16_t raw_hum  = (raw_data[2] << 8) | raw_data[3];

            sample.temperature  = -45.0f + (175.0f * ((float)raw_temp / 65535.0f));
            sample.humidity     = 100.0f * ((float)raw_hum / 65535.0f);
            sample.timestamp_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

            // Non-blocking push to the processing queue
            xQueueSend(xSensorQueue, &sample, 0); 
        } else {
            APP_LOG("[I2C_Task] Hardware NACK detected from sensor: %d\r\n", SENSOR_I2C_ADDR);
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Sample at 1Hz
    }
}

void vTask_DataProcessorAndMqtt(void *pvParameters) {
    (void)pvParameters;

    SensorSample_t sample;
    char json_buffer[128] = {0};

    char s_temp[16] = {0}; 
    char s_hum[16] = {0};

    for(;;) {
        // Sleep until data appears in the queue
        if (xQueueReceive(xSensorQueue, &sample, portMAX_DELAY) == pdTRUE) {
            
            // Secondary Processing (e.g. check thresholds, format data)
            if(sample.temperature > 80.0f) {
                // Trigger local alert flag...
            }

            snprintf(json_buffer, sizeof(json_buffer), 
                "{\"device_id\":\"sim_01\", \"temp\":%s, \"hum\":%s, \"t\":%" PRIu32 "}", // use inttypes for correct long int -> str definition
                float_to_str( sample.temperature, s_temp, sizeof(s_temp) ),
                float_to_str( sample.humidity,    s_hum,  sizeof(s_hum)  ),
                sample.timestamp_ms
            );

            APP_MQTT_Publish("telemetry/sensors", json_buffer);
        }
    }
}