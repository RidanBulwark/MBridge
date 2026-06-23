#include <stdint.h>
#include <stdbool.h>

/* CMSDK APB UART Register Map */
typedef struct {
    volatile uint32_t DATA;     /* Offset: 0x00 (R/W) Data Register */
    volatile uint32_t STATE;    /* Offset: 0x04 (R/W) Status Register */
    volatile uint32_t CTRL;     /* Offset: 0x08 (R/W) Control Register */
    volatile uint32_t INTSTATUS;/* Offset: 0x0C (R/W) Interrupt Status Register */
    volatile uint32_t BAUDDIV;  /* Offset: 0x10 (R/W) Baudrate Divider */
} CMSDK_UART_TypeDef;

#define CMSDK_UART0_BASE  (0x40004000UL)
#define CMSDK_UART0       ((CMSDK_UART_TypeDef *) CMSDK_UART0_BASE)

#define UART_STATE_TXBF   (1UL << 0)  /* Transmit Buffer Full */
#define UART_STATE_RXBF   (1UL << 1)  /* Receive Buffer Full */
#define UART_CTRL_TXEN    (1UL << 0)  /* Transmit Enable */
#define UART_CTRL_RXEN    (1UL << 1)  /* Receive Enable */

// --- Your requested custom Init Function ---
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