/*
 * Optimized version of convolution.hpp. Do not include directly.
 */

#include "algorithm/convolution.hpp"


namespace alg::conv::impl {

template<typename T, typename S>
void conv_3x3_extend(container::image<T>& out, container::image<T> const& data,
                     container::kernel<S, 3, 3> const& kern)
{
    // strides
    auto const stride_d = data.stride();
    auto const stride_k = kern.stride();

    // access helpers
    auto const k = [&](index_t dx, index_t dy) constexpr -> S {
        return kern[4 + dy * stride_k + dx];
    };

    auto const d = [&](index_t i, index_t dx, index_t dy) constexpr -> T {
        return data[i + dy * stride_d + dx];
    };

    // processing...
    index_t i = 0;

    // y = 0
    {
        // x = 0
        {
            T v = math::num<T>::zero;

            v += d(i,  0, -1) * k(-1,  0);      // extended
            v += d(i,  0, -1) * k( 0,  0);      // extended
            v += d(i,  1, -1) * k( 1,  0);      // extended

            v += d(i,  0,  0) * k(-1,  0);      // extended
            v += d(i,  0,  0) * k( 0,  0);
            v += d(i,  1,  0) * k( 1,  0);

            v += d(i,  0,  1) * k(-1,  1);      // extended
            v += d(i,  0,  1) * k( 0,  1);
            v += d(i,  1,  1) * k( 1,  1);

            out[i++] = v;
        }

        // 0 < x < n
        auto const limit = i + data.size().x - 2;
        while (i < limit) {
            T v = math::num<T>::zero;

            v += d(i, -1, -1) * k(-1,  0);      // extended
            v += d(i,  0, -1) * k( 0,  0);      // extended
            v += d(i,  1, -1) * k( 1,  0);      // extended

            v += d(i, -1,  0) * k(-1,  0);
            v += d(i,  0,  0) * k( 0,  0);
            v += d(i,  1,  0) * k( 1,  0);

            v += d(i, -1,  1) * k(-1,  1);
            v += d(i,  0,  1) * k( 0,  1);
            v += d(i,  1,  1) * k( 1,  1);

            out[i++] = v;
        }

        // x = n - 1
        {
            T v = math::num<T>::zero;

            v += d(i, -1, -1) * k(-1,  0);      // extended
            v += d(i,  0, -1) * k( 0,  0);      // extended
            v += d(i,  0, -1) * k( 1,  0);      // extended

            v += d(i, -1,  0) * k(-1,  0);
            v += d(i,  0,  0) * k( 0,  0);
            v += d(i,  0,  0) * k( 1,  0);      // extended

            v += d(i, -1,  1) * k(-1,  1);
            v += d(i,  0,  1) * k( 0,  1);
            v += d(i,  0,  1) * k( 1,  1);      // extended

            out[i++] = v;
        }
    }

    // 0 < y < n
    while (i < data.size().x * (data.size().y - 1)) {
        // x = 0
        {
            T v = math::num<T>::zero;

            v += d(i,  0, -1) * k(-1, -1);      // extended
            v += d(i,  0, -1) * k( 0, -1);
            v += d(i,  1, -1) * k( 1, -1);

            v += d(i,  0,  0) * k(-1,  0);      // extended
            v += d(i,  0,  0) * k( 0,  0);
            v += d(i,  1,  0) * k( 1,  0);

            v += d(i,  0,  1) * k(-1,  1);      // extended
            v += d(i,  0,  1) * k( 0,  1);
            v += d(i,  1,  1) * k( 1,  1);

            out[i++] = v;
        }

        // 0 < x < n
        auto const limit = i + data.size().x - 2;
        while (i < limit) {
            T v = math::num<T>::zero;

            v += d(i, -1, -1) * k(-1, -1);
            v += d(i,  0, -1) * k( 0, -1);
            v += d(i,  1, -1) * k( 1, -1);

            v += d(i, -1,  0) * k(-1,  0);
            v += d(i,  0,  0) * k( 0,  0);
            v += d(i,  1,  0) * k( 1,  0);

            v += d(i, -1,  1) * k(-1,  1);
            v += d(i,  0,  1) * k( 0,  1);
            v += d(i,  1,  1) * k( 1,  1);

            out[i++] = v;
        }

        // x = n - 1
        {
            T v = math::num<T>::zero;

            v += d(i, -1, -1) * k(-1, -1);
            v += d(i,  0, -1) * k( 0, -1);
            v += d(i,  0, -1) * k( 1, -1);      // extended

            v += d(i, -1,  0) * k(-1,  0);
            v += d(i,  0,  0) * k( 0,  0);
            v += d(i,  0,  0) * k( 1,  0);      // extended

            v += d(i, -1,  1) * k(-1,  1);
            v += d(i,  0,  1) * k( 0,  1);
            v += d(i,  0,  1) * k( 1,  1);      // extended

            out[i++] = v;
        }
    }

    // y = n - 1
    {
        // x = 0
        {
            T v = math::num<T>::zero;

            v += d(i,  0, -1) * k(-1, -1);      // extended
            v += d(i,  0, -1) * k( 0, -1);
            v += d(i,  1, -1) * k( 1, -1);

            v += d(i,  0,  0) * k(-1,  0);      // extended
            v += d(i,  0,  0) * k( 0,  0);
            v += d(i,  1,  0) * k( 1,  0);

            v += d(i,  0,  0) * k(-1,  1);      // extended
            v += d(i,  0,  0) * k( 0,  1);      // extended
            v += d(i,  1,  0) * k( 1,  1);      // extended

            out[i++] = v;
        }

        // 1 < x < n - 2
        auto const limit = i + data.size().x - 2;
        while (i < limit) {
            T v = math::num<T>::zero;

            v += d(i, -1, -1) * k(-1, -1);
            v += d(i,  0, -1) * k( 0, -1);
            v += d(i,  1, -1) * k( 1, -1);

            v += d(i, -1,  0) * k(-1,  0);
            v += d(i,  0,  0) * k( 0,  0);
            v += d(i,  1,  0) * k( 1,  0);

            v += d(i, -1,  0) * k(-1,  1);      // extended
            v += d(i,  0,  0) * k( 0,  1);      // extended
            v += d(i,  1,  0) * k( 1,  1);      // extended

            out[i++] = v;
        }

        // x = n - 1
        {
            T v = math::num<T>::zero;

            v += d(i, -1, -1) * k(-1, -1);
            v += d(i,  0, -1) * k( 0, -1);
            v += d(i,  0, -1) * k( 1, -1);      // extended

            v += d(i, -1,  0) * k(-1,  0);
            v += d(i,  0,  0) * k( 0,  0);
            v += d(i,  0,  0) * k( 1,  0);      // extended

            v += d(i, -1,  0) * k(-1,  1);      // extended
            v += d(i,  0,  0) * k( 0,  1);      // extended
            v += d(i,  0,  0) * k( 1,  1);      // extended

            out[i++] = v;
        }
    }
}

} /* namespace alg::conv::impl */
