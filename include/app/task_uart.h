#pragma once

#include "FreeRTOS.h"

// Memory requirements are:
#define UART0_RX_Q_LEN      64
#define UART0_RX_Q_ITEM_SZ  sizeof(uint8_t)

void vUart0_TaskInit(uint8_t  *pucRxQueueStorage,  StaticQueue_t *pxRxQueueStruct,
                     uint8_t  *pucPipeQueueStorage, StaticQueue_t *pxPipeQueueStruct);