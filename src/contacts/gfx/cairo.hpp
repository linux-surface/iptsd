#pragma once

#include "container/image.hpp"
#include "gfx/cmap.hpp"
#include "gfx/color.hpp"
#include "math/vec2.hpp"

#include <exception>
#include <filesystem>
#include <utility>

#include <cairo/cairo.h>


namespace gfx::cairo {

using math::vec2_t;

using status_t = cairo_status_t;

class surface;
class pattern;
class matrix;


enum class format {
    invalid   = CAIRO_FORMAT_INVALID,
    argb32    = CAIRO_FORMAT_ARGB32,
    rgb24     = CAIRO_FORMAT_RGB24,
    a8        = CAIRO_FORMAT_A8,
    a1        = CAIRO_FORMAT_A1,
    rgb16_565 = CAIRO_FORMAT_RGB16_565,
    rbg30     = CAIRO_FORMAT_RGB30,
    rgb96f    = CAIRO_FORMAT_RGB96F,
    rgba128f  = CAIRO_FORMAT_RGBA128F,
};

enum class filter {
    fast     = CAIRO_FILTER_FAST,
    good     = CAIRO_FILTER_GOOD,
    best     = CAIRO_FILTER_BEST,
    nearest  = CAIRO_FILTER_NEAREST,
    bilinear = CAIRO_FILTER_BILINEAR,
    gaussian = CAIRO_FILTER_GAUSSIAN,
};

enum class font_slant {
    normal  = CAIRO_FONT_SLANT_NORMAL,
    italic  = CAIRO_FONT_SLANT_ITALIC,
    oblique = CAIRO_FONT_SLANT_OBLIQUE,
};

enum class font_weight {
    normal = CAIRO_FONT_WEIGHT_NORMAL,
    bold   = CAIRO_FONT_WEIGHT_BOLD,
};


class exception : public std::exception {
public:
    exception(status_t code);

    auto what() const noexcept -> const char*;
    auto code() const noexcept -> status_t;

private:
    status_t m_code;
};


class cairo {
public:
    cairo();
    cairo(cairo_t* raw);
    cairo(cairo const& other);
    cairo(cairo&& other);
    ~cairo();

    static auto create(surface& surface) -> cairo;
    static auto wrap_raw(cairo_t* raw) -> cairo;

    void operator=(cairo const& rhs);
    void operator=(cairo&& rhs);

    auto raw() -> cairo_t*;
    auto operator* () -> cairo_t*;

    auto status() const -> status_t;

    void set_source(pattern& p);
    void set_source(srgb rgb);
    void set_source(srgba rgba);
    void set_source(surface& src, vec2_t<f64> origin);
    auto get_source() -> pattern;

    void set_source_filter(filter f);

    void paint();
    void fill();
    void stroke();

    void save();
    void restore();

    void translate(vec2_t<f64> s);
    void scale(vec2_t<f64> s);
    void rotate(f64 angle);

    void move_to(vec2_t<f64> pos);
    void line_to(vec2_t<f64> pos);
    void rectangle(vec2_t<f64> origin, vec2_t<f64> size);
    void arc(vec2_t<f64> center, f64 radius, f64 angle1, f64 angle2);

    void select_font_face(char const* family, font_slant slant, font_weight weight);
    void set_font_size(f64 size);
    void show_text(char const* utf8);

private:
    cairo_t* m_raw;
};


class surface {
public:
    surface();
    surface(cairo_surface_t* raw);
    surface(surface const& other);
    surface(surface&& other);
    ~surface();

    void operator=(surface const& rhs);
    void operator=(surface&& rhs);

    auto raw() -> cairo_surface_t*;
    auto operator* () -> cairo_surface_t*;

    auto status() const -> status_t;

    void write_to_png(char const* filename);
    void write_to_png(std::filesystem::path const& p);

private:
    cairo_surface_t* m_raw;
};


class pattern {
public:
    pattern();
    pattern(cairo_pattern_t* raw);
    pattern(pattern const& other);
    pattern(pattern&& other);
    ~pattern();

    static auto create_for_surface(surface &surface) -> pattern;

    void operator=(pattern const& rhs);
    void operator=(pattern&& rhs);

    auto raw() -> cairo_pattern_t*;
    auto operator* () -> cairo_pattern_t*;

    auto status() const -> status_t;

    void set_matrix(matrix &m);
    void set_filter(filter f);

private:
    cairo_pattern_t* m_raw;
};


class matrix {
public:
    matrix();
    matrix(cairo_matrix_t m);

    static auto identity() -> matrix;

    auto raw() -> cairo_matrix_t*;
    auto operator* () -> cairo_matrix_t*;

