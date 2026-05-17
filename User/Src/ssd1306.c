#include "ssd1306.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// 帧缓冲区 & 状态对象（内部全局）
// ============================================================================

static uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];
static SSD1306_t SSD1306;

// ============================================================================
// 内部工具函数（static，不对外开放）
// ============================================================================

/** @brief 角度转弧度 */
static float _deg_to_rad(float deg) {
    return deg * (3.14f / 180.0f);
}

/** @brief 将角度归一化到 [0, 360] */
static uint16_t _normalize_angle(uint16_t deg) {
    if (deg <= 360) return deg;
    uint16_t a = deg % 360;
    return (a ? a : 360);
}

/**
 * @brief 画水平线段（直接写帧缓冲区，跳过 DrawPixel 的边界检查开销）
 * @param x1, x2  起止 X 坐标（int32_t，允许负值由函数内部裁剪）
 * @param y       Y 坐标
 * @param color   像素颜色
 */
static void _draw_hline(int32_t x1, int32_t x2, int32_t y,
                        SSD1306_COLOR color) {
    if (y < 0 || y >= SSD1306_HEIGHT) return;
    if (x1 < 0) x1 = 0;
    if (x2 >= SSD1306_WIDTH) x2 = SSD1306_WIDTH - 1;
    if (x1 > x2) return;

    uint32_t base = (uint32_t)(y / 8) * SSD1306_WIDTH;
    uint8_t mask = 1 << (y % 8);
    uint32_t start = (uint32_t)x1 + base;
    uint32_t end   = (uint32_t)x2 + base;

    if (color == White) {
        for (uint32_t i = start; i <= end; i++) SSD1306_Buffer[i] |= mask;
    } else {
        mask = ~mask;
        for (uint32_t i = start; i <= end; i++) SSD1306_Buffer[i] &= mask;
    }
}

// ============================================================================
// 对外接口 — 硬件 I/O（I2C / SPI 二选一）
// ============================================================================

#if defined(SSD1306_USE_I2C)

void ssd1306_Reset(void) {
    // I2C 模式下无需额外复位操作
}

void ssd1306_WriteCommand(uint8_t byte) {
    HAL_I2C_Mem_Write(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR,
                      0x00, 1, &byte, 1, 100);
}

void ssd1306_WriteData(uint8_t *buffer, size_t buff_size) {
    HAL_I2C_Mem_Write(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR,
                      0x40, 1, buffer, buff_size, 100);
}

#elif defined(SSD1306_USE_SPI)

void ssd1306_Reset(void) {
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SSD1306_Reset_Port, SSD1306_Reset_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(SSD1306_Reset_Port, SSD1306_Reset_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
}

void ssd1306_WriteCommand(uint8_t byte) {
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SSD1306_DC_Port, SSD1306_DC_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&SSD1306_SPI_PORT, (uint8_t *)&byte, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_SET);
}

void ssd1306_WriteData(uint8_t *buffer, size_t buff_size) {
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SSD1306_DC_Port, SSD1306_DC_Pin, GPIO_PIN_SET);
    HAL_SPI_Transmit(&SSD1306_SPI_PORT, buffer, buff_size, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_SET);
}

#else
#error "You should define SSD1306_USE_SPI or SSD1306_USE_I2C macro"
#endif

// ============================================================================
// 对外接口 — 初始化与显示控制
// ============================================================================

