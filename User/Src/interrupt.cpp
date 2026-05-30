#include "stm32f1xx_hal.h"
#include "global.hpp"

// 内外环统一降采样 5 倍 → 200Hz（TIM1 基频 1kHz）
#define PID_DOWNSAMPLE  5

extern "C" {

/**
 * @brief 定时器溢出中断回调 —— 级联 PID 控制
 *        内外环统一降采样运行 (200Hz)
 *
 *        编码器位置 → [位置PID] → 角度偏置 → 修正角度目标
 *                                              ↓
 *         ADC 角度  → [角度PID] → PWM → 电机
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM1) {
        pid_isr_count++;

        static uint8_t ds_cnt = 0;

        if (++ds_cnt >= PID_DOWNSAMPLE) { // 内外环统一 200Hz
            ds_cnt = 0;

            float angle = sensor.get_angle();
            if (is_pid_running && angle > 160.0f && angle < 200.0f) {
                // 外环：位置 PID → 角度偏置（暂时屏蔽）
                //float offset = positionPID.calculate(
                //    static_cast<float>(motor.get_location()));
                //anglePID.target += offset;

                // 内环：角度 PID → PWM
                auto pwm = anglePID.calculate(angle);
                motor.set_pwm(pwm);
            } else {
                is_pid_running = false;
                motor.set_pwm(0);
                anglePID.reset();
                positionPID.reset();
            }
        }
    }
}

} // extern "C"
