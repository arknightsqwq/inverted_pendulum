#ifndef DEGREE_SENSOR_H
#define DEGREE_SENSOR_H

#include "stm32f1xx_hal.h"

typedef struct DegreeSensor {
    // 属性
    ADC_HandleTypeDef* hadc;
    uint16_t adc_buf;
    float alpha;
    int last_val;

    // 函数指针，在Init里被指定为某一函数
    int (*GetDegree)(struct DegreeSensor* self);
} DegreeSensor;

// 构造函数：负责把上面的零件组装起来
void DegreeSensor_Init(DegreeSensor* self, ADC_HandleTypeDef* hadc);

#endif