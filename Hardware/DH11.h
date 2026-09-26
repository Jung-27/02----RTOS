#ifndef __DH11_H
#define __DH11_H

#include "stm32f10x.h"

/* ==================== 硬件连接配置(按实际接线修改) ==================== */
/* DHT11 数据线(DATA) 所接的 GPIO 引脚，默认 PB12                         */
#define DHT11_GPIO_PORT    GPIOB
#define DHT11_GPIO_PIN     GPIO_Pin_12
#define DHT11_GPIO_CLK     RCC_APB2Periph_GPIOB

uint8_t DHT11_Init(void);                             /* 初始化并检测，返回 0=检测到，1=未检测到 */
uint8_t DHT11_ReadData(int8_t *temp, uint8_t *humi);  /* 读取温湿度，返回 0=成功，1=失败 */

#endif /* __DH11_H */
