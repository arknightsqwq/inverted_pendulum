#ifndef __BUTTON_H
#define __BUTTON_H

#include "main.h"
#include <stdbool.h>

/* --- 事件枚举 --- */
typedef enum {
    BTN_EVENT_NONE,
    BTN_EVENT_SHORT_PRESS,
    BTN_EVENT_LONG_PRESS,   // 达到长按时间那一刻触发（仅一次）
    BTN_EVENT_HOLDING       // 长按期间持续触发
} ButtonEvent_t;

/* --- 状态机状态 --- */
typedef enum {
    BTN_STATE_IDLE,
    BTN_STATE_DEBOUNCE,
    BTN_STATE_PRESSED,
    BTN_STATE_HOLDING,      // 正在按住
    BTN_STATE_WAIT_RELEASE
} ButtonState_t;

/* --- 按键对象结构体 --- */
typedef struct Button_t {
    // 硬件属性
    GPIO_TypeDef* port;
    uint16_t pin;
    GPIO_PinState active_lvl;

    // 状态属性
    ButtonState_t state;
    ButtonEvent_t event;
    uint32_t start_tick;
    uint32_t holding_tick; // 用于控制 Holding 触发频率的节奏器

    // 配置参数
    uint16_t debounce_ms;  //消抖时间
    uint16_t long_press_ms;  //长按触发时间
    uint16_t holding_interval_ms; // Holding 事件的触发间隔（如 100ms）

    // 成员方法
    void (*Update)(struct Button_t* self);
    ButtonEvent_t (*GetEvent)(struct Button_t* self);
} Button_t;

void Button_Init(Button_t* self, GPIO_TypeDef* port, uint16_t pin, GPIO_PinState active_lvl);

#endif