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
static void vTaskUartIngest(void *pvParameters)
{
    (void)pvParameters;
    uint8_t rx_byte;
    APP_LOG("[UART Ingest] Task spun up.\r\n");

    for (;;)
    {
        /* Block until ISR deposits a byte — 0% CPU while idle */
        if (xQueueReceive(xUartRxQueue, &rx_byte, portMAX_DELAY) == pdPASS)
        {
            xQueueSendToBack(xUartPipelineQueue, &rx_byte, pdMS_TO_TICKS(5));
        }
    }
}

/* ── Task 2: pipeline stage — transform then hand to MQTT ── */
static void vTaskUARTPipeline(void *pvParameters)
{
    (void)pvParameters;
    uint8_t rx_byte;

    APP_LOG("[UART PIPELINE] Task spun up.\r\n");

    for (;;)
    {
        if (xQueueReceive(xUartPipelineQueue, &rx_byte, portMAX_DELAY) == pdPASS)
        {
            APP_LOG("[UART PIPELINE] Raw byte: 0x%02X\r\n", rx_byte);

            if (rx_byte >= 'a' && rx_byte <= 'z')
            {
                rx_byte -= 32;
                APP_LOG("[UART PIPELINE] Uppercased: '%c'\r\n", rx_byte);
            }

            /* TODO: xQueueSendToBack(xMqttPublishQueue, &rx_byte, ...) */
        }
    }
}


/* ── Init: called once from main ── */
void vUart0_TaskInit(uint8_t  *pucRxQueueStorage,  StaticQueue_t *pxRxQueueStruct,
                     uint8_t  *pucPipeQueueStorage, StaticQueue_t *pxPipeQueueStruct)
{
    xUartRxQueue = xQueueCreateStatic(
        UART0_RX_Q_LEN, UART0_RX_Q_ITEM_SZ,
        pucRxQueueStorage, pxRxQueueStruct);
    configASSERT(xUartRxQueue != NULL);

    xUartPipelineQueue = xQueueCreateStatic(
        UART0_RX_Q_LEN, UART0_RX_Q_ITEM_SZ,
        pucPipeQueueStorage, pxPipeQueueStruct);
    configASSERT(xUartPipelineQueue != NULL);

    xTaskCreate(vTaskUartIngest,   "UART_In",
                                 UART_INGEST_STACK_WORDS,
                                 NULL, tskIDLE_PRIORITY + 2, NULL);

    xTaskCreate(vTaskUARTPipeline, "Worker",
                                 UART_PIPELINE_STACK_WORDS,
                                 NULL, tskIDLE_PRIORITY + 1, NULL);

}