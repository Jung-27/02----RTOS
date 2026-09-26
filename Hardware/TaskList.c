#include "stm32f10x.h"
#include "FreeRTOS.h"       /* FreeRTOS 核心头文件(配置/端口/链表等) */
#include "task.h"           /* 任务函数声明：xTaskCreate / vTaskDelay 等 */

#include "TaskList.h"
#include "LED.h"
#include "Buzz.h"
#include "Motor.h"
#include "MQ2.h"
#include "Light.h"
#include "DH11.h"
#include "OLED.h"

/* ==================== 报警阈值(按实际环境调试修改) ==================== */
/* 阈值单位：烟雾=ppm，光照=0~100，温度=℃，湿度=0~100                        */
#define SMOKE_THRESHOLD   300     /* 烟雾浓度上限，超过报警。洁净空气基线约十几，需调高 */
#define LIGHT_THRESHOLD   25      /* 光照下限，低于它判定"太暗"，点亮 LED1            */
#define TEMP_THRESHOLD    33      /* 温度上限，超过报警并开风扇降温                    */
#define HUMI_THRESHOLD    70      /* 湿度上限，超过报警                                */

/* ==================== 全局变量定义 ==================== */
/* volatile 告诉编译器：这些变量可能被别的任务修改，每次都要从内存重新读 */
volatile uint16_t g_ppm   = 0;    /* 烟雾浓度(ppm)      */
volatile uint16_t g_light = 0;    /* 光照亮度(0~100)    */
volatile int8_t   g_temp  = 0;    /* 温度(℃，可为负)    */
volatile uint8_t  g_humi  = 0;    /* 湿度(0~100)        */

volatile uint8_t g_smoke_flag = 0;   /* 烟雾超标标志 */
volatile uint8_t g_light_flag = 0;   /* 光照过暗标志 */
volatile uint8_t g_temp_flag  = 0;   /* 温度过高标志 */
volatile uint8_t g_humi_flag  = 0;   /* 湿度过高标志 */


/* ==================== 采集任务(优先级2) ==================== */
/* 读取所有传感器，更新数据，并与阈值比较置报警标志位 */
static void Collect_Task(void *pvParameters)
{
	int8_t  temp;      /* DHT11 温度是 int8_t，可为负数 */
	uint8_t humi;

	while (1)
	{
		/* 烟雾浓度(ppm) */
		g_ppm = (uint16_t)MQ2_GetPPM();

		/* 光照亮度(0~100，越大越亮) */
		g_light = Light_GetPercentage();

		/* 温湿度(DHT11)，读取失败则保留上一次的值 */
		if (DHT11_ReadData(&temp, &humi) == 0)
		{
			g_temp = (int8_t)(temp - 5);   /* -5：修正 DHT11 读数虚高(约5℃)，可按需调整 */
			g_humi = humi;
		}

		/* 与阈值比较，置报警标志位(1=报警) */
		g_smoke_flag = (g_ppm   > SMOKE_THRESHOLD) ? 1 : 0;
		g_light_flag = (g_light < LIGHT_THRESHOLD) ? 1 : 0;
		g_temp_flag  = (g_temp  > TEMP_THRESHOLD)  ? 1 : 0;
		g_humi_flag  = (g_humi  > HUMI_THRESHOLD)  ? 1 : 0;

		/* DHT11 两次读取之间需间隔 1 秒以上 */
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}


/* ==================== 显示任务(优先级1) ==================== */
/* 在 OLED 上四行分别显示 ppm / light / temp / humi */
static void Display_Task(void *pvParameters)
{
	while (1)
	{
		/* 第一行：烟雾浓度 */
		OLED_ShowString(1, 1, "ppm:");
		OLED_ShowNum(1, 5, g_ppm, 5);

		/* 第二行：光照 */
		OLED_ShowString(2, 1, "light:");
		OLED_ShowNum(2, 7, g_light, 3);
		OLED_ShowString(2, 10, "%");

		/* 第三行：温度(带正负号) */
		OLED_ShowString(3, 1, "temp:");
		OLED_ShowSignedNum(3, 6, g_temp, 2);
		OLED_ShowString(3, 9, "C");

		/* 第四行：湿度 */
		OLED_ShowString(4, 1, "humi:");
		OLED_ShowNum(4, 6, g_humi, 3);
		OLED_ShowString(4, 9, "%");

		vTaskDelay(pdMS_TO_TICKS(500));
	}
}


/* ==================== 报警任务(优先级3，最高) ==================== */
/* 根据各标志位，驱动 LED / 蜂鸣器 / 风扇 */
static void Alarm_Task(void *pvParameters)
{
	while (1)
	{
		/* 光照过暗 → LED1 亮(否则灭) */
		if (g_light_flag)
			LED1_Set(1);
		else
			LED1_Set(0);

		/* 烟雾超标 → LED2 闪烁(否则灭) */
		if (g_smoke_flag)
			LED2_Toggle();
		else
			LED2_Set(0);

		/* 湿度过高 → LED3 闪烁(否则灭) */
		if (g_humi_flag)
			LED3_Toggle();
		else
			LED3_Set(0);

		/* 温度过高 → 蜂鸣器响 + 风扇转动降温(否则停) */
		if (g_temp_flag)
		{
			Buzz_ON();
			Motor_SetSpeed(35);   /* 正转，35% 占空比 */
		}
		else
		{
			Buzz_OFF();
			Motor_SetSpeed(0);    /* 停转 */
		}

		vTaskDelay(pdMS_TO_TICKS(200));   /* 200ms 一个周期，决定闪烁节奏 */
	}
}


/* ==================== 启动所有任务 ==================== */
/* 在主函数里调用一次，内部创建三个任务 */
void Task_Start(void)
{
	/* FreeRTOS 中数值越大优先级越高：报警 > 采集 > 显示 */
	xTaskCreate(Alarm_Task,   "Alarm",   128, NULL, 3, NULL);  /* 报警，优先级3(最高) */
	xTaskCreate(Collect_Task, "Collect", 256, NULL, 2, NULL);  /* 采集，优先级2 */
	xTaskCreate(Display_Task, "Display", 128, NULL, 1, NULL);  /* 显示，优先级1(最低) */
}