void ssd1306_Init(void) {
    // 1. 硬件复位
    ssd1306_Reset();
    HAL_Delay(100);

    // 2. 关闭显示
    ssd1306_SetDisplayOn(0);

    // 3. 设置内存寻址模式
    ssd1306_WriteCommand(0x20);
    ssd1306_WriteCommand(0x00); // 水平寻址模式

    // 4. 设置页起始地址（页寻址模式用）
    ssd1306_WriteCommand(0xB0);

#ifdef SSD1306_MIRROR_VERT
    ssd1306_WriteCommand(0xC0); // 垂直镜像
#else
    ssd1306_WriteCommand(0xC8); // COM 输出扫描方向（从上到下）
#endif

    // 5. 设置列地址范围
    ssd1306_WriteCommand(0x00);
    ssd1306_WriteCommand(0x10);

    // 6. 设置起始行
    ssd1306_WriteCommand(0x40);

    // 7. 设置对比度最大
    ssd1306_SetContrast(0xFF);

#ifdef SSD1306_MIRROR_HORIZ
    ssd1306_WriteCommand(0xA0); // 水平镜像
#else
    ssd1306_WriteCommand(0xA1); // 段重映射（左右正确）
#endif

#ifdef SSD1306_INVERSE_COLOR
    ssd1306_WriteCommand(0xA7); // 反色
#else
    ssd1306_WriteCommand(0xA6); // 正常颜色
#endif

    // 8. 设置 multiplex ratio
    ssd1306_WriteCommand(0xA8);
#if (SSD1306_HEIGHT == 32)
    ssd1306_WriteCommand(0x1F);
#elif (SSD1306_HEIGHT == 64)
    ssd1306_WriteCommand(0x3F);
#elif (SSD1306_HEIGHT == 128)
    ssd1306_WriteCommand(0xFF);
    ssd1306_WriteCommand(0x3F);
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

    // 9. 显示跟随 RAM 内容
    ssd1306_WriteCommand(0xA4);

    // 10. 设置显示偏移
    ssd1306_WriteCommand(0xD3);
    ssd1306_WriteCommand(0x00);

    // 11. 设置时钟分频 / 振荡器频率
    ssd1306_WriteCommand(0xD5);
    ssd1306_WriteCommand(0xF0);

    // 12. 设置预充电周期
    ssd1306_WriteCommand(0xD9);
    ssd1306_WriteCommand(0x22);

    // 13. 设置 COM 引脚硬件配置
    ssd1306_WriteCommand(0xDA);
#if (SSD1306_HEIGHT == 32)
    ssd1306_WriteCommand(0x02);
#elif (SSD1306_HEIGHT == 64)
    ssd1306_WriteCommand(0x12);
#elif (SSD1306_HEIGHT == 128)
    ssd1306_WriteCommand(0x12);
#endif

    // 14. 设置 VCOMH 电压
    ssd1306_WriteCommand(0xDB);
    ssd1306_WriteCommand(0x20);

    // 15. 开启 DC-DC 电荷泵
    ssd1306_WriteCommand(0x8D);
    ssd1306_WriteCommand(0x14);

    // 16. 开启显示
    ssd1306_SetDisplayOn(1);

    // 17. 清屏
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();

    // 18. 重置光标并标记已初始化
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;
    SSD1306.Initialized = 1;
}

void ssd1306_SetContrast(const uint8_t value) {
    ssd1306_WriteCommand(0x81);
    ssd1306_WriteCommand(value);
}

void ssd1306_SetDisplayOn(const uint8_t on) {
    if (on) {
        ssd1306_WriteCommand(0xAF);
        SSD1306.DisplayOn = 1;
    } else {
        ssd1306_WriteCommand(0xAE);
        SSD1306.DisplayOn = 0;
    }
}

uint8_t ssd1306_GetDisplayOn(void) {
    return SSD1306.DisplayOn;
}

// ============================================================================
// 对外接口 — 帧缓冲区操作
// ============================================================================

SSD1306_Error_t ssd1306_FillBuffer(uint8_t *buf, uint32_t len) {
    if (len > SSD1306_BUFFER_SIZE) return SSD1306_ERR;
    memcpy(SSD1306_Buffer, buf, len);
    return SSD1306_OK;
}

void ssd1306_Fill(SSD1306_COLOR color) {
    memset(SSD1306_Buffer,
           (color == Black) ? 0x00 : 0xFF,
           sizeof(SSD1306_Buffer));
}

void ssd1306_UpdateScreen(void) {
    for (uint8_t i = 0; i < SSD1306_HEIGHT / 8; i++) {
        ssd1306_WriteCommand(0xB0 + i);
        ssd1306_WriteCommand(0x00 + SSD1306_X_OFFSET_LOWER);
        ssd1306_WriteCommand(0x10 + SSD1306_X_OFFSET_UPPER);
        ssd1306_WriteData(&SSD1306_Buffer[SSD1306_WIDTH * i],
                          SSD1306_WIDTH);
    }
}

