#include <stdint.h>

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;
extern int main(void);
extern void vPortSVCHandler(void), xPortPendSVHandler(void), xPortSysTickHandler(void);

void Reset_Handler(void) {
    uint32_t *src = &_sidata, *dest = &_sdata;
    while (dest < &_edata) *dest++ = *src++;
    dest = &_sbss;
    while (dest < &_ebss) *dest++ = 0;
    main();
    while(1);
}

void Default_Handler(void) { while(1); }

__attribute__((section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
    (void *)&_estack, Reset_Handler, Default_Handler, Default_Handler, 
    Default_Handler, Default_Handler, Default_Handler, 0, 0, 0, 0,
    vPortSVCHandler, Default_Handler, 0, xPortPendSVHandler, xPortSysTickHandler
};