#include "UART_Object.hpp"

UART_Object::UART_Object(UART_HandleTypeDef* huart) : _huart(huart) {
    // 构造函数初始化列表直接赋值[cite: 3, 4]
}

void UART_Object::send(float value) {
    char buffer[32];
    // 格式化输出并添加换行[cite: 4]
    int len = snprintf(buffer, sizeof(buffer), "%.4f\r\n", value);
    HAL_UART_Transmit(_huart, (uint8_t*)buffer, len, HAL_MAX_DELAY);
}

void UART_Object::send(double value) {
    // double 类型自动转为 float 处理[cite: 3]
    Send((float)value);
}

void UART_Object::send(int value) {
    char buffer[16];
    // 使用 long 强制转换匹配格式化字符串[cite: 4]
    int len = snprintf(buffer, sizeof(buffer), "%ld\r\n", (long)value);
    HAL_UART_Transmit(_huart, (uint8_t*)buffer, len, HAL_MAX_DELAY);
}

void UART_Object::send(const char* str) {
    if (str == nullptr) return;

    // 计算字符串长度[cite: 4]
    int len = 0;
    while (str[len]) len++;

    HAL_UART_Transmit(_huart, (uint8_t*)str, len, HAL_MAX_DELAY);
}