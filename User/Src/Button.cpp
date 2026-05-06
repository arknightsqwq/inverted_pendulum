#include "Button.hpp"

Button::Button(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState active_lvl)
    : _port(port), _pin(pin), _active_lvl(active_lvl),
      _state(IDLE), _event(NONE), _start_tick(0), _holding_tick(0),
      _debounce_ms(20), _long_press_ms(800), _holding_interval_ms(100) {
}

void Button::update() {
    uint32_t current_tick = HAL_GetTick();
    bool is_pressed = (HAL_GPIO_ReadPin(_port, _pin) == _active_lvl);

    switch (_state) {
    case IDLE:
        if (is_pressed) {
            _state = DEBOUNCE;
            _start_tick = current_tick;
        }
        break;

    case DEBOUNCE:
        if (!is_pressed) {
            _state = IDLE;
        } else if (current_tick - _start_tick >= _debounce_ms) {
            _state = PRESSED;
            _start_tick = current_tick;
        }
        break;

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

Button::Event Button::get_event() {
    Event e = _event;
    _event = NONE;
    return e;
}

void Button::set_timing(uint16_t debounce, uint16_t long_press, uint16_t holding_interval) {
    _debounce_ms = debounce;
    _long_press_ms = long_press;
    _holding_interval_ms = holding_interval;
}