// ============================================================================
// 对外接口 — 像素操作
// ============================================================================

void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;

    if (color == White) {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    } else {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

// ============================================================================
// 对外接口 — 文字输出
// ============================================================================

void ssd1306_SetCursor(uint8_t x, uint8_t y) {
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}

char ssd1306_WriteChar(char ch, SSD1306_Font_t Font, SSD1306_COLOR color) {
    // 1. 检查字符是否在可打印范围
    if (ch < 32 || ch > 126) return 0;

    // 2. 获取字符宽度（比例字体用 char_width 表，等宽字体用固定值）
    const uint8_t char_width = Font.char_width ? Font.char_width[ch - 32]
                                               : Font.width;

    // 3. 检查剩余空间
    if (SSD1306_WIDTH < (SSD1306.CurrentX + char_width) ||
        SSD1306_HEIGHT < (SSD1306.CurrentY + Font.height)) {
        return 0;
    }

    // 4. 逐行逐 bit 直接写帧缓冲区（跳过 DrawPixel 的边界检查）
    for (uint32_t i = 0; i < Font.height; i++) {
        uint16_t b = Font.data[(ch - 32) * Font.height + i];
        uint32_t row_start = (uint32_t)(SSD1306.CurrentY + i) / 8 * SSD1306_WIDTH;
        uint8_t  bit_mask  = 1 << ((SSD1306.CurrentY + i) % 8);

        if (color == White) {
            for (uint32_t j = 0; j < char_width; j++) {
                uint32_t idx = row_start + SSD1306.CurrentX + j;
                if ((b << j) & 0x8000) {
                    SSD1306_Buffer[idx] |= bit_mask;   // White 前景
                } else {
                    SSD1306_Buffer[idx] &= ~bit_mask;  // Black 背景
                }
            }
        } else {
            for (uint32_t j = 0; j < char_width; j++) {
                uint32_t idx = row_start + SSD1306.CurrentX + j;
                if ((b << j) & 0x8000) {
                    SSD1306_Buffer[idx] &= ~bit_mask;  // Black 前景
                } else {
                    SSD1306_Buffer[idx] |= bit_mask;   // White 背景
                }
            }
        }
    }

    // 5. 光标右移
    SSD1306.CurrentX += char_width;
    return ch;
}

char ssd1306_WriteString(const char *str, SSD1306_Font_t Font,
                         SSD1306_COLOR color) {
    while (*str) {
        if (ssd1306_WriteChar(*str, Font, color) != *str) {
            return *str;
        }
        str++;
    }
    return *str;
}

// ============================================================================
// 对外接口 — 直线 & 折线
// ============================================================================

void ssd1306_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                  SSD1306_COLOR color) {
    int32_t deltaX = abs(x2 - x1);
    int32_t deltaY = abs(y2 - y1);
    int32_t signX = (x1 < x2) ? 1 : -1;
    int32_t signY = (y1 < y2) ? 1 : -1;
    int32_t error = deltaX - deltaY;

    ssd1306_DrawPixel(x2, y2, color);

    while ((x1 != x2) || (y1 != y2)) {
        ssd1306_DrawPixel(x1, y1, color);
        int32_t error2 = error * 2;
        if (error2 > -deltaY) {
            error -= deltaY;
            x1 += signX;
        }
        if (error2 < deltaX) {
            error += deltaX;
            y1 += signY;
        }
    }
}

void ssd1306_Polyline(const SSD1306_VERTEX *vertex, uint16_t size,
                      SSD1306_COLOR color) {
    if (vertex == NULL) return;

    for (uint16_t i = 1; i < size; i++) {
        ssd1306_Line(vertex[i - 1].x, vertex[i - 1].y,
                     vertex[i].x, vertex[i].y, color);
    }
}

// ============================================================================
// 对外接口 — 圆 & 圆弧
// ============================================================================

