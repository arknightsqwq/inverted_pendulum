/**
 * @file    swing_up.cpp
 * @brief   倒立摆自动起摆状态机
 *
 *   State 0: 停止 — 电机断电
 *   State 1: 判断 — 检测峰值，谐振推摆
 *   State 21→24:  左推序列 → 回 1
 *   State 31→34:  右推序列 → 回 1
 *   State 4:  PID 控制
 *   State 5→6:  转圈最后一推 → 停机
 */

#include "swing_up.hpp"
#include "global.hpp"

//=== 起摆参数 ============================================================
#define CENTER_ANGLE   179.0f
#define CENTER_RANGE   30.0f
#define START_PWM      35
#define START_TIME     100
#define FINAL_PWM      60          // 转圈最后一推力度
#define FINAL_TIME     150         // 转圈最后一推时长

//=== 分频 ================================================================
#define ANGLE_DOWNSAMPLE  5
#define POS_DOWNSAMPLE    50
#define JUDGE_DOWNSAMPLE  40

//=== 静态变量 =============================================================
static uint16_t count_time = 0;
static float    angle_s0 = 0, angle_s1 = 0, angle_s2 = 0;
static uint8_t  angle_cnt = 0, pos_cnt = 0, judge_cnt = 0;

//=== 方向 =================================================================
static inline int8_t push_pwm()    { return (run_state < 30) ? START_PWM : -START_PWM; }
static inline int8_t reverse_pwm() { return (run_state < 30) ? -START_PWM : START_PWM; }

//=== 前向声明 =============================================================
static void state_stop(float);
static void state_judge(float);
static void state_push_enter(float);
static void state_wait(float);
static void state_push_reverse(float);
static void state_wait_final(float);
static void state_pid(float);
static void state_final_push(float);

//=== 派发 =================================================================
void state_handler(float angle) {
    switch (run_state) {
        case 0:  state_stop(angle);           break;
        case 1:  state_judge(angle);          break;
        case 4:  state_pid(angle);            break;
        case 5:  state_final_push(angle);     break;
        case 6:  state_final_push(angle);     break;
        case 21: state_push_enter(angle);     break;
        case 22: state_wait(angle);           break;
        case 23: state_push_reverse(angle);   break;
        case 24: state_wait_final(angle);     break;
        case 31: state_push_enter(angle);     break;
        case 32: state_wait(angle);           break;
        case 33: state_push_reverse(angle);   break;
        case 34: state_wait_final(angle);     break;
    }
}

//=== State 0 ==============================================================
static void state_stop(float) {
    motor.set_pwm(0);
}

//=== State 1 ==============================================================
static void state_judge(float angle) {
    motor.set_pwm(0);
    if (++judge_cnt < JUDGE_DOWNSAMPLE) return;
    judge_cnt = 0;

    angle_s2 = angle_s1;  angle_s1 = angle_s0;  angle_s0 = angle;
    float hi = CENTER_ANGLE + CENTER_RANGE;
    float lo = CENTER_ANGLE - CENTER_RANGE;

    if (angle_s0 > hi && angle_s1 > hi && angle_s2 > hi
     && angle_s1 < angle_s0 && angle_s1 < angle_s2) {
        run_state = 21;
    }
    else if (angle_s0 < lo && angle_s1 < lo && angle_s2 < lo
     && angle_s1 > angle_s0 && angle_s1 > angle_s2) {
        run_state = 31;
    }
    else if (angle_s0 > lo && angle_s0 < hi && angle_s1 > lo && angle_s1 < hi) {
        if (spin_mode) {
            run_state = 5;          // 转圈 → 最后一推
        } else {
            anglePID.reset();
            positionPID.reset();
            run_state = 4;          // 起摆 → PID
        }
    }
}

//=== 推摆序列 =============================================================
static void state_push_enter(float) {
    motor.set_pwm(push_pwm());
    count_time = START_TIME;
    run_state++;
}
static void state_wait(float) {
    if (--count_time == 0) run_state++;
}
static void state_push_reverse(float) {
    motor.set_pwm(reverse_pwm());
    count_time = START_TIME;
    run_state++;
}
static void state_wait_final(float) {
    if (--count_time == 0) { motor.set_pwm(0); run_state = 1; }
}

//=== State 5 & 6: 转圈最后一推 → 停机 ======================================
static void state_final_push(float angle) {
    if (run_state == 5) {
        motor.set_pwm((angle > CENTER_ANGLE) ? FINAL_PWM : -FINAL_PWM);
        count_time = FINAL_TIME;
        run_state = 6;
    } else {
        if (--count_time == 0) { motor.set_pwm(0); run_state = 0; }
    }
}

//=== State 4: PID ==========================================================
static void state_pid(float angle) {
    float hi = CENTER_ANGLE + CENTER_RANGE;
    float lo = CENTER_ANGLE - CENTER_RANGE;

    if (++angle_cnt >= ANGLE_DOWNSAMPLE) {
        angle_cnt = 0;
        if (angle > lo && angle < hi)
            motor.set_pwm(anglePID.calculate(angle));
        else {
            motor.set_pwm(0); anglePID.reset(); positionPID.reset();
            run_state = 0;
        }
    }
    if (++pos_cnt >= POS_DOWNSAMPLE) {
        pos_cnt = 0;
        anglePID.target = CENTER_ANGLE + positionPID.calculate(motor.get_angle());
    }
}
