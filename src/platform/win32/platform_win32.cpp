//
// Created by kapil on 3.11.2026.
//

#include "defines.h"
#ifdef BSRENDERER_WINDOWS

#include <cstring>
#include <iostream>
#include <vector>

#include <windows.h>

#include "platform/framebuffer.h"
#include "platform/platform.h"
#include "platform/input.h"
#include "platform/window.h"
#include "util/log.h"

const static HINSTANCE MODULE_HANDLE = GetModuleHandleA(nullptr);

keyboard_key translate_key_from_windows_key(const WPARAM key);

void platform::init() {}
void platform::log_output(const char *str, bool is_error, foreground_console_color foreground, background_console_color background) {
    constexpr const char* foreground_codes[] = {
        "\033[39m",
        "\033[97m",
        "\033[32m",
        "\033[31m",
        "\033[33m"
    };

    constexpr const char* background_codes[] = {
        "\033[49m",
        "\033[41m",
        "\033[43m",
    };

    auto& stream = is_error ? std::cerr : std::cout;
    stream << foreground_codes[static_cast<int>(foreground)];
    stream << background_codes[static_cast<int>(background)];
    stream << str;
}

class inner_framebuffer {
public:
    inner_framebuffer(const i32 width, const i32 height) : width(width), height(height) {
        BITMAPINFO info{};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = width;
        info.bmiHeader.biHeight = -height; // top-down
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;

        _dc = CreateCompatibleDC(nullptr);

        FATAL_ASSERT(_dc, "platform_windows: Failed to create framebuffer dc");

        _bitmap = CreateDIBSection(_dc, &info, DIB_RGB_COLORS,
            reinterpret_cast<void**>(&pixels), nullptr, 0);

        FATAL_ASSERT(_bitmap, "platform_windows: Failed to create DIB section");
        _previous_bitmap = SelectObject(_dc, _bitmap);

        std::memset(pixels, 0, width*height*sizeof(u32));
    }

    void present(HWND hwnd) const {
        HDC target = GetDC(hwnd);
        if (!target) return;

        BitBlt(target, 0, 0, width, height, _dc, 0, 0, SRCCOPY);
        ReleaseDC(hwnd, target);
    }

    i32 width, height;
    u32* pixels{};
private:
    HDC _dc;
    HBITMAP _bitmap;
    HGDIOBJ _previous_bitmap;
};

class inner_window {
public:
    inner_window(const std::string& title, const window_options& opts, window* frontend)
        : width(opts.width), height(opts.height), _frontend(frontend) {
        WNDCLASSA wc{};
        wc.lpfnWndProc = _window_proc;
        wc.hInstance = MODULE_HANDLE;
        wc.lpszClassName = "BSRenderer";
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.style = CS_HREDRAW | CS_VREDRAW;

        static bool registered = false;
        if (!registered) {
            FATAL_ASSERT(RegisterClassA(&wc), "platform_windows: Failed to register window class.");
            registered = true;
        }


        RECT rect{0, 0, opts.width, opts.height};
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        _hwnd = CreateWindowExA(
            0,
            "BSRenderer",
            title.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            rect.right - rect.left,
            rect.bottom - rect.top,
            nullptr,
            nullptr,
            MODULE_HANDLE,
            this
        );

        FATAL_ASSERT(_hwnd, "platform_windows: Failed to create window.");

        // Prevent resize
        SetWindowPos(_hwnd, nullptr, 0, 0,
            rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);

        RAWINPUTDEVICE mouse{};
        mouse.usUsagePage = 0x01;
        mouse.usUsage = 0x02;
        mouse.dwFlags = 0;
        mouse.hwndTarget = _hwnd;

        FATAL_ASSERT(
            RegisterRawInputDevices(&mouse, 1, sizeof(mouse)),
            "platform_windows: Failed to register mouse"
        );

        ShowWindow(_hwnd, SW_SHOW);
        UpdateWindow(_hwnd);
    }

    void process_pending_events() {
        MSG msg{};

        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
            }

            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    void present_framebuffer(const inner_framebuffer* buf) const {
        buf->present(_hwnd);
    }

    void capture_mouse() {
        if (_mouse_captured) return;
        _mouse_captured = true;

        SetCapture(_hwnd);

        RECT rect{};
        GetClientRect(_hwnd, &rect);

        POINT top_left{rect.left, rect.top};
        POINT bottom_right{rect.right, rect.bottom};

        ClientToScreen(_hwnd, &top_left);
        ClientToScreen(_hwnd, &bottom_right);

        rect = {
            top_left.x,
            top_left.y,
            bottom_right.x,
            bottom_right.y
        };

        ClipCursor(&rect);

        // lol
        while (ShowCursor(false) >= 0) {}
    }

    void release_mouse() {
        if (!_mouse_captured) return;
        _mouse_captured = false;

        ClipCursor(nullptr);
        ReleaseCapture();

        while (ShowCursor(true) < 0) {}
    }

    u32 width, height;
    bool running = true;
private:
    window* _frontend;
    HWND _hwnd;
    bool _mouse_captured = false;

