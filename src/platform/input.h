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

enum class mouse_button {
    UNKNOWN,
    LEFT,
    RIGHT,
    MIDDLE,

    COUNT
};

class input {
public:
    [[nodiscard]] key_state get_key_state(keyboard_key key) const {
        return _kb_states[static_cast<u64>(key)];
    }

    [[nodiscard]] bool is_key_down(keyboard_key key) const {
        return _kb_states[static_cast<u64>(key)] == key_state::DOWN;
    }

    [[nodiscard]] key_state get_mouse_state(mouse_button button) const {
        return _mb_states[static_cast<u64>(button)];
    }

    [[nodiscard]] std::pair<double, double> get_mouse_delta() const {
        return {_mouse_dx, _mouse_dy};
    }
private:
    void _update_key_states() {
        for (auto& state : _kb_states) {
            if (state == key_state::PRESSED) {
                state = key_state::DOWN;
            } else if (state == key_state::RELEASED) {
                state = key_state::UP;
            }
        }
    }

    void _update_mouse_states() {
        for (auto& state : _mb_states) {
            if (state == key_state::PRESSED) {
                state = key_state::DOWN;
            } else if (state == key_state::RELEASED) {
                state = key_state::UP;
            }
        }

        _mouse_dx = _mouse_dy = 0;
    }

    void _press_key(keyboard_key key) {
        auto& k = _kb_states[static_cast<u64>(key)];
        if (k == key_state::UP || k == key_state::RELEASED) {
            k = key_state::PRESSED;
        }
    }

    void _release_key(keyboard_key key) {
        _kb_states[static_cast<u64>(key)] = key_state::RELEASED;
    }

    void _update_mouse_xy(const double x, const double y) {
        _mouse_dx += x;
        _mouse_dy += y;
    }

    friend class inner_window;
    friend class window;

    double _mouse_dx{}, _mouse_dy{};

    std::array<key_state, static_cast<u64>(keyboard_key::COUNT)> _kb_states{};
    std::array<key_state, static_cast<u64>(mouse_button::COUNT)> _mb_states{};
};

#endif //BSRENDERER_INPUT_H