# 倒立摆 (Inverted Pendulum)

基于 STM32F103C8T6 的串级 PID 倒立摆控制系统，支持自动起摆、手动 PID 控制和强制旋转一圈功能。

## 硬件

| 模块 | 型号/参数 | 说明 |
|------|----------|------|
| MCU | STM32F103C8T6 | 72MHz, 64KB Flash |
| 角度传感器 | 电位器 (ADC1 Ch8) | 摆杆角度 0~360° |
| 电机 | 直流减速电机 | PWM 驱动, TB6612 |
| 编码器 | 102 线 × 4× = 408 CPR | TIM3 编码器模式 |
| 显示 | SSD1306 128×64 OLED | I2C 接口 |
| 按键 | 4 个 | PB10/PB11/PA11/PA12 |

## 编译

CMake + ARM GCC (`arm-none-eabi-gcc`), CLion 打开即用。

```
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -B build -S .
cmake --build build
```

> 也支持 clang: 换 `cmake/starm-clang.cmake` 即可。

## 按键功能

| 按键 | 功能 |
|------|------|
| **Btn1** | **手动模式** — 手扶着摆杆到竖直位置，按一下直接进 PID。再按停机 |
| **Btn2** | **转一圈** — 自动谐振推摆，摆杆进入中心区间后推一把，靠惯性转一圈后停机 |
| **Btn3** | **位置控制** — 短按 `-360°`（反转一圈），长按 `+360°`（正转一圈），仅 PID 模式下生效 |
| **Btn4** | **自动起摆** — 先踹一脚，然后谐振推摆判断，进入中心区间后自动切换 PID 控制 |

> 任意按键在运行状态下按，都会回到停止状态。

## 状态机

OLED 左上角显示当前状态码 `St:XX`。

```
  0    停止
  1    判断（自由摆动，检测峰值，谐振推摆）
 21~24 左推序列 → 回 1
 31~34 右推序列 → 回 1
  4    PID 控制
  5→6  转圈最后一推 → 0
```

### 起摆流程 (Btn4)

```
State 0 ─→ 初次踹一脚 (21→22→23→24) ─→ State 1
            │                              │
            └── 右峰→21... / 左峰→31... ──┘  循环摆荡
                                              
            ── 进中心 → PID (4)
            ── 倒下   → 停止 (0)
```

### 转圈流程 (Btn2)

```
State 0 ─→ 初次踹一脚 ─→ State 1 谐振推摆
                            │
                            └→ 进中心 → 最后一推 (5→6) → 停止 (0)
```

## OLED 显示

```
┌──────────────────────────────┐
│ Angle      XX  │ Location    │     XX = 状态码
│ Kp:04.000      │ Kp:00.050   │
│ Ki:00.114      │ Ki:00.010   │
│ Kd:04.550      │ Kd:00.500   │
│ Tar:0179       │ Tar:0.00    │
│ Act:0179       │ Act:0.00    │
└──────────────────────────────┘
```

## PID 参数

| 参数 | 角度环 | 位置环 | 说明 |
|------|--------|--------|------|
| Kp | 2.28 | 0.05 | 比例 |
| Ki | 0.114 | 0.01 | 积分 |
| Kd | 4.55 | 0.5 | 微分 |
| Out | ±100 | ±6° | 输出限幅 |
| i_limit | 200 | 30 | 积分限幅 |
| Target | 179° | 0 | 目标值 |



## 控制频率

| 回路 | 频率 | 说明 |
|------|------|------|
| 角度环 | **200 Hz** | 内环，角度 PID → PWM |
| 位置环 | **20 Hz** | 外环，位置 PID → 修正角度目标 |
| 判断采样 | **25 Hz** | 40ms 采一次，三样本滑动窗口检测峰值 |
| ISR 基频 | **1 kHz** | TIM1 溢出中断 |

## 串级结构

