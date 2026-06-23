#ifndef DRV_UART_H
#define DRV_UART_H

#include "FreeRTOS.h"
#include "queue.h"
#include <stdbool.h>
#include "CMSDK_CM3.h"

/* ── CTRL register bits ── */
#define UART_CTRL_TXEN      (1ul << 0)   /* transmit enable        */
#define UART_CTRL_RXEN      (1ul << 1)   /* receive enable         */
#define UART_CTRL_TXIRQEN   (1ul << 2)   /* TX interrupt enable    */
#define UART_CTRL_RXIRQEN   (1ul << 3)   /* RX interrupt enable    */

/* ── STATE register bits (aliases for the long CMSDK names) ── */
#define UART_STATE_TXBF     CMSDK_UART_STATE_TXBF_Msk   /* bit 0 — TX buffer full */
#define UART_STATE_RXBF     CMSDK_UART_STATE_RXBF_Msk   /* bit 1 — RX buffer full */

/* ── INTCLEAR bits ── */
#define UART_INT_TX         (1ul << 0)
#define UART_INT_RX         (1ul << 1)

/* ── queue sizing ── */
#define UART0_RX_Q_LEN      64
#define UART0_RX_Q_ITEM_SZ  sizeof(uint8_t)

extern QueueHandle_t xUartRxQueue;
extern QueueHandle_t xUartPipelineQueue;

void  UART0_Init(uint32_t cpu_freq_hz, uint32_t target_baud);
bool  UART0_GetByte(uint8_t *pByte);
void  UART0_PutByte(uint8_t byte);

#endif /* DRV_UART_H */