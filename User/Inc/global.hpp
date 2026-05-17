#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include "degree_sensor.hpp"
#include "motor.hpp"
#include "button.hpp"
#include "uart_object.hpp"
#include "pid.hpp"

extern DegreeSensor sensor;
extern Motor motor;
extern Button<1> button1;
extern Button<2> button2;
extern Button<3> button3;
extern Button<4> button4;
extern UART_Object pclink;

extern PIDConfig angleloop;
extern PIDConfig positionloop;
extern PID anglePID;
extern PID positionPID;

extern volatile uint32_t pid_isr_count;

#endif // GLOBAL_HPP