```
编码器位置 ─→ [位置 PID, 20Hz] ─→ 角度偏置(+)
                                    │
                                    ▼
                        anglePID.target = 179° + 偏置
                                    │
角度传感器 ─→ [角度 PID, 200Hz] ─→ PWM ─→ 电机
```

## 起摆原理（谐振能量注入法）

在摆杆摆到最高点时顺着运动方向给短脉冲推力：

1. 每 40ms 采样角度，三样本滑动窗口判断运动方向
2. 中间样本最小 → 右侧最高点，往左推
3. 中间样本最大 → 左侧最高点，往右推
4. 推 100ms → 反推 100ms 收回，每次净注入能量刚好够振幅增大
5. 进入中心区间后自动切 PID 接管

## 可调参数

编辑 `User/Src/swing_up.cpp` 顶部：

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `CENTER_ANGLE` | 179.0 | 平衡点角度 |
| `CENTER_RANGE` | 30.0 | 中心区间 ±30° |
| `START_PWM` | 35 | 谐振推力 (30~40) |
| `START_TIME` | 100 | 推摆时长 ms (80~120) |
| `FINAL_PWM` | 60 | 转圈最后一推 |
| `FINAL_TIME` | 150 | 转圈最后一推时长 |

## 文件结构

```
├── CMakeLists.txt
├── README.md
├── cmake/
│   ├── gcc-arm-none-eabi.cmake
│   ├── starm-clang.cmake
│   └── stm32cubemx/           STM32CubeMX 生成的 CMake
├── Core/
│   ├── Inc/
│   │   ├── adc.h              ADC 初始化
│   │   ├── dma.h              DMA 初始化
│   │   ├── gpio.h             GPIO 初始化
│   │   ├── i2c.h              I2C 初始化
│   │   ├── main.h             主程序头文件
│   │   ├── tim.h              定时器初始化
│   │   ├── usart.h            串口初始化
│   │   ├── stm32f1xx_hal_conf.h
│   │   └── stm32f1xx_it.h    中断处理声明
│   └── Src/
│       ├── main.c             主循环 + OLED 显示
│       ├── adc.c
│       ├── dma.c
│       ├── gpio.c
│       ├── i2c.c
│       ├── tim.c              定时器配置 (TIM1 ISR / TIM2 PWM / TIM3 Encoder)
│       ├── usart.c
│       ├── stm32f1xx_hal_msp.c
│       ├── stm32f1xx_it.c     中断向量 (TIM1 / TIM3 / DMA)
│       ├── system_stm32f1xx.c 系统时钟
│       ├── syscalls.c
│       └── sysmem.c
└── User/
    ├── Inc/
    │   ├── global.hpp         全局变量声明
    │   ├── swing_up.hpp       状态机头文件
    │   ├── pid.hpp            PID 控制器类
    │   ├── motor.hpp          电机类
    │   ├── degree_sensor.hpp  角度传感器类
    │   ├── button.hpp         按键类
    │   ├── uart_object.hpp    串口输出
    │   ├── lqr.hpp            LQR 控制器 (实验)
    │   ├── delay_us.h         微秒延时
    │   ├── ssd1306.h          OLED 驱动
    │   ├── ssd1306_conf.h     OLED 配置
    │   ├── ssd1306_fonts.h    字体
    │   └── ssd1306_tests.h    屏幕测试
    └── Src/
        ├── global.cpp         全局变量 + 按钮事件
        ├── interrupt.cpp      ISR 薄层转发
        ├── swing_up.cpp       起摆/转圈/PID 状态机
        ├── pid.cpp            PID 控制器 (Q12 定点)
        ├── motor.cpp          电机 (PWM + 编码器)
        ├── degree_sensor.cpp  角度传感器 (ADC DMA + 低通滤波)
        ├── uart_object.cpp    串口输出
        ├── lqr.cpp            LQR 控制器 (实验)
        ├── delay_us.c         微秒延时
        ├── ssd1306.c          OLED 驱动
        ├── ssd1306_fonts.c    字体
        └── ssd1306_tests.c    屏幕测试
```
