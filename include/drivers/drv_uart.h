#ifndef DRV_UART_H
#define DRV_UART_H

#include "FreeRTOS.h"
#include "queue.h"
#include <stdbool.h>
#include "CMSDK_CM3.h"

/* ── CTRL register bits ── */
/* Standardized short-hand wrappers using the official CMSDK tokens */
#define UART_CTRL_TXEN          CMSDK_UART_CTRL_TXEN_Msk       /* Bit 0: Transmit Enable */
#define UART_CTRL_RXEN          CMSDK_UART_CTRL_RXEN_Msk       /* Bit 1: Receive Enable */
#define UART_CTRL_TXIRQEN       CMSDK_UART_CTRL_TXIRQEN_Msk    /* Bit 2: TX Interrupt Enable */
#define UART_CTRL_RXIRQEN       CMSDK_UART_CTRL_RXIRQEN_Msk    /* Bit 3: RX Interrupt Enable */
#define UART_DATA       CMSDK_UART_DATA_Msk    /* UART Data RX */
#define UART0_RXINT CMSDK_UART_CTRL_RXORIRQ_Msk

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

#endif /* DRV_UART_H */