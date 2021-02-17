/*
 * Optimized version of hessian.hpp. Do not include directly.
 */

#include "algorithm/hessian.hpp"


namespace alg::hess::impl {

template<typename T>
void hessian_zero(container::image<math::mat2s_t<T>>& out, container::image<T> const& in)
{
    // kernels
    auto const& kxx = conv::kernels::sobel3_xx<T>;
    auto const& kyy = conv::kernels::sobel3_yy<T>;
    auto const& kxy = conv::kernels::sobel3_xy<T>;

    // strides
    auto const stride_d = in.stride();
    auto const stride_k = 3;

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
        auto h = math::num<math::mat2s_t<T>>::zero;

        h.xx += d(i,  0,  0) * k(kxx,  0,  0);
        h.xy += d(i,  0,  0) * k(kxy,  0,  0);
        h.yy += d(i,  0,  0) * k(kyy,  0,  0);

        h.xx += d(i,  1,  0) * k(kxx,  1,  0);
        h.xy += d(i,  1,  0) * k(kxy,  1,  0);
        h.yy += d(i,  1,  0) * k(kyy,  1,  0);

        h.xx += d(i,  0,  1) * k(kxx,  0,  1);
        h.xy += d(i,  0,  1) * k(kxy,  0,  1);
        h.yy += d(i,  0,  1) * k(kyy,  0,  1);

        h.xx += d(i,  1,  1) * k(kxx,  1,  1);
        h.xy += d(i,  1,  1) * k(kxy,  1,  1);
        h.yy += d(i,  1,  1) * k(kyy,  1,  1);

        out[i] = h;
    }
    ++i;

    // 0 < x < n - 1, y = 0
    for (; i < in.size().x - 1; ++i) {
        auto h = math::num<math::mat2s_t<T>>::zero;

        h.xx += d(i, -1,  0) * k(kxx, -1,  0);
        h.xy += d(i, -1,  0) * k(kxy, -1,  0);
        h.yy += d(i, -1,  0) * k(kyy, -1,  0);

        h.xx += d(i,  0,  0) * k(kxx,  0,  0);
        h.xy += d(i,  0,  0) * k(kxy,  0,  0);
        h.yy += d(i,  0,  0) * k(kyy,  0,  0);

        h.xx += d(i,  1,  0) * k(kxx,  1,  0);
        h.xy += d(i,  1,  0) * k(kxy,  1,  0);
        h.yy += d(i,  1,  0) * k(kyy,  1,  0);

        h.xx += d(i, -1,  1) * k(kxx, -1,  1);
        h.xy += d(i, -1,  1) * k(kxy, -1,  1);
        h.yy += d(i, -1,  1) * k(kyy, -1,  1);

        h.xx += d(i,  0,  1) * k(kxx,  0,  1);
        h.xy += d(i,  0,  1) * k(kxy,  0,  1);
        h.yy += d(i,  0,  1) * k(kyy,  0,  1);

        h.xx += d(i,  1,  1) * k(kxx,  1,  1);
        h.xy += d(i,  1,  1) * k(kxy,  1,  1);
        h.yy += d(i,  1,  1) * k(kyy,  1,  1);

        out[i] = h;
    }

    // x = n - 1, y = 0
    {
        auto h = math::num<math::mat2s_t<T>>::zero;

        h.xx += d(i, -1,  0) * k(kxx, -1,  0);
        h.xy += d(i, -1,  0) * k(kxy, -1,  0);
        h.yy += d(i, -1,  0) * k(kyy, -1,  0);

        h.xx += d(i,  0,  0) * k(kxx,  0,  0);
        h.xy += d(i,  0,  0) * k(kxy,  0,  0);
        h.yy += d(i,  0,  0) * k(kyy,  0,  0);

        h.xx += d(i, -1,  1) * k(kxx, -1,  1);
        h.xy += d(i, -1,  1) * k(kxy, -1,  1);
        h.yy += d(i, -1,  1) * k(kyy, -1,  1);

        h.xx += d(i,  0,  1) * k(kxx,  0,  1);
        h.xy += d(i,  0,  1) * k(kxy,  0,  1);
        h.yy += d(i,  0,  1) * k(kyy,  0,  1);

        out[i] = h;
    }
    ++i;

