//
// Created by dawid on 10/23/26.
//

#include "defines.h"
#ifdef BSRENDERER_LINUX
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>

#include "platform/platform.h"

#include <linux/input-event-codes.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>

#include "relative-pointer-unstable-v1.h"
#include "pointer-constraints-unstable.h"
#include "relative-pointer-unstable-v1.h"
#include "xdg-shell-client-protocol.h"

#include "util/log.h"

constexpr u32 WL_COMPOSITOR_VERSION_TARGET = 4;

keyboard_key translate_key_from_wayland_key(u32 key);

class wayland {
public:
    wayland() {
        display = wl_display_connect(nullptr);
        FATAL_ASSERT(display, "platform_wayland: Cannot connect to Wayland display!");

        registry = wl_display_get_registry(display);
        wl_registry_add_listener(registry, &_registry_listener, this);

        FATAL_ASSERT(wl_display_roundtrip(display) > 0, "platform_wayland: Failed display roundtrip.");
        FATAL_ASSERT(
            compositor && shm && wm_base && seat && _relative_pointer_manager && _pointer_constraints,
            "platform_wayland: Required Wayland interfaces unavailable.");
    }

    void process_pending_events() const {
        wl_display_dispatch_pending(display);
    }

    void wait_for_next_frame() const {
        wl_display_dispatch(display);
    }

    wl_display* display;
    wl_registry* registry;
    wl_compositor* compositor{};
    wl_shm* shm{};
    xdg_wm_base* wm_base{};
    wl_seat* seat{};
    zwp_relative_pointer_manager_v1* _relative_pointer_manager{};
    zwp_pointer_constraints_v1* _pointer_constraints{};
private:
    static void _registry_global(
        void* data,
        wl_registry* registry,
        u32 name,
        const char* interface,
        u32 version);

    static constexpr wl_registry_listener _registry_listener = {
        .global = _registry_global,
        .global_remove = [](void*, wl_registry*, u32) {}
    };
};

static wayland* wl_inst = nullptr;

class inner_framebuffer {
public:
    inner_framebuffer(const i32 width, const i32 height) : width(width), height(height) {
        const u64 size = width * height * 4; // 4 bytes per pixel
        const i32 fd = _create_shm_file(size);
        FATAL_ASSERT(fd != -1, "platform_wayland: Failed to create shared memory file for a framebuffer.");

        pixels = static_cast<u32*>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        FATAL_ASSERT(pixels != MAP_FAILED, "platform_wayland: Failed to map shared memory.");

        std::memset(pixels, 0, size);

        wl_shm_pool* pool = wl_shm_create_pool(
            wl_inst->shm,
            fd,
            static_cast<i32>(size)
        );

        close(fd);

        FATAL_ASSERT(pool != nullptr, "platform_wayland: Failed to create shared memory pool.");

        buffer = wl_shm_pool_create_buffer(pool,
            0,
            width,
            height,
            width*4,
            WL_SHM_FORMAT_ARGB8888);

        wl_shm_pool_destroy(pool);

        FATAL_ASSERT(buffer != nullptr, "platform_wayland: Failed to create shared memory buffer.");
        wl_buffer_add_listener(buffer, &_buffer_listener, this);
    }

    ~inner_framebuffer() {
        wl_buffer_destroy(buffer);
        munmap(pixels, width*height*4);
    }

    wl_buffer* buffer;
    i32 width, height;
    u32* pixels;
    bool busy = false;
private:
    static i32 _create_shm_file(u64 size) {
        char path[] = "/tmp/bs-renderer-XXXXXX";
        const i32 fd = mkstemp(path);
        if (fd < 0) {
            log::error("platform_wayland: Failed to create a temp file for shm.");
            return -1;
        }

        unlink(path);

        if (size > static_cast<u64>(std::numeric_limits<off_t>::max())
            || ftruncate(fd, static_cast<off_t>(size)) < 0) {
            close(fd);
            log::error("platform_wayland: Failed to create a temp file for shm.");
            return -1;
        }

        return fd;
    }

    static void _buffer_release(void* data, wl_buffer*) {
        auto* inner = static_cast<inner_framebuffer*>(data);
        inner->busy = false;
    }

    static constexpr wl_buffer_listener _buffer_listener = {
        .release = _buffer_release
    };
};

