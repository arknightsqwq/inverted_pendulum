#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "stm32f1xx_hal.h"

typedef struct Encoder {
    TIM_HandleTypeDef* htim;

    int16_t (*GetLocation)(struct Encoder* self);
} Encoder;

// 构造函数：负责组装零件
void Encoder_Init(Encoder* self, TIM_HandleTypeDef* htim);

#endif