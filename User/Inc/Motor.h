#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "stm32f1xx_hal.h"

typedef enum {
    Channel_1 = 1,
    Channel_2 = 2,
    Channel_3 = 3,
    Channel_4 = 4
} Channel_t;

typedef struct Motor_t {
    TIM_HandleTypeDef* htim_Encoder;
    TIM_HandleTypeDef* htim_Driver;
    uint8_t Channel;
    GPIO_TypeDef* DIR_Port_A;
    uint16_t DIR_Pin_A;
    GPIO_TypeDef* DIR_Port_B;
    uint16_t DIR_Pin_B;

    // 成员方法
    int16_t (*GetLocation)(struct Motor_t* self);
    void (*SetPWM)(struct Motor_t* self, int8_t dutyCycle);
} Motor_t;

void Motor_Init(Motor_t* self,
                TIM_HandleTypeDef* htim_Encoder,
                TIM_HandleTypeDef* htim_Driver,
                uint8_t Channel,
                GPIO_TypeDef* DIR_Port_A,
                uint16_t DIR_Pin_A,
                GPIO_TypeDef* DIR_Port_B,
                uint16_t DIR_Pin_B);

#endif