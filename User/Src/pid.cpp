#include "pid.hpp"

PID::PID(const PIDConfig& config)
    : _cfg(config), _integral(0.0f), _last_error(0.0f) {
}

float PID::calculate(float target, float measured) {
    // 1. 计算当前误差
    float error = target - measured;

    // 2. 计算积分项
    _integral += error;

    // 积分限幅 (Anti-Windup)
    if (_integral > _cfg.i_limit) _integral = _cfg.i_limit;
    else if (_integral < -_cfg.i_limit) _integral = -_cfg.i_limit;

    // 3. 计算微分项 (Error - Last_Error)
    float derivative = error - _last_error;

    // 4. PID 公式叠加
    float output = (_cfg.kp * error) +
                   (_cfg.ki * _integral) +
                   (_cfg.kd * derivative);

    // 5. 保存误差用于下一次计算
    _last_error = error;

    // 6. 输出限幅
    if (output > _cfg.out_max) output = _cfg.out_max;
    else if (output < _cfg.out_min) output = _cfg.out_min;

    return output;
}

void PID::reset() {
    _integral = 0.0f;
    _last_error = 0.0f;
}

void PID::update_config(const PIDConfig& new_config) {
    _cfg = new_config;
}
