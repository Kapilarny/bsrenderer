//
// Created by dawid on 10/26/26.
//

#ifndef BSRENDERER_INPUT_H
#define BSRENDERER_INPUT_H
#include <array>
#include <memory>

#include "defines.h"

enum class key_state {
    UP,
    DOWN,
    PRESSED, // held from this frame
    RELEASED // released this frame
};

enum class keyboard_key {
    UNKNOWN,
    A, B, C, D, E, F, G, H, I,
    J, K, L, M, N, O, P, Q, R,
    S, T, U, V, W, X, Y, Z,

    SPACE,
    ESCAPE,
    ENTER,
    L_SHIFT, R_SHIFT,
    L_CTRL, R_CTRL,
    L_ALT, R_ALT,

    COUNT
};

class input {
public:
    [[nodiscard]] key_state get_key_state(keyboard_key key) const {
        return _states[static_cast<u64>(key)];
    }
private:
    void _update_key_states() {
        for (auto& state : _states) {
            if (state == key_state::PRESSED) {
                state = key_state::DOWN;
            } else if (state == key_state::RELEASED) {
                state = key_state::UP;
            }
        }
    }

    void _press_key(keyboard_key key) {
        auto& k = _states[static_cast<u64>(key)];
        if (k == key_state::UP || k == key_state::RELEASED) {
            k = key_state::PRESSED;
        }
    }

    void _release_key(keyboard_key key) {
        _states[static_cast<u64>(key)] = key_state::RELEASED;
    }

    friend class inner_window;
    friend class window;
    std::array<key_state, static_cast<u64>(keyboard_key::COUNT)> _states{};
};

#endif //BSRENDERER_INPUT_H