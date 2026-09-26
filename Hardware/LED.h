 #ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

/* ==================== 硬件连接配置 ==================== */
/* LED1、LED2 在 GPIOB；LED3 在 GPIOA(PA7)                            */
#define LED1_GPIO_PORT   GPIOB
#define LED1_GPIO_PIN    GPIO_Pin_0
#define LED2_GPIO_PORT   GPIOB
#define LED2_GPIO_PIN    GPIO_Pin_1
#define LED3_GPIO_PORT   GPIOA
#define LED3_GPIO_PIN    GPIO_Pin_7

/* 点亮电平：1=高电平亮(IO→电阻→LED→GND)；0=低电平亮(3.3V→LED→电阻→IO) */
#define LED_ON_LEVEL     1

void LED_Init(void);

void LED1_Set(uint8_t state);   /* 1=亮，0=灭 */
void LED2_Set(uint8_t state);
void LED3_Set(uint8_t state);

void LED1_Toggle(void);         /* 翻转(亮↔灭) */
void LED2_Toggle(void);
void LED3_Toggle(void);

#endif /* __LED_H */
