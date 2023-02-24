#pragma once

#include <common/types.hpp>
#include <container/image.hpp>
#include "../detector.hpp"

#include <queue>
#include <numeric>

using namespace iptsd::container;


namespace iptsd::contacts::advanced::alg {

using iptsd::contacts::advanced::BlobDetector;

template<index_t DX, index_t DY>
[[gnu::always_inline]] [[nodiscard]] inline f32 get_cost(BlobDetector *d, index_t i)
{
    f32 constexpr c_dist = 0.1f;
    f32 constexpr c_ridge = 9.0f;
    f32 constexpr c_grad = 1.0f;

    static_assert(DX == -1 || DX == 0 || DX == 1);
    static_assert(DY == -1 || DY == 0 || DY == 1);
    // auto const dist = std::hypotf(gsl::narrow<f32>(d.x), gsl::narrow<f32>(d.y));
    f32 constexpr dist = DX * DY == 0 ? 1 : M_SQRT2;

    auto const [ev1, ev2] = common::unchecked<std::array<f32, 2>>(d->m_img_stev, i);
    auto const grad = std::max(ev1, 0.0f) + std::max(ev2, 0.0f);
    auto const ridge = common::unchecked<f32>(d->m_img_rdg, i);

    return c_ridge * ridge + c_grad * grad + c_dist * dist;
}

auto constexpr th_inc = 0.6f;

constexpr bool mask(BlobDetector *d, index_t i) {
    return common::unchecked<f32>(d->m_img_pp, i) > 0.0f && common::unchecked<u16>(d->m_img_lbl, i) == 0;
};

constexpr bool wdt_inc_bin(BlobDetector *d, index_t i) {
    u16 const lbl = common::unchecked<u16>(d->m_img_lbl, i);
    return lbl > 0 && common::unchecked<f32>(d->m_cscore, lbl - 1) > th_inc;
};

constexpr bool wdt_exc_bin(BlobDetector *d, index_t i) {
    u16 const lbl = common::unchecked<u16>(d->m_img_lbl, i);
    return lbl && common::unchecked<f32>(d->m_cscore, lbl - 1) <= th_inc;
};

#define is_masked(i) !mask(d, i)
#define is_foreground(i) (INC_OR_EXC ? wdt_inc_bin(d, i) : wdt_exc_bin(d, i))
#define is_compute(i) (!is_foreground(i) && !is_masked(i))

constexpr f32 limit = 6.0f;

#define qpush(i, c) do { \
if (INC_OR_EXC)  {       \
if ((c) < limit) {       \
if ((c) < young_limit) { \
if ((c) < eden_limit) {  \
eq.push_back ({ i, c});  \
} else {                 \
yq.push_back ({ i, c});  \
}                        \
} else {                 \
q.push({i, c});          \
}                        \
}                        \
} else {                 \
if ((c) < limit) {       \
if (c < 1 || (c > 1 && i & 1)) {    \
eq.push_back({ i, c});   \
} else {                 \
yq.push_back({i, c});    \
}                        \
}                        \
}                        \
} while (0)
index2_t constexpr out_size = { 64, 44 };

template<index_t DX, index_t DY, bool INC_OR_EXC, index_t stride, typename T, typename Q>
inline void evaluate(BlobDetector *d, Image<T>& out, Q& queue, index_t i)
{
    if (!is_compute(i + stride))
        return;

    auto const c = out[i] + get_cost<DX, DY>(d, i);

    if (c < out[i + stride] && c < limit) {
        queue.push({ i + stride, c });
    }
}

