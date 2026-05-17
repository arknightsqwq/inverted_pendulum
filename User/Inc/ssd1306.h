/**
 * @brief SSD1306 OLED 显示屏驱动库
 *
 * 基于 afiskon/stm32-ssd1306 移植
 * https://github.com/afiskon/stm32-ssd1306
 *
 * 支持 I2C / SPI 双协议
 */

#ifndef SSD1306_H
#define SSD1306_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "ssd1306_conf.h"

// ============================================================================
// MCU 系列选择 — 根据 ssd1306_conf.h 中定义的宏引入对应 HAL 头文件
// ============================================================================

#if defined(STM32WB)
#include "stm32wbxx_hal.h"
#elif defined(STM32F0)
#include "stm32f0xx_hal.h"
#elif defined(STM32F1)
#include "stm32f1xx_hal.h"
#elif defined(STM32F4)
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#elif defined(STM32L0)
#include "stm32l0xx_hal.h"
#elif defined(STM32L1)
#include "stm32l1xx_hal.h"
#elif defined(STM32L4)
#include "stm32l4xx_hal.h"
#elif defined(STM32L5)
#include "stm32l5xx_hal.h"
#elif defined(STM32F3)
#include "stm32f3xx_hal.h"
#elif defined(STM32H7)
#include "stm32h7xx_hal.h"
#elif defined(STM32F7)
#include "stm32f7xx_hal.h"
#elif defined(STM32G0)
#include "stm32g0xx_hal.h"
#elif defined(STM32G4)
#include "stm32g4xx_hal.h"
#elif defined(STM32C0)
#include "stm32c0xx_hal.h"
#else
#error "SSD1306 library was tested only on STM32F0, F1, F3, F4, F7, L0, L1, L4, H7, G0, G4, WB, C0 families"
#endif

// ============================================================================
// X 轴偏移（当屏幕横向不从 column 0 开始时使用）
// ============================================================================

#ifdef SSD1306_X_OFFSET
#define SSD1306_X_OFFSET_LOWER (SSD1306_X_OFFSET & 0x0F)
#define SSD1306_X_OFFSET_UPPER ((SSD1306_X_OFFSET >> 4) & 0x07)
#else
#define SSD1306_X_OFFSET_LOWER 0
#define SSD1306_X_OFFSET_UPPER 0
#endif

// ============================================================================
// I2C / SPI 默认配置（可在 ssd1306_conf.h 中覆盖）
// ============================================================================

#ifndef SSD1306_I2C_PORT
#define SSD1306_I2C_PORT hi2c1
#endif
#ifndef SSD1306_I2C_ADDR
#define SSD1306_I2C_ADDR (0x3C << 1)
#endif

#ifndef SSD1306_SPI_PORT
#define SSD1306_SPI_PORT hspi2
#endif
#ifndef SSD1306_CS_Port
#define SSD1306_CS_Port  GPIOB
#endif
#ifndef SSD1306_CS_Pin
#define SSD1306_CS_Pin   GPIO_PIN_12
#endif
#ifndef SSD1306_DC_Port
#define SSD1306_DC_Port  GPIOB
#endif
#ifndef SSD1306_DC_Pin
#define SSD1306_DC_Pin   GPIO_PIN_14
#endif
#ifndef SSD1306_Reset_Port
#define SSD1306_Reset_Port GPIOA
#endif
#ifndef SSD1306_Reset_Pin
#define SSD1306_Reset_Pin  GPIO_PIN_8
#endif

#if defined(SSD1306_USE_I2C)
extern I2C_HandleTypeDef SSD1306_I2C_PORT;
#elif defined(SSD1306_USE_SPI)
extern SPI_HandleTypeDef SSD1306_SPI_PORT;
#else
#error "You should define SSD1306_USE_SPI or SSD1306_USE_I2C macro!"
#endif

// ============================================================================
// 屏幕尺寸 & 帧缓冲区大小
// ============================================================================

#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH 128
#endif
#ifndef SSD1306_HEIGHT
#define SSD1306_HEIGHT 64
#endif
#ifndef SSD1306_BUFFER_SIZE
#define SSD1306_BUFFER_SIZE (SSD1306_WIDTH * SSD1306_HEIGHT / 8)
#endif

// ============================================================================
// 基础类型定义
// ============================================================================

/** @brief 像素颜色 */
typedef enum { Black = 0x00, White = 0x01 } SSD1306_COLOR;

/** @brief 操作返回值 */
typedef enum { SSD1306_OK = 0x00, SSD1306_ERR = 0x01 } SSD1306_Error_t;

/** @brief 显示屏状态 */
typedef struct {
    uint16_t CurrentX;     // 当前光标 X
    uint16_t CurrentY;     // 当前光标 Y
    uint8_t Initialized;   // 初始化标志
    uint8_t DisplayOn;     // 显示开关
} SSD1306_t;

/** @brief 二维坐标点 */
typedef struct {
    uint8_t x;
    uint8_t y;
} SSD1306_VERTEX;

