#pragma once

#include "types.hpp"

#include "container/image.hpp"
#include "container/kernel.hpp"

#include "algorithm/border.hpp"
#include "algorithm/convolution.hpp"

#include "math/num.hpp"
#include "math/mat2.hpp"

#include "algorithm/opt/hessian.zero.hpp"

#include <cassert>


namespace alg {
namespace hess::impl {

template<typename B=border::zero, typename T>
void hessian_generic(container::image<math::mat2s_t<T>>& out, container::image<T> const& in)
{
    auto const& kxx = conv::kernels::sobel3_xx<T>;
    auto const& kyy = conv::kernels::sobel3_yy<T>;
    auto const& kxy = conv::kernels::sobel3_xy<T>;

    index_t const nx = kxx.size().x;
    index_t const ny = kxx.size().y;

    index_t const dx = (nx - 1) / 2;
    index_t const dy = (ny - 1) / 2;

    for (index_t cy = 0; cy < in.size().y; ++cy) {
        for (index_t cx = 0; cx < in.size().x; ++cx) {
            T hxx = math::num<T>::zero;
            T hxy = math::num<T>::zero;
            T hyy = math::num<T>::zero;

            for (index_t iy = 0; iy < ny; ++iy) {
                for (index_t ix = 0; ix < nx; ++ix) {
                    hxx += B::value(in, {cx - dx + ix, cy - dy + iy}) * kxx[{ix, iy}];
                    hxy += B::value(in, {cx - dx + ix, cy - dy + iy}) * kxy[{ix, iy}];
                    hyy += B::value(in, {cx - dx + ix, cy - dy + iy}) * kyy[{ix, iy}];
                }
            }

            out[{cx, cy}].xx = hxx;
            out[{cx, cy}].xy = hxy;
            out[{cx, cy}].yy = hyy;
        }
    }
}

} /* namespace hess::impl */


template<typename B=border::zero, typename T>
void hessian(container::image<math::mat2s_t<T>>& out, container::image<T> const& in)
{
    assert(in.size() == out.size());

    if constexpr (std::is_same_v<B, border::zero>) {
        hess::impl::hessian_zero<T>(out, in);
    } else {
        hess::impl::hessian_generic<B, T>(out, in);
    }
}

} /* namespace alg */
