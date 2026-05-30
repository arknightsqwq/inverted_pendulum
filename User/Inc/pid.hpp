#ifndef PID_HPP
#define PID_HPP

class PID {
public:
    float kp, ki, kd;
    float out_min, out_max, i_limit;
    float target;

    float Out;       // 上次计算结果（用于 OLED 显示）
    float Error0;    // 本次误差
    float Error1;    // 上次误差
    float ErrorInt;  // 误差积分

    PID(float kp, float ki, float kd,
        float out_min, float out_max, float i_limit,
        float target = 0);

    float calculate(float measured);
    float get_integral() const { return ErrorInt; }
    void reset();
    void sync() {}  // 保留接口兼容，无操作
};

#endif // PID_HPP
