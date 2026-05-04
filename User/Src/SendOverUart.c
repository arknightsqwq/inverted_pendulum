#include "SendOverUart.h"

// 静态实现函数
static void UART_SendFloat(struct UART_Object* self, float value) {
    char buffer[32];
    int len = sprintf(buffer, "%.4f\r\n", value);
    HAL_UART_Transmit(self->huart, (uint8_t*)buffer, len, HAL_MAX_DELAY);
}

static void UART_SendInt(struct UART_Object* self, int32_t value) {
    char buffer[16];
    int len = sprintf(buffer, "%ld\r\n", (long)value);
    HAL_UART_Transmit(self->huart, (uint8_t*)buffer, len, HAL_MAX_DELAY);
}

static void UART_SendString(struct UART_Object* self, const char* str) {
    int len = 0;
    while(str[len]) len++;
    HAL_UART_Transmit(self->huart, (uint8_t*)str, len, HAL_MAX_DELAY);
}

// 初始化并绑定
void UART_Object_Init(UART_Object* self, UART_HandleTypeDef* huart) {
    self->huart = huart;
    self->SendFloat = UART_SendFloat;
    self->SendInt = UART_SendInt;
    self->SendString = UART_SendString;
}