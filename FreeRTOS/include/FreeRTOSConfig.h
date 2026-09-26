#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f10x.h"

/* ==================== 基础配置 ==================== */
#define configUSE_PREEMPTION            1                    /* 抢占式调度 */
#define configUSE_IDLE_HOOK             0
#define configUSE_TICK_HOOK             0
#define configCPU_CLOCK_HZ              ( ( unsigned long ) 72000000 )   /* 系统时钟 72MHz */
#define configTICK_RATE_HZ              ( ( TickType_t ) 1000 )          /* 心跳 1kHz(1ms) */
#define configMAX_PRIORITIES            ( 5 )                /* 可用优先级 0~4，数字越大优先级越高 */
#define configMINIMAL_STACK_SIZE        ( ( unsigned short ) 128 )       /* 空闲任务最小栈(字) */
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 10 * 1024 ) )     /* 内核堆 10KB(芯片仅20KB RAM) */
#define configMAX_TASK_NAME_LEN         ( 16 )
#define configUSE_TRACE_FACILITY        1
#define configUSE_16_BIT_TICKS          0
#define configIDLE_SHOULD_YIELD         1

/* ==================== 信号量/互斥量/队列 ==================== */
#define configUSE_MUTEXES               1
#define configQUEUE_REGISTRY_SIZE       8
#define configCHECK_FOR_STACK_OVERFLOW  0
#define configUSE_RECURSIVE_MUTEXES     1
#define configUSE_MALLOC_FAILED_HOOK    0
#define configUSE_APPLICATION_TASK_TAG  0
#define configUSE_COUNTING_SEMAPHORES   1
#define configGENERATE_RUN_TIME_STATS   0

/* ==================== 协程 ==================== */
#define configUSE_CO_ROUTINES           0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* ==================== 软件定时器 ==================== */
#define configUSE_TIMERS                1
#define configTIMER_TASK_PRIORITY       ( 2 )
#define configTIMER_QUEUE_LENGTH        10
#define configTIMER_TASK_STACK_DEPTH    ( configMINIMAL_STACK_SIZE * 2 )

/* ==================== 可选 API ==================== */
#define INCLUDE_vTaskPrioritySet        1
#define INCLUDE_uxTaskPriorityGet       1
#define INCLUDE_vTaskDelete             1
#define INCLUDE_vTaskCleanUpResources   0
#define INCLUDE_vTaskSuspend            1
#define INCLUDE_vTaskDelayUntil         1
#define INCLUDE_vTaskDelay              1

/* ==================== Cortex-M3 中断优先级 ==================== */
/* STM32F103 是 Cortex-M3，抢占优先级 4 位 */
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS              __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS              4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

/* FreeRTOS 内部使用的 8 位优先级(左对齐到高 4 位) */
#define configKERNEL_INTERRUPT_PRIORITY \
    ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )

/* 断言：条件为假则停在这里 */
#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/* ==================== 异常处理函数映射 ==================== */
/* 把 FreeRTOS 的异常处理函数映射到启动文件向量表里的名字 */
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
