#include "degree_sensor.hpp"

DegreeSensor::DegreeSensor(ADC_HandleTypeDef* hadc, uint16_t alpha_q12)
    : _hadc(hadc),
      _adc_buf(0),
      _alpha_q12(alpha_q12),
      _last_val(0) {
}

void DegreeSensor::start() {
    HAL_ADCEx_Calibration_Start(_hadc);
    HAL_ADC_Start_DMA(_hadc, reinterpret_cast<uint32_t*>(&_adc_buf), 1);
}

int DegreeSensor::get_degree() {
    int raw = static_cast<int>(_adc_buf);
    // 一阶低通滤波: last += alpha * (raw - last), Q12 定点
    int diff = raw - _last_val;
    _last_val += (static_cast<int>(_alpha_q12) * diff + 2048) >> 12;
    return _last_val;
}