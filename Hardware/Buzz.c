#include "stm32f10x.h"
#include "Buzz.h"

void Buzz_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(BUZZ_GPIO_CLK, ENABLE);

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;   /* 推挽输出 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = BUZZ_GPIO_PIN;
	GPIO_Init(BUZZ_GPIO_PORT, &GPIO_InitStructure);

	Buzz_OFF();   /* 初始关闭 */
}

void Buzz_ON(void)
{
#if BUZZ_ON_LEVEL == 1
	GPIO_SetBits(BUZZ_GPIO_PORT, BUZZ_GPIO_PIN);   /* 高电平响 */
#else
	GPIO_ResetBits(BUZZ_GPIO_PORT, BUZZ_GPIO_PIN);
#endif
}

void Buzz_OFF(void)
{
#if BUZZ_ON_LEVEL == 1
	GPIO_ResetBits(BUZZ_GPIO_PORT, BUZZ_GPIO_PIN);
#else
	GPIO_SetBits(BUZZ_GPIO_PORT, BUZZ_GPIO_PIN);
#endif
}

void Buzz_Toggle(void)
{
	if (GPIO_ReadOutputDataBit(BUZZ_GPIO_PORT, BUZZ_GPIO_PIN) == 0)
		GPIO_SetBits(BUZZ_GPIO_PORT, BUZZ_GPIO_PIN);
	else
		GPIO_ResetBits(BUZZ_GPIO_PORT, BUZZ_GPIO_PIN);
}
