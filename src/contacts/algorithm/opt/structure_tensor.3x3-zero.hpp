/*
 * Optimized version of structure_tensor.hpp. Do not include directly.
 */

#include "algorithm/structure_tensor.hpp"


namespace alg::stensor::impl {

template<typename T>
void structure_tensor_3x3_zero(container::image<math::mat2s_t<T>>& out,
                               container::image<T> const& in,
                               container::kernel<T, 3, 3> const& kx,
                               container::kernel<T, 3, 3> const& ky)
{
    assert(in.size() == out.size());
    assert(kx.stride() == ky.stride());

    // strides
    auto const stride_d = in.stride();
    auto const stride_k = kx.stride();

    // access helpers
    auto const k = [&](auto const& kern, index_t dx, index_t dy) constexpr -> T {
        return kern[4 + dy * stride_k + dx];
    };

    auto const d = [&](index_t i, index_t dx, index_t dy) constexpr -> T {
        return in[i + dy * stride_d + dx];
    };

    // processing...
    index_t i = 0;

    // x = 0, y = 0
    {
        T gx = math::num<T>::zero;
        T gy = math::num<T>::zero;

        gx += d(i,  0,  0) * k(kx,  0,  0);
        gy += d(i,  0,  0) * k(ky,  0,  0);

        gx += d(i,  1,  0) * k(kx,  1,  0);
        gy += d(i,  1,  0) * k(ky,  1,  0);

        gx += d(i,  0,  1) * k(kx,  0,  1);
        gy += d(i,  0,  1) * k(ky,  0,  1);

        gx += d(i,  1,  1) * k(kx,  1,  1);
        gy += d(i,  1,  1) * k(ky,  1,  1);

        out[i] = { gx * gx, gx * gy, gy * gy };
    }
    ++i;

    // 0 < x < n - 1, y = 0
    for (; i < in.size().x - 1; ++i) {
        T gx = math::num<T>::zero;
        T gy = math::num<T>::zero;

        gx += d(i, -1,  0) * k(kx, -1,  0);
        gy += d(i, -1,  0) * k(ky, -1,  0);

        gx += d(i,  0,  0) * k(kx,  0,  0);
        gy += d(i,  0,  0) * k(ky,  0,  0);

        gx += d(i,  1,  0) * k(kx,  1,  0);
        gy += d(i,  1,  0) * k(ky,  1,  0);

        gx += d(i, -1,  1) * k(kx, -1,  1);
        gy += d(i, -1,  1) * k(ky, -1,  1);

        gx += d(i,  0,  1) * k(kx,  0,  1);
        gy += d(i,  0,  1) * k(ky,  0,  1);

        gx += d(i,  1,  1) * k(kx,  1,  1);
        gy += d(i,  1,  1) * k(ky,  1,  1);

        out[i] = { gx * gx, gx * gy, gy * gy };
    }

    // x = n - 1, y = 0
    {
        T gx = math::num<T>::zero;
        T gy = math::num<T>::zero;

        gx += d(i, -1,  0) * k(kx, -1,  0);
        gy += d(i, -1,  0) * k(ky, -1,  0);

        gx += d(i,  0,  0) * k(kx,  0,  0);
        gy += d(i,  0,  0) * k(ky,  0,  0);

        gx += d(i, -1,  1) * k(kx, -1,  1);
        gy += d(i, -1,  1) * k(ky, -1,  1);

        gx += d(i,  0,  1) * k(kx,  0,  1);
        gy += d(i,  0,  1) * k(ky,  0,  1);

        out[i] = { gx * gx, gx * gy, gy * gy };
    }
    ++i;

    // 0 < y < n - 1
    while (i < in.size().x * (in.size().y - 1)) {
        // x = 0
        {
            T gx = math::num<T>::zero;
            T gy = math::num<T>::zero;

            gx += d(i,  0, -1) * k(kx,  0, -1);
            gy += d(i,  0, -1) * k(ky,  0, -1);

            gx += d(i,  1, -1) * k(kx,  1, -1);
            gy += d(i,  1, -1) * k(ky,  1, -1);

            gx += d(i,  0,  0) * k(kx,  0,  0);
            gy += d(i,  0,  0) * k(ky,  0,  0);

            gx += d(i,  1,  0) * k(kx,  1,  0);
            gy += d(i,  1,  0) * k(ky,  1,  0);

            gx += d(i,  0,  1) * k(kx,  0,  1);
            gy += d(i,  0,  1) * k(ky,  0,  1);

            gx += d(i,  1,  1) * k(kx,  1,  1);
            gy += d(i,  1,  1) * k(ky,  1,  1);

            out[i] = { gx * gx, gx * gy, gy * gy };
        }
        ++i;

        // 0 < x < n - 1
        auto const limit = i + in.size().x - 2;
        for (; i < limit; ++i) {
            T gx = math::num<T>::zero;
            T gy = math::num<T>::zero;

            gx += d(i, -1, -1) * k(kx, -1, -1);
            gy += d(i, -1, -1) * k(ky, -1, -1);

            gx += d(i,  0, -1) * k(kx,  0, -1);
            gy += d(i,  0, -1) * k(ky,  0, -1);

            gx += d(i,  1, -1) * k(kx,  1, -1);
            gy += d(i,  1, -1) * k(ky,  1, -1);

            gx += d(i, -1,  0) * k(kx, -1,  0);
            gy += d(i, -1,  0) * k(ky, -1,  0);

            gx += d(i,  0,  0) * k(kx,  0,  0);
            gy += d(i,  0,  0) * k(ky,  0,  0);

            gx += d(i,  1,  0) * k(kx,  1,  0);
            gy += d(i,  1,  0) * k(ky,  1,  0);

            gx += d(i, -1,  1) * k(kx, -1,  1);
            gy += d(i, -1,  1) * k(ky, -1,  1);

            gx += d(i,  0,  1) * k(kx,  0,  1);
            gy += d(i,  0,  1) * k(ky,  0,  1);

            gx += d(i,  1,  1) * k(kx,  1,  1);
            gy += d(i,  1,  1) * k(ky,  1,  1);

            out[i] = { gx * gx, gx * gy, gy * gy };
        }

        // x = n - 1
        {
            T gx = math::num<T>::zero;
            T gy = math::num<T>::zero;

            gx += d(i, -1, -1) * k(kx, -1, -1);
            gy += d(i, -1, -1) * k(ky, -1, -1);

            gx += d(i,  0, -1) * k(kx,  0, -1);
            gy += d(i,  0, -1) * k(ky,  0, -1);

            gx += d(i, -1,  0) * k(kx, -1,  0);
            gy += d(i, -1,  0) * k(ky, -1,  0);

            gx += d(i,  0,  0) * k(kx,  0,  0);
            gy += d(i,  0,  0) * k(ky,  0,  0);

            gx += d(i, -1,  1) * k(kx, -1,  1);
            gy += d(i, -1,  1) * k(ky, -1,  1);

            gx += d(i,  0,  1) * k(kx,  0,  1);
            gy += d(i,  0,  1) * k(ky,  0,  1);

            out[i] = { gx * gx, gx * gy, gy * gy };
        }
        ++i;
    }

    // x = 0, y = n - 1
    {
        T gx = math::num<T>::zero;
        T gy = math::num<T>::zero;

        gx += d(i,  0, -1) * k(kx,  0, -1);
        gy += d(i,  0, -1) * k(ky,  0, -1);

        gx += d(i,  1, -1) * k(kx,  1, -1);
        gy += d(i,  1, -1) * k(ky,  1, -1);

        gx += d(i,  0,  0) * k(kx,  0,  0);
        gy += d(i,  0,  0) * k(ky,  0,  0);

        gx += d(i,  1,  0) * k(kx,  1,  0);
        gy += d(i,  1,  0) * k(ky,  1,  0);

        out[i] = { gx * gx, gx * gy, gy * gy };
    }
    ++i;

    // 0 < x < n - 1, y = n - 1
    for (; i < in.size().product() - 1; ++i) {
        T gx = math::num<T>::zero;
        T gy = math::num<T>::zero;

        gx += d(i, -1, -1) * k(kx, -1, -1);
        gy += d(i, -1, -1) * k(ky, -1, -1);

        gx += d(i,  0, -1) * k(kx,  0, -1);
        gy += d(i,  0, -1) * k(ky,  0, -1);

        gx += d(i,  1, -1) * k(kx,  1, -1);
        gy += d(i,  1, -1) * k(ky,  1, -1);

        gx += d(i, -1,  0) * k(kx, -1,  0);
        gy += d(i, -1,  0) * k(ky, -1,  0);

        gx += d(i,  0,  0) * k(kx,  0,  0);
        gy += d(i,  0,  0) * k(ky,  0,  0);

        gx += d(i,  1,  0) * k(kx,  1,  0);
        gy += d(i,  1,  0) * k(ky,  1,  0);

        out[i] = { gx * gx, gx * gy, gy * gy };
    }

    // x = n - 1, y = n - 1
    {
        T gx = math::num<T>::zero;
        T gy = math::num<T>::zero;

        gx += d(i, -1, -1) * k(kx, -1, -1);
        gy += d(i, -1, -1) * k(ky, -1, -1);

        gx += d(i,  0, -1) * k(kx,  0, -1);
        gy += d(i,  0, -1) * k(ky,  0, -1);

        gx += d(i, -1,  0) * k(kx, -1,  0);
        gy += d(i, -1,  0) * k(ky, -1,  0);

        gx += d(i,  0,  0) * k(kx,  0,  0);
        gy += d(i,  0,  0) * k(ky,  0,  0);

        out[i] = { gx * gx, gx * gy, gy * gy };
    }
}

} /* namespace alg::stensor::impl */
