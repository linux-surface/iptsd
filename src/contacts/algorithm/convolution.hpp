#pragma once

#include "types.hpp"

#include "algorithm/border.hpp"

#include "container/image.hpp"
#include "container/kernel.hpp"
#include "container/ops.hpp"

#include "math/num.hpp"
#include "math/vec2.hpp"

#include "algorithm/opt/convolution.3x3-extend.hpp"
#include "algorithm/opt/convolution.5x5-extend.hpp"


namespace alg {
namespace conv {
namespace kernels {

template<class T>
inline constexpr container::kernel<T, 3, 3> sobel3_x {
    static_cast<T>( 1), static_cast<T>( 0), static_cast<T>(-1),
    static_cast<T>( 2), static_cast<T>( 0), static_cast<T>(-2),
    static_cast<T>( 1), static_cast<T>( 0), static_cast<T>(-1),
};

template<class T>
inline constexpr container::kernel<T, 3, 3> sobel3_y {
    static_cast<T>( 1), static_cast<T>( 2), static_cast<T>( 1),
    static_cast<T>( 0), static_cast<T>( 0), static_cast<T>( 0),
    static_cast<T>(-1), static_cast<T>(-2), static_cast<T>(-1),
};

template<class T>
inline constexpr container::kernel<T, 3, 3> sobel3_xx {
    static_cast<T>( 1), static_cast<T>(-2), static_cast<T>( 1),
    static_cast<T>( 2), static_cast<T>(-4), static_cast<T>( 2),
    static_cast<T>( 1), static_cast<T>(-2), static_cast<T>( 1),
};

template<class T>
inline constexpr container::kernel<T, 3, 3> sobel3_yy {
    static_cast<T>( 1), static_cast<T>( 2), static_cast<T>( 1),
    static_cast<T>(-2), static_cast<T>(-4), static_cast<T>(-2),
    static_cast<T>( 1), static_cast<T>( 2), static_cast<T>( 1),
};

template<class T>
inline constexpr container::kernel<T, 3, 3> sobel3_xy {
    static_cast<T>( 1), static_cast<T>( 0), static_cast<T>(-1),
    static_cast<T>( 0), static_cast<T>( 0), static_cast<T>( 0),
    static_cast<T>(-1), static_cast<T>( 0), static_cast<T>( 1),
};


template<class T, index_t Nx, index_t Ny>
auto gaussian(T sigma) -> container::kernel<T, Nx, Ny>
{
    static_assert(Nx % 2 == 1);
    static_assert(Ny % 2 == 1);

    auto k = container::kernel<T, Nx, Ny>{};

    T sum = static_cast<T>(0.0);

    for (index_t j = 0; j < Ny; j++) {
        for (index_t i = 0; i < Nx; i++) {
            auto const x = (math::vec2_t<T> {
                static_cast<T>(i - (Nx - 1) / 2),
                static_cast<T>(j - (Ny - 1) / 2)
            } / sigma).norm_l2();

            auto const v = std::exp(-static_cast<T>(0.5) * x * x);

            k[{i, j}] = v;
            sum += v;
        }
    }

    container::ops::transform(k, [&](auto const& x) {
        return x / sum;
    });

    return k;
}

} /* namespace kernels */


namespace impl {

template<typename B, typename T, typename S, index_t Nx, index_t Ny>
void conv_generic(container::image<T>& out, container::image<T> const& in,
                  container::kernel<S, Nx, Ny> const& k)
{
    index_t const dx = (Nx - 1) / 2;
    index_t const dy = (Ny - 1) / 2;

    for (index_t cy = 0; cy < in.shape().y; ++cy) {
        for (index_t cx = 0; cx < in.shape().x; ++cx) {
            out[{cx, cy}] = math::num<T>::zero;

            for (index_t iy = 0; iy < Ny; ++iy) {
                for (index_t ix = 0; ix < Nx; ++ix) {
                    out[{cx, cy}] += B::value(in, {cx - dx + ix, cy - dy + iy}) * k[{ix, iy}];
                }
            }
        }
    }
}

} /* namespace impl */
} /* namespace conv */


template<typename B=border::extend, typename T, typename S, index_t Nx, index_t Ny>
void convolve(container::image<T>& out, container::image<T> const& in,
              container::kernel<S, Nx, Ny> const& k)
{
    // workaround for partial function template specialization
    if constexpr (Nx == 5 && Ny == 5 && std::is_same_v<B, border::extend>) {
        conv::impl::conv_5x5_extend<T, S>(out, in, k);
    } else if constexpr (Nx == 3 && Ny == 3 && std::is_same_v<B, border::extend>) {
        conv::impl::conv_3x3_extend<T, S>(out, in, k);
    } else {
        conv::impl::conv_generic<B, T, S, Nx, Ny>(out, in, k);
    }
}

} /* namespace alg */