class inner_window {
public:
    inner_window(const std::string& title, const window_options& opts, window* frontend)
        : width(opts.width), height(opts.height), _frontend(frontend) {
        FATAL_ASSERT(wl_inst != nullptr, "platform_wayland: Wayland instance does not exist before creating window!");

        _surface = wl_compositor_create_surface(wl_inst->compositor);
        _xdg_surface = xdg_wm_base_get_xdg_surface(wl_inst->wm_base, _surface);
        xdg_surface_add_listener(_xdg_surface, &_xdg_surface_listener, this);

        _xdg_toplevel = xdg_surface_get_toplevel(_xdg_surface);
        xdg_toplevel_add_listener(_xdg_toplevel, &_xdg_toplevel_listener, this);

        xdg_toplevel_set_title(_xdg_toplevel, title.c_str());
        xdg_toplevel_set_app_id(_xdg_toplevel, "com.kapilarny.bsrenderer");

        xdg_toplevel_set_min_size(_xdg_toplevel, opts.width, opts.height);
        xdg_toplevel_set_max_size(_xdg_toplevel, opts.width, opts.height);

        // input
        _keyboard = wl_seat_get_keyboard(wl_inst->seat);
        wl_keyboard_add_listener(_keyboard, &_wl_keyboard_listener, this);

        _pointer = wl_seat_get_pointer(wl_inst->seat);
        wl_pointer_add_listener(_pointer, &_wl_pointer_listener, this);

        wl_surface_commit(_surface);

        while (!_configured) {
            FATAL_ASSERT(wl_display_dispatch(wl_inst->display) > 0, "platform_wayland: Compositor failed to configure window.");
        }
    }

    ~inner_window() {
        // TODO: perform cleanup
    }

    void present_framebuffer(const inner_framebuffer* buf) const {
        wl_surface_attach(_surface, buf->buffer, 0, 0);
        wl_surface_damage_buffer(_surface, 0, 0, buf->width, buf->height);
        wl_surface_commit(_surface);
    }

    void capture_mouse() {
        _locked_pointer = zwp_pointer_constraints_v1_lock_pointer(
            wl_inst->_pointer_constraints,
            _surface,
            _pointer,
            nullptr,
            ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT
        );

        _relative_pointer = zwp_relative_pointer_manager_v1_get_relative_pointer(
            wl_inst->_relative_pointer_manager,
            _pointer
        );

        zwp_relative_pointer_v1_add_listener(_relative_pointer, &_zwp_relative_pointer_listener, this);

        // hide cursor

    }

    void release_mouse() {
        if (_relative_pointer) {
            zwp_relative_pointer_v1_destroy(_relative_pointer);
            _relative_pointer = nullptr;
        }

        if (_locked_pointer) {
            zwp_locked_pointer_v1_destroy(_locked_pointer);
            _locked_pointer = nullptr;
        }
    }

    bool running = true;
    i32 width, height;
private:
    window* _frontend;
    wl_surface* _surface;
    wl_keyboard* _keyboard;
    wl_pointer* _pointer;
    xdg_surface* _xdg_surface;
    xdg_toplevel* _xdg_toplevel;

    zwp_locked_pointer_v1* _locked_pointer{};
    zwp_relative_pointer_v1* _relative_pointer{};

    u32 _pointer_serial;

    bool _configured = false;

    static void _wm_base_ping(void*,xdg_wm_base* wm_base,const u32 serial) {
        xdg_wm_base_pong(wm_base, serial);
    }
    static constexpr xdg_wm_base_listener _wm_base_listener = {
        _wm_base_ping
    };

    static void _surface_configure(void* data,xdg_surface* surface,const u32 serial) {
        auto* inner = static_cast<inner_window*>(data);

        xdg_surface_ack_configure(surface, serial);
        inner->_configured = true;
    }
    static constexpr xdg_surface_listener _xdg_surface_listener = {
        _surface_configure
    };

    static void _toplevel_close(void* data, xdg_toplevel*) {
        auto* inner = static_cast<inner_window*>(data);
        inner->running = false;
    }
    static void _toplevel_configure(void* data, xdg_toplevel* toplevel, i32 w, i32 h, wl_array* states) {

    }
    static constexpr xdg_toplevel_listener _xdg_toplevel_listener = {
        .configure = _toplevel_configure,
        .close = _toplevel_close
    };

    static void _wl_keyboard_key_event(void* data, wl_keyboard*, u32 serial, u32 time, u32 key, u32 state) {
        auto* inner = static_cast<inner_window*>(data);

        const auto translated = translate_key_from_wayland_key(key);
        auto& input = inner->_frontend->get_input();
        if (state == WL_KEYBOARD_KEY_STATE_PRESSED || state == WL_KEYBOARD_KEY_STATE_REPEATED) {
            input._press_key(translated);
        } else {
            input._release_key(translated);
        }
    }
    static constexpr wl_keyboard_listener _wl_keyboard_listener = {
        .keymap = [](void*, wl_keyboard*, u32, i32, u32) {},
        .enter = [](void*, wl_keyboard*, u32, wl_surface*, wl_array*) {},
        .leave = [](void*, wl_keyboard*, u32, wl_surface*) {},
        .key = _wl_keyboard_key_event,
        .modifiers = [](void*, wl_keyboard*, u32, u32, u32, u32, u32) {}
    };


    static void _wl_pointer_enter_event(void* data, wl_pointer*, u32 serial, wl_surface*, wl_fixed_t, wl_fixed_t) {
        auto* inner = static_cast<inner_window*>(data);
        inner->_pointer_serial = serial;
        wl_pointer_set_cursor(inner->_pointer, inner->_pointer_serial, NULL, 0, 0);
    }
    static constexpr wl_pointer_listener _wl_pointer_listener = {
        .enter = _wl_pointer_enter_event,
        .leave = [](void*, wl_pointer*, u32, wl_surface*) {},
        // .motion = _wl_pointer_motion_event,
        .motion = [](void*, wl_pointer*, u32, wl_fixed_t, wl_fixed_t) {},
        .button = [](void*, wl_pointer*, u32, u32, u32, u32) {},
    };

