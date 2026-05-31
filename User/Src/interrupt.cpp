/**
 * @file    interrupt.cpp
 * @brief   TIM1 中断回调 — 仅做薄层转发，具体逻辑见 swing_up.cpp
 */

#include "stm32f1xx_hal.h"
#include "global.hpp"
#include "swing_up.hpp"

extern "C" {

/**
 * @brief TIM1 溢出中断（1kHz）
 *        每 1ms 进入一次，读取传感器角度，派发到状态机处理
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM1) {
        pid_isr_count++;
        state_handler(sensor.get_angle());
    }
}

} // extern "C"