/** @brief 字体描述结构体 */
typedef struct {
    const uint8_t width;               // 字体宽度（像素）
    const uint8_t height;              // 字体高度（像素）
    const uint16_t *const data;        // 字形数据指针
    const uint8_t *const char_width;   // 比例字符宽度表（等宽字体为 NULL）
} SSD1306_Font_t;

// ============================================================================
// 基础操作
// ============================================================================

/** @brief 初始化 OLED 显示屏 */
void ssd1306_Init(void);

/** @brief 全屏填充指定颜色 */
void ssd1306_Fill(SSD1306_COLOR color);

/** @brief 将帧缓冲区刷入显示屏 */
void ssd1306_UpdateScreen(void);

/** @brief 在指定坐标画一个像素 */
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color);

// ============================================================================
// 文字输出
// ============================================================================

/**
 * @brief 输出单个字符
 * @param ch    ASCII 字符 (32~126)
 * @param Font  字体描述
 * @param color 像素颜色
 * @return 写入的字符，失败返回 0
 */
char ssd1306_WriteChar(char ch, SSD1306_Font_t Font, SSD1306_COLOR color);

/**
 * @brief 输出字符串
 * @param str   NULL 结尾的字符串
 * @param Font  字体描述
 * @param color 像素颜色
 * @return 写入失败的字符（成功返回 '\0'）
 */
char ssd1306_WriteString(const char *str, SSD1306_Font_t Font, SSD1306_COLOR color);

/** @brief 设置光标位置 */
void ssd1306_SetCursor(uint8_t x, uint8_t y);

// ============================================================================
// 几何图形绘制
// ============================================================================

/** @brief 画直线（Bresenham 算法） */
void ssd1306_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                  SSD1306_COLOR color);

/**
 * @brief 画圆弧
 * @param x, y       圆心坐标
 * @param radius     半径
 * @param start_angle 起始角度（度）
 * @param sweep      扫描角度（度）
 */
void ssd1306_DrawArc(uint8_t x, uint8_t y, uint8_t radius,
                     uint16_t start_angle, uint16_t sweep,
                     SSD1306_COLOR color);

/**
 * @brief 画圆弧并包含起始／终止角度到圆心的半径线
 */
void ssd1306_DrawArcWithRadiusLine(uint8_t x, uint8_t y, uint8_t radius,
                                   uint16_t start_angle, uint16_t sweep,
                                   SSD1306_COLOR color);

/** @brief 画空心圆（Bresenham 算法） */
void ssd1306_DrawCircle(uint8_t x, uint8_t y, uint8_t r,
                        SSD1306_COLOR color);

/** @brief 画实心圆（Bresenham 算法） */
void ssd1306_FillCircle(uint8_t x, uint8_t y, uint8_t r,
                        SSD1306_COLOR color);

/** @brief 画折线 */
void ssd1306_Polyline(const SSD1306_VERTEX *vertex, uint16_t size,
                      SSD1306_COLOR color);

/** @brief 画矩形边框 */
void ssd1306_DrawRectangle(uint8_t x1, uint8_t y1,
                           uint8_t x2, uint8_t y2,
                           SSD1306_COLOR color);

/** @brief 画实心矩形 */
void ssd1306_FillRectangle(uint8_t x1, uint8_t y1,
                           uint8_t x2, uint8_t y2,
                           SSD1306_COLOR color);

/**
 * @brief 矩形区域反色（含边框）
 * @return SSD1306_OK 或 SSD1306_ERR（坐标越界时）
 */
SSD1306_Error_t ssd1306_InvertRectangle(uint8_t x1, uint8_t y1,
                                        uint8_t x2, uint8_t y2);

// ============================================================================
// 位图绘制
// ============================================================================

/**
 * @brief 画位图
 * @param x, y    左上角坐标
 * @param bitmap  位图数据（每行按整字节对齐）
 * @param w, h    位图宽高（像素）
 */
void ssd1306_DrawBitmap(uint8_t x, uint8_t y, const unsigned char *bitmap,
                        uint8_t w, uint8_t h, SSD1306_COLOR color);

// ============================================================================
// 显示控制
// ============================================================================

/** @brief 设置对比度（0~255，RESET = 0x7F） */
void ssd1306_SetContrast(const uint8_t value);

/** @brief 控制显示开关（0 = 关，非 0 = 开） */
void ssd1306_SetDisplayOn(const uint8_t on);

/** @brief 查询显示状态（0 = 关，1 = 开） */
uint8_t ssd1306_GetDisplayOn(void);

// ============================================================================
// 底层硬件接口
// ============================================================================

/** @brief 硬件复位 */
void ssd1306_Reset(void);

/** @brief 发送命令字节 */
void ssd1306_WriteCommand(uint8_t byte);

/** @brief 发送数据 */
void ssd1306_WriteData(uint8_t *buffer, size_t buff_size);

/**
 * @brief 用外部数据覆盖帧缓冲区
 * @param buf 数据源
 * @param len 数据长度
 * @return SSD1306_OK / SSD1306_ERR
 */
SSD1306_Error_t ssd1306_FillBuffer(uint8_t *buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif // SSD1306_H
