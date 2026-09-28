# 智能家居 RTOS 项目

基于 **STM32F103C8** + **FreeRTOS** 的模拟智能家居环境监测与报警系统。

## 功能特性

实时采集环境数据并显示，超阈值时自动报警：

| 传感器 | 检测内容 | 超限动作 |
|--------|---------|---------|
| MQ2 | 烟雾浓度（ppm） | LED2 闪烁 |
| 光敏传感器 | 光照亮度（0~100%） | LED1 点亮 |
| DHT11 | 温度（℃） | 蜂鸣器响 + 风扇转动降温 |
| DHT11 | 湿度（0~100%） | LED3 闪烁 |

所有数据通过 **OLED 屏** 四行实时显示（烟雾 / 光照 / 温度 / 湿度）。

## 软件架构

FreeRTOS 三个任务，数值越大优先级越高：

| 任务 | 优先级 | 周期 | 职责 |
|------|:-----:|:----:|------|
| `Alarm` | 3（最高） | 200ms | 根据报警标志驱动 LED / 蜂鸣器 / 风扇 |
| `Collect` | 2 | 1s | 读取所有传感器并更新报警标志 |
| `Display` | 1 | 500ms | OLED 显示四项数据 |

## 报警阈值

定义在 `Hardware/TaskList.c` 顶部，可按实际环境调整：

```c
#define SMOKE_THRESHOLD   300   // 烟雾浓度上限(ppm)
#define LIGHT_THRESHOLD   25    // 光照下限(低于点亮LED1)
#define TEMP_THRESHOLD    33    // 温度上限(℃)
#define HUMI_THRESHOLD    70    // 湿度上限(%)
```

## 目录结构

```
├── User/          # 主程序 main.c、中断、配置
├── Hardware/      # 外设驱动与任务(MQ2/LED/OLED/DHT11/蜂鸣器/电机/PWM)
├── FreeRTOS/      # FreeRTOS 内核源码与移植
├── Library/       # STM32 标准外设库
├── Start/         # 启动文件、内核文件
├── System/        # 系统延时
└── DebugConfig/   # 调试配置
```

## 开发环境

- 芯片：STM32F103C8（C8T6）
- RTOS：FreeRTOS
- 标准库：STM32 标准外设库
- 工具链：Keil MDK / EIDE
- 语言：C

## 使用说明

1. 上电后 MQ2 预热约 20 秒，并在干净空气（无烟）中自动标定基准电压。
2. 采集 → 显示 → 报警循环运行，OLED 实时显示环境数据。
3. 触发阈值时对应 LED 闪烁 / 蜂鸣器报警 / 风扇降温。
