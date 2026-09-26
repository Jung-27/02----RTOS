#include "stm32f10x.h"
#include "MQ2.h"
#include <math.h>                       /* powf() 对数曲线换算 */

/**
  * @brief  MQ2 初始化：配置 AO 引脚为模拟输入并初始化 ADC1
  * @param  无
  * @retval 无
  * @note   上电后 MQ2 需预热约 20 秒，读数才趋于稳定
  */
void MQ2_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef  ADC_InitStructure;

	/* 1. 使能 GPIO 与 ADC1 时钟，并设置 ADC 时钟(72MHz / 6 = 12MHz <= 14MHz) */
	RCC_APB2PeriphClockCmd(MQ2_ADC_GPIO_CLK | RCC_APB2Periph_ADC1, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	/* 2. AO 引脚配置为模拟输入 */
	GPIO_InitStructure.GPIO_Pin  = MQ2_ADC_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(MQ2_ADC_GPIO_PORT, &GPIO_InitStructure);

	/* 3. 配置 ADC1：独立模式、单次转换、软件触发、右对齐 */
	ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode       = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel       = 1;
	ADC_Init(MQ2_ADCx, &ADC_InitStructure);

	/* 4. 配置规则组采样通道与采样时间(采样时间越长越稳定) */
	ADC_RegularChannelConfig(MQ2_ADCx, MQ2_ADC_CHANNEL, 1, ADC_SampleTime_239Cycles5);

	/* 5. 使能 ADC 并执行自校准 */
	ADC_Cmd(MQ2_ADCx, ENABLE);

	ADC_ResetCalibration(MQ2_ADCx);
	while (ADC_GetResetCalibrationStatus(MQ2_ADCx));
	ADC_StartCalibration(MQ2_ADCx);
	while (ADC_GetCalibrationStatus(MQ2_ADCx));
}

/**
  * @brief  读取一次 MQ2 的 ADC 原始值(单次转换)
  * @param  无
  * @retval ADC 原始值(0 ~ 4095)
  */
uint16_t MQ2_GetADCValue(void)
{
	uint16_t adc;

	/* 每次读取前重新选择本传感器通道，以支持与光敏等其它传感器共用 ADC1 */
	ADC_RegularChannelConfig(MQ2_ADCx, MQ2_ADC_CHANNEL, 1, ADC_SampleTime_239Cycles5);

	/* 关键：启动前清 EOC 标志，否则会读到上一通道的旧结果(串通道) */
	ADC_ClearFlag(MQ2_ADCx, ADC_FLAG_EOC);

	/* 第一次转换丢弃：切换通道后第一次结果可能残留上一通道的电荷 */
	ADC_SoftwareStartConvCmd(MQ2_ADCx, ENABLE);
	while (ADC_GetFlagStatus(MQ2_ADCx, ADC_FLAG_EOC) == RESET);
	ADC_GetConversionValue(MQ2_ADCx);
	ADC_ClearFlag(MQ2_ADCx, ADC_FLAG_EOC);

	/* 第二次转换才是本次通道的真实值 */
	ADC_SoftwareStartConvCmd(MQ2_ADCx, ENABLE);
	while (ADC_GetFlagStatus(MQ2_ADCx, ADC_FLAG_EOC) == RESET);
	adc = ADC_GetConversionValue(MQ2_ADCx);
	ADC_ClearFlag(MQ2_ADCx, ADC_FLAG_EOC);

	return adc;
}

/**
  * @brief  连续采样 times 次并取平均值，用于平滑噪声
  * @param  times 采样次数，建议 8~32
  * @retval ADC 平均值(0 ~ 4095)
  */
uint16_t MQ2_GetAverageADCValue(uint16_t times)
{
	uint32_t sum = 0;
	uint16_t i;

	if (times == 0)
	{
		return MQ2_GetADCValue();
	}

	for (i = 0; i < times; i++)
	{
		sum += MQ2_GetADCValue();
	}

	return (uint16_t)(sum / times);
}

/**
  * @brief  获取 MQ2 模拟输出换算后的电压值
  * @param  无
  * @retval 电压值(V)
  */
float MQ2_GetVoltage(void)
{
	return (float)MQ2_GetADCValue() * MQ2_VREF / MQ2_ADC_RES;
}

/**
  * @brief  获取气体浓度百分比(线性近似)
  * @param  无
  * @retval 浓度百分比(0 ~ 100)
  * @note   以 MQ2_V_CLEAN / MQ2_V_FULL 为基准线性映射，实际气体浓度
  *         与电压为对数关系，精确值需依据 Rs/Ro 曲线标定
  */
uint8_t MQ2_GetPercentage(void)
{
	float voltage = MQ2_GetVoltage();

	if (voltage <= MQ2_V_CLEAN)
	{
		return 0;
	}
	if (voltage >= MQ2_V_FULL)
	{
		return 100;
	}

	return (uint8_t)((voltage - MQ2_V_CLEAN) / (MQ2_V_FULL - MQ2_V_CLEAN) * 100.0f);
}

/* 洁净空气基准电压(即 Ro 对应的分压输出)，可由 MQ2_Calibrate() 更新 */
static float MQ2_cleanVoltage = MQ2_V_CLEAN;

/**
  * @brief  标定：在洁净空气中采集多次电压，作为 Rs/Ro = 1 的基准(Ro)
  * @param  无
  * @retval 无
  * @note   需在无目标气体的洁净环境下调用一次，之后 PPM 换算才准确
  */
void MQ2_Calibrate(void)
{
	float v = (float)MQ2_GetAverageADCValue(MQ2_CALI_SAMPLES) * MQ2_VREF / MQ2_ADC_RES;

	if (v > 0.0f && v < MQ2_VCC)
	{
		MQ2_cleanVoltage = v;
	}
}

/**
  * @brief  获取可燃气体浓度 PPM 值(基于 Rs/Ro 对数曲线)
  * @param  无
  * @retval 气体浓度(ppm)
  * @note   公式: ppm = A * (Rs/Ro)^B
  *         Rs/Ro = (Vcc/Vout - 1) / (Vcc/Vout_clean - 1)，RL 在比值中约去
  *         洁净空气中 Rs/Ro≈1，会得到约等于 A 的基线值，属正常现象；
  *         报警阈值应设为明显高于该基线的值(如 500~1000)
  */
float MQ2_GetPPM(void)
{
	float voltage = MQ2_GetVoltage();
	float ratio;

	/* 电压限幅，防止除零或对数越界 */
	if (voltage < 0.05f)
	{
		voltage = 0.05f;
	}
	if (voltage > MQ2_VCC - 0.05f)
	{
		voltage = MQ2_VCC - 0.05f;
	}

	ratio = (MQ2_VCC / voltage - 1.0f) / (MQ2_VCC / MQ2_cleanVoltage - 1.0f);

	return MQ2_CURVE_A * powf(ratio, MQ2_CURVE_B);
}