void ssd1306_DrawCircle(uint8_t cx, uint8_t cy, uint8_t r,
                        SSD1306_COLOR color) {
    if (cx >= SSD1306_WIDTH || cy >= SSD1306_HEIGHT) return;

    int32_t x = -r;
    int32_t y = 0;
    int32_t err = 2 - 2 * r;

    do {
        ssd1306_DrawPixel(cx - x, cy + y, color);
        ssd1306_DrawPixel(cx + x, cy + y, color);
        ssd1306_DrawPixel(cx + x, cy - y, color);
        ssd1306_DrawPixel(cx - x, cy - y, color);

        int32_t e2 = err;
        if (e2 <= y) {
            y++;
            err += y * 2 + 1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) {
            x++;
            err += x * 2 + 1;
        }
    } while (x <= 0);
}

void ssd1306_FillCircle(uint8_t cx, uint8_t cy, uint8_t r,
                        SSD1306_COLOR color) {
    if (cx >= SSD1306_WIDTH || cy >= SSD1306_HEIGHT) return;

    // 逐行画水平跨度：对每个 y 计算 x = sqrt(r² - y²)（整数逼近）
    int32_t x = r;
    for (int32_t y = 0; y <= r; y++) {
        while (x * x + y * y > r * r) x--;
        _draw_hline(cx - x, cx + x, cy + y, color);
        if (y > 0) _draw_hline(cx - x, cx + x, cy - y, color);
    }
}

void ssd1306_DrawArc(uint8_t x, uint8_t y, uint8_t radius,
                     uint16_t start_angle, uint16_t sweep,
                     SSD1306_COLOR color) {
    static const uint8_t SEGMENTS = 36;
    uint16_t sweep_norm = _normalize_angle(sweep);

    uint32_t count = (_normalize_angle(start_angle) * SEGMENTS) / 360;
    uint32_t approx_segments = (sweep_norm * SEGMENTS) / 360;
    float approx_degree = sweep_norm / (float)approx_segments;

    while (count < approx_segments) {
        float rad = _deg_to_rad(count * approx_degree);
        uint8_t xp1 = x + (int8_t)(sinf(rad) * radius);
        uint8_t yp1 = y + (int8_t)(cosf(rad) * radius);
        count++;

        if (count != approx_segments) {
            rad = _deg_to_rad(count * approx_degree);
        } else {
            rad = _deg_to_rad(sweep_norm);
        }
        uint8_t xp2 = x + (int8_t)(sinf(rad) * radius);
        uint8_t yp2 = y + (int8_t)(cosf(rad) * radius);

        ssd1306_Line(xp1, yp1, xp2, yp2, color);
    }
}

void ssd1306_DrawArcWithRadiusLine(uint8_t x, uint8_t y, uint8_t radius,
                                   uint16_t start_angle, uint16_t sweep,
                                   SSD1306_COLOR color) {
    static const uint8_t SEGMENTS = 36;
    uint16_t sweep_norm = _normalize_angle(sweep);

    uint32_t count = (_normalize_angle(start_angle) * SEGMENTS) / 360;
    uint32_t approx_segments = (sweep_norm * SEGMENTS) / 360;
    float approx_degree = sweep_norm / (float)approx_segments;

    float rad = _deg_to_rad(count * approx_degree);
    uint8_t first_x = x + (int8_t)(sinf(rad) * radius);
    uint8_t first_y = y + (int8_t)(cosf(rad) * radius);

    uint8_t xp2 = 0, yp2 = 0;
    while (count < approx_segments) {
        rad = _deg_to_rad(count * approx_degree);
        uint8_t xp1 = x + (int8_t)(sinf(rad) * radius);
        uint8_t yp1 = y + (int8_t)(cosf(rad) * radius);
        count++;

        if (count != approx_segments) {
            rad = _deg_to_rad(count * approx_degree);
        } else {
            rad = _deg_to_rad(sweep_norm);
        }
        xp2 = x + (int8_t)(sinf(rad) * radius);
        yp2 = y + (int8_t)(cosf(rad) * radius);

        ssd1306_Line(xp1, yp1, xp2, yp2, color);
    }

    ssd1306_Line(x, y, first_x, first_y, color);
    ssd1306_Line(x, y, xp2, yp2, color);
}

