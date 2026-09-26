#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"

#include "LED.h"
#include "Buzz.h"
#include "Motor.h"
#include "MQ2.h"
#include "Light.h"
#include "DH11.h"
#include "OLED.h"
#include "TaskList.h"
#include "Delay.h"

int main(void)
{
	/* FreeRTOS 要求：中断优先级分组设为 4 位抢占优先级 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* 各外设初始化 */
	LED_Init();
	Buzz_Init();
	Motor_Init();      /* 内部会调用 PWM_Init() */
	MQ2_Init();
	Light_Init();
	DHT11_Init();
	OLED_Init();       /* 内部已清屏 */

	/* MQ2 预热约 20 秒，然后在干净空气里标定一次基准电压(需无烟环境上电) */
	Delay_s(20);
	MQ2_Calibrate();

	/* 创建三个应用任务(采集/显示/报警) */
	Task_Start();

	/* 启动调度器，正常情况不会返回 */
	vTaskStartScheduler();

	/* 调度器因故返回(比如内存不够)才走到这里 */
	while (1)
	{
	}
}