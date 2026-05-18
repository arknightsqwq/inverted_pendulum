#ifndef LQR_HPP
#define LQR_HPP

#include <cstdint>

class LQR {
public:
    /**
     * @brief 构造函数
     * @param n 状态维度（默认 4: 角度, 角速度, 臂位置, 臂速度）
     * @param K_gain 增益矩阵 K 指针（列优先，长度 n）
     */
    LQR(int n = 4, const float* K_gain = nullptr);

    /**
     * @brief 计算控制量 u = -K * x
     * @param state 状态向量指针（长度 n）
     * @return 控制输出
     */
    float calculate(const float* state);

    /**
     * @brief 更新增益矩阵
     */
    void update_gain(const float* K_gain);

private:
    int _n;                // 状态维度
    const float* _K;       // 增益矩阵指针（外部传入，不管理生命周期）
};

#endif // LQR_HPP