// ============================================================================
// 对外接口 — 矩形
// ============================================================================

void ssd1306_DrawRectangle(uint8_t x1, uint8_t y1,
                           uint8_t x2, uint8_t y2,
                           SSD1306_COLOR color) {
    ssd1306_Line(x1, y1, x2, y1, color);
    ssd1306_Line(x2, y1, x2, y2, color);
    ssd1306_Line(x2, y2, x1, y2, color);
    ssd1306_Line(x1, y2, x1, y1, color);
}

void ssd1306_FillRectangle(uint8_t x1, uint8_t y1,
                           uint8_t x2, uint8_t y2,
                           SSD1306_COLOR color) {
    if (x1 > x2) { uint8_t t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { uint8_t t = y1; y1 = y2; y2 = t; }

    if (x1 >= SSD1306_WIDTH || y1 >= SSD1306_HEIGHT) return;
    if (x2 >= SSD1306_WIDTH) x2 = SSD1306_WIDTH - 1;
    if (y2 >= SSD1306_HEIGHT) y2 = SSD1306_HEIGHT - 1;

    uint8_t page_start = y1 / 8;
    uint8_t page_end   = y2 / 8;

    for (uint8_t page = page_start; page <= page_end; page++) {
        uint32_t base = (uint32_t)page * SSD1306_WIDTH;
        uint8_t mask = 0xFF;
        if (page == page_start) mask &= (uint8_t)(0xFF << (y1 % 8));
        if (page == page_end)   mask &= (uint8_t)(0xFF >> (7 - (y2 % 8)));

        if (mask == 0xFF) {
            memset(&SSD1306_Buffer[x1 + base],
                   (color == White) ? 0xFF : 0x00,
                   x2 - x1 + 1);
        } else {
            for (uint32_t x = x1; x <= x2; x++) {
                uint32_t idx = x + base;
                if (color == White) {
                    SSD1306_Buffer[idx] |= mask;
                } else {
                    SSD1306_Buffer[idx] &= ~mask;
                }
            }
        }
    }
}

SSD1306_Error_t ssd1306_InvertRectangle(uint8_t x1, uint8_t y1,
                                        uint8_t x2, uint8_t y2) {
    if (x2 >= SSD1306_WIDTH || y2 >= SSD1306_HEIGHT) return SSD1306_ERR;
    if (x1 > x2 || y1 > y2) return SSD1306_ERR;

    if ((y1 / 8) != (y2 / 8)) {
        for (uint32_t x = x1; x <= x2; x++) {
            uint32_t i = x + (y1 / 8) * SSD1306_WIDTH;
            SSD1306_Buffer[i] ^= 0xFF << (y1 % 8);
            i += SSD1306_WIDTH;
            for (; i < x + (y2 / 8) * SSD1306_WIDTH; i += SSD1306_WIDTH) {
                SSD1306_Buffer[i] ^= 0xFF;
            }
            SSD1306_Buffer[i] ^= 0xFF >> (7 - (y2 % 8));
        }
    } else {
        const uint8_t mask = (0xFF << (y1 % 8)) & (0xFF >> (7 - (y2 % 8)));
        for (uint32_t i = x1 + (y1 / 8) * SSD1306_WIDTH;
             i <= (uint32_t)x2 + (y2 / 8) * SSD1306_WIDTH; i++) {
            SSD1306_Buffer[i] ^= mask;
        }
    }
    return SSD1306_OK;
}

// ============================================================================
// 对外接口 — 位图
// ============================================================================

void ssd1306_DrawBitmap(uint8_t x, uint8_t y, const unsigned char *bitmap,
                        uint8_t w, uint8_t h, SSD1306_COLOR color) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;

    int16_t byte_width = (w + 7) / 8;

    for (uint8_t j = 0; j < h; j++, y++) {
        for (uint8_t i = 0; i < w; i++) {
            uint8_t byte;
            if (i & 7) {
                byte <<= 1;
            } else {
                byte = bitmap[j * byte_width + i / 8];
            }
            if (byte & 0x80) {
                ssd1306_DrawPixel(x + i, y, color);
            }
        }
    }
}
