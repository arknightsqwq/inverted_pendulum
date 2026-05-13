#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include "DegreeSensor.hpp"
#include "Motor.hpp"
#include "Button.hpp"
#include "UART_Object.hpp"
#include "PID.hpp"

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


#endif // GLOBAL_HPP
