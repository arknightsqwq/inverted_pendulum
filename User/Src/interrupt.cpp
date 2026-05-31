#include "stm32f1xx_hal.h"
#include "global.hpp"

// 内环 200Hz (÷5)，外环 20Hz (÷50)，TIM1 基频 1kHz
#define ANGLE_DOWNSAMPLE  5
#define POS_DOWNSAMPLE    50

extern "C" {

/**
 * @brief 定时器溢出中断回调 —— 串级 PID 控制
 *        内环 200Hz：ADC 角度 → [角度PID] → PWM → 电机
 *        外环 20Hz： 编码器位置 → [位置PID] → 修正角度目标（绝对设置）
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM1) {
        pid_isr_count++;

        static uint8_t angle_cnt = 0;
        static uint8_t pos_cnt = 0;

        //=== 内环 200Hz ===
        if (++angle_cnt >= ANGLE_DOWNSAMPLE) {
            angle_cnt = 0;

            float angle = sensor.get_angle();
            if (is_pid_running && angle > 160.0f && angle < 200.0f) {
                auto pwm = anglePID.calculate(angle);
                motor.set_pwm(pwm);
            } else {
                is_pid_running = false;
                motor.set_pwm(0);
                anglePID.reset();
                positionPID.reset();
            }
        }

        //=== 外环 20Hz ===
        if (++pos_cnt >= POS_DOWNSAMPLE) {
            pos_cnt = 0;

            if (is_pid_running) {
                anglePID.target = 179.0f + positionPID.calculate(motor.get_angle());
            }
        }
    }
}

} // extern "C"
