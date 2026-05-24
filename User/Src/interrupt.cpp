#include "stm32f1xx_hal.h"
#include "global.hpp"

// 位置环（外环）降采样倍数，内环角度环保持 1kHz
#define POSITION_DOWNSAMPLE  5

extern "C" {

/**
 * @brief 定时器溢出中断回调 —— 级联 PID 控制
 *        内环（角度 PID）每次中断运行 (1kHz)，直接输出 PWM
 *        外环（位置 PID）降采样运行 (200Hz)，输出角度偏置
 *
 *        编码器位置 → [位置PID] → 角度偏置 → 修正角度目标
 *                                              ↓
 *         ADC 角度  → [角度PID] → PWM → 电机
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM3) {
        pid_isr_count++;

        static uint8_t pos_cnt = 0;

        /*if (++pos_cnt >= POSITION_DOWNSAMPLE) { //外环 200Hz
            pos_cnt = 0;
            float offset = positionPID.calculate(
                static_cast<float>(motor.get_location()));
            anglePID.target = anglePID.target + offset;
        }*/

        //内环 1kHz
        float angle = sensor.get_degree();
        if (angle > 90.0f && angle < 270.0f) {
            auto pwm = anglePID.calculate(angle);
            motor.set_pwm(pwm);
        } else {
            motor.set_pwm(0);
            anglePID.reset();
        }

        //=== 单环位置 PID（测试用）===
        /*positionPID.target = 20;
        auto pwm = positionPID.calculate(static_cast<float>(motor.get_angle()));
        motor.set_pwm(static_cast<int8_t>(pwm));*/
    }
}

} // extern "C"
