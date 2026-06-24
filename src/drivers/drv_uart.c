#include "FreeRTOS.h"
#include "CMSDK_CM3.h"
#include "drivers/drv_uart.h"

QueueHandle_t xUartRxQueue       = NULL;   /* ISR  → ingest task  */
QueueHandle_t xUartPipelineQueue = NULL;   /* ingest → pipeline task */

// Init Function
void UART0_Init(uint32_t cpu_freq_hz, uint32_t target_baud)
{
    if (cpu_freq_hz == 0)
        cpu_freq_hz = 25000000U;          /* QEMU mps2-an385 default */

    CMSDK_UART0->CTRL    = 0;             /* Disable everything first  */
    CMSDK_UART0->BAUDDIV = cpu_freq_hz / target_baud;

    /* Clear any stale interrupts before unmasking */
    CMSDK_UART0->INTCLEAR = UART_INT_RX | UART_INT_TX;

    /* Enable TX, RX, and the RX interrupt — TX interrupt stays off */
    CMSDK_UART0->CTRL = UART_CTRL_TXEN
                      | UART_CTRL_RXEN
                      | UART_CTRL_RXIRQEN;

    NVIC_SetPriority(UARTRX0_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_ClearPendingIRQ(UARTRX0_IRQn); // Clear garbage
    NVIC_EnableIRQ(UARTRX0_IRQn);          /*  UART0_IRQn       */
}

void UARTRX0_Handler(void)
{   
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Check if Receive Interrupt is active
    if (CMSDK_UART0->INTSTATUS & UART_INT_RX) {
        /* Read DATA first — on CMSDK this also clears the RX status flag */
        uint8_t ucByte = (uint8_t)(CMSDK_UART0->DATA & UART_DATA);
        /* Explicitly clear the RX interrupt status */
        CMSDK_UART0->INTCLEAR = UART_INT_RX;

        if (xUartRxQueue != NULL)
        {
            /* Drop byte silently if queue full — ISR cannot block */
            xQueueSendFromISR(xUartRxQueue, &ucByte, &xHigherPriorityTaskWoken);
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
