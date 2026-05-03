#include "Botton.h"

/* --- 方法实现 --- */

static void Button_Update(Button* self) {
    // 1. 获取当前系统时间 (毫秒) 和当前引脚电平
    uint32_t current_tick = HAL_GetTick();
    bool is_pressed = (HAL_GPIO_ReadPin(self->port, self->pin) == self->active_lvl);

    // 2. 状态机流转逻辑
    switch (self->state) {
        
        case BTN_STATE_IDLE:
            if (is_pressed) {
                // 刚刚检测到按下，进入消抖状态，并记录当前时间
                self->state = BTN_STATE_DEBOUNCE;
                self->start_tick = current_tick;
            }
            break;

        case BTN_STATE_DEBOUNCE:
            if (!is_pressed) {
                // 消抖期间松开了，说明是抖动或误触，退回空闲
                self->state = BTN_STATE_IDLE;
            } else if (current_tick - self->start_tick >= self->debounce_ms) {
                // 稳定按下了 20ms，确认是有效按下
                self->state = BTN_STATE_PRESSED;
                self->start_tick = current_tick; // 重置时间，为了接下来的长按计时
            }
            break;

        case BTN_STATE_PRESSED:
            if (!is_pressed) {
                // 在长按时间到达前松开了，触发【短按事件】
                self->event = BTN_EVENT_SHORT_PRESS;
                self->state = BTN_STATE_IDLE;
            } else if (current_tick - self->start_tick >= self->long_press_ms) {
                // 一直按着没松开，且达到了长按时间，触发【长按事件】
                self->event = BTN_EVENT_LONG_PRESS;
                self->state = BTN_STATE_WAIT_RELEASE;
            }
            break;

        case BTN_STATE_WAIT_RELEASE:
            if (!is_pressed) {
                // 长按触发后，必须等手完全松开，才允许下一次按键
                self->state = BTN_STATE_IDLE;
            }
            break;
    }
}

static ButtonEvent_t Button_GetEvent(Button* self) {
    // 提取事件，并在提取后立刻清空（防止重复触发）
    ButtonEvent_t evt = self->event;
    self->event = BTN_EVENT_NONE;
    return evt;
}

/* --- 构造函数 --- */
void Button_Init(Button* self, GPIO_TypeDef* port, uint16_t pin, GPIO_PinState active_lvl) {
    self->port = port;
    self->pin = pin;
    self->active_lvl = active_lvl;
    
    self->state = BTN_STATE_IDLE;
    self->start_tick = 0;
    self->event = BTN_EVENT_NONE;
    
    // 默认参数配置
    self->debounce_ms = 20;     // 20毫秒消抖
    self->long_press_ms = 1000; // 1秒算长按
    
    // 绑定方法
    self->Update = Button_Update;
    self->GetEvent = Button_GetEvent;
}