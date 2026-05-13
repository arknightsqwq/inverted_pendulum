#include "adc.h"
#include "tim.h"
#include "usart.h"

#include "DegreeSensor.hpp"
#include "Motor.hpp"
#include "Button.hpp"
#include "UART_Object.hpp"
#include "PID.hpp"
#include "global.hpp"

// 按钮特化声明 — 非公共接口，放在实现文件中
template <> void Button<1>::processEvent();
template <> void Button<2>::processEvent();
template <> void Button<3>::processEvent();
template <> void Button<4>::processEvent();

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
void Button<1>::processEvent() {
    switch (get_event()) {
    case SHORT_PRESS: break;
    case LONG_PRESS:  break;
    case HOLDING:     break;
    default:          break;
    }
}

template <>
void Button<2>::processEvent() {
    switch (get_event()) {
    case SHORT_PRESS: break;
    case LONG_PRESS:  break;
    case HOLDING:     break;
    default:          break;
    }
}

template <>
void Button<3>::processEvent() {
    switch (get_event()) {
    case SHORT_PRESS: break;
    case LONG_PRESS:  break;
    case HOLDING:     break;
    default:          break;
    }
}

template <>
void Button<4>::processEvent() {
    switch (get_event()) {
    case SHORT_PRESS: break;
    case LONG_PRESS:  break;
    case HOLDING:     break;
    default:          break;
    }
}
