#include "adc.h"
#include "tim.h"
#include "usart.h"

#include "degree_sensor.hpp"
#include "motor.hpp"
#include "button.hpp"
#include "uart_object.hpp"
#include "pid.hpp"
#include "global.hpp"

// 按钮特化声明 — 非公共接口，放在实现文件中
template <> void Button<1>::process_event();
template <> void Button<2>::process_event();
template <> void Button<3>::process_event();
template <> void Button<4>::process_event();

//角度传感器
DegreeSensor sensor(&hadc1, 0.3, 1050);  // bias = 4096 - 3046（最下方原生值）

//电机
Motor motor(&htim3, &htim2, TIM_CHANNEL_1,
             GPIOB, GPIO_PIN_12,
             GPIOB, GPIO_PIN_13, 3040);

//串口
UART_Object pclink(&huart1);

//按钮
Button<1> button1(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);
Button<2> button2(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);
Button<3> button3(GPIOA,GPIO_PIN_11,GPIO_PIN_RESET);
Button<4> button4(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET);

//PID
PID anglePID(4.0f, 0.05f, 2.0f, -100.0f, 100.0f, 200, 170.6);
PID positionPID(5.0f, 0.0f, 5.0f, -30.0f, 30.0f, 30, 20);

volatile uint32_t pid_isr_count = 0;

static float step_scale = 1.0f;  // 1.0 → 粗调 (kp/ki/kd: 0.1/0.05/0.1), 0.1 → 细调 (0.01/0.005/0.01)

// --- 各按钮事件处理特化 ---

template <>
void Button<1>::process_event() {
    switch (get_event()) {
    case SHORT_PRESS:
        anglePID.kp += 0.1f * step_scale;
        anglePID.sync();
        break;
    case LONG_PRESS:  break;
    case HOLDING:
        anglePID.kp -= 0.1f * step_scale;
        anglePID.sync();
        break;
    default:          break;
    }
}


template <>
void Button<2>::process_event() {
    switch (get_event()) {
    case SHORT_PRESS:
        anglePID.ki += 0.05f * step_scale;
        anglePID.sync();
        break;
    case LONG_PRESS:  break;
    case HOLDING:
        anglePID.ki -= 0.05f * step_scale;
        anglePID.sync();
        break;
    default:          break;
    }
}

template <>
void Button<3>::process_event() {
    switch (get_event()) {
    case SHORT_PRESS:
        anglePID.kd += 0.1f * step_scale;
        anglePID.sync();
        break;
    case LONG_PRESS:  break;
    case HOLDING:
        anglePID.kd -= 0.1f * step_scale;
        anglePID.sync();
        break;
    default:          break;
    }
}

template <>
void Button<4>::process_event() {
    switch (get_event()) {
    case SHORT_PRESS:
        step_scale = (step_scale == 1.0f) ? 0.1f : 1.0f;
        break;
    case LONG_PRESS:  break;
    case HOLDING:     break;
    default:          break;
    }
}
