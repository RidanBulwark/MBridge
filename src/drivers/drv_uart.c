#include "drivers/drv_uart.h"

// --- Init Function ---
void UART0_Init(uint32_t cpu_freq_hz, uint32_t target_baud) {
    if(cpu_freq_hz == 0) cpu_freq_hz = 25000000; // QEMU mps2-an385 default clock

    CMSDK_UART0->CTRL = 0; // Shut off during config
    CMSDK_UART0->BAUDDIV = cpu_freq_hz / target_baud;
    CMSDK_UART0->CTRL = UART_CTRL_TXEN | UART_CTRL_RXEN;
}

bool UART0_GetByte(uint8_t *pByte) {
    if (CMSDK_UART0->STATE & UART_STATE_RXBF) {
        *pByte = (uint8_t)(CMSDK_UART0->DATA & 0xFF);
        return true;
    }
    return false;
}

void UART0_PutByte(uint8_t byte) {
    while (CMSDK_UART0->STATE & UART_STATE_TXBF) {
        /* Spinwait for QEMU's virtual holding register to clear */
    }
    CMSDK_UART0->DATA = byte;
}