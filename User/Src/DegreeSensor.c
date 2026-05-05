#include "DegreeSensor.h"

// 1. 编写实际的逻辑函数
static int Get_Degree(DegreeSensor_t* self) {
    int raw = self->adc_buf; // 直接读取 DMA 自动更新的缓存
    self->last_val = (int)(self->alpha * raw + (1.0f - self->alpha) * self->last_val);
    return self->last_val;
}

// 2. 构造函数：建立绑定关系
void DegreeSensor_Init(DegreeSensor_t* self, ADC_HandleTypeDef* hadc) {
    self->hadc = hadc;
    self->alpha = 0.3f;
    self->last_val = 0;

    // 【关键】将函数指针指向具体的实现逻辑
    self->GetDegree = Get_Degree;

    // 启动硬件驱动
    HAL_ADCEx_Calibration_Start(self->hadc);
    HAL_ADC_Start_DMA(self->hadc, (uint32_t*)&(self->adc_buf), 1);
}