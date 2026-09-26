#ifndef __MQ2_H
#define __MQ2_H

#include "stm32f10x.h"

/* ==================== 硬件连接配置(按实际接线修改) ==================== */
/* MQ2 模块模拟量输出(AO) 所接的 ADC 引脚，PA0 -> ADC1_Channel0             */
#define MQ2_ADC_GPIO_PORT    GPIOA
#define MQ2_ADC_GPIO_PIN     GPIO_Pin_0
#define MQ2_ADC_GPIO_CLK     RCC_APB2Periph_GPIOA
#define MQ2_ADCx             ADC1
#define MQ2_ADC_CHANNEL      ADC_Channel_0

/* ==================== 检测换算参数 ==================== */
#define MQ2_VREF        3.3f     /* ADC 参考电压 (V)                        */
#define MQ2_ADC_RES     4096     /* 12 位 ADC 满量程                        */

/* 气体浓度百分比换算基准(线性近似，现已由 PPM 取代，仅供参考)              */
#define MQ2_V_CLEAN     1.00f    /* 洁净空气 AO 分压后电压(V)，实际用 MQ2_Calibrate 标定 */
#define MQ2_V_FULL      3.00f    /* 满量程气体浓度对应分压后电压(V)，记 100% */

/* ==================== PPM 换算参数(对数曲线) ==================== */
/* 公式: ppm = A * (Rs/Ro)^B，Rs/Ro 由电压分压比算出                        */
/* A、B 依据 datasheet 图3「灵敏度特性曲线」200/1000 ppm 两点拟合            */
#define MQ2_VCC         3.21f    /* 分压后 ADC 满量程电压(V)。模块实际 4.8V 供电， */
                                 /* AO 经约 2/3 分压(10k+20k)后满量程≈3.3V  */
#define MQ2_RL          4700     /* 模块负载电阻(Ω)，datasheet 图5 标注 4.7k  */

#define MQ2_CURVE_A_LPG      17.5f    /* 液化气 / 丙烷 */
#define MQ2_CURVE_B_LPG      -1.76f
#define MQ2_CURVE_A_CH4      12.2f    /* 甲烷 / 天然气 */
#define MQ2_CURVE_B_CH4      -2.32f
#define MQ2_CURVE_A_SMOKE    11.8f    /* 烟雾 */
#define MQ2_CURVE_B_SMOKE    -1.76f
#define MQ2_CURVE_A_ALCOHOL  27.2f    /* 酒精 / 乙醇 */
#define MQ2_CURVE_B_ALCOHOL  -1.90f

/* 当前检测的目标气体(四选一，改下面两行即可切换) */
#define MQ2_CURVE_A   MQ2_CURVE_A_SMOKE
#define MQ2_CURVE_B   MQ2_CURVE_B_SMOKE

#define MQ2_CALI_SAMPLES 50      /* 标定时取平均的采样次数                 */

void     MQ2_Init(void);
uint16_t MQ2_GetADCValue(void);
uint16_t MQ2_GetAverageADCValue(uint16_t times);
float    MQ2_GetVoltage(void);
uint8_t  MQ2_GetPercentage(void);
float    MQ2_GetPPM(void);
void     MQ2_Calibrate(void);

#endif /* __MQ2_H */
