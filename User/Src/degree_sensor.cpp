#include "degree_sensor.hpp"

DegreeSensor::DegreeSensor(ADC_HandleTypeDef* hadc, float alpha, int adc_bias)
    : _hadc(hadc),
      _adc_buf(0),
      _alpha_q12(static_cast<uint16_t>(alpha * 4096.0f)),
      _last_val(0),
      _adc_bias(adc_bias) {
}

void DegreeSensor::start() {
    HAL_ADCEx_Calibration_Start(_hadc);
    HAL_ADC_Start_DMA(_hadc, reinterpret_cast<uint32_t*>(&_adc_buf), 1);
}

float DegreeSensor::get_degree() {
    int raw = static_cast<int>(_adc_buf);
    // 一阶低通滤波: last += alpha * (raw - last), Q12 定点
    int diff = raw - _last_val;
    _last_val += (static_cast<int>(_alpha_q12) * diff + 2048) >> 12;
    // (ADC + bias) % 4096 → 0~360°，避免负值
    int wrapped = (_last_val + _adc_bias) % 4096;
    return static_cast<float>(wrapped) * (360.0f / 4096.0f);
}

int DegreeSensor::get_nativedegree() {
    int raw = static_cast<int>(_adc_buf);
    // 一阶低通滤波: last += alpha * (raw - last), Q12 定点
    int diff = raw - _last_val;
    _last_val += (static_cast<int>(_alpha_q12) * diff + 2048) >> 12;
    return static_cast<float>(_last_val);
}