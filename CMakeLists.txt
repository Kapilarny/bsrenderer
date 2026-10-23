cmake_minimum_required(VERSION 4.1)
project(bsrenderer)

set(CMAKE_CXX_STANDARD 20)

add_executable(bsrenderer src/main.cpp
        src/platform/platform.h
        src/defines.h
        src/platform/linux/platform_wayland.cpp
        src/platform/linux/xdg-shell-client-protocol.h
        src/platform/linux/xdg-shell-client-protocol.cpp
        src/util/log.h
        src/util/log.cpp
        src/platform/linux/platform_linux.cpp
        src/platform/platform_agnostic.cpp
)

target_include_directories(bsrenderer PUBLIC "src/")
target_link_libraries(bsrenderer PUBLIC wayland-client m)