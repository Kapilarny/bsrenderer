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
        src/util/model.cpp
        src/util/model.h
        src/util/parsers/waveform_parser.cpp
        src/util/parsers/waveform_parser.h
        src/renderer/renderer.h
        src/renderer/software/software_renderer.cpp
        src/renderer/software/software_renderer.h
        src/renderer/renderer.cpp
)

target_include_directories(bsrenderer PUBLIC "src/")
target_link_libraries(bsrenderer PUBLIC wayland-client m)

add_custom_command(TARGET bsrenderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
)