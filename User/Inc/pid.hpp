#ifndef PID_HPP
#define PID_HPP

#include <cstdint>

class PID {
public:
    float kp, ki, kd;
    float out_min, out_max, i_limit;
    float target;

    PID(float kp, float ki, float kd,
        float out_min, float out_max, float i_limit,
        float target = 0);

    float calculate(float measured);
    float get_integral() const;  // 返回积分累加值（用于观察积分限幅）
    void reset();
    void sync();  // 更改 kp/ki/kd 后调用，同步 Q12 定点值

private:
    static constexpr int Q12 = 12;
    static constexpr int Q12_SCALE = 1 << Q12;

    int32_t _q_kp, _q_ki, _q_kd;
    int32_t _q_out_min, _q_out_max, _q_i_limit;

    int32_t _integral;
    int32_t _last_error;

    static int32_t to_q12(float v)   { return static_cast<int32_t>(v * Q12_SCALE); }
    static float   from_q12(int32_t v) { return static_cast<float>(v) / Q12_SCALE; }

    void _sync_config();
};

#endif // PID_HPP
