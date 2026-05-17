#include "pid.hpp"

PID::PID(const PIDConfig& config)
    : _cfg(config), _integral(0), _last_error(0) {
    _sync_config();
}

void PID::_sync_config() {
    _q_kp      = to_q12(_cfg.kp);
    _q_ki      = to_q12(_cfg.ki);
    _q_kd      = to_q12(_cfg.kd);
    _q_out_min = to_q12(_cfg.out_min);
    _q_out_max = to_q12(_cfg.out_max);
    _q_i_limit = to_q12(_cfg.i_limit);
}

float PID::calculate(float target, float measured) {
    int32_t error = to_q12(target - measured);

    _integral += error;
    if (_integral > _q_i_limit)       _integral = _q_i_limit;
    else if (_integral < -_q_i_limit) _integral = -_q_i_limit;

    int32_t derivative = error - _last_error;

    // Q12 × Q12 → Q24，右移 12 位回到 Q12；用 int64_t 防溢出
    int32_t output = static_cast<int32_t>(
        ((int64_t)_q_kp * error      >> Q12) +
        ((int64_t)_q_ki * _integral  >> Q12) +
        ((int64_t)_q_kd * derivative >> Q12));

    _last_error = error;

    if (output > _q_out_max)       output = _q_out_max;
    else if (output < _q_out_min)  output = _q_out_min;

    return from_q12(output);
}

void PID::reset() {
    _integral   = 0;
    _last_error = 0;
}

void PID::update_config(const PIDConfig& new_config) {
    _cfg = new_config;
    _sync_config();
}