template<int N=8, bool INC_OR_EXC>
[[gnu::always_inline]] inline void weighted_distance_transform_1(BlobDetector *d)
{
    static_assert(N == 4 || N == 8);
    auto &q = d->m_wdt_queue;
    auto &yq = d->m_wdt_queue_young_gen;
    auto &eq = d->m_wdt_queue_eden_gen;
    auto& out = INC_OR_EXC ? d->m_img_dm1 : d->m_img_dm2;
    constexpr f64 young_limit = 0.2f;
    constexpr f64 eden_limit = 0.15f;
    using T = f32;

    // strides
    index_t constexpr s_left      = -1;
    index_t constexpr s_right     =  1;
    index_t constexpr s_top       = -64;
    index_t constexpr s_top_left  = s_top + s_left;
    index_t constexpr s_top_right = s_top + s_right;
    index_t constexpr s_bot       = -s_top;
    index_t constexpr s_bot_left  = s_bot + s_left;
    index_t constexpr s_bot_right = s_bot + s_right;

    // step 1: initialize output, queue all non-masked pixels
    index_t i = 0;

    // x = 0, y = 0
    if (!is_foreground(i)) {
        out[i] = std::numeric_limits<T>::max();

        if (!is_masked(i)) {
            auto c = std::numeric_limits<T>::max();

            if (is_foreground(i + s_right)) {
                c = std::min(c, get_cost<-1, 0>(d, i + s_right));
            }

            if (is_foreground(i + s_bot)) {
                c = std::min(c, get_cost<0, -1>(d, i + s_bot));
            }

            if (N == 8 && is_foreground(i + s_bot_right)) {
                c = std::min(c, get_cost<-1, -1>(d, i + s_bot_right));
            }

            qpush(i, c);
        }
    } else {
        out[i] = static_cast<T>(0);
    }
    ++i;

    // 0 < x < n - 1, y = 0
    for (; i < out_size.x - 1; ++i) {
        if (is_foreground(i)) {
            out[i] = static_cast<T>(0);
            continue;
        }

        out[i] = std::numeric_limits<T>::max();

        if (is_masked(i))
            continue;

        auto c = std::numeric_limits<T>::max();

        if (is_foreground(i + s_left)) {
            c = std::min(c, get_cost<1, 0>(d, i + s_left));
        }

        if (is_foreground(i + s_right)) {
            c = std::min(c, get_cost<-1, 0>(d, i + s_right));
        }

        if (N == 8 && is_foreground(i + s_bot_left)) {
            c = std::min(c, get_cost<1, -1>(d, i + s_bot_left));
        }

        if (is_foreground(i + s_bot)) {
            c = std::min(c, get_cost<0, -1>(d, i + s_bot));
        }

        if (N == 8 && is_foreground(i + s_bot_right)) {
            c = std::min(c, get_cost<-1, -1>(d, i + s_bot_right));
        }

        qpush(i, c);
    }

    // x = n - 1, y = 0
    if (!is_foreground(i)) {
        out[i] = std::numeric_limits<T>::max();

        if (!is_masked(i)) {
            auto c = std::numeric_limits<T>::max();

            if (is_foreground(i + s_left)) {
                c = std::min(c, get_cost<1, 0>(d, i + s_left));
            }

            if (N == 8 && is_foreground(i + s_bot_left)) {
                c = std::min(c, get_cost<1, -1>(d, i + s_bot_left));
            }

            if (is_foreground(i + s_bot)) {
                c = std::min(c, get_cost<0, -1>(d, i + s_bot));
            }

            qpush(i, c);
        }
    } else {
        out[i] = static_cast<T>(0);
    }
    ++i;

    // 0 < y < n - 1
    while (i < out_size.x * (out_size.y - 1)) {
        // x = 0
        if (!is_foreground(i)) {
            out[i] = std::numeric_limits<T>::max();

            if (!is_masked(i)) {
                auto c = std::numeric_limits<T>::max();

                if (is_foreground(i + s_right)) {
                    c = std::min(c, get_cost<-1, 0>(d, i + s_right));
                }

                if (is_foreground(i + s_top)) {
                    c = std::min(c, get_cost<0, 1>(d, i + s_top));
                }

                if (N == 8 && is_foreground(i + s_top_right)) {
                    c = std::min(c, get_cost<-1, 1>(d, i + s_top_right));
                }

                if (is_foreground(i + s_bot)) {
                    c = std::min(c, get_cost<0, -1>(d, i + s_bot));
                }

                if (N == 8 && is_foreground(i + s_bot_right)) {
                    c = std::min(c, get_cost<-1, -1>(d, i + s_bot_right));
                }

                qpush(i, c);
            }
        } else {
            out[i] = static_cast<T>(0);
        }
        ++i;

        // 0 < x < n - 1
        auto const limit = i + out_size.x - 2;
        for (; i < limit; ++i) {
            // if this is a foreground pixel, set it to zero and skip the rest
            if (is_foreground(i)) {
                out[i] = static_cast<T>(0);
                continue;
            }

            // initialize all background pixels to maximum
            out[i] = std::numeric_limits<T>::max();

            // don't evaluate pixels that are excluded by mask
            if (is_masked(i))
                continue;

            // compute minimum cost to any neighboring foreground pixel, if available
            auto c = std::numeric_limits<T>::max();

            if (is_foreground(i + s_left)) {
                c = std::min(c, get_cost<1, 0>(d, i + s_left));
            }

            if (is_foreground(i + s_right)) {
                c = std::min(c, get_cost<-1, 0>(d, i + s_right));
            }

            if (N == 8 && is_foreground(i + s_top_left)) {
                c = std::min(c, get_cost<1, 1>(d, i + s_top_left));
            }

            if (is_foreground(i + s_top)) {
                c = std::min(c, get_cost<0, 1>(d, i + s_top));
            }

            if (N == 8 && is_foreground(i + s_top_right)) {
                c = std::min(c, get_cost<-1, 1>(d, i + s_top_right));
            }

            if (N == 8 && is_foreground(i + s_bot_left)) {
                c = std::min(c, get_cost<1, -1>(d, i + s_bot_left));
            }

            if (is_foreground(i + s_bot)) {
                c = std::min(c, get_cost<0, -1>(d, i + s_bot));
            }

            if (N == 8 && is_foreground(i + s_bot_right)) {
                c = std::min(c, get_cost<-1, -1>(d, i + s_bot_right));
            }

            // if we have a finite projected cost, add this pixel
            qpush(i, c);
        }

        // x = n - 1
        if (!is_foreground(i)) {
            out[i] = std::numeric_limits<T>::max();

            if (!is_masked(i)) {
                auto c = std::numeric_limits<T>::max();

                if (is_foreground(i + s_left)) {
                    c = std::min(c, get_cost<1, 0>(d, i + s_left));
                }

                if (N == 8 && is_foreground(i + s_top_left)) {
                    c = std::min(c, get_cost<1, 1>(d, i + s_top_left));
                }

                if (is_foreground(i + s_top)) {
                    c = std::min(c, get_cost<0, 1>(d, i + s_top));
                }

                if (N == 8 && is_foreground(i + s_bot_left)) {
                    c = std::min(c, get_cost<1, -1>(d, i + s_bot_left));
                }

                if (is_foreground(i + s_bot)) {
                    c = std::min(c, get_cost<0, -1>(d, i + s_bot));
                }

                qpush(i, c);
            }
        } else {
            out[i] = static_cast<T>(0);
        }
        ++i;
    }

    // x = 0, y = n - 1
    if (!is_foreground(i)) {
        out[i] = std::numeric_limits<T>::max();

        if (!is_masked(i)) {
            auto c = std::numeric_limits<T>::max();

            if (is_foreground(i + s_right)) {
                c = std::min(c, get_cost<-1, 0>(d, i + s_right));
            }

            if (is_foreground(i + s_top)) {
                c = std::min(c, get_cost<0, 1>(d, i + s_top));
            }

            if (N == 8 && is_foreground(i + s_top_right)) {
                c = std::min(c, get_cost<-1, 1>(d, i + s_top_right));
            }

            qpush(i, c);
        }
    } else {
        out[i] = static_cast<T>(0);
    }
    ++i;

    // 0 < x < n - 1, y = n - 1
    for (; i < out_size.span() - 1; ++i) {
        if (is_foreground(i)) {
            out[i] = static_cast<T>(0);
            continue;
        }

        out[i] = std::numeric_limits<T>::max();

        if (is_masked(i))
            continue;

        auto c = std::numeric_limits<T>::max();

        if (is_foreground(i + s_left)) {
            c = std::min(c, get_cost<1, 0>(d, i + s_left));
        }

        if (is_foreground(i + s_right)) {
            c = std::min(c, get_cost<-1, 0>(d, i + s_right));
        }

        if (N == 8 && is_foreground(i + s_top_left)) {
            c = std::min(c, get_cost<1, 1>(d, i + s_top_left));
        }

        if (is_foreground(i + s_top)) {
            c = std::min(c, get_cost<0, 1>(d, i + s_top));
        }

        if (N == 8 && is_foreground(i + s_top_right)) {
            c = std::min(c, get_cost<-1, 1>(d, i + s_top_right));
        }

        qpush(i, c);
    }

    // x = n - 1, y = n - 1
    if (!is_foreground(i)) {
        out[i] = std::numeric_limits<T>::max();

        if (!is_masked(i)) {
            auto c = std::numeric_limits<T>::max();

            if (is_foreground(i + s_left)) {
                c = std::min(c, get_cost<1, 0>(d, i + s_left));
            }

            if (N == 8 && is_foreground(i + s_top_left)) {
                c = std::min(c, get_cost<1, 1>(d, i + s_top_left));
            }

            if (is_foreground(i + s_top)) {
                c = std::min(c, get_cost<0, 1>(d, i + s_top));
            }

            qpush(i, c);
        }
    } else {
        out[i] = static_cast<T>(0);
    }
}

