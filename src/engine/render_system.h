//
// Created by dawid on 10/31/26.
//

#ifndef BSRENDERER_RENDER_SYSTEM_H
#define BSRENDERER_RENDER_SYSTEM_H
#include <stack>
#include <vector>

#include "transform.h"
#include "renderer/renderer.h"
#include "resources/materials/imaterial.h"
#include "util/model.h"

struct render_entry {
    model* mesh;
    imaterial* material;
};

// TODO: Make static entities and dynamic separate to cheap out on updating matrices
class render_system {
public:
    u32 register_entry(render_entry entry) {
        const u32 res = _provide_lookup_idx();

        _entries.emplace_back(entry);
        _matrices.emplace_back();
        _transforms.emplace_back();

        _entry_lookup[res] = _entries.size() - 1;
        return res;
    }

    void deregister_entry(const u32 lookup_idx) {
        DEBUG_ASSERT(lookup_idx < _entry_lookup.size(), "Trying to deregister an entry out of bounds of lookup!");

        const auto idx = _entry_lookup[lookup_idx];
        DEBUG_ASSERT(idx != -1, "Trying to deregister a non-existing entry!");

        _entries[idx] = _entries.back();
        _transforms[idx] = _transforms.back();
        _matrices[idx] = _matrices.back();

        _entry_lookup[lookup_idx] = -1;
        _entries.pop_back();
        _matrices.pop_back();
        _transforms.pop_back();

        _free_slots.push(lookup_idx);
    }

    void update_all() {
        ZoneScoped;

#pragma omp parallel for
        for (i32 i = 0; i < _entries.size(); i++) {
            auto& trans = _transforms[i];
            if (trans.should_rebuild()) {
                _matrices[i] = trans.rebuild_matrix();
            }
        }
    }

    void draw_all(renderer* rend) {
        ZoneScoped;
        for (i32 i = 0; i < _entries.size(); ++i) {
            auto& [mesh, material] = _entries[i];
            rend->draw_model(mesh, material, _matrices[i]);
        }
    }

    transform& get_transform(const u32 lookup_idx) {
        const auto idx = _entry_lookup[lookup_idx];
        return _transforms[idx];
    }

    mat<4,4>& get_matrix(const u32 lookup_idx) {
        const auto idx = _entry_lookup[lookup_idx];
        return _matrices[idx];
    }

    render_entry& get_entry(const u32 lookup_idx) {
        const auto idx = _entry_lookup[lookup_idx];
        return _entries[idx];
    }

    void clear() {
        _entries.clear();
        _free_slots = {};
    }
private:
    u32 _provide_lookup_idx() {
        if (!_free_slots.empty()) {
            const auto top = _free_slots.top();
            _free_slots.pop();

            return top;
        }

        _entry_lookup.push_back({});
        return _entry_lookup.size() - 1;
    }

    std::stack<u32> _free_slots;
    std::vector<i32> _entry_lookup; // to keep _entries contiguous

    std::vector<render_entry> _entries;
    std::vector<mat<4,4>> _matrices;
    std::vector<transform> _transforms;
};

#endif //BSRENDERER_RENDER_SYSTEM_H