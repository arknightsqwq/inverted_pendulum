#include "lqr.hpp"

LQR::LQR(int n, const float* K_gain)
    : _n(n), _K(K_gain) {
}

float LQR::calculate(const float* state) {
    float u = 0.0f;
    for (int i = 0; i < _n; i++) {
        u += _K[i] * state[i];
    }
    return -u;  // u = -K * x
}

void LQR::update_gain(const float* K_gain) {
    _K = K_gain;
}
