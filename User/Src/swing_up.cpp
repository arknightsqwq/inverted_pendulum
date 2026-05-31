/**
 * @file    swing_up.cpp
 * @brief   倒立摆自动起摆状态机
 *
 * 状态机结构：
 *
 *   State 0: 停止 — 电机断电，等待按键启动
 *   State 1: 判断 — 自由摆动，采样检测峰值，决定推摆方向和时机
 *   State 21→24:  左推序列（检测到右侧最高点时触发）
 *   State 31→34:  右推序列（检测到左侧最高点时触发）
 *   State 4:  PID 控制 — 角度环 200Hz + 位置环 20Hz
 *
 *
 *   按键启动流程：
 *
 *     State 0 ──button4──→ State 21 ──→ 22 ──→ 23 ──→ 24 ──→ State 1
 *                            ↑         首次踹一脚，让摆杆离开底部盲区
 *                            │
 *     State 1 ──右峰检测──→ 21 ──→ 22 ──→ 23 ──→ 24 ──→ State 1  循环摆荡
 *             ──左峰检测──→ 31 ──→ 32 ──→ 33 ──→ 34 ──→ State 1  循环摆荡
 *             ──进入中心──→ 4 (PID接管)
 *
 *     State 4 ──倒下──→ State 0
 *             ──button4──→ State 0（手动停）
 *
 *
 *   共振原理：在摆杆摆到最高点时，顺着运动方向给一个短脉冲推力。
 *   - 不需要测速度，只用三次连续采样的大小关系判断运动方向
 *   - 顺向推力使摆的机械能和电机驱动力同向叠加，振幅逐渐增大
 *   - 进入中心区间后自动切 PID 接管
 *
 *   参数：
 *     CENTER_ANGLE  — 平衡点角度 (度)
 *     CENTER_RANGE  — 中心区间 ±30°，进入此区间认为摆够高
 *     START_PWM     — 推摆力度 (PWM ±100)
 *     START_TIME    — 每段脉冲时长 (ms)
 *     JUDGE_DOWNSAMPLE — 判断采样间隔，40 × 1ms = 40ms
 */

#include "swing_up.hpp"
#include "global.hpp"

//=== 起摆参数 ============================================================
#define CENTER_ANGLE   179.0f      // 平衡点角度值（度）
#define CENTER_RANGE   30.0f       // 中心区间范围，149° ~ 209°
#define START_PWM      35          // 起摆推力（PWM 范围 -100 ~ 100）
#define START_TIME     100         // 推摆时长（ms）

//=== 分频（TIM1 基频 1kHz = 每 1ms 进一次 ISR）===========================
#define ANGLE_DOWNSAMPLE  5        // 内环角度 PID 频率 = 1kHz / 5 = 200Hz
#define POS_DOWNSAMPLE    50       // 外环位置 PID 频率 = 1kHz / 50 = 20Hz
#define JUDGE_DOWNSAMPLE  40       // 判断采样频率   = 1kHz / 40 = 25Hz（40ms 采一次）

//=== 状态机静态变量 ========================================================
static uint16_t count_time = 0;             // 推摆延时计数器（每 1ms 减 1）
static float    angle_s0 = 0,               // 最近一次角度采样（t）
                angle_s1 = 0,                 // 上一次       （t - 40ms）
                angle_s2 = 0;                 // 上上次       （t - 80ms）
static uint8_t  angle_cnt = 0,              // 内环分频计数
                pos_cnt = 0,                 // 外环分频计数
                judge_cnt = 0;               // 判断采样分频计数

//=== 辅助函数 ==============================================================

/**
 * @brief 推摆方向
 *        State 21（左推序列）→ +START_PWM
 *        State 31（右推序列）→ -START_PWM
 */
static inline int8_t push_pwm() {
    return (run_state < 30) ? START_PWM : -START_PWM;
}

/**
 * @brief 反向脉冲方向（推完收回一下，防过推）
 */
static inline int8_t reverse_pwm() {
    return (run_state < 30) ? -START_PWM : START_PWM;
}

//=== 状态函数前向声明 ======================================================
static void state_stop(float);
static void state_judge(float);
static void state_push_enter(float);
static void state_wait(float);
static void state_push_reverse(float);
static void state_wait_final(float);
static void state_pid(float);

typedef void (*StateFunc)(float);

//=== 状态跳转（switch 派发）===============================================
// 21~24 和 31~34 共用同一组函数，通过 push_pwm()/reverse_pwm() 区分方向

void state_handler(float angle) {
    switch (run_state) {
        case 0:  state_stop(angle);           break;
        case 1:  state_judge(angle);          break;
        case 4:  state_pid(angle);            break;
        case 21: state_push_enter(angle);     break;  // ┐ 左推序列
        case 22: state_wait(angle);           break;  // │
        case 23: state_push_reverse(angle);   break;  // │ 21→22→23→24→1
        case 24: state_wait_final(angle);     break;  // ┘
        case 31: state_push_enter(angle);     break;  // ┐ 右推序列
        case 32: state_wait(angle);           break;  // │
        case 33: state_push_reverse(angle);   break;  // │ 31→32→33→34→1
        case 34: state_wait_final(angle);     break;  // ┘
    }
}

