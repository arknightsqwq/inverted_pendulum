#include "motor.hpp"

Motor::Motor(TIM_HandleTypeDef* htim_Encoder,
             TIM_HandleTypeDef* htim_Driver,
             uint32_t Channel,
             GPIO_TypeDef* DIR_Port_A, uint16_t DIR_Pin_A,
             GPIO_TypeDef* DIR_Port_B, uint16_t DIR_Pin_B,
             int16_t counts_per_rev)
    : _htim_encoder(htim_Encoder),
      _htim_driver(htim_Driver),
      _channel(Channel),
      _dir_port_a(DIR_Port_A),
      _dir_pin_a(DIR_Pin_A),
      _dir_port_b(DIR_Port_B),
      _dir_pin_b(DIR_Pin_B),
      _counts_per_rev(counts_per_rev) {
    // 构造函数仅负责成员赋值
}

void Motor::start() {
    __HAL_TIM_SET_COUNTER(_htim_encoder, 0);
    _last_count = 0; // 记录值同步清零
    HAL_TIM_Encoder_Start(_htim_encoder, TIM_CHANNEL_ALL);
    HAL_TIM_PWM_Start(_htim_driver, _channel);
}

int16_t Motor::get_location() const {
    return static_cast<int16_t>(__HAL_TIM_GET_COUNTER(_htim_encoder));
}
int16_t Motor::get_speed() {
    // 1. 获取当前位置
    int16_t current_count = static_cast<int16_t>(__HAL_TIM_GET_COUNTER(_htim_encoder));

    // 2. 计算差值（考虑了 16 位计数器溢出的自动处理）
    int16_t speed = current_count - _last_count;

    // 3. 更新上一次的值，供下次使用
    _last_count = current_count;

    return speed;
}

float Motor::get_angle() const {
    return static_cast<float>(get_location()) / _counts_per_rev * 360.0f;
}

float Motor::get_angular_velocity() {
    // get_speed() 返回每调用间隔的计数值变化量
    // 调用间隔通常为 1ms (ISR 频率)，乘以 1000 得到 counts/s
    // 再映射到度/秒
    return static_cast<float>(get_speed()) * 1000.0f / _counts_per_rev * 360.0f;
}

void Motor::set_pwm(float duty_cycle) {
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(_htim_driver);
    float abs_duty = duty_cycle > 0 ? duty_cycle : -duty_cycle;
    uint32_t compare_value = static_cast<uint32_t>(abs_duty / 100.0f * arr);

    if (duty_cycle > 0.0f) {
        HAL_GPIO_WritePin(_dir_port_a, _dir_pin_a, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(_dir_port_b, _dir_pin_b, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(_dir_port_a, _dir_pin_a, GPIO_PIN_SET);
        HAL_GPIO_WritePin(_dir_port_b, _dir_pin_b, GPIO_PIN_RESET);
    }

    __HAL_TIM_SET_COMPARE(_htim_driver, _channel, static_cast<uint16_t>(compare_value));
}