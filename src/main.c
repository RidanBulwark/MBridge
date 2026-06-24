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
#include "sys/data_types.h"
#include "services/logger.h"
#include "drivers/drv_uart.h"
#include "app/task_uart.h"
#include "app/task_uart.h"

// Extern tasks
extern void vTask_I2CSensorReader(void *pvParameters);
extern void vTask_DataProcessorAndMqtt(void *pvParameters);

// UART Memory:
/* UART RX queue: ISR → ingest task */
static uint8_t       ucUart0RxStorage[UART0_RX_Q_LEN * UART0_RX_Q_ITEM_SZ];
static StaticQueue_t xUart0RxQueueStruct;

/* UART pipeline queue: ingest task → pipeline task */
static uint8_t       ucUart0PipeStorage[UART0_RX_Q_LEN * UART0_RX_Q_ITEM_SZ];
static StaticQueue_t xUart0PipeQueueStruct;

/* Sensor queue: I2C reader → MQTT publisher */
QueueHandle_t        xSensorQueue = NULL;   /* extern'd by sensor tasks */

#define QUEUE_LENGTH 10
static StaticQueue_t xQueueBuffer;
static uint8_t ucQueueStorage[QUEUE_LENGTH * sizeof(SensorSample_t)];

int main(void) {
    // Init UART HW
    UART0_Init(25000000U, 115200U);
    // Init UART Task
    vUart0_TaskInit(ucUart0RxStorage,   &xUart0RxQueueStruct,
        ucUart0PipeStorage, &xUart0PipeQueueStruct);
    
    APP_LOG_Init();
    // Static allocation
    xSensorQueue = xQueueCreateStatic(QUEUE_LENGTH, sizeof(SensorSample_t), ucQueueStorage, &xQueueBuffer);
    xTaskCreate(vTask_I2CSensorReader, "I2C_Read", 1024, NULL, 2, NULL);
    xTaskCreate(vTask_DataProcessorAndMqtt, "MQTT_Tx", 1024, NULL, 2, NULL);
 
    APP_LOG("[main] All tasks created. Starting scheduler.\r\n");
    UART_DebugPrintf("UART_DebugPrintf\r\n");

    vTaskStartScheduler();
    for(;;); // Execution will never reach here
    return 0;
}

/* -------------------------------------------------------------------------
   MANDATORY FREERTOS HOOKS (Triggered by our settings in FreeRTOSConfig.h)
   ------------------------------------------------------------------------- */
void vApplicationMallocFailedHook(void) {
    APP_LOG("\n[Fatal] RTOS Malloc Failed!\n");
    for(;;);
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
    (void)pxTask;
    APP_LOG("\n[Fatal] Stack Overflow caught in task: %s\n", pcTaskName);
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