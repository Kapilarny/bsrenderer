//
// Created by dawid on 10/26/26.
//

#ifndef BSRENDERER_VEC_H
#define BSRENDERER_VEC_H

#include "defines.h"
#include "util/log.h"

template<typename T>
struct vec4 {
    union {
        struct {
            T x, y, z, w;
        };

        T data[4];
    };

    vec4() = default;
    vec4(const T& x, const T& y, const T& z, const T& w) : x(x), y(y), z(z), w(w) {}
    vec4(const vec4<T>& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

    T& operator[](u64 i) {
        DEBUG_ASSERT(i < 4, "vec3[] -> subscript of out range.");
        return data[i];
    }

    const T& operator[](u64 i) const {
        DEBUG_ASSERT(i < 4, "vec3[] -> subscript of out range.");
        return data[i];
    }

    constexpr vec4<T> operator+(const vec4<T>& v) const {
        return vec4<T>(x + v.x, y + v.y, z + v.z, w + v.w);
    }

    constexpr vec4<T> operator-(const vec4<T>& v) const {
        return vec4<T>(x - v.x, y - v.y, z - v.z, w - v.w);
    }

    constexpr void operator+=(const vec4<T>& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        w += v.w;
    }

    constexpr void operator-=(const vec4<T>& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        w -= v.w;
    }
};

template<typename T>
struct vec3 {
    union {
        struct {
            T x, y, z;
        };

        T data[3];
    };

    vec3() = default;
    vec3(const T& x, const T& y, const T& z) : x(x), y(y), z(z) {}
    vec3(const vec3<T>& v) : x(v.x), y(v.y), z(v.z) {}

    T& operator[](u64 i) {
        DEBUG_ASSERT(i < 3, "vec3[] -> subscript of out range.");
        return data[i];
    }

    const T& operator[](u64 i) const {
        DEBUG_ASSERT(i < 3, "vec3[] -> subscript of out range.");
        return data[i];
    }

    constexpr vec3<T> operator+(const vec3<T>& v) const {
        return vec3<T>(x + v.x, y + v.y, z + v.z);
    }

    constexpr vec3<T> operator-(const vec3<T>& v) const {
        return vec3<T>(x - v.x, y - v.y, z - v.z);
    }

    constexpr void operator+=(const vec3<T>& v) {
        x += v.x;
        y += v.y;
        z += v.z;
    }

    constexpr void operator-=(const vec3<T>& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
    }

    constexpr vec3<T> operator/(T v) const {
        return vec3<T>(x / v, y / v, z / v);
    }

    constexpr vec3<T> operator/(vec3<T> v) const {
        return vec3<T>(x / v.x, y / v.y, z / v.z);
    }

};

template<typename T>
struct vec2 {
    union {
        struct {
            T x, y;
        };

        T data[2];
    };

    vec2() = default;
    vec2(T& x, T& y) : x(x), y(y) {}
    vec2(const vec3<T>& v) : x(v.x), y(v.y) {}

    T& operator[](u64 i) {
        DEBUG_ASSERT(i < 2, "vec2[] -> subscript of out range.");
        return data[i];
    }

    T& operator[](u64 i) const {
        DEBUG_ASSERT(i < 2, "vec2[] -> subscript of out range.");
        return data[i];
    }

    constexpr vec2<T> operator+(const vec2<T>& v) const {
        return vec2<T>(x + v.x, y + v.y);
    }

    constexpr vec2<T> operator-(const vec2<T>& v) const {
        return vec2<T>(x - v.x, y - v.y);
    }

    constexpr void operator+=(const vec2<T>& v) {
        x += v.x;
        y += v.y;
    }

    constexpr void operator-=(const vec2<T>& v) {
        x -= v.x;
        y -= v.y;
    }
};


#endif //BSRENDERER_VEC_H