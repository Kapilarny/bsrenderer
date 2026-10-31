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
        src/math/bmath.h
        src/math/vec.h
        src/math/mat.h
        src/platform/window.h
        src/platform/framebuffer.h
        src/platform/input.h
        src/renderer/camera.cpp
        src/renderer/camera.h
        src/renderer/software/triangle_clip.cpp
        src/renderer/software/triangle_clip.h
        src/platform/linux/relative-pointer-unstable-v1.h
        src/platform/linux/relative-pointer-unstable-v1.cpp
        src/platform/linux/pointer-constraints-unstable.cpp
        src/platform/linux/pointer-constraints-unstable.h
        src/font/font.c
        src/renderer/software/shaders/ishader.h
        src/renderer/software/shaders/flat_shader.h
        src/external/stb_image.h
        src/external/stb_image.cpp
        src/renderer/software/texture.cpp
        src/renderer/software/texture.h
        src/renderer/software/shaders/texture_shader.h
        src/renderer/software/materials/imaterial.h
        src/renderer/software/materials/material_textured.h
        src/renderer/software/vertex.h
)

include(FetchContent)

FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG v0.14.1
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(tracy)

#set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
set(TRACY_ON_DEMAND OFF CACHE BOOL "" FORCE)
set(TRACY_NO_EXIT OFF CACHE BOOL "" FORCE)

target_include_directories(bsrenderer PUBLIC "src/")
target_link_libraries(bsrenderer PUBLIC wayland-client m TracyClient)

add_custom_command(TARGET bsrenderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
)