#ifndef BUTTON_H
#define BUTTON_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// 1. 定义按键会触发的事件
typedef enum {
    BTN_EVENT_NONE = 0,    // 没动作
    BTN_EVENT_SHORT_PRESS, // 短按触发
    BTN_EVENT_LONG_PRESS   // 长按触发
} ButtonEvent_t;

// 2. 定义按键内部的状态机的状态
typedef enum {
    BTN_STATE_IDLE = 0,    // 空闲状态（松开）
    BTN_STATE_DEBOUNCE,    // 刚刚按下，等待消抖
    BTN_STATE_PRESSED,     // 确认按下，等待松开或长按
    BTN_STATE_WAIT_RELEASE // 长按已触发，等待完全松开
} ButtonState_t;

// 3. 定义按键对象
typedef struct Button {
    /* --- 属性 (私有) --- */
    GPIO_TypeDef* port;       // 绑定的 GPIO 端口 (如 GPIOA)
    uint16_t pin;             // 绑定的引脚 (如 GPIO_PIN_0)
    GPIO_PinState active_lvl; // 按下时的有效电平 (如低电平触发选 GPIO_PIN_RESET)
    
    ButtonState_t state;      // 当前状态机的状态
    uint32_t start_tick;      // 记录状态开始的时间戳
    ButtonEvent_t event;      // 产生的最终事件
    
    // 参数配置
    uint32_t debounce_ms;     // 消抖时间 (默认 20ms)
    uint32_t long_press_ms;   // 长按触发时间 (默认 1000ms)

    /* --- 方法 --- */
    void (*Update)(struct Button* self);            // 驱动状态机运转的心脏
    ButtonEvent_t (*GetEvent)(struct Button* self); // 获取按键事件
} Button;

/* 构造函数 */
void Button_Init(Button* self, GPIO_TypeDef* port, uint16_t pin, GPIO_PinState active_lvl);

#endif