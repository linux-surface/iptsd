#pragma once

#include "types.hpp"

#include "algorithm/border.hpp"
#include "algorithm/convolution.hpp"

#include "container/image.hpp"
#include "container/kernel.hpp"

#include "math/num.hpp"
#include "math/mat2.hpp"

#include <cassert>

#include "algorithm/opt/structure_tensor.3x3-zero.hpp"


namespace alg {
namespace stensor::impl {

template<typename Bx, typename By, typename T, index_t Nx, index_t Ny>
void structure_tensor_generic(container::image<math::mat2s_t<T>>& out,
                              container::image<T> const& in,
                              container::kernel<T, Nx, Ny> const& kx,
                              container::kernel<T, Nx, Ny> const& ky)
{
    index_t const dx = (Nx - 1) / 2;
    index_t const dy = (Ny - 1) / 2;

    for (index_t cy = 0; cy < in.size().y; ++cy) {
        for (index_t cx = 0; cx < in.size().x; ++cx) {
            T gx = math::num<T>::zero;
            T gy = math::num<T>::zero;

            for (index_t iy = 0; iy < Ny; ++iy) {
                for (index_t ix = 0; ix < Nx; ++ix) {
                    gx += Bx::value(in, {cx - dx + ix, cy - dy + iy}) * kx[{ix, iy}];
                    gy += By::value(in, {cx - dx + ix, cy - dy + iy}) * ky[{ix, iy}];
                }
            }

            out[{cx, cy}].xx = gx * gx;
            out[{cx, cy}].xy = gx * gy;
            out[{cx, cy}].yy = gy * gy;
        }
    }
}

} /* namespace stensor::impl */


template<typename Bx=border::zero, typename By=border::zero, typename T, index_t Nx=3, index_t Ny=3>
void structure_tensor(container::image<math::mat2s_t<T>>& out,
                      container::image<T> const& in,
                      container::kernel<T, Nx, Ny> const& kx=conv::kernels::sobel3_x<T>,
                      container::kernel<T, Nx, Ny> const& ky=conv::kernels::sobel3_y<T>)
{
    assert(in.size() == out.size());

    // workaround for partial function template specialization
    if constexpr (Nx == 3 && Ny == 3 && std::is_same_v<Bx, border::zero> && std::is_same_v<By, border::zero>) {
        stensor::impl::structure_tensor_3x3_zero<T>(out, in, kx, ky);
    } else {
        stensor::impl::structure_tensor_generic<Bx, By, T, Nx, Ny>(out, in, kx, ky);
    }
}

} /* namespace alg */