    // 0 < y < n - 1
    while (i < in.size().x * (in.size().y - 1)) {
        // x = 0
        {
            auto h = math::num<math::mat2s_t<T>>::zero;

            h.xx += d(i,  0, -1) * k(kxx,  0, -1);
            h.xy += d(i,  0, -1) * k(kxy,  0, -1);
            h.yy += d(i,  0, -1) * k(kyy,  0, -1);

            h.xx += d(i,  1, -1) * k(kxx,  1, -1);
            h.xy += d(i,  1, -1) * k(kxy,  1, -1);
            h.yy += d(i,  1, -1) * k(kyy,  1, -1);

            h.xx += d(i,  0,  0) * k(kxx,  0,  0);
            h.xy += d(i,  0,  0) * k(kxy,  0,  0);
            h.yy += d(i,  0,  0) * k(kyy,  0,  0);

            h.xx += d(i,  1,  0) * k(kxx,  1,  0);
            h.xy += d(i,  1,  0) * k(kxy,  1,  0);
            h.yy += d(i,  1,  0) * k(kyy,  1,  0);

            h.xx += d(i,  0,  1) * k(kxx,  0,  1);
            h.xy += d(i,  0,  1) * k(kxy,  0,  1);
            h.yy += d(i,  0,  1) * k(kyy,  0,  1);

            h.xx += d(i,  1,  1) * k(kxx,  1,  1);
            h.xy += d(i,  1,  1) * k(kxy,  1,  1);
            h.yy += d(i,  1,  1) * k(kyy,  1,  1);

            out[i] = h;
        }
        ++i;

        // 0 < x < n - 1
        auto const limit = i + in.size().x - 2;
        for (; i < limit; ++i) {
            auto h = math::num<math::mat2s_t<T>>::zero;

            h.xx += d(i, -1, -1) * k(kxx, -1, -1);
            h.xy += d(i, -1, -1) * k(kxy, -1, -1);
            h.yy += d(i, -1, -1) * k(kyy, -1, -1);

            h.xx += d(i,  0, -1) * k(kxx,  0, -1);
            h.xy += d(i,  0, -1) * k(kxy,  0, -1);
            h.yy += d(i,  0, -1) * k(kyy,  0, -1);

            h.xx += d(i,  1, -1) * k(kxx,  1, -1);
            h.xy += d(i,  1, -1) * k(kxy,  1, -1);
            h.yy += d(i,  1, -1) * k(kyy,  1, -1);

            h.xx += d(i, -1,  0) * k(kxx, -1,  0);
            h.xy += d(i, -1,  0) * k(kxy, -1,  0);
            h.yy += d(i, -1,  0) * k(kyy, -1,  0);

            h.xx += d(i,  0,  0) * k(kxx,  0,  0);
            h.xy += d(i,  0,  0) * k(kxy,  0,  0);
            h.yy += d(i,  0,  0) * k(kyy,  0,  0);

            h.xx += d(i,  1,  0) * k(kxx,  1,  0);
            h.xy += d(i,  1,  0) * k(kxy,  1,  0);
            h.yy += d(i,  1,  0) * k(kyy,  1,  0);

            h.xx += d(i, -1,  1) * k(kxx, -1,  1);
            h.xy += d(i, -1,  1) * k(kxy, -1,  1);
            h.yy += d(i, -1,  1) * k(kyy, -1,  1);

            h.xx += d(i,  0,  1) * k(kxx,  0,  1);
            h.xy += d(i,  0,  1) * k(kxy,  0,  1);
            h.yy += d(i,  0,  1) * k(kyy,  0,  1);

            h.xx += d(i,  1,  1) * k(kxx,  1,  1);
            h.xy += d(i,  1,  1) * k(kxy,  1,  1);
            h.yy += d(i,  1,  1) * k(kyy,  1,  1);

            out[i] = h;
        }

        // x = n - 1
        {
            auto h = math::num<math::mat2s_t<T>>::zero;

            h.xx += d(i, -1, -1) * k(kxx, -1, -1);
            h.xy += d(i, -1, -1) * k(kxy, -1, -1);
            h.yy += d(i, -1, -1) * k(kyy, -1, -1);

            h.xx += d(i,  0, -1) * k(kxx,  0, -1);
            h.xy += d(i,  0, -1) * k(kxy,  0, -1);
            h.yy += d(i,  0, -1) * k(kyy,  0, -1);

            h.xx += d(i, -1,  0) * k(kxx, -1,  0);
            h.xy += d(i, -1,  0) * k(kxy, -1,  0);
            h.yy += d(i, -1,  0) * k(kyy, -1,  0);

            h.xx += d(i,  0,  0) * k(kxx,  0,  0);
            h.xy += d(i,  0,  0) * k(kxy,  0,  0);
            h.yy += d(i,  0,  0) * k(kyy,  0,  0);

            h.xx += d(i, -1,  1) * k(kxx, -1,  1);
            h.xy += d(i, -1,  1) * k(kxy, -1,  1);
            h.yy += d(i, -1,  1) * k(kyy, -1,  1);

            h.xx += d(i,  0,  1) * k(kxx,  0,  1);
            h.xy += d(i,  0,  1) * k(kxy,  0,  1);
            h.yy += d(i,  0,  1) * k(kyy,  0,  1);

            out[i] = h;
        }
        ++i;
    }

