#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "services/logger.h"
#include "drivers/drv_uart.h"
#include "app/task_uart.h"

// Private to this file
#define UART_INGEST_STACK_WORDS   256
#define UART_PIPELINE_STACK_WORDS 512   // MQTT/lwIP needs headroom

/* * TASK 1: The Ingest (Producer)
 * Drains the hardware FIFO and shoves bytes into the FreeRTOS Queue.
 */
static void vTaskUartIngest(void *pvParameters) {
    (void)pvParameters;

    uint8_t rx_byte;
    const TickType_t xPollRate = pdMS_TO_TICKS(15); 

    for(;;) {
        // Drain all bytes currently sitting in the CMSDK hardware buffer
        while (UART0_GetByte(&rx_byte)) {
            // Push to pipeline. If pipeline is backed up, don't drop the byte; wait 5ms.
            xQueueSendToBack(xUartRxQueue, &rx_byte, pdMS_TO_TICKS(5));
        }
        
        // Yield CPU to the rest of the RTOS
        vTaskDelay(xPollRate);
    }
}

/* * TASK 2: The Pipeline Stage (Consumer)
 * Sits fully asleep (0% CPU) until a byte hits the queue. 
 */
static void vTaskUARTPipeline(void *pvParameters) {
    (void)pvParameters;

    uint8_t rx_byte;

    APP_LOG("[UART PIPELINE] Task spun up successfully.\r\n");

    for(;;) {
        if (xQueueReceive(xUartRxQueue, &rx_byte, portMAX_DELAY) == pdPASS) {
            
            APP_LOG("[UART PIPELINE] Ingested raw byte: '0x%02X'\r\n", rx_byte);

            if (rx_byte >= 'a' && rx_byte <= 'z') {
                rx_byte -= 32; 
                APP_LOG("[UART PIPELINE] Transformed to Uppercase: '%c'\r\n", rx_byte);
            }
        }
    }
}

void vUart0_TaskInit(uint8_t *pucQueueStorage, StaticQueue_t *pxQueueStruct)
{
    xUartRxQueue = xQueueCreateStatic(
        UART0_RX_Q_LEN,
        UART0_RX_Q_ITEM_SZ,
        pucQueueStorage,
        pxQueueStruct
    );
    configASSERT(xUartRxQueue != NULL);

    xTaskCreate(vTaskUartIngest,   "UART_In",
                                UART_INGEST_STACK_WORDS,
                                NULL, tskIDLE_PRIORITY + 2, NULL);

    xTaskCreate(vTaskUARTPipeline, "Worker",
                                UART_PIPELINE_STACK_WORDS,
                                NULL, tskIDLE_PRIORITY + 1, NULL);

    configASSERT(r1 == pdPASS);
    configASSERT(r2 == pdPASS);
}