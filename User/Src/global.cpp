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
DegreeSensor sensor(&hadc1, 1, 0);  // 无偏置，ADC 0~4096 直接映射 0~360°

//电机
Motor motor(&htim3, &htim2, TIM_CHANNEL_1,
             GPIOB, GPIO_PIN_12,
             GPIOB, GPIO_PIN_13, 408);  // 编码器 408 计数/圈，映射 0~360°

//串口
UART_Object pclink(&huart1);

//按钮
Button<1> button1(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);
Button<2> button2(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);
Button<3> button3(GPIOA,GPIO_PIN_11,GPIO_PIN_RESET);
Button<4> button4(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET);

//PID
//内环角度PID：输入 ADC计数值 → 输出 %PWM，运行频率 200Hz
PID anglePID(0.2f, 0.01f, 0.4f,
             -100.0f, 100.0f, 5000.0f, 2056.0f);
//外环位置PID：输入 编码器计数值 → 输出 ADC偏置，运行频率 200Hz
PID positionPID(0.4f, 0.0f, 4.0f,
                -100.0f, 100.0f, 500.0f, 0.0f);

volatile uint32_t pid_isr_count = 0;

volatile bool is_pid_running = false;

volatile int g_angle = 0;
volatile int16_t g_location = 0;

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
        is_pid_running = !is_pid_running;
        break;
    case LONG_PRESS:  break;
    case HOLDING:     break;
    default:          break;
    }
}
