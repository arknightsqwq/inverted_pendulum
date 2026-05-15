#include "motor.hpp"

Motor::Motor(TIM_HandleTypeDef* htim_Encoder,
             TIM_HandleTypeDef* htim_Driver,
             uint32_t Channel,
             GPIO_TypeDef* DIR_Port_A, uint16_t DIR_Pin_A,
             GPIO_TypeDef* DIR_Port_B, uint16_t DIR_Pin_B)
    : _htim_encoder(htim_Encoder),
      _htim_driver(htim_Driver),
      _channel(Channel),
      _dir_port_a(DIR_Port_A),
      _dir_pin_a(DIR_Pin_A),
      _dir_port_b(DIR_Port_B),
      _dir_pin_b(DIR_Pin_B) {
    // 构造函数仅负责成员赋值
}

void Motor::start() {
    __HAL_TIM_SET_COUNTER(_htim_encoder, 0);
    _last_count = 0; // 记录值同步清零
    HAL_TIM_Encoder_Start(_htim_encoder, TIM_CHANNEL_ALL);
    /* TIM1 仅作 PWM；控制环路节拍由 TIM3 的更新中断在 main 中启动 */
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

void Motor::set_pwm(int8_t duty_cycle) {
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(_htim_driver);
    uint32_t compare_value = std::abs(static_cast<int>(duty_cycle)) * arr / 100;

    if (duty_cycle > 0) {
        HAL_GPIO_WritePin(_dir_port_a, _dir_pin_a, GPIO_PIN_SET);
        HAL_GPIO_WritePin(_dir_port_b, _dir_pin_b, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(_dir_port_a, _dir_pin_a, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(_dir_port_b, _dir_pin_b, GPIO_PIN_SET);
    }

    __HAL_TIM_SET_COMPARE(_htim_driver, _channel, static_cast<uint16_t>(compare_value));
}