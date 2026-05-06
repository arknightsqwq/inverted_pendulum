#ifndef __BUTTON_HPP
#define __BUTTON_HPP

#include "stm32f1xx_hal.h"

class Button {
public:
    /* --- 事件枚举 --- */
    enum Event {
        NONE,
        SHORT_PRESS,
        LONG_PRESS,
        HOLDING
    };

    /* --- 构造函数 --- */
    Button(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState active_lvl);

    /**
     * @brief 状态机更新逻辑，建议在主循环或 1ms 中断中调用
     */
    void update();

    /**
     * @brief 获取并清除当前事件
     */
    Event get_event();

    // 配置参数接口（可选）
    void set_timing(uint16_t debounce, uint16_t long_press, uint16_t holding_interval);

private:
    /* --- 内部状态机状态 --- */
    enum State {
        IDLE,
        DEBOUNCE,
        PRESSED,
        HOLDING_STATE, // 避免与 Event::HOLDING 冲突
    };

    // 硬件属性
    GPIO_TypeDef* _port;
    uint16_t _pin;
    GPIO_PinState _active_lvl;

    // 状态属性
    State _state;
    Event _event;
    uint32_t _start_tick;
    uint32_t _holding_tick;

    // 配置参数
    uint16_t _debounce_ms;
    uint16_t _long_press_ms;
    uint16_t _holding_interval_ms;
};

#endif