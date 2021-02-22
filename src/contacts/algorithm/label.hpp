#pragma once

/*
 * Labeling algorithm inspired by python-skimage.
 * Optimized and adapted.
 *
 * References:
 *  - Christophe Fiorio and Jens Gustedt, "Two linear time Union-Find
 *    strategies for image processing", Theoretical Computer Science 154 (1996),
 *    pp. 165-181.
 *
 *  - Kensheng Wu, Ekow Otoo and Arie Shoshani, "Optimizing connected component
 *    labeling algorithms", Paper LBNL-56864, 2005, Lawrence Berkeley National
 *    Laboratory (University of California),
 *    http://repositories.cdlib.org/lbnl/LBNL-56864
 */

#include "types.hpp"
#include "container/image.hpp"

#include <numeric>


namespace iptsd::alg {
namespace impl {

inline auto is_root(Image<u16> const& forest, u16 idx) -> bool
{
    return idx == forest[idx];
}

inline auto find_root(Image<u16> const& forest, u16 idx) -> u16
{
    while (!is_root(forest, idx)) {
        idx = forest[idx];
    }

    return idx;
}

inline void set_root(Image<u16>& forest, u16 idx, u16 new_root)
{
    while (!is_root(forest, idx)) {
        idx = std::exchange(forest[idx], new_root);
    }

    forest[idx] = new_root;
}

inline auto merge(Image<u16>& forest, u16 t1_index, u16 t1_root, u16 t2_index, u16 bg)
        -> std::pair<u16, u16>
{
    if (forest[t2_index] == bg) {
        return { t1_index, t1_root };
    }

    auto const t2_root = find_root(forest, t2_index);
    if (t2_root < t1_root) {
        set_root(forest, t1_index, t2_root);
        return { t2_index, t2_root };

    } else if (t1_root < t2_root) {
        set_root(forest, t2_index, t1_root);
        return { t1_index, t1_root };
    }

    return { t1_index, t1_root };
}

inline auto resolve(Image<u16>& forest, u16 background) -> u16
{
    u16 n_labels = 0;
    for (index_t i = 0; i < forest.size().span(); ++i) {
        if (i != background) {
            if (!is_root(forest, i)) {
                forest[i] = forest[forest[i]];
            } else {
                forest[i] = ++n_labels;
            }
        } else {
            forest[i] = 0;
        }
    }

    return n_labels;
}

template<typename T>
inline auto find_background(Image<T> const& data, T threshold) -> u16
{
    for (index_t i = 0; i < data.size().span(); ++i) {
        if (data[i] <= threshold) {
            return i;
        }
    }

    return std::numeric_limits<u16>::max();
}

} /* namespace impl */

template<int C=4, typename T>
auto label(Image<u16>& out, Image<T> const& data, T threshold) -> u16
{
    static_assert(C == 4 || C == 8);

    // strides
    index_t const s_left = 1;
    index_t const s_up = data.stride();
    index_t const s_up_left = s_up + 1;
    index_t const s_up_right = s_up - 1;

    // pass 0: find first backgorund node
    auto const background = impl::find_background(data, threshold);

    // pass 1: build forest
    index_t i;

    // x = 0, y = 0
    out[0] = 0;

    // 0 < x < n, y = 0
    for (i = 1; i < data.size().x; ++i) {
        // background
        if (data[i] <= threshold) {
            out[i] = background;
            continue;
        }

        out[i] = i;

        auto tr = std::pair<u16, u16> { i, i };
        impl::merge(out, tr.first, tr.second, i - s_left, background);
    }

    // 0 < y < n
    while (i < data.size().span()) {

        // x = 0
        if (data[i] <= threshold) {
            out[i] = background;

        } else {
            out[i] = i;

            auto tr = std::pair<u16, u16> { i, i };
            tr = impl::merge(out, tr.first, tr.second, i - s_up, background);

            if constexpr (C == 8) {
                tr = impl::merge(out, tr.first, tr.second, i - s_up_right, background);
            }
        }

        ++i;

        // 0 < x < n - 1
        u16 const limit = i + data.size().x - 2;
        for (; i < limit; ++i) {

            // background
            if (data[i] <= threshold) {
                out[i] = background;
                continue;
            }

            // start by assuming we are a new root, creating a new tree...
            out[i] = i;

            // ... then merge our newly created tree with all neighboring trees
            auto tr = std::pair<u16, u16> { i, i };
            tr = impl::merge(out, tr.first, tr.second, i - s_left, background);

            if constexpr (C == 8) {
                tr = impl::merge(out, tr.first, tr.second, i - s_up_left, background);
            }

            tr = impl::merge(out, tr.first, tr.second, i - s_up, background);

            if constexpr (C == 8) {
                tr = impl::merge(out, tr.first, tr.second, i - s_up_right, background);
            }
        }

        // x = n - 1, y > 0
        if (data[i] <= threshold) {
            out[i] = background;

        } else {
            out[i] = i;

            auto tr = std::pair<u16, u16> { i, i };
            tr = impl::merge(out, tr.first, tr.second, i - s_left, background);

            if constexpr (C == 8) {
                impl::merge(out, tr.first, tr.second, i - s_up_left, background);
            }

            tr = impl::merge(out, tr.first, tr.second, i - s_up, background);
        }

        ++i;
    }

    // pass 2: assign labels
    return impl::resolve(out, background);
}

} /* namespace iptsd::alg */
