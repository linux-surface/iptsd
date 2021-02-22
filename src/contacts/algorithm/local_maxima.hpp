#pragma once

#include "types.hpp"
#include "container/image.hpp"


namespace iptsd::alg {

template<int C=8, typename T, typename O>
void find_local_maximas(Image<T> const& data, T threshold, O output_iter)
{
    static_assert(C == 4 || C == 8);

    index_t i = 0;

    /*
     * We use the following kernel to compare entries:
     *
     *   [< ] [< ] [< ]
     *   [< ] [  ] [<=]
     *   [<=] [<=] [<=]
     *
     * Half of the entries use "less or equal", the other half "less than" as
     * operators to ensure that we don't either discard any local maximas or
     * report some multiple times.
     */

    // stride
    auto const stride = data.stride();

    // access helper
    auto const d = [&](index_t i, index_t dx, index_t dy) constexpr -> T {
        return data[i + dy * stride + dx];
    };

    // x = 0, y = 0
    if (data[i] > threshold) {
        bool max = true;

        max &= d(i, 1, 0) <= d(i, 0, 0);
        max &= d(i, 0, 1) <= d(i, 0, 0);

        if constexpr (C == 8) {
            max &= d(i, 1, 1) <= d(i, 0, 0);
        }

        if (max) {
            *output_iter++ = i;
        }
    }
    ++i;

    // 0 < x < n - 1, y = 0
    for (; i < data.size().x - 1; ++i) {
        if (data[i] <= threshold)
            continue;

        bool max = true;

        max &= d(i, -1, 0) <  d(i, 0, 0);
        max &= d(i,  1, 0) <= d(i, 0, 0);

        if constexpr (C == 8) {
            max &= d(i, -1, 1) <= d(i, 0, 0);
        }

        max &= d(i, 0, 1) <= d(i, 0, 0);

        if constexpr (C == 8) {
            max &= d(i, 1, 1) <= d(i, 0, 0);
        }

        if (max) {
            *output_iter++ = i;
        }
    }

    // x = n - 1, y = 0
    if (data[i] > threshold) {
        bool max = true;

        max &= d(i, -1, 0) < d(i, 0, 0);

        if constexpr (C == 8) {
            max &= d(i, -1, 1) <= d(i, 0, 0);
        }

        max &= d(i, 0, 1) <= d(i, 0, 0);

        if (max) {
            *output_iter++ = i;
        }
    }
    ++i;

    // 0 < y < n - 1
    while (i < data.size().x * (data.size().y - 1)) {
        // x = 0
        if (data[i] > threshold) {
            bool max = true;

            max &= d(i, 1,  0) <= d(i, 0, 0);
            max &= d(i, 0, -1) <  d(i, 0, 0);

            if constexpr (C == 8) {
                max &= d(i, 1, -1) < d(i, 0, 0);
            }

            max &= d(i, 0, 1) <= d(i, 0, 0);

            if constexpr (C == 8) {
                max &= d(i, 1, 1) <= d(i, 0, 0);
            }

            if (max) {
                *output_iter++ = i;
            }
        }
        ++i;

        // 0 < x < n - 1
        auto const limit = i + data.size().x - 2;
        for (; i < limit; ++i) {
            if (data[i] <= threshold)
                continue;

            bool max = true;

            max &= d(i, -1, 0) <  d(i, 0, 0);
            max &= d(i,  1, 0) <= d(i, 0, 0);

            if constexpr (C == 8) {
                max &= d(i, -1, -1) < d(i, 0, 0);
            }

            max &= d(i, 0, -1) < d(i, 0, 0);

            if constexpr (C == 8) {
                max &= d(i,  1, -1) <  d(i, 0, 0);
                max &= d(i, -1,  1) <= d(i, 0, 0);
            }

            max &= d(i, 0, 1) <= d(i, 0, 0);

            if constexpr (C == 8) {
                max &= d(i, 1, 1) <= d(i, 0, 0);
            }

            if (max) {
                *output_iter++ = i;
            }
        }

        // x = n - 1
        if (data[i] > threshold) {
            bool max = true;

            max &= d(i, -1, 0) < d(i, 0, 0);

            if constexpr (C == 8) {
                max &= d(i, -1, -1) < d(i, 0, 0);
            }

            max &= d(i, 0, -1) < d(i, 0, 0);

            if constexpr (C == 8) {
                max &= d(i, -1, 1) <= d(i, 0, 0);
            }

            max &= d(i, 0, 1) <= d(i, 0, 0);

            if (max) {
                *output_iter++ = i;
            }
        }
        ++i;
    }

    // x = 0, y = n - 1
    if (data[i] > threshold) {
        bool max = true;

        max &= d(i, 1,  0) <= d(i, 0, 0);
        max &= d(i, 0, -1) <  d(i, 0, 0);

        if constexpr (C == 8) {
            max &= d(i,  1, -1) <  d(i, 0, 0);
        }

        if (max) {
            *output_iter++ = i;
        }
    }
    ++i;

    // 0 < x < n - 1, y = n - 1
    for (; i < data.size().span() - 1; ++i) {
        if (data[i] <= threshold)
            continue;

        bool max = true;

        max &= d(i, -1, 0) <  d(i, 0, 0);
        max &= d(i,  1, 0) <= d(i, 0, 0);

        if constexpr (C == 8) {
            max &= d(i, -1, -1) < d(i, 0, 0);
        }

        max &= d(i, 0, -1) < d(i, 0, 0);

        if constexpr (C == 8) {
            max &= d(i,  1, -1) <  d(i, 0, 0);
        }

        if (max) {
            *output_iter++ = i;
        }
    }

    // x = n - 1, y = n - 1
    if (data[i] > threshold) {
        bool max = true;

        max &= d(i, -1, 0) < d(i, 0, 0);

        if constexpr (C == 8) {
            max &= d(i, -1, -1) < d(i, 0, 0);
        }

        max &= d(i, 0, -1) < d(i, 0, 0);

        if (max) {
            *output_iter++ = i;
        }
    }
}

} /* namespace iptsd::alg */
