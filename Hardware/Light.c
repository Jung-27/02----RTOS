#include "stm32f10x.h"
#include "Light.h"

/**
  * @brief  光敏传感器初始化：配置 AO 引脚为模拟输入并初始化 ADC1
  * @param  无
  * @retval 无
  * @note   与 MQ2 共用 ADC1，二者必须接不同通道；本驱动每次读取前会
  *         重新选择自己的通道，因此可与 MQ2 安全共存
  */
void Light_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef  ADC_InitStructure;

	/* 1. 使能 GPIO 与 ADC1 时钟，并设置 ADC 时钟(72MHz / 6 = 12MHz <= 14MHz) */
	RCC_APB2PeriphClockCmd(LIGHT_ADC_GPIO_CLK | RCC_APB2Periph_ADC1, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	/* 2. AO 引脚配置为模拟输入 */
	GPIO_InitStructure.GPIO_Pin  = LIGHT_ADC_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(LIGHT_ADC_GPIO_PORT, &GPIO_InitStructure);

	/* 3. 配置 ADC1：独立模式、单次转换、软件触发、右对齐 */
	ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode       = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel       = 1;
	ADC_Init(LIGHT_ADCx, &ADC_InitStructure);

	/* 4. 配置规则组采样通道与采样时间 */
	ADC_RegularChannelConfig(LIGHT_ADCx, LIGHT_ADC_CHANNEL, 1, ADC_SampleTime_239Cycles5);

	/* 5. 使能 ADC 并执行自校准 */
	ADC_Cmd(LIGHT_ADCx, ENABLE);

	ADC_ResetCalibration(LIGHT_ADCx);
	while (ADC_GetResetCalibrationStatus(LIGHT_ADCx));
	ADC_StartCalibration(LIGHT_ADCx);
	while (ADC_GetCalibrationStatus(LIGHT_ADCx));
}

/**
  * @brief  读取一次光敏传感器的 ADC 原始值(单次转换)
  * @param  无
  * @retval ADC 原始值(0 ~ 4095)
  */
uint16_t Light_GetADCValue(void)
{
	uint16_t adc;

	/* 每次读取前重新选择本传感器通道，以支持与 MQ2 共用 ADC1 */
	ADC_RegularChannelConfig(LIGHT_ADCx, LIGHT_ADC_CHANNEL, 1, ADC_SampleTime_239Cycles5);

	/* 关键：启动前清 EOC 标志，否则会读到上一通道的旧结果(串通道) */
	ADC_ClearFlag(LIGHT_ADCx, ADC_FLAG_EOC);

	/* 第一次转换丢弃：切换通道后第一次结果可能残留上一通道的电荷 */
	ADC_SoftwareStartConvCmd(LIGHT_ADCx, ENABLE);
	while (ADC_GetFlagStatus(LIGHT_ADCx, ADC_FLAG_EOC) == RESET);
	ADC_GetConversionValue(LIGHT_ADCx);
	ADC_ClearFlag(LIGHT_ADCx, ADC_FLAG_EOC);

	/* 第二次转换才是本次通道的真实值 */
	ADC_SoftwareStartConvCmd(LIGHT_ADCx, ENABLE);
	while (ADC_GetFlagStatus(LIGHT_ADCx, ADC_FLAG_EOC) == RESET);
	adc = ADC_GetConversionValue(LIGHT_ADCx);
	ADC_ClearFlag(LIGHT_ADCx, ADC_FLAG_EOC);

	return adc;
}

/**
  * @brief  连续采样 times 次并取平均值，用于平滑噪声
  * @param  times 采样次数，建议 8~32
  * @retval ADC 平均值(0 ~ 4095)
  */
uint16_t Light_GetAverageADCValue(uint16_t times)
{
	uint32_t sum = 0;
	uint16_t i;

	if (times == 0)
	{
		return Light_GetADCValue();
	}

	for (i = 0; i < times; i++)
	{
		sum += Light_GetADCValue();
	}

	return (uint16_t)(sum / times);
}

/**
  * @brief  获取光敏传感器模拟输出换算后的电压值
  * @param  无
  * @retval 电压值(V)
  */
float Light_GetVoltage(void)
{
	return (float)Light_GetADCValue() * LIGHT_VREF / LIGHT_ADC_RES;
}

/**
  * @brief  获取光照强度百分比
  * @param  无
  * @retval 亮度百分比(0 ~ 100)，数值越大表示越亮
  * @note   你的光敏模块是「越亮电压越低」的反相接法，此处已翻转：
  *         电压最低(最亮)→100，电压最高(最暗)→0。
  *         若换用「越亮电压越高」的模块，改回 adc*100/4096 即可。
  */
uint8_t Light_GetPercentage(void)
{
	uint16_t adc = Light_GetADCValue();

	if (adc >= LIGHT_ADC_RES - 1)
	{
		return 0;   /* 电压最高 = 最暗 */
	}
	return (uint8_t)(100 - (uint32_t)adc * 100 / LIGHT_ADC_RES);
}
