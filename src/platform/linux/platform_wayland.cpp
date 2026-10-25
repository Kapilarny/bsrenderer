//
// Created by dawid on 10/23/26.
//

#include "defines.h"
#ifdef BSRENDERER_LINUX
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>

#include "platform/platform.h"

#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include "xdg-shell-client-protocol.h"

#include "util/log.h"

constexpr u32 WL_COMPOSITOR_VERSION_TARGET = 4;

class wayland {
public:
    wayland() {
        display = wl_display_connect(nullptr);
        FATAL_ASSERT(display, "platform_wayland: Cannot connect to Wayland display!");

        registry = wl_display_get_registry(display);
        wl_registry_add_listener(registry, &_registry_listener, this);

        FATAL_ASSERT(wl_display_roundtrip(display) > 0, "platform_wayland: Failed display roundtrip.");
        FATAL_ASSERT(compositor || shm || wm_base, "platform_wayland: Required Wayland interfaces unavailable.");
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
private:
    static void _registry_global(
        void* data,
        wl_registry* registry,
        u32 name,
        const char* interface,
        u32 version);

    static void _registry_remove(void*, wl_registry*, uint32_t);

    static constexpr wl_registry_listener _registry_listener = {
        _registry_global,
        _registry_remove
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
    static i32 _create_shm_file(u64 size);

    static void _buffer_release(void* data, wl_buffer*) {
        auto* inner = static_cast<inner_framebuffer*>(data);
        inner->busy = false;
    }

    static constexpr wl_buffer_listener _buffer_listener = {
        .release = _buffer_release
    };
};

i32 inner_framebuffer::_create_shm_file(u64 size) {
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

class inner_window {
public:
    inner_window(const std::string& title, const window_options& opts) : width(opts.width), height(opts.height) {
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

        wl_surface_commit(_surface);

        while (!_configured) {
            FATAL_ASSERT(wl_display_dispatch(wl_inst->display) > 0, "platform_wayland: Compositor failed to configure window.");
        }
    }

    void present_framebuffer(const inner_framebuffer* buf) const {
        wl_surface_attach(_surface, buf->buffer, 0, 0);
        wl_surface_damage_buffer(_surface, 0, 0, buf->width, buf->height);
        wl_surface_commit(_surface);
    }

    bool running = true;
    i32 width, height;
private:
    wl_surface* _surface;
    xdg_surface* _xdg_surface;
    xdg_toplevel* _xdg_toplevel;

    bool _configured = false;

    static void _wm_base_ping(
        void*,
        xdg_wm_base* wm_base,
        const u32 serial) {
        xdg_wm_base_pong(wm_base, serial);
    }

    static constexpr xdg_wm_base_listener _wm_base_listener = {
        _wm_base_ping
    };

    static void _surface_configure(
    void* data,
    xdg_surface* surface,
    const u32 serial) {
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
};

window::window(const std::string &name, window_options opts) {
    _inner = new inner_window(name, opts);
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
    wl_inst->process_pending_events();
}

void window::_run_postframe_events() {
    wl_inst->wait_for_next_frame();
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
    }
}

void wayland::_registry_remove(void *data, wl_registry *registry, uint32_t name) {
}

void platform::init() {
    wl_inst = new wayland();
}

#endif
