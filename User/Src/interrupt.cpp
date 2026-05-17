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
        /*static float angle_target = static_cast<float>(anglePID._cfg.target);

        if (++pos_cnt >= POSITION_DOWNSAMPLE) { //外环 200Hz
            pos_cnt = 0;
            float offset = positionPID.calculate(
                static_cast<float>(positionPID._cfg.target),
                static_cast<float>(motor.get_location()));
            angle_target = static_cast<float>(anglePID._cfg.target) + offset;
        }

        //内环 1kHz
        auto pwm = anglePID.calculate(
            angle_target,
            static_cast<float>(sensor.get_degree()));
        motor.set_pwm(static_cast<int8_t>(pwm));*/

        //=== 单环位置 PID（测试用）===
        auto pwm = positionPID.calculate(
            static_cast<float>(positionPID._cfg.target),
            static_cast<float>(motor.get_location()));
        motor.set_pwm(static_cast<int8_t>(pwm));
    }
}

} // extern "C"
