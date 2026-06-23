#pragma once

#include "queue.h"
#include <stdint.h>
#include <stdbool.h>

// CMSDK UART CTRL bits 
#define UART_CTRL_TXEN      (1ul << 0)
#define UART_CTRL_RXEN      (1ul << 1)
#define UART_CTRL_TXIRQEN   (1ul << 2)
#define UART_CTRL_RXIRQEN   (1ul << 3) 

/* CMSDK UART INTCLEAR bits */
#define UART_INT_TX         (1ul << 0)
#define UART_INT_RX         (1ul << 1)

/* Names taken from cmsdk_cm3.h */
#define UART_STATE_RXBF   CMSDK_UART_STATE_RXBF_Msk   /* bit 1 — RX buffer full */
#define UART_STATE_TXBF   CMSDK_UART_STATE_TXBF_Msk   /* bit 0 — TX buffer full */

extern QueueHandle_t xUartRxQueue;

void UART0_Init(uint32_t cpu_freq_hz, uint32_t target_baud);
bool UART0_GetByte(uint8_t *pByte);
void UART0_PutByte(uint8_t byte);