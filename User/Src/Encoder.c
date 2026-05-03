#include "Encoder.h"

static int16_t Get_Location(struct Encoder* self) {
    // 获取计数器的当前值[cite: 1]
    return (int16_t)__HAL_TIM_GET_COUNTER(self->htim);
}

// 2. 构造函数：建立绑定关系
void Encoder_Init(Encoder* self, TIM_HandleTypeDef* htim) {
    self->htim = htim;
    HAL_TIM_Encoder_Start(self->htim, TIM_CHANNEL_ALL);
    self->GetLocation = Get_Location;
}