    // x = 0, y = n - 1
    {
        auto h = math::num<math::mat2s_t<T>>::zero;

        h.xx += d(i,  0, -1) * k(kxx,  0, -1);
        h.xy += d(i,  0, -1) * k(kxy,  0, -1);
        h.yy += d(i,  0, -1) * k(kyy,  0, -1);

        h.xx += d(i,  1, -1) * k(kxx,  1, -1);
        h.xy += d(i,  1, -1) * k(kxy,  1, -1);
        h.yy += d(i,  1, -1) * k(kyy,  1, -1);

        h.xx += d(i,  0,  0) * k(kxx,  0,  0);
        h.xy += d(i,  0,  0) * k(kxy,  0,  0);
        h.yy += d(i,  0,  0) * k(kyy,  0,  0);

        h.xx += d(i,  1,  0) * k(kxx,  1,  0);
        h.xy += d(i,  1,  0) * k(kxy,  1,  0);
        h.yy += d(i,  1,  0) * k(kyy,  1,  0);

        out[i] = h;
    }
    ++i;

    // 0 < x < n - 1, y = n - 1
    for (; i < in.size().product() - 1; ++i) {
        auto h = math::num<math::mat2s_t<T>>::zero;

        h.xx += d(i, -1, -1) * k(kxx, -1, -1);
        h.xy += d(i, -1, -1) * k(kxy, -1, -1);
        h.yy += d(i, -1, -1) * k(kyy, -1, -1);

        h.xx += d(i,  0, -1) * k(kxx,  0, -1);
        h.xy += d(i,  0, -1) * k(kxy,  0, -1);
        h.yy += d(i,  0, -1) * k(kyy,  0, -1);

        h.xx += d(i,  1, -1) * k(kxx,  1, -1);
        h.xy += d(i,  1, -1) * k(kxy,  1, -1);
        h.yy += d(i,  1, -1) * k(kyy,  1, -1);

        h.xx += d(i, -1,  0) * k(kxx, -1,  0);
        h.xy += d(i, -1,  0) * k(kxy, -1,  0);
        h.yy += d(i, -1,  0) * k(kyy, -1,  0);

        h.xx += d(i,  0,  0) * k(kxx,  0,  0);
        h.xy += d(i,  0,  0) * k(kxy,  0,  0);
        h.yy += d(i,  0,  0) * k(kyy,  0,  0);

        h.xx += d(i,  1,  0) * k(kxx,  1,  0);
        h.xy += d(i,  1,  0) * k(kxy,  1,  0);
        h.yy += d(i,  1,  0) * k(kyy,  1,  0);

        out[i] = h;
    }

    // x = n - 1, y = n - 1
    {
        auto h = math::num<math::mat2s_t<T>>::zero;

        h.xx += d(i, -1, -1) * k(kxx, -1, -1);
        h.xy += d(i, -1, -1) * k(kxy, -1, -1);
        h.yy += d(i, -1, -1) * k(kyy, -1, -1);

        h.xx += d(i,  0, -1) * k(kxx,  0, -1);
        h.xy += d(i,  0, -1) * k(kxy,  0, -1);
        h.yy += d(i,  0, -1) * k(kyy,  0, -1);

        h.xx += d(i, -1,  0) * k(kxx, -1,  0);
        h.xy += d(i, -1,  0) * k(kxy, -1,  0);
        h.yy += d(i, -1,  0) * k(kyy, -1,  0);

        h.xx += d(i,  0,  0) * k(kxx,  0,  0);
        h.xy += d(i,  0,  0) * k(kxy,  0,  0);
        h.yy += d(i,  0,  0) * k(kyy,  0,  0);

        out[i] = h;
    }
}

} /* namespace alg::hess::impl */
