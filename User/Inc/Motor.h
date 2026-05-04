#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "stm32f1xx_hal.h"

typedef struct Motor {
    TIM_HandleTypeDef* htim_Encoder;
    TIM_HandleTypeDef* htim_Driver;
    uint8_t Channel;
    GPIO_TypeDef* DIR_Port_A;
    uint16_t DIR_Pin_A;
    GPIO_TypeDef* DIR_Port_B;
    uint16_t DIR_Pin_B;

    // 成员方法
    int16_t (*GetLocation)(struct Motor* self);
    void (*SetPWM)(struct Motor* self, int8_t dutyCycle);
} Motor;

#endif