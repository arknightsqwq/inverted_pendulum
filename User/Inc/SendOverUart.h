#ifndef SENDOVERUART_H
#define SENDOVERUART_H

#include "stm32f1xx_hal.h"
#include <stdio.h>

// 定义对象结构体
typedef struct UART_Object {
    UART_HandleTypeDef* huart;

    // 底层实现函数指针（内部使用）
    void (*SendFloat)(struct UART_Object* self, float value);
    void (*SendInt)(struct UART_Object* self, int32_t value);
    void (*SendString)(struct UART_Object* self, const char* str);
} UART_Object;

// 构造函数
void UART_Object_Init(UART_Object* self, UART_HandleTypeDef* huart);

/**
 * 使用 C11 _Generic 模拟函数重载
 * 根据传入参数的类型，自动选择调用哪个底层函数
 */
#define Send(obj, val) _Generic((val), \
float: (obj)->SendFloat,          \
double: (obj)->SendFloat,         \
char*: (obj)->_endString,         \
const char*: (obj)->SendString,   \
default: (obj)->SendInt           /* 只要不是浮点或字符串，统一当做 int 处理 */ \
)(obj, val)

#endif