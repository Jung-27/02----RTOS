#include "stm32f10x.h"                  // Device header

/* PWM 输出初始化：PA2 -> TIM2_CH3，频率 10kHz，用于 TB6612 电机调速 */
void PWM_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);   /* 定时器2时钟(挂APB1) */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  /* PA2 时钟(挂APB2) */

	TIM_InternalClockConfig(TIM2);                          /* 内部时钟源 72MHz */

	/* PA2 配置为复用推挽输出(TIM2_CH3) */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;       /* 复用推挽输出 */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;            /* PA2 → TIM2_CH3 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 定时器基本配置：72MHz / 72 = 1MHz，再 / 100 = 10kHz */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode       = TIM_CounterMode_Up;  /* 向上计数 */
	TIM_TimeBaseInitStructure.TIM_Period            = 100 - 1;   /* ARR，决定频率 10kHz */
	TIM_TimeBaseInitStructure.TIM_Prescaler         = 72 - 1;    /* 预分频 */
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

	/* PWM 输出配置(通道3) */
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);                /* 按默认值初始化 */
	TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse       = 0;               /* 初始占空比 0(停止) */
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);               /* CH3 → PA2 输出 PWM */

	TIM_Cmd(TIM2, ENABLE);                                 /* 启动定时器 */
}

/* 设置电机转速：compare 范围 0~99，0=停止，99=全速 */
void PWM_SetCompare3(uint16_t compare)
{
	TIM_SetCompare3(TIM2, compare);
}
