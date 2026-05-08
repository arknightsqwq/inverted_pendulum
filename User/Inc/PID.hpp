#ifndef PID_HPP
#define PID_HPP

/**
 * @brief PID 配置参数结构体
 */
struct PIDConfig {
    float kp;
    float ki;
    float kd;
    float out_min;  // 输出下限（如 -100.0f）
    float out_max;  // 输出上限（如 100.0f）
    float i_limit;  // 积分限幅，防止积分饱和
};

class PID {
public:
    PIDConfig _cfg;
    /**
     * @brief 构造函数
     * @param config PID 初始配置参数
     */
    explicit PID(const PIDConfig& config);

    /**
     * @brief 执行 PID 计算
     * @param target 设定目标值
     * @param measured 实际测量值
     * @return 控制输出量
     */
    float Calculate(float target, float measured);

    /**
     * @brief 重置积分项和历史误差（在电机重启或切换模式时使用）
     */
    void Reset();

    /**
     * @brief 运行时动态更新 PID 参数（用于上位机调参）
     */
    void UpdateConfig(const PIDConfig& new_config);

private:
    float _integral;
    float _last_error;
};


#endif // PID_HPP