#include "stm32f1xx_hal.h"
#include "global.hpp"

/* 1. 声明外部定义的 C++ 对象，以便在此文件中访问 */


/* 2. 必须使用 extern "C" 告诉编译器按照 C 语言规则生成函数名 */
extern "C" {

/**
 * @brief 定时器溢出中断回调（通常用于控制环路，如 PID）
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    // 检查是否是用于控制的定时器（例如 TIM1）
    if (htim->Instance == TIM1) {
        float target_speed = anglePID.Calculate(anglePID._cfg.target, sensor.getdegree());
        int8_t out_pwm = speedPID.Calculate(target_speed, motor.get_speed());
        motor.set_pwm(out_pwm);
    }
}

/**
 * @brief 外部中断回调（例如按钮、编码器 Z 信号）
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_0) {
        pclink.Send("External Interrupt Triggered!\r\n");
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