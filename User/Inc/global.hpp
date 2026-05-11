#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include "DegreeSensor.hpp"
#include "Motor.hpp"
#include "Button.hpp"
#include "UART_Object.hpp"
#include "PID.hpp"

extern DegreeSensor sensor;
extern Motor motor;
extern Button button1;
extern Button button2;
extern Button button3;
extern Button button4;
extern UART_Object pclink;

extern PIDConfig angleloop;
extern PIDConfig positionloop;
extern PID anglePID;
extern PID positionPID;

#endif // GLOBAL_HPP
