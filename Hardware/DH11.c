#include "stm32f10x.h"
#include "DH11.h"
#include "Delay.h"                          /* 工程自带的 Delay_us / Delay_ms */

/* 将数据线配置为推挽输出 */
static void DHT11_PinOut(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 使能 GPIO 时钟 */
	RCC_APB2PeriphClockCmd(DHT11_GPIO_CLK, ENABLE);

	GPIO_InitStructure.GPIO_Pin   = DHT11_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);
}

/* 将数据线配置为浮空输入 */
static void DHT11_PinIn(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(DHT11_GPIO_CLK, ENABLE);

	GPIO_InitStructure.GPIO_Pin  = DHT11_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);
}

/* 读取数据线当前电平 */
static uint8_t DHT11_ReadPin(void)
{
	return (uint8_t)GPIO_ReadInputDataBit(DHT11_GPIO_PORT, DHT11_GPIO_PIN);
}

/* 复位 DHT11：主机拉低至少 18ms，再释放 */
static void DHT11_Reset(void)
{
	DHT11_PinOut();
	GPIO_ResetBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN);   /* 拉低数据线 */
	Delay_ms(20);                                       /* 保持 18ms 以上 */
	GPIO_SetBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN);      /* 释放数据线 */
	Delay_us(30);                                       /* 等待 20~40us */
}

/* 等待 DHT11 应答：返回 0=有应答，1=无应答 */
static uint8_t DHT11_Check(void)
{
	uint8_t retry = 0;

	DHT11_PinIn();
	while (DHT11_ReadPin() && retry < 100)              /* DHT11 拉低 40~80us */
	{
		retry++;
		Delay_us(1);
	}
	if (retry >= 100) return 1;

	retry = 0;
	while (!DHT11_ReadPin() && retry < 100)             /* DHT11 拉高 40~80us */
	{
		retry++;
		Delay_us(1);
	}
	if (retry >= 100) return 1;

	return 0;
}

/* 读取一个位：返回 1 或 0 */
static uint8_t DHT11_ReadBit(void)
{
	uint8_t retry = 0;

	while (DHT11_ReadPin() && retry < 100)              /* 等待变为低电平 */
	{
		retry++;
		Delay_us(1);
	}
	retry = 0;
	while (!DHT11_ReadPin() && retry < 100)             /* 等待变为高电平 */
	{
		retry++;
		Delay_us(1);
	}
	Delay_us(40);                                       /* 等待 40us */
	if (DHT11_ReadPin()) return 1;                      /* 仍为高 => 数据 1 */
	return 0;                                           /* 已为低 => 数据 0 */
}

/* 读取一个字节(8 位) */
static uint8_t DHT11_ReadByte(void)
{
	uint8_t i, dat = 0;

	for (i = 0; i < 8; i++)
	{
		dat <<= 1;
		if (DHT11_ReadBit()) dat |= 1;
	}
	return dat;
}

/**
  * @brief  DHT11 初始化并检测是否存在
  * @param  无
  * @retval 0: 检测到 DHT11；1: 未检测到
  */
uint8_t DHT11_Init(void)
{
	DHT11_Reset();
	return DHT11_Check();
}

/**
  * @brief  读取一次温湿度数据
  * @param  temp: 温度输出(℃，int8 类型，支持负数)
  * @param  humi: 湿度输出(%RH)
  * @retval 0: 成功；1: 失败(无应答或校验错误)
  * @note   两次读取之间需间隔 1 秒以上；上电后需稳定约 1 秒再读取
  */
uint8_t DHT11_ReadData(int8_t *temp, uint8_t *humi)
{
	uint8_t buf[5];
	uint8_t i;

	DHT11_Reset();
	if (DHT11_Check() != 0) return 1;                   /* 无应答 */

	for (i = 0; i < 5; i++)                             /* 读取 40 位数据 */
	{
		buf[i] = DHT11_ReadByte();
	}

	/* 校验：前四字节之和 == 第五字节(校验和) */
	if ((uint8_t)(buf[0] + buf[1] + buf[2] + buf[3]) != buf[4]) return 1;

	*humi = buf[0];                                     /* 湿度整数(0~100) */
	*temp = (int8_t)buf[2];                             /* 温度整数，int8 可表示负数 */
	return 0;
}
