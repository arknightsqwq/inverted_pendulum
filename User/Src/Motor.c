#include "Motor.h"
#include <stdlib.h>

static int16_t Get_Location(Motor_t* self) {
    // 获取计数器的当前值
    return (int16_t)__HAL_TIM_GET_COUNTER(self->htim_Encoder);
}

static void Set_PWM(Motor_t* self, int8_t dutyCycle) {
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(self->htim_Driver);
    uint32_t compare_value = abs(dutyCycle) * arr / 100;
    if (dutyCycle > 0) {
        HAL_GPIO_WritePin(self->DIR_Port_A, self->DIR_Pin_A, GPIO_PIN_SET);   // 示例
        HAL_GPIO_WritePin(self->DIR_Port_B, self->DIR_Pin_B, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(self->DIR_Port_A, self->DIR_Pin_A, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(self->DIR_Port_B, self->DIR_Pin_B, GPIO_PIN_SET);
    }
    __HAL_TIM_SET_COMPARE(self->htim_Driver, self->Channel, (uint16_t)compare_value);
}

void Motor_Init(Motor_t* self,
                TIM_HandleTypeDef* htim_Encoder,
                TIM_HandleTypeDef* htim_Driver,
                uint8_t Channel,
                GPIO_TypeDef* DIR_Port_A,
                uint16_t DIR_Pin_A,
                GPIO_TypeDef* DIR_Port_B,
                uint16_t DIR_Pin_B) {

    // 1. 硬件句柄与通道赋值
    self->htim_Encoder = htim_Encoder;
    self->htim_Driver = htim_Driver;
    self->Channel = Channel;

    // 2. GPIO 端口与引脚赋值（这是之前漏掉的关键步骤）
    self->DIR_Port_A = DIR_Port_A;
    self->DIR_Pin_A = DIR_Pin_A;
    self->DIR_Port_B = DIR_Port_B;
    self->DIR_Pin_B = DIR_Pin_B;

    // 3. 绑定成员方法 (指向具体实现函数)
    self->GetLocation = Get_Location;
    self->SetPWM = Set_PWM;

    // 4. 启动硬件外设
    HAL_TIM_Encoder_Start(self->htim_Encoder, TIM_CHANNEL_ALL);
    HAL_TIM_PWM_Start(self->htim_Driver, self->Channel);
}