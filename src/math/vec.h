//
// Created by dawid on 10/26/26.
//

#ifndef BSRENDERER_VEC_H
#define BSRENDERER_VEC_H

#include <cmath>

#include "defines.h"
#include "util/log.h"

template <i32 N, typename T>
struct vec {
    T data[N]{};

    double magnitude() {
        return (*this) * (*this);
    }

    vec normalized() {
        return *this / magnitude();
    }

    constexpr double& operator[](const i32 idx) { return data[idx]; }
    constexpr const double& operator[](const i32 idx) const { return data[idx]; }
};

template<i32 N, typename T>
double operator*(const vec<N, T>& a, const vec<N, T>& b) {
    double res = 0;
    for (i32 i = 0; i < N; i++) {
        res += a[i] * b[i];
    }
    return res;
}

template<i32 N, typename T>
vec<N, T> operator+(const vec<N, T>& a, const vec<N, T>& b) {
    vec<N, T> res;
    for (i32 i = 0; i < N; i++) {
        res[i] = a[i] + b[i];
    }
    return res;
}

template<i32 N, typename T>
vec<N, T> operator-(const vec<N, T>& a, const vec<N, T>& b) {
    vec<N, T> res;
    for (i32 i = 0; i < N; i++) {
        res[i] = a[i] - b[i];
    }
    return res;
}

template<i32 N, typename T>
vec<N, T> operator*(const vec<N, T>& a, const T& scalar) {
    vec<N, T> res;
    for (i32 i = 0; i < N; i++) {
        res[i] = a[i] * scalar;
    }
    return res;
}

template<i32 N, typename T>
vec<N, T> operator*(const T& scalar, const vec<N, T>& a) {
    return a * scalar;
}

template<i32 N, typename T>
vec<N, T> operator/(const vec<N, T>& a, const T& scalar) {
    vec<N, T> res;
    for (i32 i = 0; i < N; i++) {
        res[i] = a[i] / scalar;
    }
    return res;
}


template<typename T>
using vec2 = vec<2, T>;

template<typename T>
using vec3 = vec<3, T>;

template<typename T>
using vec4 = vec<4, T>;

template<typename T>
struct vec<2, T> {
    T x{}, y{};

    constexpr T& operator[](const i32 idx) { DEBUG_ASSERT(idx < 2, "subscript out of range"); return idx ? y : x; }
    constexpr const T& operator[](const i32 idx) const { DEBUG_ASSERT(idx < 2, "subscript out of range"); return idx ? y : x; }
};

template<typename T>
struct vec<3, T> {
    T x{}, y{}, z{};

    vec2<T> xy() { return {x, y}; }

    constexpr T& operator[](const i32 idx) { DEBUG_ASSERT(idx < 3, "subscript out of range"); return idx ? (idx == 1 ? y : z) : x; }
    constexpr const T& operator[](const i32 idx) const { DEBUG_ASSERT(idx < 3, "subscript out of range"); return idx ? (idx == 1 ? y : z) : x; }

    double magnitude() {
        return std::sqrt((*this) * (*this));
    }

    vec normalized() {
        return *this / magnitude();
    }
};

template<typename T>
struct vec<4, T> {
    T x{}, y{}, z{}, w{};

    vec2<T> xy() { return {x, y}; }
    vec3<T> xyz() { return {x, y, z}; }

    constexpr T& operator[](const i32 idx) {
        DEBUG_ASSERT(idx < 4, "subscript out of range");
        return idx < 2 ? (idx ? y : x) : (idx == 2 ? z : w);
    }

    constexpr const T& operator[](const i32 idx) const {
        DEBUG_ASSERT(idx < 4, "subscript out of range");
        return idx < 2 ? (idx ? y : x) : (idx == 2 ? z : w);
    }

    double magnitude() {
        return (*this) * (*this);
    }

    vec normalized() {
        return *this / magnitude();
    }
};

namespace bmath {
    template<typename T>
    vec3<T> cross(const vec3<T>& a, const vec3<T>& b) {
        return {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
    }
}

#endif //BSRENDERER_VEC_H