    static LRESULT CALLBACK _window_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
        if (msg == WM_NCCREATE) {
            auto* create = reinterpret_cast<CREATESTRUCTA*>(l_param);
            auto* inner = static_cast<inner_window*>(create->lpCreateParams);

            SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(inner));
        }

        auto* inner = reinterpret_cast<inner_window*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
        if (!inner) return DefWindowProcA(hwnd, msg, w_param, l_param);

        switch (msg) {
            case WM_CLOSE:
                inner->running = false;
                return 0;

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            case WM_GETMINMAXINFO: {
                auto* info = reinterpret_cast<MINMAXINFO*>(l_param);
                RECT rect{0, 0, static_cast<LONG>(inner->width), static_cast<LONG>(inner->height)};

                AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
                const LONG w = rect.right - rect.left;
                const LONG h = rect.bottom - rect.top;

                info->ptMinTrackSize = {w, h};
                info->ptMinTrackSize = {w, h};

                return 0;
            }

            case WM_KEYDOWN:
            case WM_SYSKEYDOWN: {
                const auto key = translate_key_from_windows_key(w_param);
                inner->_frontend->get_input()._press_key(key);

                return 0;
            }

            case WM_KEYUP:
            case WM_SYSKEYUP: {
                const auto key = translate_key_from_windows_key(w_param);
                inner->_frontend->get_input()._release_key(key);

                return 0;
            }

            case WM_INPUT: {
                UINT size = sizeof(RAWINPUT);
                RAWINPUT input{};

                if (GetRawInputData(
                    reinterpret_cast<HRAWINPUT>(l_param),
                    RID_INPUT,
                    &input,
                    &size,
                    sizeof(RAWINPUTHEADER)
                ) == size) {
                    if (input.header.dwType == RIM_TYPEMOUSE) {
                        inner->_frontend->get_input()._update_mouse_xy(
                            static_cast<double>(input.data.mouse.lLastX),
                            static_cast<double>(input.data.mouse.lLastY)
                        );
                    }
                }

                return 0;
            }

            case WM_SETFOCUS:
                inner->capture_mouse();
                return 0;

            case WM_KILLFOCUS:
                inner->release_mouse();
                return 0;

            default: break;
        }

        return DefWindowProcA(hwnd, msg, w_param, l_param);
    }
};

window::window(const std::string &name, const window_options opts) {
    _inner = new inner_window(name, opts, this);
    _framebuffers[0] = new framebuffer(opts.width, opts.height);
    _framebuffers[1] = new framebuffer(opts.width, opts.height);
}

window::~window() {
    delete _inner;
}

i32 window::get_width() const {
    return _inner->width;
}

i32 window::get_height() const {
    return _inner->height;
}

void window::_run_preframe_events() {
    _input._update_key_states();
    _input._update_mouse_states();

    _inner->process_pending_events();
}

void window::_run_postframe_events() {
    _inner->process_pending_events();
}

void window::capture_mouse() {
    _inner->capture_mouse();
}

void window::release_mouse() {
    _inner->release_mouse();
}

framebuffer *window::begin_frame() {
    FATAL_ASSERT(!_drawing_frame, "window::begin_frame called whilst already drawing a frame!");
    _run_preframe_events();
    _drawing_frame = true;

    const auto& framebuffer = _framebuffers[_frame_counter % 2];
    return framebuffer;
}

void window::end_frame() {
    _inner->present_framebuffer(_framebuffers[_frame_counter % 2]->_inner);
    _drawing_frame = false;
    _frame_counter++;
    _run_postframe_events();
}

bool window::should_close() const {
    return !_inner->running;
}

input & window::get_input() {
    return _input;
}



framebuffer::framebuffer(const i32 width, const i32 height) {
    _inner = new inner_framebuffer(width, height);
}

u32 *framebuffer::get_pixels() const {
    return _inner->pixels;
}

i32 framebuffer::get_height() const {
    return _inner->height;
}

i32 framebuffer::get_width() const {
    return _inner->width;
}

keyboard_key translate_key_from_windows_key(const WPARAM key) {
    switch (key) {
        case 'A': return keyboard_key::A;
        case 'B': return keyboard_key::B;
        case 'C': return keyboard_key::C;
        case 'D': return keyboard_key::D;
        case 'E': return keyboard_key::E;
        case 'F': return keyboard_key::F;
        case 'G': return keyboard_key::G;
        case 'H': return keyboard_key::H;
        case 'I': return keyboard_key::I;
        case 'J': return keyboard_key::J;
        case 'K': return keyboard_key::K;
        case 'L': return keyboard_key::L;
        case 'M': return keyboard_key::M;
        case 'N': return keyboard_key::N;
        case 'O': return keyboard_key::O;
        case 'P': return keyboard_key::P;
        case 'Q': return keyboard_key::Q;
        case 'R': return keyboard_key::R;
        case 'S': return keyboard_key::S;
        case 'T': return keyboard_key::T;
        case 'U': return keyboard_key::U;
        case 'V': return keyboard_key::V;
        case 'W': return keyboard_key::W;
        case 'X': return keyboard_key::X;
        case 'Y': return keyboard_key::Y;
        case 'Z': return keyboard_key::Z;

        case VK_SPACE: return keyboard_key::SPACE;
        case VK_ESCAPE: return keyboard_key::ESCAPE;
        case VK_RETURN: return keyboard_key::ENTER;

        case VK_LSHIFT: return keyboard_key::L_SHIFT;
        case VK_RSHIFT: return keyboard_key::R_SHIFT;
        case VK_LCONTROL: return keyboard_key::L_CTRL;
        case VK_RCONTROL: return keyboard_key::R_CTRL;
        case VK_LMENU: return keyboard_key::L_ALT;
        case VK_RMENU: return keyboard_key::R_ALT;

        default: return keyboard_key::UNKNOWN;
    }
}

#endif