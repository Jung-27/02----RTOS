#ifndef __LIGHT_H
#define __LIGHT_H

#include "stm32f10x.h"

/* ==================== 硬件连接配置(按实际接线修改) ==================== */
/* 光敏传感器模拟量输出(AO) 所接的 ADC 引脚，默认 PA1 -> ADC1_Channel1    */
/* 注意：与 MQ2 共用 ADC1，二者必须接在不同通道(MQ2 用 PA0/通道0)        */
#define LIGHT_ADC_GPIO_PORT    GPIOA
#define LIGHT_ADC_GPIO_PIN     GPIO_Pin_1
#define LIGHT_ADC_GPIO_CLK     RCC_APB2Periph_GPIOA
#define LIGHT_ADCx             ADC1
#define LIGHT_ADC_CHANNEL      ADC_Channel_1

/* ==================== 检测换算参数 ==================== */
#define LIGHT_VREF        3.3f    /* ADC 参考电压 (V)                        */
#define LIGHT_ADC_RES     4096    /* 12 位 ADC 满量程                        */

void     Light_Init(void);
uint16_t Light_GetADCValue(void);
uint16_t Light_GetAverageADCValue(uint16_t times);
float    Light_GetVoltage(void);
uint8_t  Light_GetPercentage(void);

#endif /* __LIGHT_H */
