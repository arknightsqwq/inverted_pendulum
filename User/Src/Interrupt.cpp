#include "stm32f1xx_hal.h"
#include "Motor.hpp"
#include "UART_Object.hpp"

/* 1. 声明外部定义的 C++ 对象，以便在此文件中访问 */
extern Motor motor;
extern UART_Object pclink;

/* 2. 必须使用 extern "C" 告诉编译器按照 C 语言规则生成函数名 */
extern "C" {

/**
 * @brief 定时器溢出中断回调（通常用于控制环路，如 PID）
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    // 检查是否是用于控制的定时器（例如 TIM1）
    if (htim->Instance == TIM1) {
        // 在中断里直接调用 C++ 对象的方法
        // 注意：中断里的逻辑要尽可能快！
        float current_angle = 0.0f; // 假设从传感器获取
        motor.get_location();
    }
}

/**
 * @brief 外部中断回调（例如按钮、编码器 Z 信号）
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_0) {
        pc_link.Send("External Interrupt Triggered!\r\n");
    }
}

/**
 * @brief 串口接收完成中断回调
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        // 处理接收到的数据
    }
}

} // extern "C" 结束