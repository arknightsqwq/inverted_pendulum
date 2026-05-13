#ifndef UART_OBJECT_HPP
#define UART_OBJECT_HPP

#include "stm32f1xx_hal.h"
#include <cstdio>

class UART_Object {
public:
    /**
     * @brief 构造函数，替代原有的 Init 函数
     * @param huart UART 硬件句柄指针
     */
    explicit UART_Object(UART_HandleTypeDef* huart);

    /* --- 函数重载实现统一接口 --- */

    /**
     * @brief 发送浮点数（保留4位小数）
     */
    void send(float value);
    void send(double value);

    /**
     * @brief 发送整数
     */
    void send(int value);

    /**
     * @brief 发送字符串
     */
    void send(const char* str);

private:
    UART_HandleTypeDef* _huart; // 封装硬件句柄[cite: 3]
};

#endif