//=== State 0: 停止 ========================================================

/**
 * @brief 电机完全断电，等待 button4 按下启动
 */
static void state_stop(float) {
    motor.set_pwm(0);
}

//=== State 1: 判断 ========================================================

/**
 * @brief 每 40ms 采样一次角度，维护三样本滑动窗口。
 *        检测到峰值 → 进入对应推摆序列
 *        检测到摆杆进入中心区间 → 切换到 PID 控制
 */
static void state_judge(float angle) {
    motor.set_pwm(0);         // 判断期间不驱动，摆杆自由摆动

    // 降采样到 25Hz
    if (++judge_cnt < JUDGE_DOWNSAMPLE) return;
    judge_cnt = 0;

    // 三样本滑动窗口：s2(最老) → s1 → s0(最新)
    angle_s2 = angle_s1;
    angle_s1 = angle_s0;
    angle_s0 = angle;

    float hi = CENTER_ANGLE + CENTER_RANGE;
    float lo = CENTER_ANGLE - CENTER_RANGE;

    /*
     * 峰值检测原理（以右侧最高点为例）：
     *
     *      angle_s0         摆杆：右侧最高点 ─┐
     *     /       \                          │ 即将往左摆
     *    /  angle_s2 \                       │
     *   /   angle_s1   \                     │
     * ─┴────────────────┴──                 │
     *                    └── 左推，顺向加能量
     *
     *  条件：三个样本都在右侧 AND 中间样本最小
     */

    // 右侧最高点 → 进入左推序列 (21)
    if (angle_s0 > hi && angle_s1 > hi && angle_s2 > hi
     && angle_s1 < angle_s0 && angle_s1 < angle_s2) {
        run_state = 21;
    }
    // 左侧最高点 → 进入右推序列 (31)
    else if (angle_s0 < lo && angle_s1 < lo && angle_s2 < lo
     && angle_s1 > angle_s0 && angle_s1 > angle_s2) {
        run_state = 31;
    }
    // 连续两次在中心区间内 → 摆够高，PID 接管
    else if (angle_s0 > lo && angle_s0 < hi
     && angle_s1 > lo && angle_s1 < hi) {
        anglePID.reset();
        positionPID.reset();
        run_state = 4;
    }
}

//=== 起摆序列（21/31 共用）=================================================
//
//   时序：推 100ms → 等 → 反推 100ms → 等 → 回到判断
//
//   之所以推完再反向推一次：
//     单方向推太久会破坏谐振节奏，反向短脉冲"收回来"，
//     每次净注入的能量刚好够振幅增大一点

/** @brief 第一阶段：施加推力，设置计时器，转到等待 */
static void state_push_enter(float) {
    motor.set_pwm(push_pwm());
    count_time = START_TIME;
    run_state++;  // 21→22 / 31→32
}

/** @brief 第二阶段：等待计时结束 */
static void state_wait(float) {
    if (--count_time == 0) run_state++;  // 22→23 / 32→33
}

/** @brief 第三阶段：施加反向脉冲 */
static void state_push_reverse(float) {
    motor.set_pwm(reverse_pwm());
    count_time = START_TIME;
    run_state++;  // 23→24 / 33→34
}

/** @brief 第四阶段：等待计时结束，回到判断状态 */
static void state_wait_final(float) {
    if (--count_time == 0) {
        motor.set_pwm(0);
        run_state = 1;
    }
}

//=== State 4: PID 控制 ====================================================

/**
 * @brief 串级 PID 倒立摆控制
 *        内环（200Hz）：传感器角度 → 角度 PID → 电机 PWM
 *        外环（20Hz）： 编码器位置 → 位置 PID → 修正角度目标值
 *
 *        摆杆倒下（角度超出中心区间）→ 自动停机到 State 0
 */
static void state_pid(float angle) {
    float hi = CENTER_ANGLE + CENTER_RANGE;
    float lo = CENTER_ANGLE - CENTER_RANGE;

    //--- 内环 200Hz ---
    if (++angle_cnt >= ANGLE_DOWNSAMPLE) {
        angle_cnt = 0;
        if (angle > lo && angle < hi) {
            // 摆杆在中心区间内 → 正常 PID
            motor.set_pwm(anglePID.calculate(angle));
        } else {
            // 摆杆倒下 → 安全停机
            motor.set_pwm(0);
            anglePID.reset();
            positionPID.reset();
            run_state = 0;
        }
    }

    //--- 外环 20Hz ---
    //  位置 PID 输出作为角度目标偏置（绝对设置方式）
    //  正偏差 = 往正方向修正角度目标
    if (++pos_cnt >= POS_DOWNSAMPLE) {
        pos_cnt = 0;
        anglePID.target = CENTER_ANGLE
                        + positionPID.calculate(motor.get_angle());
    }
}