template<int N=8, bool INC_OR_EXC>
[[gnu::always_inline]] inline void weighted_distance_transform_2(BlobDetector *d)
{
    static_assert(N == 4 || N == 8);
    auto &q = d->m_wdt_queue;
    auto& out = INC_OR_EXC ? d->m_img_dm1 : d->m_img_dm2;
    using T = f32;

    // strides
    index_t constexpr s_left      = -1;
    index_t constexpr s_right     =  1;
    index_t constexpr s_top       = -64;
    index_t constexpr s_top_left  = s_top + s_left;
    index_t constexpr s_top_right = s_top + s_right;
    index_t constexpr s_bot       = -s_top;
    index_t constexpr s_bot_left  = s_bot + s_left;
    index_t constexpr s_bot_right = s_bot + s_right;

    // step 2: while queue is not empty, get next pixel, write down cost, and add neighbors
    while (!q.empty()) {
        // get next pixel and remove it from queue
        wdt::QItem<T> pixel = q.top();
        q.pop();

        // check if someone has been here before; if so, skip this one
        if (out[pixel.idx] <= pixel.cost)
            continue;

        // no one has been here before, so we're guaranteed to be on the lowes cost path
        out[pixel.idx] = pixel.cost;

        // evaluate neighbors
        auto const [x, y] = Image<T>::unravel(out_size, pixel.idx);

        if (x > 0) {
            evaluate<-1, 0, INC_OR_EXC, s_left>(d, out, q, pixel.idx);
        }

        if (x < out_size.x - 1) {
            evaluate<1, 0, INC_OR_EXC, s_right>(d, out, q, pixel.idx);
        }

        if (y > 0) {
            if (N == 8 && x > 0) {
                evaluate<-1, -1, INC_OR_EXC, s_top_left>(d, out, q, pixel.idx);
            }

            evaluate<0, -1, INC_OR_EXC, s_top>(d, out, q, pixel.idx);

            if (N == 8 && x < out_size.x - 1) {
                evaluate<1, -1, INC_OR_EXC, s_top_right>(d, out, q, pixel.idx);
            }
        }

        if (y < out_size.y - 1) {
            if (N == 8 && x > 0) {
                evaluate<-1, 1, INC_OR_EXC, s_bot_left>(d, out, q, pixel.idx);
            }

            evaluate<0, 1, INC_OR_EXC, s_bot>(d, out, q, pixel.idx);

            if (N == 8 && x < out_size.x - 1) {
                evaluate<1, 1, INC_OR_EXC, s_bot_right>(d, out, q, pixel.idx);
            }
        }
    }
}

template<int N=8, bool INC_OR_EXC>
[[gnu::always_inline]] inline void weighted_distance_transform(BlobDetector *d)
{
    weighted_distance_transform_1<N, INC_OR_EXC>(d);
    printf("%lu, %lu, %lu\n", d->m_wdt_queue.size(), d->m_wdt_queue_young_gen.size(), d->m_wdt_queue_eden_gen.size());
    weighted_distance_transform_2<N, INC_OR_EXC>(d);
    auto& out = INC_OR_EXC ? d->m_img_dm1 : d->m_img_dm2;
    for (auto &pixel : d->m_wdt_queue_young_gen) {
        if (out[pixel.idx] <= pixel.cost)
            continue;
        d->m_wdt_queue.push(pixel);
    }
    d->m_wdt_queue_young_gen.clear();
    weighted_distance_transform_2<N, INC_OR_EXC>(d);
    for (auto &pixel : d->m_wdt_queue_eden_gen) {
        if (out[pixel.idx] <= pixel.cost)
            continue;
        d->m_wdt_queue.push(pixel);
    }
    d->m_wdt_queue_eden_gen.clear();
    weighted_distance_transform_2<N, INC_OR_EXC>(d);
}

} /* namespace iptsd::contacts::advanced::alg */
