#ifndef MOTOR_HPP
#define MOTOR_HPP

#include "stm32f1xx_hal.h"
#include <cstdlib> // C++ 风格的 stdlib

class Motor {
public:
    /**
     * @brief 构造函数：替代原有的 Motor_Init
     * @param htim_Encoder 编码器定时器句柄
     * @param htim_Driver  PWM 驱动定时器句柄
     * @param Channel      PWM 通道 (如 TIM_CHANNEL_1)
     */
    Motor(TIM_HandleTypeDef* htim_Encoder,
          TIM_HandleTypeDef* htim_Driver,
          uint32_t Channel,
          GPIO_TypeDef* DIR_Port_A, uint16_t DIR_Pin_A,
          GPIO_TypeDef* DIR_Port_B, uint16_t DIR_Pin_B);

    // 获取编码器位置
    int16_t get_location() const;

    // 获取速度函数
    int16_t get_speed();

    // 设置 PWM 占空比 (-100 到 100)
    void set_pwm(int8_t duty_cycle);

    // 启动硬件外设
    void start();

private:
    TIM_HandleTypeDef* _htim_encoder;
    TIM_HandleTypeDef* _htim_driver;
    uint32_t _channel;

    // 方向控制引脚
    GPIO_TypeDef* _dir_port_a;
    uint16_t _dir_pin_a;
    GPIO_TypeDef* _dir_port_b;
    uint16_t _dir_pin_b;

    mutable int16_t _last_count = 0; // 记录上一次的计数值
};

#endif // MOTOR_HPP