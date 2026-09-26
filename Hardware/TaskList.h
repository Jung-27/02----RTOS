#ifndef __TASKLIST_H
#define __TASKLIST_H

#include "stm32f10x.h"

/* ==================== 共享数据(任务间传递) ==================== */
/* 这些变量在 TaskList.c 里定义，其他文件用 extern 声明后即可读取          */
extern volatile uint16_t g_ppm;      /* 烟雾浓度(ppm)   */
extern volatile uint16_t g_light;    /* 光照亮度(0~100) */
extern volatile int8_t   g_temp;     /* 温度(℃，可为负) */
extern volatile uint8_t  g_humi;     /* 湿度(0~100)     */

extern volatile uint8_t g_smoke_flag;   /* 烟雾超标标志 */
extern volatile uint8_t g_light_flag;   /* 光照过暗标志 */
extern volatile uint8_t g_temp_flag;    /* 温度过高标志 */
extern volatile uint8_t g_humi_flag;    /* 湿度过高标志 */

/* ==================== 函数 ==================== */
/* 创建并启动所有应用任务(采集/显示/报警)，内部调用 xTaskCreate */
void Task_Start(void);

#endif /* __TASKLIST_H */
