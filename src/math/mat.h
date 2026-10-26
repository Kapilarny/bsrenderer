//
// Created by dawid on 10/26/26.
//

#ifndef BSRENDERER_MAT_H
#define BSRENDERER_MAT_H
#include <array>

#include "defines.h"
#include "vec.h"

template<i32 Cols, i32 Rows>
struct mat {
    std::array<std::array<double, Cols>, Rows> data{};

    constexpr std::array<double, Cols> operator[](i32 i) const { return data[i]; }
    constexpr const std::array<double, Cols>& operator[](i32 i) { return data[i]; }
    constexpr double& operator()(i32 row, i32 col) { return data[row][col]; }
    constexpr const double& operator()(i32 row, i32 col) const { return data[row][col]; }
};

template<typename T>
constexpr vec3<T> operator*(const mat<3, 3>& m, const vec3<T>& v) {
    return {
        m(0, 0) * v.x + m(0, 1) * v.y + m(0, 2) * v.z,
        m(1, 0) * v.x + m(1, 1) * v.y + m(1, 2) * v.z,
        m(2, 0) * v.x + m(2, 1) * v.y + m(2, 2) * v.z,
    };
}

#endif //BSRENDERER_MAT_H