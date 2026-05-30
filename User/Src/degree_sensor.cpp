#include "degree_sensor.hpp"

DegreeSensor::DegreeSensor(ADC_HandleTypeDef* hadc, float alpha, int adc_bias)
    : _hadc(hadc),
      _adc_buf(0),
      _alpha(alpha),
      _last_val(0.0f),
      _adc_bias(adc_bias) {
}

void DegreeSensor::start() {
    HAL_ADCEx_Calibration_Start(_hadc);
    HAL_ADC_Start_DMA(_hadc, reinterpret_cast<uint32_t*>(&_adc_buf), 1);
}

float DegreeSensor::get_angle() {
    int raw = static_cast<int>(_adc_buf);
    // 一阶低通滤波: last += alpha * (raw - last)
    _last_val += _alpha * (static_cast<float>(raw) - _last_val);
    // 限幅后直接映射 0~4096 → 0~360°
    if (_last_val < 0.0f)   _last_val = 0.0f;
    if (_last_val > 4096.0f) _last_val = 4096.0f;
    return _last_val * (360.0f / 4096.0f);
}

int DegreeSensor::get_raw() {
    int raw = static_cast<int>(_adc_buf);
    // 一阶低通滤波: last += alpha * (raw - last)
    _last_val += _alpha * (static_cast<float>(raw) - _last_val);
    return static_cast<int>(_last_val);
}
