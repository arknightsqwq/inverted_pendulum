#include "stm32f1xx_hal.h"
#include "global.hpp"

// 位置环（外环）降采样倍数，内环角度环保持 1kHz
#define POSITION_DOWNSAMPLE  5
#define ANGLE_DOWNSAMPLE  5
extern "C" {

/**
 * @brief 定时器溢出中断回调 —— 级联 PID 控制
 *        内环（角度 PID）降采样运行 (200Hz)，直接输出 PWM
 *        外环（位置 PID）降采样运行 (200Hz)，输出角度偏置
 *
 *        编码器角度 → [位置PID] → 角度偏置 → 修正角度目标
 *                                              ↓
 *         ADC 角度  → [角度PID] → PWM → 电机
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM1) {
        pid_isr_count++;

        static uint8_t angle_cnt = 0;   // 内环降采样计数
        static uint8_t pos_cnt = 0;     // 外环降采样计数

        // 每次中断更新全局值（供主循环显示，避免主循环调 get_raw 破坏滤波器）
        g_angle = sensor.get_raw();
        g_location = motor.get_location();

        // 内环 200Hz (5ms)
        if (++angle_cnt >= ANGLE_DOWNSAMPLE) {
            angle_cnt = 0;

            if (g_angle > 1556 && g_angle < 2556 && is_pid_running) {
                auto pwm = anglePID.calculate(static_cast<float>(g_angle));
                motor.set_pwm(pwm);
            } else {
                motor.set_pwm(0);
                anglePID.reset();
                is_pid_running = false;
            }
        }

        /*if (++pos_cnt >= POSITION_DOWNSAMPLE) { //外环 200Hz
            pos_cnt = 0;
            float offset = positionPID.calculate(
                static_cast<float>(motor.get_location()));
            anglePID.target = anglePID.target - offset;
        }*/
    }
}

} // extern "C"
