#include "adc.h"
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
#include "global.hpp"

//角度传感器
DegreeSensor sensor(&hadc1);

//电机
Motor motor(&htim2, &htim1, TIM_CHANNEL_1,
             GPIOB, GPIO_PIN_1,
             GPIOB, GPIO_PIN_1);

//串口
UART_Object pclink(&huart2);

//按钮
Button button1(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
Button button2(GPIOB,GPIO_PIN_8,GPIO_PIN_SET);
Button button3(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);
Button button4(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);

//PID
PIDConfig angleloop{0,0,0,0,0,0,0};
PIDConfig speedloop{0,0,0,0,0,0,0};
PID anglePID(angleloop);
PID speedPID(speedloop);