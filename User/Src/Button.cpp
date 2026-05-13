#include "Button.hpp"

// 显式实例化，每个 ButtonID 生成一份独立代码
// 如果添加新的 ButtonID，在这里加上对应的显式实例化
template class Button<1>;
template class Button<2>;
template class Button<3>;
template class Button<4>;
