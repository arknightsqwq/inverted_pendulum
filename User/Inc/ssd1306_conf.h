/**
 * @brief SSD1306 显示屏驱动 — 项目配置
 *
 * STM32F1, I2C 模式
 */

#ifndef SSD1306_CONF_H
#define SSD1306_CONF_H

#include "main.h"

// ============================================================================
// MCU 系列选择
// ============================================================================

// #define STM32F0
#define STM32F1
// #define STM32F4
// #define STM32L0
// #define STM32L1
// #define STM32L4
// #define STM32F3
// #define STM32H7
// #define STM32F7
// #define STM32G0
// #define STM32C0

// ============================================================================
// 通信接口选择（三选一）
// ============================================================================

#define SSD1306_USE_I2C
// #define SSD1306_USE_SW_I2C
// #define SSD1306_USE_SPI

// --- 硬件 I2C 配置 ---
#define SSD1306_I2C_PORT hi2c1
#define SSD1306_I2C_ADDR (0x3C << 1)

// --- 软件 I2C 配置（仅当使用 SSD1306_USE_SW_I2C 时需要） ---
// 使用开漏输出 + 内部/外部上拉电阻
#define SSD1306_SW_I2C_SCL_Port   GPIOB
#define SSD1306_SW_I2C_SCL_Pin    GPIO_PIN_6
#define SSD1306_SW_I2C_SDA_Port   GPIOB
#define SSD1306_SW_I2C_SDA_Pin    GPIO_PIN_7
// 半周期延时循环次数（需根据 MCU 主频调节，目标 ~5us → 100kHz）
// STM32F103 @72MHz: ~72  → 实测接近 100kHz
#define SSD1306_SW_I2C_DELAY_COUNT 72

// --- SPI 配置 ---
// #define SSD1306_SPI_PORT    hspi1
// #define SSD1306_CS_Port     OLED_CS_GPIO_Port
// #define SSD1306_CS_Pin      OLED_CS_Pin
// #define SSD1306_DC_Port     OLED_DC_GPIO_Port
// #define SSD1306_DC_Pin      OLED_DC_Pin
// #define SSD1306_Reset_Port  OLED_Res_GPIO_Port
// #define SSD1306_Reset_Pin   OLED_Res_Pin

// ============================================================================
// 可选的屏幕调整
// ============================================================================

#define SSD1306_MIRROR_VERT
#define SSD1306_MIRROR_HORIZ
// #define SSD1306_INVERSE_COLOR
// #define SSD1306_X_OFFSET
// #define SSD1306_WIDTH  64
// #define SSD1306_HEIGHT 64

// ============================================================================
// 选择需要编译的字体
// ============================================================================

#define SSD1306_INCLUDE_FONT_6x8
#define SSD1306_INCLUDE_FONT_7x10
#define SSD1306_INCLUDE_FONT_11x18
#define SSD1306_INCLUDE_FONT_16x26
#define SSD1306_INCLUDE_FONT_16x24
#define SSD1306_INCLUDE_FONT_16x15

#endif // SSD1306_CONF_H
