#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include "degree_sensor.hpp"
#include "motor.hpp"
#include "button.hpp"
#include "uart_object.hpp"
#include "pid.hpp"

// 按钮 process_event 特化声明 — 必须在调用前可见，否则编译器会实例化空模板
template <> void Button<1>::process_event();
template <> void Button<2>::process_event();
template <> void Button<3>::process_event();
template <> void Button<4>::process_event();

extern DegreeSensor sensor;
extern Motor motor;
extern Button<1> button1;
extern Button<2> button2;
extern Button<3> button3;
extern Button<4> button4;
extern UART_Object pclink;

extern PID anglePID;
extern PID positionPID;

extern volatile uint32_t pid_isr_count;

extern volatile bool is_pid_running;

// 传感器和位置全局值（ISR 更新，主循环只读，避免反复调用 get_raw 破坏滤波器）
extern volatile int g_angle;
extern volatile int16_t g_location;

#endif // GLOBAL_HPP
