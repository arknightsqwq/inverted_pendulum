#ifndef DEGREE_SENSOR_HPP
#define DEGREE_SENSOR_HPP

#include "stm32f1xx_hal.h"

class DegreeSensor {
public:
    /**
     * @brief 构造函数
     * @param hadc ADC 句柄
     * @param alpha_q12 一阶低通滤波系数 (Q12 定点, 默认 1229 ≈ 0.3)
     */
    explicit DegreeSensor(ADC_HandleTypeDef* hadc, uint16_t alpha_q12 = 1229);

    /**
     * @brief 启动传感器（校准并开启 DMA）
     */
    void start();

    /**
     * @brief 获取滤波后的角度值
     * @return 经过一阶滤波后的 ADC 读数
     */
    int get_degree();

private:
    ADC_HandleTypeDef* _hadc; // ADC 硬件句柄
    uint16_t _adc_buf;        // DMA 原始缓冲区
    uint16_t _alpha_q12;      // 滤波系数 (Q12 定点)
    int _last_val;            // 上一次的滤波结果
};

#endif // DEGREE_SENSOR_HPP