    void translate(vec2_t<f64> v);
    void scale(vec2_t<f64> v);

private:
    cairo_matrix_t m_raw;
};


inline exception::exception(status_t code)
    : m_code{code}
{}

inline auto exception::what() const noexcept -> const char*
{
    return cairo_status_to_string(m_code);
}

inline auto exception::code() const noexcept -> status_t
{
    return m_code;
}


inline cairo::cairo()
    : m_raw{nullptr}
{}

inline cairo::cairo(cairo_t* raw)
    : m_raw{raw}
{}

inline cairo::cairo(cairo const& other)
    : m_raw{other.m_raw ? cairo_reference(other.m_raw) : nullptr}
{}

inline cairo::cairo(cairo&& other)
    : m_raw{std::exchange(other.m_raw, nullptr)}
{}

inline cairo::~cairo()
{
    if (m_raw) {
        cairo_destroy(m_raw);
    }
}

inline auto cairo::create(surface& target) -> cairo
{
    cairo cr { cairo_create(target.raw()) };

    if (cr.status() != CAIRO_STATUS_SUCCESS) {
        throw exception{cr.status()};
    }

    return cr;
}

inline auto cairo::wrap_raw(cairo_t* raw) -> cairo
{
    return { cairo_reference(raw) };
}

inline void cairo::operator=(cairo const& rhs)
{
    m_raw = rhs.m_raw ? cairo_reference(rhs.m_raw) : nullptr;
}

inline void cairo::operator=(cairo&& rhs)
{
    m_raw = std::exchange(rhs.m_raw, nullptr);
}

inline auto cairo::raw() -> cairo_t*
{
    return m_raw;
}

inline auto cairo::operator* () -> cairo_t*
{
    return m_raw;
}

inline auto cairo::status() const -> cairo_status_t
{
    return cairo_status(m_raw);
}

inline void cairo::set_source(pattern& p)
{
    cairo_set_source(m_raw, *p);
}

inline void cairo::set_source(srgb c)
{
    cairo_set_source_rgb(m_raw, c.r, c.g, c.b);
}

inline void cairo::set_source(srgba c)
{
    cairo_set_source_rgba(m_raw, c.r, c.g, c.b, c.a);
}

inline void cairo::set_source(surface& src, vec2_t<f64> origin)
{
    cairo_set_source_surface(m_raw, *src, origin.x, origin.y);
}

inline auto cairo::get_source() -> pattern
{
    return { cairo_pattern_reference(cairo_get_source(m_raw)) };
}

inline void cairo::set_source_filter(filter f)
{
    cairo_pattern_set_filter(cairo_get_source(m_raw), static_cast<cairo_filter_t>(f));
}

inline void cairo::paint()
{
    cairo_paint(m_raw);
}

inline void cairo::fill()
{
    cairo_fill(m_raw);
}

inline void cairo::stroke()
{
    cairo_stroke(m_raw);
}

inline void cairo::save()
{
    cairo_save(m_raw);
}

inline void cairo::restore()
{
    cairo_restore(m_raw);
}

inline void cairo::translate(vec2_t<f64> v)
{
    cairo_translate(m_raw, v.x, v.y);
}

inline void cairo::scale(vec2_t<f64> s)
{
    cairo_scale(m_raw, s.x, s.y);
}

inline void cairo::rotate(f64 angle)
{
    cairo_rotate(m_raw, angle);
}

inline void cairo::move_to(vec2_t<f64> pos)
{
    cairo_move_to(m_raw, pos.x, pos.y);
}

inline void cairo::line_to(vec2_t<f64> pos)
{
    cairo_line_to(m_raw, pos.x, pos.y);
}

inline void cairo::rectangle(vec2_t<f64> origin, vec2_t<f64> size)
{
    cairo_rectangle(m_raw, origin.x, origin.y, size.x, size.y);
}

inline void cairo::arc(vec2_t<f64> center, f64 radius, f64 angle1, f64 angle2)
{
    cairo_arc(m_raw, center.x, center.y, radius, angle1, angle2);
}

inline void cairo::select_font_face(char const* family, font_slant slant, font_weight weight)
{
    auto const s = static_cast<cairo_font_slant_t>(slant);
    auto const w = static_cast<cairo_font_weight_t>(weight);

    cairo_select_font_face(m_raw, family, s, w);
}

inline void cairo::set_font_size(f64 size)
{
    cairo_set_font_size(m_raw, size);
}

inline void cairo::show_text(char const* utf8)
{
    cairo_show_text(m_raw, utf8);
}


inline surface::surface()
    : m_raw{nullptr}
{}

inline surface::surface(cairo_surface_t* raw)
    : m_raw{raw}
{}

inline surface::surface(surface const& other)
    : m_raw{other.m_raw ? cairo_surface_reference(other.m_raw) : nullptr}
{}

inline surface::surface(surface&& other)
    : m_raw{std::exchange(other.m_raw, nullptr)}
{}

inline surface::~surface()
{
    if (m_raw) {
        cairo_surface_destroy(m_raw);
    }
}

inline void surface::operator=(surface const& rhs)
{
    m_raw = rhs.m_raw ? cairo_surface_reference(rhs.m_raw) : nullptr;
}

inline void surface::operator=(surface&& rhs)
{
    m_raw = std::exchange(rhs.m_raw, nullptr);
}

inline auto surface::raw() -> cairo_surface_t*
{
    return m_raw;
}

inline auto surface::operator* () -> cairo_surface_t*
{
    return m_raw;
}

inline auto surface::status() const -> cairo_status_t
{
    return cairo_surface_status(m_raw);
}

inline void surface::write_to_png(char const* filename)
{
    status_t status;

    status = cairo_surface_write_to_png(m_raw, filename);
    if (status) {
        throw exception{ status };
    }
}

inline void surface::write_to_png(std::filesystem::path const& p)
{
    write_to_png(p.c_str());
}


inline pattern::pattern()
    : m_raw{}
{}

inline pattern::pattern(cairo_pattern_t* raw)
    : m_raw{raw}
{}

inline pattern::pattern(pattern const& other)
    : m_raw{other.m_raw ? cairo_pattern_reference(other.m_raw) : nullptr}
{}

inline pattern::pattern(pattern&& other)
    : m_raw{std::exchange(other.m_raw, nullptr)}
{}

inline pattern::~pattern()
{
    if (m_raw) {
        cairo_pattern_destroy(m_raw);
    }
}

inline auto pattern::create_for_surface(surface &surface) -> pattern
{
    return { cairo_pattern_create_for_surface(*surface) };
}

inline void pattern::operator=(pattern const& rhs)
{
    m_raw = rhs.m_raw ? cairo_pattern_reference(rhs.m_raw) : nullptr;
}

inline void pattern::operator=(pattern&& rhs)
{
    m_raw = std::exchange(rhs.m_raw, nullptr);
}

inline auto pattern::raw() -> cairo_pattern_t*
{
    return m_raw;
}

inline auto pattern::operator* () -> cairo_pattern_t*
{
    return m_raw;
}

inline auto pattern::status() const -> status_t
{
    return cairo_pattern_status(m_raw);
}

inline void pattern::set_matrix(matrix &m)
{
    return cairo_pattern_set_matrix(m_raw, *m);
}

inline void pattern::set_filter(filter f)
{
    return cairo_pattern_set_filter(m_raw, static_cast<cairo_filter_t>(f));
}


inline matrix::matrix()
    : m_raw{}
{}

inline matrix::matrix(cairo_matrix_t m)
    : m_raw{m}
{}

inline auto matrix::identity() -> matrix
{
    auto m = matrix{};
    cairo_matrix_init_identity(&m.m_raw);
    return m;
}

inline auto matrix::raw() -> cairo_matrix_t*
{
    return &m_raw;
}

inline auto matrix::operator* () -> cairo_matrix_t*
{
    return &m_raw;
}

inline void matrix::translate(vec2_t<f64> v)
{
    cairo_matrix_translate(&m_raw, v.x, v.y);
}

inline void matrix::scale(vec2_t<f64> s)
{
    cairo_matrix_scale(&m_raw, s.x, s.y);
}


template<class T>
constexpr auto pixel_format() -> format;

template<>
inline constexpr auto pixel_format<srgba>() -> format
{
    return format::rgba128f;
}

template<>
inline constexpr auto pixel_format<srgb>() -> format
{
    return format::rgb96f;
}


inline auto image_surface_create(format fmt, vec2_t<i32> size) -> surface
{
    surface s { cairo_image_surface_create(static_cast<cairo_format_t>(fmt), size.x, size.y) };

    if (s.status() != CAIRO_STATUS_SUCCESS) {
        throw exception{s.status()};
    }

    return s;
}

inline auto format_stride_for_width(format fmt, int width) -> int
{
    int stride = cairo_format_stride_for_width(static_cast<cairo_format_t>(fmt), width);

    if (stride < 0) {
        throw exception{CAIRO_STATUS_INVALID_STRIDE};
    }

    return stride;
}

template<class T>
inline auto image_surface_create(container::image<T>& image) -> surface
{
    auto const format = pixel_format<T>();
    auto const size = image.size();
    auto const data = reinterpret_cast<u8*>(image.data());

    auto const stride = format_stride_for_width(format, size.x);
    auto const cf = static_cast<cairo_format_t>(format);
    auto const ptr = cairo_image_surface_create_for_data(data, cf, size.x, size.y, stride);
    auto const s = surface(ptr);

    if (s.status() != CAIRO_STATUS_SUCCESS) {
        throw exception{s.status()};
    }

    return s;
}

} /* namespace gfx::cairo */
