//
// Created by dawid on 10/23/26.
//

#ifndef BSRENDERER_DEFINES_H
#define BSRENDERER_DEFINES_H
#include <cstdint>

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

#if defined(_WIN32)
#define BSRENDERER_WINDOWS
#elif defined(__linux__)
#define BSRENDERER_LINUX
#endif

#endif //BSRENDERER_DEFINES_H