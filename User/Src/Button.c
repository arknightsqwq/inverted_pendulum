#include "Button.h"

static void Button_Update(Button_t* self) {
    uint32_t current_tick = HAL_GetTick();
    bool is_pressed = (HAL_GPIO_ReadPin(self->port, self->pin) == self->active_lvl);

    switch (self->state) {
        case BTN_STATE_IDLE:
            if (is_pressed) {
                self->state = BTN_STATE_DEBOUNCE;
                self->start_tick = current_tick;
            }
            break;

        case BTN_STATE_DEBOUNCE:
            if (!is_pressed) {
                self->state = BTN_STATE_IDLE;
            } else if (current_tick - self->start_tick >= self->debounce_ms) {
                self->state = BTN_STATE_PRESSED;
                self->start_tick = current_tick;
            }
            break;

        case BTN_STATE_PRESSED:
            if (!is_pressed) {
                self->event = BTN_EVENT_SHORT_PRESS;
                self->state = BTN_STATE_IDLE;
            } else if (current_tick - self->start_tick >= self->long_press_ms) {
                // 瞬间：触发长按事件，并进入 Holding 状态
                self->event = BTN_EVENT_LONG_PRESS;
                self->state = BTN_STATE_HOLDING;
                self->holding_tick = current_tick; // 初始化节奏器
            }
            break;

        case BTN_STATE_HOLDING:
            if (!is_pressed) {
                self->state = BTN_STATE_IDLE;
            } else {
                // 持续按住：每隔一定时间产生一个 Holding 事件
                if (current_tick - self->holding_tick >= self->holding_interval_ms) {
                    self->event = BTN_EVENT_HOLDING;
                    self->holding_tick = current_tick; // 重置节奏器
                }
            }
            break;

        default:
            self->state = BTN_STATE_IDLE;
            break;
    }
}

static ButtonEvent_t Button_GetEvent(Button_t* self) {
    ButtonEvent_t evt = self->event;
    self->event = BTN_EVENT_NONE; // 提取后立即清空
    return evt;
}

void Button_Init(Button_t* self, GPIO_TypeDef* port, uint16_t pin, GPIO_PinState active_lvl) {
    self->port = port;
    self->pin = pin;
    self->active_lvl = active_lvl;

    self->state = BTN_STATE_IDLE;
    self->event = BTN_EVENT_NONE;

    // 默认参数配置（你可以根据倒立摆调参需求修改）
    self->debounce_ms = 20;
    self->long_press_ms = 800;        // 按住 0.8s 判定为长按
    self->holding_interval_ms = 100;   // 长按时每 0.1s 触发一次 Holding

    // 绑定方法
    self->Update = Button_Update;
    self->GetEvent = Button_GetEvent;
}