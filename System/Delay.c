#include "stm32f10x.h"

/* =============================================================
 * 用 Cortex-M3 的 DWT 周期计数器做微秒/毫秒级延时。
 * - 不占用 TIM4，也避开 FreeRTOS 占用的 SysTick，互不冲突。
 * - DWT->CYCCNT 按系统时钟(72MHz)自增，1us = 72 个计数。
 * - 用无符号减法处理 32 位计数器回绕，最长可延时约 59 秒。
 * ============================================================= */

/* DWT / DEMCR 寄存器地址(Cortex-M3 固定地址) */
#define DWT_CONTROL  (*(volatile uint32_t *)0xE0001000)   /* DWT 控制寄存器   */
#define DWT_CYCCNT   (*(volatile uint32_t *)0xE0001004)   /* DWT 周期计数器   */
#define DEMCR        (*(volatile uint32_t *)0xE000EDFC)   /* 调试异常监视寄存器 */

static uint8_t Delay_Ready = 0;

static void Delay_Init(void)
{
	DEMCR       |= 0x01000000;   /* TRCENA：使能 DWT 跟踪 */
	DWT_CYCCNT   = 0;
	DWT_CONTROL |= 0x00000001;   /* CYCCNTENA：使能周期计数器 */
	Delay_Ready  = 1;
}

/**
  * @brief  微秒级延时(基于 DWT 周期计数器)
  * @param  xus 延时时长(微秒)
  */
void Delay_us(uint32_t xus)
{
	uint32_t start, ticks;

	if (!Delay_Ready) Delay_Init();

	ticks = xus * (SystemCoreClock / 1000000UL);   /* 72MHz → 72 计数/us */
	start = DWT_CYCCNT;
	while ((uint32_t)(DWT_CYCCNT - start) < ticks);
}

/**
  * @brief  毫秒级延时
  * @param  xms 延时时长(毫秒)
  */
void Delay_ms(uint32_t xms)
{
	while (xms--)
	{
		Delay_us(1000);
	}
}

/**
  * @brief  秒级延时
  * @param  xs 延时时长(秒)
  */
void Delay_s(uint32_t xs)
{
	while (xs--)
	{
		Delay_ms(1000);
	}
}