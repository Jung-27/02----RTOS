#include "stm32f10x.h"
#include "LED.h"

/* 内部函数：设置一个 LED 亮/灭，自动处理高低电平极性 */
static void LED_Write(GPIO_TypeDef *GPIOx, uint16_t Pin, uint8_t State)
{
#if LED_ON_LEVEL == 1          /* 高电平点亮 */
	GPIO_WriteBit(GPIOx, Pin, (BitAction)State);
#else                          /* 低电平点亮 */
	GPIO_WriteBit(GPIOx, Pin, (BitAction)(!State));
#endif
}

/* 内部函数：翻转一个 LED */
static void LED_Toggle(GPIO_TypeDef *GPIOx, uint16_t Pin)
{
	if (GPIO_ReadOutputDataBit(GPIOx, Pin) == 0)
		GPIO_WriteBit(GPIOx, Pin, Bit_SET);
	else
		GPIO_WriteBit(GPIOx, Pin, Bit_RESET);
}

void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* LED1、LED2 用 GPIOB，LED3(PA7) 用 GPIOA，两个端口的时钟都要开 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;   /* 推挽输出 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	/* LED1、LED2：都在 GPIOB，一次初始化 */
	GPIO_InitStructure.GPIO_Pin = LED1_GPIO_PIN | LED2_GPIO_PIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* LED3：在 GPIOA(PA7)，不同端口必须单独初始化一次 */
	GPIO_InitStructure.GPIO_Pin = LED3_GPIO_PIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	LED1_Set(0);   /* 初始全部熄灭 */
	LED2_Set(0);
	LED3_Set(0);
}

void LED1_Set(uint8_t state) { LED_Write(LED1_GPIO_PORT, LED1_GPIO_PIN, state); }
void LED2_Set(uint8_t state) { LED_Write(LED2_GPIO_PORT, LED2_GPIO_PIN, state); }
void LED3_Set(uint8_t state) { LED_Write(LED3_GPIO_PORT, LED3_GPIO_PIN, state); }

void LED1_Toggle(void) { LED_Toggle(LED1_GPIO_PORT, LED1_GPIO_PIN); }
void LED2_Toggle(void) { LED_Toggle(LED2_GPIO_PORT, LED2_GPIO_PIN); }
void LED3_Toggle(void) { LED_Toggle(LED3_GPIO_PORT, LED3_GPIO_PIN); }
