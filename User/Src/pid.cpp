#include "pid.hpp"

PID::PID(float kp, float ki, float kd,
         float out_min, float out_max, float i_limit,
         float target)
    : kp(kp), ki(ki), kd(kd),
      out_min(out_min), out_max(out_max), i_limit(i_limit),
      target(target),
      _integral(0), _last_error(0) {
    _sync_config();
}

void PID::_sync_config() {
    _q_kp      = to_q12(kp);
    _q_ki      = to_q12(ki);
    _q_kd      = to_q12(kd);
    _q_out_min = to_q12(out_min);
    _q_out_max = to_q12(out_max);
    _q_i_limit = to_q12(i_limit);
}

void PID::sync() {
    _sync_config();
}

float PID::calculate(float measured) {
    int32_t error = to_q12(target - measured);

    _integral += error;
    if (_integral > _q_i_limit)       _integral = _q_i_limit;
    else if (_integral < -_q_i_limit) _integral = -_q_i_limit;

    int32_t derivative = error - _last_error;

    int32_t output = static_cast<int32_t>(
        ((int64_t)_q_kp * error      >> Q12) +
        ((int64_t)_q_ki * _integral  >> Q12) +
        ((int64_t)_q_kd * derivative >> Q12));

    _last_error = error;

    if (output > _q_out_max)       output = _q_out_max;
    else if (output < _q_out_min)  output = _q_out_min;

    return from_q12(output);
}

float PID::get_integral() const {
    return from_q12(_integral);
}

void PID::reset() {
    _integral   = 0;
    _last_error = 0;
}
