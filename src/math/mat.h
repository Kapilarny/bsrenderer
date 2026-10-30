//
// Created by dawid on 10/26/26.
//

#ifndef BSRENDERER_MAT_H
#define BSRENDERER_MAT_H
#include <array>

#include "defines.h"
#include "vec.h"

template<i32 n> struct dt;

template<i32 Cols, i32 Rows>
struct mat {
    vec<Cols, double> data[Rows];

    constexpr vec<Cols, double>& operator[](i32 i) { return data[i]; }
    constexpr const vec<Cols, double>& operator[](i32 i) const { return data[i]; }
    constexpr double& operator()(i32 row, i32 col) { return data[row][col]; }
    constexpr const double& operator()(i32 row, i32 col) const { return data[row][col]; }

    [[nodiscard]] double determinant() const {
        return dt<Cols>::determinant(*this);
    }

    [[nodiscard]] double cofactor(const i32 row, const i32 col) const {
        mat<Cols-1, Rows-1> submatrix;
        for (i32 i=0; i < Cols-1; ++i) {
            for (i32 j = 0; j < Rows-1; ++j) {
                submatrix[i][j] = data[i+(i>=row)][j+(j>=col)];
            }
        }

        return submatrix.determinant() * ((row+col) % 2 ? -1 : 1);
    }

    [[nodiscard]] mat invert_transpose() const {
        mat result;
        for (i32 i = 0; i < Cols; ++i) {
            for (i32 j = 0; j < Rows; ++j) {
                result[i][j] = cofactor(i, j);
            }
        }

        return result/(result[0]*data[0]);
    }

    [[nodiscard]] mat transpose() const {
        mat result;
        for (i32 i = 0; i < Cols; ++i) {
            for (i32 j = 0; j < Rows; ++j) {
                result[i][j] = data[j][i];
            }
        }
        return result;
    }

    [[nodiscard]] mat invert() const {
        return invert_transpose().transpose();
    }
};

template<i32 Cols, i32 Rows>
vec<Rows, double> operator*(const vec<Rows, double>& lhs, const mat<Cols, Rows>& rhs) {
    return (mat<1, Rows>(lhs)*rhs)[0];
}

template<i32 Cols, i32 Rows>
vec<Rows, double> operator*(const mat<Cols, Rows>& lhs, const vec<Cols, double>& rhs) {
    vec<Rows, double> result;
    for (i32 i = 0; i < Rows; ++i) {
        result[i] = lhs[i] * rhs;
    }
    return result;
}

template<i32 A, i32 B, i32 C>
mat<B, C> operator*(const mat<A, C>& lhs, const mat<B, A>& rhs) {
    mat<B, C> result{};

    for (i32 i = 0; i < C; ++i) {
        for (i32 j = 0; j < B; ++j) {
            for (i32 k = 0; k < A; ++k) {
                result[i][j] += lhs[i][k] * rhs[k][j];
            }
        }
    }

    return result;
}

template<i32 Rows, i32 Cols>
mat<Rows, Cols> operator*(const mat<Rows, Cols>& lhs, const double& scalar) {
    mat<Rows, Cols> result;
    for (i32 i = 0; i < Rows; ++i) {
        result[i] = lhs[i] * scalar;
    }
    return result;
}

template<i32 Rows, i32 Cols>
mat<Rows, Cols> operator/(const mat<Rows, Cols>& lhs, const double& scalar) {
    mat<Rows, Cols> result;
    for (i32 i = 0; i < Rows; ++i) {
        result[i] = lhs[i] / scalar;
    }
    return result;
}

template<i32 Rows, i32 Cols>
mat<Rows, Cols> operator+(const mat<Rows, Cols>& lhs, const mat<Rows, Cols>& rhs) {
    mat<Rows, Cols> result;
    for (i32 i = 0; i < Rows; ++i) {
        for (i32 j = 0; j < Cols; ++j) {
            result[i][j] = lhs[i][j] + rhs[i][j];
        }
    }
    return result;
}

template<i32 Rows, i32 Cols>
mat<Rows, Cols> operator-(const mat<Rows, Cols>& lhs, const mat<Rows, Cols>& rhs) {
    mat<Rows, Cols> result;
    for (i32 i = 0; i < Rows; ++i) {
        for (i32 j = 0; j < Cols; ++j) {
            result[i][j] = lhs[i][j] - rhs[i][j];
        }
    }
    return result;
}

template<i32 n>
struct dt {
    static double determinant(const mat<n, n>& m) {
        double result = 0;
        for (i32 i = 0; i < n; i++) {
            result += m[0][i] * m.cofactor(0, i);
        }
        return result;
    }
};

template<>
struct dt<1> {
    static double determinant(const mat<1, 1>& m) {
        return m[0][0];
    }
};

#endif //BSRENDERER_MAT_H