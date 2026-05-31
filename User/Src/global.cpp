#include "adc.h"
#include "tim.h"
#include "usart.h"

#include "degree_sensor.hpp"
#include "motor.hpp"
#include "button.hpp"
#include "uart_object.hpp"
#include "pid.hpp"
#include "global.hpp"


//角度传感器
DegreeSensor sensor(&hadc1, 1.0f, 0);  // bias = 4096 - 3046（最下方原生值）

//电机
Motor motor(&htim3, &htim2, TIM_CHANNEL_1,
             GPIOB, GPIO_PIN_12,
             GPIOB, GPIO_PIN_13, 408);

//串口
UART_Object pclink(&huart1);

//按钮
Button<1> button1(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);
Button<2> button2(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);
Button<3> button3(GPIOA,GPIO_PIN_11,GPIO_PIN_RESET);
Button<4> button4(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET);

//PID
PID anglePID(2.28f, 0.114f, 4.55f, -100.0f, 100.0f, 200, 179.0);
//PID positionPID(0.04f, 0.0f, 0.4f, -9.0f, 9.0f, 30, 0);
PID positionPID(0.08f, 0.0f, 0.8f, -6.0f, 6.0f, 30, 0);

volatile uint32_t pid_isr_count = 0;

volatile bool is_pid_running = false;

template <>
void Button<1>::process_event() {
    switch (get_event()) {
    case SHORT_PRESS:
        anglePID.kp += 0.1f;
        anglePID.sync();
        break;
    case LONG_PRESS:  break;
    case HOLDING:
        anglePID.kp -= 0.1f;
        anglePID.sync();
        break;
    default:          break;
    }
}


template <>
void Button<2>::process_event() {
    switch (get_event()) {
    case SHORT_PRESS:
        positionPID.target += 360;
        break;
    case LONG_PRESS:  break;
    case HOLDING:     break;
    default:          break;
    }
}

template <>
void Button<3>::process_event() {
    switch (get_event()) {
    case SHORT_PRESS:
        positionPID.target -= 360;
        break;
    case LONG_PRESS:  break;
    case HOLDING:     break;
    default:          break;
    }
}

template <>
void Button<4>::process_event() {
    switch (get_event()) {
    case SHORT_PRESS:
    is_pid_running = !is_pid_running;
        break;
    case LONG_PRESS:  break;
    case HOLDING:     break;
    default:          break;
    }
}
