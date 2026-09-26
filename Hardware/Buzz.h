#ifndef __BUZZ_H
#define __BUZZ_H

#include "stm32f10x.h"

/* ==================== 硬件连接配置 ==================== */
#define BUZZ_GPIO_PORT   GPIOA
#define BUZZ_GPIO_PIN    GPIO_Pin_8
#define BUZZ_GPIO_CLK    RCC_APB2Periph_GPIOA

/* 有源蜂鸣器触发极性：1=高电平响(高电平触发)，0=低电平响(低电平触发) */
#define BUZZ_ON_LEVEL    0

void Buzz_Init(void);
void Buzz_ON(void);       /* 蜂鸣器响 */
void Buzz_OFF(void);      /* 蜂鸣器停 */
void Buzz_Toggle(void);   /* 翻转(响↔停) */

#endif /* __BUZZ_H */
