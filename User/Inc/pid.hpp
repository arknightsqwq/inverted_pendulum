#ifndef PID_HPP
#define PID_HPP

#include <cstdint>

/**
 * @brief PID 配置参数结构体（浮点输入，由 PID 类自动转为 Q12 定点数）
 */
struct PIDConfig {
    float kp;
    float ki;
    float kd;
    float out_min;
    float out_max;
    float i_limit;
    int target;
};

class PID {
public:
    PIDConfig _cfg;

    explicit PID(const PIDConfig& config);

    /**
     * @brief 执行 PID 计算（内部使用 Q12 定点数运算）
     * @param target 设定目标值
     * @param measured 实际测量值
     * @return 控制输出量
     */
    float calculate(float target, float measured);

    void reset();
    void update_config(const PIDConfig& new_config);

private:
    static constexpr int Q12 = 12;
    static constexpr int Q12_SCALE = 1 << Q12;   // 4096

    // Q12 定点数配置
    int32_t _q_kp, _q_ki, _q_kd;
    int32_t _q_out_min, _q_out_max, _q_i_limit;

    int32_t _integral;    // Q12
    int32_t _last_error;  // Q12

    static int32_t to_q12(float v)   { return static_cast<int32_t>(v * Q12_SCALE); }
    static float   from_q12(int32_t v) { return static_cast<float>(v) / Q12_SCALE; }

    void _sync_config();
};

#endif // PID_HPP