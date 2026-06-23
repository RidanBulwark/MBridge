#include "FreeRTOS.h"
#include "CMSDK_CM3.h"
#include "drivers/drv_uart.h"

QueueHandle_t xUartRxQueue = NULL;

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
                      | UART_CTRL_RXIRQEN;  /* ← was missing */

    NVIC_SetPriority(UARTRX0_IRQn, 5);     /* below max, above idle  */
    NVIC_EnableIRQ(UARTRX0_IRQn);          /* ← was UART0_IRQn       */
}

void UARTRX0_Handler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Read DATA first — on CMSDK this also clears the RX status flag */
    uint8_t ucByte = (uint8_t)(CMSDK_UART0->DATA & CMSDK_UART_DATA_Msk);

    /* Explicitly clear the RX interrupt status */
    CMSDK_UART0->INTCLEAR = UART_INT_RX;

    if (xUartRxQueue != NULL)
    {
        /* Drop byte silently if queue full — ISR cannot block */
        xQueueSendFromISR(xUartRxQueue, &ucByte, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


/*
 * UART0_GetByte
 * Non-blocking. Returns true and writes *pByte only if a byte is waiting.
 * Safe to call from task context; do NOT call from the ISR path —
 * the queue-based ISR already handles that.
 */
bool UART0_GetByte(uint8_t *pByte)
{
    configASSERT(pByte != NULL);

    if (CMSDK_UART0->STATE & UART_STATE_RXBF)
    {
        /* Mask to 8 bits — DATA register is 32-bit wide on CMSDK */
        *pByte = (uint8_t)(CMSDK_UART0->DATA & CMSDK_UART_DATA_Msk);
        return true;
    }
    return false;
}

/*
 * UART0_PutByte
 * Blocking spinwait is acceptable for QEMU debug output.
 * Do NOT use this on a real chip inside a FreeRTOS task at high baud —
 * replace with a TX queue + TX interrupt when moving to hardware.
 */
void UART0_PutByte(uint8_t byte)
{
    /* Wait until the TX holding register is empty */
    while (CMSDK_UART0->STATE & UART_STATE_TXBF)
    {
        /* QEMU clears this immediately; on real HW add a watchdog here */
    }
    CMSDK_UART0->DATA = (uint32_t)byte;
}