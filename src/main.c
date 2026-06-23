// Measurement Bridge FreeRTOS App (MBridge)
// FreeRTOS headers
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
// C headers
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
// Custom headers
#include "logger.h"
#include "data_types.h"
#include "task_uart.h"

// Extern functions
extern void UART0_Init(uint32_t cpu_freq_hz, uint32_t target_baud);

// Extern tasks
extern void vTask_I2CSensorReader(void *pvParameters);
extern void vTask_DataProcessorAndMqtt(void *pvParameters);

// UART Memory:
static uint8_t       ucUart0Storage[ UART0_RX_Q_LEN * UART0_RX_Q_ITEM_SZ ];
static StaticQueue_t xUart0QueueStruct;

#define QUEUE_LENGTH 10
QueueHandle_t xSensorQueue = NULL;
static StaticQueue_t xQueueBuffer;
static uint8_t ucQueueStorage[QUEUE_LENGTH * sizeof(SensorSample_t)];

int main(void) {
    APP_LOG_Init();

    // UART Init
    UART0_Init(25000000, 115200);
    vUart0_TaskInit(ucUart0Storage, &xUart0QueueStruct);

    // Static allocation
    xSensorQueue = xQueueCreateStatic(QUEUE_LENGTH, sizeof(SensorSample_t), ucQueueStorage, &xQueueBuffer);
    xTaskCreate(vTask_I2CSensorReader, "I2C_Read", 1024, NULL, 2, NULL);
    xTaskCreate(vTask_DataProcessorAndMqtt, "MQTT_Tx", 1024, NULL, 2, NULL);
 
    vTaskStartScheduler();
    for(;;); // Execution will never reach here
    return 0;
}

/* -------------------------------------------------------------------------
   MANDATORY FREERTOS HOOKS (Triggered by our settings in FreeRTOSConfig.h)
   ------------------------------------------------------------------------- */
void vApplicationMallocFailedHook(void) {
    printf("\n[Fatal] RTOS Malloc Failed!\n");
    for(;;);
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
    printf("\n[Fatal] Stack Overflow caught in task: %s\n", pcTaskName);
    for(;;);
}

// When configSUPPORT_STATIC_ALLOCATION == 1, the user must supply the RAM for the Idle & Timer tasks:
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, configSTACK_DEPTH_TYPE *pulIdleTaskStackSize) {
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, configSTACK_DEPTH_TYPE *pulTimerTaskStackSize) {
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

// Add low power sleep feat
void vApplicationIdleHook( void )
{
    /* Put the simulated ARM Cortex into low-power sleep */
    __asm volatile ( "wfi" );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
    // Watchdog comes here
}
/*-----------------------------------------------------------*/

void vAssertCalled( void )
{
    volatile unsigned long looping = 0;

    taskENTER_CRITICAL();
    {
        /* Use the debugger to set ul to a non-zero value in order to step out
         *      of this function to determine why it was called. */
        while( looping == 0LU )
        {
            portNOP();
        }
    }
    taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/
void vLoggingPrintf( const char * pcFormat,
                     ... )
{
    va_list arg;

    va_start( arg, pcFormat );
    vprintf( pcFormat, arg );
    va_end( arg );
}