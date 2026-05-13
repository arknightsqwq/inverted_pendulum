#include "adc.h"
#include "tim.h"
#include "usart.h"

#include "DegreeSensor.hpp"
#include "Motor.hpp"
#include "Button.hpp"
#include "UART_Object.hpp"
#include "PID.hpp"
#include "global.hpp"

//角度传感器
DegreeSensor sensor(&hadc1);

//电机（TIM1_CH2 -> PA9；方向 PB0 / PB1）
Motor motor(&htim2, &htim1, TIM_CHANNEL_2,
             GPIOB, GPIO_PIN_0,
             GPIOB, GPIO_PIN_1);

//串口
UART_Object pclink(&huart2);

//按钮
Button<1> button1(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
Button<2> button2(GPIOB,GPIO_PIN_8,GPIO_PIN_SET);
Button<3> button3(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);
Button<4> button4(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);

//PID
PIDConfig angleloop{0,0,0,0,0,0,0};
PIDConfig positionloop{0,0,0,0,0,0,0};
PID anglePID(angleloop);
PID positionPID(positionloop);

// --- 各按钮事件处理特化 ---

template <>
void onButtonEvent<1>(Button<1>::Event e) {
    switch (e) {
    case Button<1>::SHORT_PRESS: break;
    case Button<1>::LONG_PRESS:  break;
    case Button<1>::HOLDING:     break;
    default:                     break;
    }
}

template <>
void onButtonEvent<2>(Button<2>::Event e) {
    switch (e) {
    case Button<2>::SHORT_PRESS: break;
    case Button<2>::LONG_PRESS:  break;
    case Button<2>::HOLDING:     break;
    default:                     break;
    }
}

template <>
void onButtonEvent<3>(Button<3>::Event e) {
    switch (e) {
    case Button<3>::SHORT_PRESS: break;
    case Button<3>::LONG_PRESS:  break;
    case Button<3>::HOLDING:     break;
    default:                     break;
    }
}

template <>
void onButtonEvent<4>(Button<4>::Event e) {
    switch (e) {
    case Button<4>::SHORT_PRESS: break;
    case Button<4>::LONG_PRESS:  break;
    case Button<4>::HOLDING:     break;
    default:                     break;
    }
}