#include "uart_object.hpp"

UART_Object::UART_Object(UART_HandleTypeDef* huart) : _huart(huart) {
}

void UART_Object::send(float value) {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "%.4f\r\n", value);
    HAL_UART_Transmit(_huart, (uint8_t*)buffer, len, 100);
}

void UART_Object::send(double value) {
    send((float)value);
}

void UART_Object::send(int value) {
    char buffer[16];
    int len = snprintf(buffer, sizeof(buffer), "%ld\r\n", (long)value);
    HAL_UART_Transmit(_huart, (uint8_t*)buffer, len, 100);
}

void UART_Object::send(const char* str) {
    if (str == nullptr) return;

    int len = 0;
    while (str[len]) len++;

    HAL_UART_Transmit(_huart, (uint8_t*)str, len, 100);
}