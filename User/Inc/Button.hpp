#ifndef __BUTTON_HPP
#define __BUTTON_HPP

#include "stm32f1xx_hal.h"

// ButtonID 用于编译期区分不同按键，processEvent 通过它分发到对应的业务逻辑
template <int ButtonID>
class Button {
public:
    enum Event { NONE, SHORT_PRESS, LONG_PRESS, HOLDING };

    Button(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState active_lvl)
        : _port(port), _pin(pin), _active_lvl(active_lvl),
          _state(IDLE), _event(NONE), _start_tick(0), _holding_tick(0),
          _debounce_ms(20), _long_press_ms(800), _holding_interval_ms(100) {}

    // 非阻塞状态机，每次调用推进一次，需在主循环或定时器中周期性调用
    void update() {
        uint32_t current_tick = HAL_GetTick();
        bool is_pressed = (HAL_GPIO_ReadPin(_port, _pin) == _active_lvl);

        switch (_state) {
        case IDLE:
            if (is_pressed) {
                _state = DEBOUNCE;
                _start_tick = current_tick;
            }
            break;

        // 消抖：持续按下达到 debounce_ms 才确认，期间松手视为抖动
        case DEBOUNCE:
            if (!is_pressed) {
                _state = IDLE;
            } else if (current_tick - _start_tick >= _debounce_ms) {
                _state = PRESSED;
                _start_tick = current_tick;
            }
            break;

        // 确认按下：松手 → SHORT_PRESS，按够 long_press_ms 不松 → LONG_PRESS
        case PRESSED:
            if (!is_pressed) {
                _event = SHORT_PRESS;
                _state = IDLE;
            } else if (current_tick - _start_tick >= _long_press_ms) {
                _event = LONG_PRESS;
                _state = HOLDING_STATE;
                _holding_tick = current_tick;
            }
            break;

        // 持续按住：每 holding_interval_ms 产生一次 HOLDING 事件
        case HOLDING_STATE:
            if (!is_pressed) {
                _state = IDLE;
            } else {
                if (current_tick - _holding_tick >= _holding_interval_ms) {
                    _event = HOLDING;
                    _holding_tick = current_tick;
                }
            }
            break;
        }
    }

    // 读取并清除事件确保同一事件不被重复处理
    Event get_event() {
        Event e = _event;
        _event = NONE;
        return e;
    }

    void set_timing(uint16_t debounce, uint16_t long_press, uint16_t holding_interval) {
        _debounce_ms = debounce;
        _long_press_ms = long_press;
        _holding_interval_ms = holding_interval;
    }

    // processEvent 在 global.cpp 中通过特化注入具体业务逻辑
    void processEvent();

private:
    enum State { IDLE, DEBOUNCE, PRESSED, HOLDING_STATE };

    GPIO_TypeDef* _port;
    uint16_t _pin;
    GPIO_PinState _active_lvl;

    State _state;
    Event _event;
    uint32_t _start_tick;
    uint32_t _holding_tick;

    uint16_t _debounce_ms;
    uint16_t _long_press_ms;
    uint16_t _holding_interval_ms;
};

template <int ButtonID>
void Button<ButtonID>::processEvent() {
    // 默认空操作，在 global.cpp 中通过特化注入具体业务逻辑
}

template <typename... Buttons>
void updateAllButton(Buttons&... btns) {
    (btns.update(), ...);
}

template <typename... Buttons>
void processAllButtons(Buttons&... btns) {
    (btns.processEvent(), ...);
}

#endif
