#include "stm32f1xx_hal.h"
#include "global.hpp"

// 起摆参数
#define CENTER_ANGLE   179.0f
#define CENTER_RANGE   30.0f        // 中心区间 ±30°（149~209）
#define START_PWM      35           // 起摆推力
#define START_TIME     100          // 推摆时长 (ms)

// 分频（TIM1 基频 1kHz）
#define ANGLE_DOWNSAMPLE  5         // 内环 200Hz
#define POS_DOWNSAMPLE    50        // 外环 50Hz
#define JUDGE_DOWNSAMPLE  40        // 判断采样 25Hz

extern "C" {

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM1) {
        pid_isr_count++;

        static uint8_t  angle_cnt = 0, pos_cnt = 0, judge_cnt = 0;
        static uint16_t count_time = 0;
        static float    angle_s0 = 0, angle_s1 = 0, angle_s2 = 0;

        float sensor_angle = sensor.get_angle();

        //=== State 0: 停止 ===
        if (run_state == 0) {
            motor.set_pwm(0);
        }
        //=== State 1: 判断（40ms 采一次，检测峰值）===
        else if (run_state == 1) {
            motor.set_pwm(0);

            if (++judge_cnt >= JUDGE_DOWNSAMPLE) {
                judge_cnt = 0;

                angle_s2 = angle_s1;
                angle_s1 = angle_s0;
                angle_s0 = sensor_angle;

                // 右侧最高点 → 左推
                if (angle_s0 > CENTER_ANGLE + CENTER_RANGE
                 && angle_s1 > CENTER_ANGLE + CENTER_RANGE
                 && angle_s2 > CENTER_ANGLE + CENTER_RANGE
                 && angle_s1 < angle_s0 && angle_s1 < angle_s2) {
                    run_state = 21;
                }
                // 左侧最高点 → 右推
                else if (angle_s0 < CENTER_ANGLE - CENTER_RANGE
                 && angle_s1 < CENTER_ANGLE - CENTER_RANGE
                 && angle_s2 < CENTER_ANGLE - CENTER_RANGE
                 && angle_s1 > angle_s0 && angle_s1 > angle_s2) {
                    run_state = 31;
                }
                // 进入中心区间 → PID 接管
                else if (angle_s0 > CENTER_ANGLE - CENTER_RANGE
                 && angle_s0 < CENTER_ANGLE + CENTER_RANGE
                 && angle_s1 > CENTER_ANGLE - CENTER_RANGE
                 && angle_s1 < CENTER_ANGLE + CENTER_RANGE) {
                    anglePID.reset();
                    positionPID.reset();
                    run_state = 4;
                }
            }
        }
        //=== 起摆：左侧序列 21→22→23→24 ===
        else if (run_state == 21) {
            motor.set_pwm(START_PWM);
            count_time = START_TIME;
            run_state = 22;
        }
        else if (run_state == 22) {
            if (--count_time == 0) run_state = 23;
        }
        else if (run_state == 23) {
            motor.set_pwm(-START_PWM);
            count_time = START_TIME;
            run_state = 24;
        }
        else if (run_state == 24) {
            if (--count_time == 0) {
                motor.set_pwm(0);
                run_state = 1;
            }
        }
        //=== 起摆：右侧序列 31→32→33→34 ===
        else if (run_state == 31) {
            motor.set_pwm(-START_PWM);
            count_time = START_TIME;
            run_state = 32;
        }
        else if (run_state == 32) {
            if (--count_time == 0) run_state = 33;
        }
        else if (run_state == 33) {
            motor.set_pwm(START_PWM);
            count_time = START_TIME;
            run_state = 34;
        }
        else if (run_state == 34) {
            if (--count_time == 0) {
                motor.set_pwm(0);
                run_state = 1;
            }
        }
        //=== State 4: PID 控制 ===
        else if (run_state == 4) {
            // 内环 200Hz
            if (++angle_cnt >= ANGLE_DOWNSAMPLE) {
                angle_cnt = 0;

                if (sensor_angle > CENTER_ANGLE - CENTER_RANGE
                 && sensor_angle < CENTER_ANGLE + CENTER_RANGE) {
                    auto pwm = anglePID.calculate(sensor_angle);
                    motor.set_pwm(pwm);
                } else {
                    motor.set_pwm(0);
                    anglePID.reset();
                    positionPID.reset();
                    run_state = 0;
                }
            }

            // 外环 20Hz
            if (++pos_cnt >= POS_DOWNSAMPLE) {
                pos_cnt = 0;
                anglePID.target = CENTER_ANGLE
                    + positionPID.calculate(motor.get_angle());
            }
        }
    }
}

} // extern "C"