    static void _zwp_relative_pointer_motion_event
        (void* data, zwp_relative_pointer_v1*, u32, u32, wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t, wl_fixed_t) {

        auto inner = static_cast<inner_window*>(data);
        inner->_frontend->get_input()._update_mouse_xy(wl_fixed_to_double(dx), wl_fixed_to_double(dy));
    }
    static constexpr zwp_relative_pointer_v1_listener _zwp_relative_pointer_listener = {
        .relative_motion = _zwp_relative_pointer_motion_event
    };
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
    wl_inst->process_pending_events();
}

void window::_run_postframe_events() {
    wl_inst->wait_for_next_frame();
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
    return _framebuffers[_frame_counter % 2];
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

void wayland::_registry_global(void *data, wl_registry *registry, u32 name, const char *interface, u32 version) {
    auto* inner = static_cast<wayland*>(data);

    if (std::strcmp(interface, wl_compositor_interface.name) == 0) {
        inner->compositor = static_cast<wl_compositor*>(
            wl_registry_bind(registry, name, &wl_compositor_interface,
                std::min(version, WL_COMPOSITOR_VERSION_TARGET))
        );
    } else if (std::strcmp(interface, wl_shm_interface.name) == 0) {
        inner->shm = static_cast<wl_shm*>(
            wl_registry_bind(registry, name, &wl_shm_interface,
                std::min(version, 1u)) // TODO: add non-magic numbers version targets
        );
    } else if (std::strcmp(interface, xdg_wm_base_interface.name) == 0) {
        inner->wm_base = static_cast<xdg_wm_base*>(
            wl_registry_bind(registry, name, &xdg_wm_base_interface,
                std::min(version, 1u))
        );
    } else if (std::strcmp(interface, wl_seat_interface.name) == 0) {
        inner->seat = static_cast<wl_seat*>(
            wl_registry_bind(registry, name, &wl_seat_interface,
                std::min(version, 1u))
        );
    } else if (std::strcmp(interface, zwp_relative_pointer_manager_v1_interface.name) == 0) {
        inner->_relative_pointer_manager = static_cast<zwp_relative_pointer_manager_v1*>(
            wl_registry_bind(registry, name, &zwp_relative_pointer_manager_v1_interface,
                std::min(version, 1u))
        );
    } else if (std::strcmp(interface, zwp_pointer_constraints_v1_interface.name) == 0) {
        inner->_pointer_constraints = static_cast<zwp_pointer_constraints_v1*>(
            wl_registry_bind(registry, name, &zwp_pointer_constraints_v1_interface,
                std::min(version, 1u))
        );
    }
}

void platform::init() {
    wl_inst = new wayland();
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

keyboard_key translate_key_from_wayland_key(const u32 key) {
    switch (key) {
        case KEY_A: return keyboard_key::A;
        case KEY_B: return keyboard_key::B;
        case KEY_C: return keyboard_key::C;
        case KEY_D: return keyboard_key::D;
        case KEY_E: return keyboard_key::E;
        case KEY_F: return keyboard_key::F;
        case KEY_G: return keyboard_key::G;
        case KEY_H: return keyboard_key::H;
        case KEY_I: return keyboard_key::I;
        case KEY_J: return keyboard_key::J;
        case KEY_K: return keyboard_key::K;
        case KEY_L: return keyboard_key::L;
        case KEY_M: return keyboard_key::M;
        case KEY_N: return keyboard_key::N;
        case KEY_O: return keyboard_key::O;
        case KEY_P: return keyboard_key::P;
        case KEY_Q: return keyboard_key::Q;
        case KEY_R: return keyboard_key::R;
        case KEY_S: return keyboard_key::S;
        case KEY_T: return keyboard_key::T;
        case KEY_U: return keyboard_key::U;
        case KEY_V: return keyboard_key::V;
        case KEY_W: return keyboard_key::W;
        case KEY_X: return keyboard_key::X;
        case KEY_Y: return keyboard_key::Y;
        case KEY_Z: return keyboard_key::Z;

        case KEY_SPACE:         return keyboard_key::SPACE;
        case KEY_ESC:           return keyboard_key::ESCAPE;
        case KEY_ENTER:         return keyboard_key::ENTER;
        case KEY_LEFTSHIFT:     return keyboard_key::L_SHIFT;
        case KEY_RIGHTSHIFT:    return keyboard_key::R_SHIFT;
        case KEY_LEFTCTRL:      return keyboard_key::L_CTRL;
        case KEY_RIGHTCTRL:     return keyboard_key::R_CTRL;
        case KEY_LEFTALT:       return keyboard_key::L_ALT;
        case KEY_RIGHTALT:      return keyboard_key::R_ALT;

        default: return keyboard_key::UNKNOWN;
    }
}
#endif
