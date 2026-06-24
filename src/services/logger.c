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

#define UART0_ADDRESS                         ( 0x40004000UL )
#define UART0_DATA                            ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 0UL ) ) ) )
#define UART0_STATE                           ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 4UL ) ) ) )
#define UART0_CTRL                            ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 8UL ) ) ) )
#define UART0_BAUDDIV                         ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 16UL ) ) ) )
#define TX_BUFFER_MASK                        ( 1UL )

#define LOG_BUFFER_TOTAL_SIZE  2048
#define DEBUG_MSG_MAX_LEN  128

static uint8_t          ucLogBufferStorage[LOG_BUFFER_BYTES];
static MessageBufferHandle_t xLogBuffer = NULL;
static StaticMessageBuffer_t xLogBufferStruct;

// =========================================================================
// 1. THE FRONTEND (Called by your Tasks)
// =========================================================================
void APP_LOG(const char *format, ...) {
    if (xLogBuffer == NULL) return;

    char buf[LOG_MAX_MSG_LEN] = {0};
    va_list args;

    va_start(args, format);
    int len = vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    if (len <= 0) return;
    if (len >= (int)sizeof(buf)) {
        len = (int)sizeof(buf) - 1; // Clamp to fit buffer
    }

    // Send RAW bytes. NO '\0' terminator.
    xMessageBufferSend(xLogBuffer, buf, (size_t)len, pdMS_TO_TICKS(2));
}

// =========================================================================
// 2. THE BACKEND (The Dedicated Consumer Task)
// =========================================================================
void vTask_AsyncLogger(void *pvParameters) {
    (void)pvParameters;
    
    char rx_buf[LOG_MAX_MSG_LEN] = {0};

    for(;;) {
        // xMessageBufferReceive returns the EXACT number of raw bytes it unpacked
        size_t bytes_read = xMessageBufferReceive(
            xLogBuffer, 
            rx_buf,
            sizeof(rx_buf),
            portMAX_DELAY
        );

        if (bytes_read > 0) {
            // 1. Blast the raw payload bytes down the copper
            for (size_t i = 0; i < bytes_read; i++) {
                while (CMSDK_UART0->STATE & UART_STATE_TXBF); 
                CMSDK_UART0->DATA = rx_buf[i];
            }

            // 2. Manually slam the Carriage Return / Line Feed on the end
            // while (CMSDK_UART0->STATE & UART_STATE_TXBF);
            // CMSDK_UART0->DATA = '\r';

            // while (CMSDK_UART0->STATE & UART_STATE_TXBF);
            // CMSDK_UART0->DATA = '\n';
        }
    }
}

// Call this main()
void APP_LOG_Init(void) {
    xLogBuffer = xMessageBufferCreateStatic(
        sizeof(ucLogBufferStorage),
        ucLogBufferStorage,
        &xLogBufferStruct);

    configASSERT(xLogBuffer != NULL);

    xTaskCreate(vTask_AsyncLogger, "LogSys",
                                512, NULL, 1, NULL);
}