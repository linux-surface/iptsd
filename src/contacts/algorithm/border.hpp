#pragma once

#include "types.hpp"
#include "container/image.hpp"
#include "math/num.hpp"

#include <algorithm>


namespace alg::border {

struct mirror {
    template<class T>
    static constexpr auto value(container::image<T> const& img, index2_t const& i) -> T;
};

template<class T>
constexpr auto mirror::value(container::image<T> const& img, index2_t const& i) -> T
{
    index_t const x = i.x >= 0 ? (i.x < img.shape().x ? i.x : 2 * img.shape().x - i.x - 1) : (-1 - i.x);
    index_t const y = i.y >= 0 ? (i.y < img.shape().y ? i.y : 2 * img.shape().y - i.y - 1) : (-1 - i.y);

    return img[{x, y}];
}


struct mirror_x {
    template<class T>
    static constexpr auto value(container::image<T> const& img, index2_t const& i) -> T;
};

template<class T>
constexpr auto mirror_x::value(container::image<T> const& img, index2_t const& i) -> T
{
    index_t const x = i.x >= 0 ? (i.x < img.shape().x ? i.x : 2 * img.shape().x - i.x - 1) : (-1 - i.x);

    return i.y >= 0 && i.y < img.shape().y ? img[{x, i.y}] : math::num<T>::zero;
}


struct mirror_y {
    template<class T>
    static constexpr auto value(container::image<T> const& img, index2_t const& i) -> T;
};

template<class T>
constexpr auto mirror_y::value(container::image<T> const& img, index2_t const& i) -> T
{
    index_t const y = i.y >= 0 ? (i.y < img.shape().y ? i.y : 2 * img.shape().y - i.y - 1) : (-1 - i.y);

    return i.x >= 0 && i.x < img.shape().x ? img[{i.x, y}] : math::num<T>::zero;
}


struct extend {
    template<class T>
    static constexpr auto value(container::image<T> const& img, index2_t const& i) -> T;
};

template<class T>
constexpr auto extend::value(container::image<T> const& img, index2_t const& i) -> T
{
    index_t const x = std::clamp(i.x, 0, img.shape().x - 1);
    index_t const y = std::clamp(i.y, 0, img.shape().y - 1);

    return img[{x, y}];
}


struct zero {
    template<class T>
    static constexpr auto value(container::image<T> const& img, index2_t const& i) -> T;
};

template<class T>
constexpr auto zero::value(container::image<T> const& img, index2_t const& i) -> T
{
    return i.x >= 0 && i.x < img.shape().x && i.y >= 0 && i.y < img.shape().y ?
        img[{i.x, i.y}] : math::num<T>::zero;
}

} /* namespace alg::border */
