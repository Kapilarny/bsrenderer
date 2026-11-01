//
// Created by dawid on 10/31/26.
//

#ifndef BSRENDERER_DIRTY_VALUE_H
#define BSRENDERER_DIRTY_VALUE_H
#include <algorithm>

template<typename T>
class dirty_value {
public:
    explicit dirty_value(T value = {}) : _value(std::move(value)) {}

    dirty_value& operator=(const T& value) {
        _value = value;
        _dirty = true;
        return *this;
    }

    T& get_m() {
        _dirty = true;
        return _value;
    }

    const T& get_i() const {
        return _value;
    }

    bool is_dirty() const {
        return _dirty;
    }

    void clear_dirty() {
        _dirty = false;
    }
private:
    bool _dirty = false;
    T _value;
};

#endif //BSRENDERER_DIRTY_VALUE_H