#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "CMSDK_CM3.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "services/logger.h"
#include "drivers/drv_uart.h"

#define LOG_MAX_MSG_LEN   128U
#define LOG_BUFFER_BYTES  512U

static uint8_t          ucLogBufferStorage[LOG_BUFFER_BYTES];
static StaticMessageBuffer_t xLogBufferStruct;

#define UART0_ADDRESS                         ( 0x40004000UL )
#define UART0_DATA                            ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 0UL ) ) ) )
#define UART0_STATE                           ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 4UL ) ) ) )
#define UART0_CTRL                            ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 8UL ) ) ) )
#define UART0_BAUDDIV                         ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 16UL ) ) ) )
#define TX_BUFFER_MASK                        ( 1UL )

#define LOG_BUFFER_TOTAL_SIZE  2048

static MessageBufferHandle_t xLogBuffer = NULL;
static StaticMessageBuffer_t xLogBufferStruct;

static void prvLoggerUARTInit(void)
{
    CMSDK_UART0->CTRL    = 0;
    CMSDK_UART0->BAUDDIV = 25000000U / 115200U;   /* 217 — explicit */
    CMSDK_UART0->CTRL    = UART_CTRL_TXEN | UART_CTRL_RXEN;
}

// =========================================================================
// 1. THE FRONTEND (Called by your Tasks)
// =========================================================================
void APP_LOG(const char *format, ...) {
    if(xLogBuffer == NULL) return;

    char buf[LOG_MAX_MSG_LEN] = {0};
    va_list args;

    va_start(args, format);
    // Format the string inside the calling task's local stack
    int len = vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    if (len <= 0) return;
    if (len >= (int)sizeof(buf)) len = (int)sizeof(buf) - 1;  /* clamp */

    /* Non-blocking send — drop if logger is backed up rather than stall task */
    xMessageBufferSend(xLogBuffer, buf, (size_t)len, pdMS_TO_TICKS(2));
}

// =========================================================================
// 2. THE BACKEND (The Dedicated Consumer Task)
// =========================================================================
void vTask_AsyncLogger(void *pvParameters) {
    (void)pvParameters;
    
    char rx_buf[LOG_MAX_MSG_LEN] = {0};

    for(;;) {
        // Sleep in Blocked state until a task writes to the buffer
        size_t bytes_read = xMessageBufferReceive(
            xLogBuffer, rx_buf,
            sizeof(rx_buf) - 1,   /* leave room for terminator */
            portMAX_DELAY);

        if(bytes_read > 0) {
            rx_buf[bytes_read] = '\0'; // Terminate buffer
            printf(rx_buf, 1, bytes_read, stdout);
            fflush(stdout); 
        }
    }
}

// Call this main()
void APP_LOG_Init(void) {
    prvLoggerUARTInit(); // init UART for printing

    xLogBuffer = xMessageBufferCreateStatic(
        sizeof(ucLogBufferStorage),
        ucLogBufferStorage,
        &xLogBufferStruct);

    configASSERT(xLogBuffer != NULL);

    xTaskCreate(vTask_AsyncLogger, "LogSys",
                                512, NULL, 1, NULL);
}