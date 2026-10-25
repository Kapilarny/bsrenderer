//
// Created by dawid on 10/23/26.
//

#ifndef BSRENDERER_DEFINES_H
#define BSRENDERER_DEFINES_H
#include <cstdint>

// TODO: define this in cmake
#define BSRENDERER_DEBUG

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

template<typename T>
struct vec3 {
    T x, y, z;

    T& operator[](u64 i) {
        // DEBUG_ASSERT(i < 3, "vec3[] -> out of bounds subscript");
        return *(&x + i);
    }

    const T& operator[](u64 i) const {
        // DEBUG_ASSERT(i < 3, "vec3[] -> out of bounds subscript");
        return *(&x + i);
    }
};

template<typename T>
struct vec2 {
    T x, y;

    vec2(T& x, T& y) : x(x), y(y) {}
    vec2(const vec3<T>& v) : x(v.x), y(v.y) {}
};

#if defined(_WIN32)
#define BSRENDERER_WINDOWS
#elif defined(__linux__)
#define BSRENDERER_LINUX
#endif

#endif //BSRENDERER_DEFINES_H