#ifndef DEGREE_SENSOR_HPP
#define DEGREE_SENSOR_HPP

#include "stm32f1xx_hal.h"

class DegreeSensor {
public:
    /**
     * @brief 构造函数
     * @param hadc ADC 句柄
     * @param alpha 一阶低通滤波系数 (0.0 ~ 1.0, 默认 0.3)
     */
    explicit DegreeSensor(ADC_HandleTypeDef* hadc, float alpha = 0.3f, int adc_bias = 0);

    /**
     * @brief 启动传感器（校准并开启 DMA）
     */
    void start();

    /**
     * @brief 获取滤波后的角度值
     * @return 角度 (0~360°)，映射自 ADC 0~4095
     */
    float get_angle();

    /**
     * @brief 获取 ADC 滤波原始值
     * @return ADC 0~4096
     */
    int get_raw();

private:
    ADC_HandleTypeDef* _hadc; // ADC 硬件句柄
    uint16_t _adc_buf;        // DMA 原始缓冲区
    float _alpha;             // 一阶低通滤波系数 (0.0 ~ 1.0)
    float _last_val;          // 上一次的滤波结果
    int _adc_bias;            // ADC 偏置，使 (原生值 + bias) % 4096 → 0°
};

#endif // DEGREE_SENSOR_HPP