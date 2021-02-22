#pragma once

#include "types.hpp"
#include "container/image.hpp"
#include "math/num.hpp"

#include <algorithm>


namespace iptsd::alg::border {

struct Mirror {
    template<class T>
    static constexpr auto value(Image<T> const& img, index2_t const& i) -> T;
};

template<class T>
constexpr auto Mirror::value(Image<T> const& img, index2_t const& i) -> T
{
    index_t const x = i.x >= 0 ? (i.x < img.shape().x ? i.x : 2 * img.shape().x - i.x - 1) : (-1 - i.x);
    index_t const y = i.y >= 0 ? (i.y < img.shape().y ? i.y : 2 * img.shape().y - i.y - 1) : (-1 - i.y);

    return img[{x, y}];
}


struct MirrorX {
    template<class T>
    static constexpr auto value(Image<T> const& img, index2_t const& i) -> T;
};

template<class T>
constexpr auto MirrorX::value(Image<T> const& img, index2_t const& i) -> T
{
    index_t const x = i.x >= 0 ? (i.x < img.shape().x ? i.x : 2 * img.shape().x - i.x - 1) : (-1 - i.x);

    return i.y >= 0 && i.y < img.shape().y ? img[{x, i.y}] : math::num<T>::zero;
}


struct MirrorY {
    template<class T>
    static constexpr auto value(Image<T> const& img, index2_t const& i) -> T;
};

template<class T>
constexpr auto MirrorY::value(Image<T> const& img, index2_t const& i) -> T
{
    index_t const y = i.y >= 0 ? (i.y < img.shape().y ? i.y : 2 * img.shape().y - i.y - 1) : (-1 - i.y);

    return i.x >= 0 && i.x < img.shape().x ? img[{i.x, y}] : math::num<T>::zero;
}


struct Extend {
    template<class T>
    static constexpr auto value(Image<T> const& img, index2_t const& i) -> T;
};

template<class T>
constexpr auto Extend::value(Image<T> const& img, index2_t const& i) -> T
{
    index_t const x = std::clamp(i.x, 0, img.shape().x - 1);
    index_t const y = std::clamp(i.y, 0, img.shape().y - 1);

    return img[{x, y}];
}


struct Zero {
    template<class T>
    static constexpr auto value(Image<T> const& img, index2_t const& i) -> T;
};

template<class T>
constexpr auto Zero::value(Image<T> const& img, index2_t const& i) -> T
{
    return i.x >= 0 && i.x < img.shape().x && i.y >= 0 && i.y < img.shape().y ?
        img[{i.x, i.y}] : math::num<T>::zero;
}

} /* namespace iptsd::alg::border */
