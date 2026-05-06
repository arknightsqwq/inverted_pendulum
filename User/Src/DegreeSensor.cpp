#include "DegreeSensor.hpp"

// 使用初始化列表赋值，比在函数体内存取更高效
DegreeSensor::DegreeSensor(ADC_HandleTypeDef* hadc, float alpha)
    : _hadc(hadc),
      _adc_buf(0),
      _alpha(alpha),
      _last_val(0) {
    // 构造函数仅负责属性初始化
}

void DegreeSensor::start() {
    // 1. 硬件校准
    HAL_ADCEx_Calibration_Start(_hadc);
    // 2. 启动 DMA 连续采样，指向私有变量地址
    HAL_ADC_Start_DMA(_hadc, reinterpret_cast<uint32_t*>(&_adc_buf), 1);
}

int DegreeSensor::getdegree() {
    // 读取 DMA 自动更新的缓存
    int raw = static_cast<int>(_adc_buf);

    // 一阶低通滤波算法逻辑
    _last_val = static_cast<int>(_alpha * raw + (1.0f - _alpha) * _last_val);

    return _last_val;
}