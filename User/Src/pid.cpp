#include "pid.hpp"

PID::PID(float kp, float ki, float kd,
         float out_min, float out_max, float i_limit,
         float target)
    : kp(kp), ki(ki), kd(kd),
      out_min(out_min), out_max(out_max), i_limit(i_limit),
      target(target),
      Out(0), Error0(0), Error1(0), ErrorInt(0) {
}

float PID::calculate(float measured) {
    Error1 = Error0;
    Error0 = target - measured;

    // Ki=0 时不累加积分（与旧代码一致，避免 Ki 从 0 启用时积分已过大）
    if (ki != 0.0f)
        ErrorInt += Error0;
    else
        ErrorInt = 0;

    // 位置式 PID
    Out = kp * Error0
        + ki * ErrorInt
        + kd * (Error0 - Error1);

    // 输出限幅
    if (Out > out_max) Out = out_max;
    if (Out < out_min) Out = out_min;

    return Out;
}

void PID::reset() {
    ErrorInt = 0;
    Error0   = 0;
    Error1   = 0;
    Out      = 0;
}
