#pragma once

#include "container/image.hpp"
#include "gfx/cmap.hpp"
#include "gfx/color.hpp"
#include "math/vec2.hpp"

#include <gsl/pointers>

#include <exception>
#include <filesystem>
#include <utility>

#include <cairo/cairo.h>


namespace iptsd::gfx::cairo {

using math::Vec2;

using status_t = cairo_status_t;

class Surface;
class Pattern;
class Matrix;


enum class Format {
    Invalid   = CAIRO_FORMAT_INVALID,
    Argb32    = CAIRO_FORMAT_ARGB32,
    Rgb24     = CAIRO_FORMAT_RGB24,
    A8        = CAIRO_FORMAT_A8,
    A1        = CAIRO_FORMAT_A1,
    Rgb16_565 = CAIRO_FORMAT_RGB16_565,
    Rbg30     = CAIRO_FORMAT_RGB30,
    Rgb96f    = CAIRO_FORMAT_RGB96F,
    Rgba128f  = CAIRO_FORMAT_RGBA128F,
};

enum class Filter {
    Fast     = CAIRO_FILTER_FAST,
    Good     = CAIRO_FILTER_GOOD,
    Best     = CAIRO_FILTER_BEST,
    Nearest  = CAIRO_FILTER_NEAREST,
    Bilinear = CAIRO_FILTER_BILINEAR,
    Gaussian = CAIRO_FILTER_GAUSSIAN,
};

enum class FontSlant {
    Normal  = CAIRO_FONT_SLANT_NORMAL,
    Italic  = CAIRO_FONT_SLANT_ITALIC,
    Oblique = CAIRO_FONT_SLANT_OBLIQUE,
};

enum class FontWeight {
    Normal = CAIRO_FONT_WEIGHT_NORMAL,
    Bold   = CAIRO_FONT_WEIGHT_BOLD,
};


class Exception : public std::exception {
public:
    Exception(status_t code);

    auto what() const noexcept -> const char*;
    auto code() const noexcept -> status_t;

private:
    status_t m_code;
};


class Cairo {
public:
    Cairo();
    Cairo(gsl::owner<cairo_t*> raw);
    Cairo(Cairo const& other);
    Cairo(Cairo&& other);
    ~Cairo();

    static auto create(Surface& Surface) -> Cairo;
    static auto wrap_raw(cairo_t* raw) -> Cairo;

    void operator=(Cairo const& rhs);
    void operator=(Cairo&& rhs);

    auto raw() -> cairo_t*;
    auto operator* () -> cairo_t*;

    auto status() const -> status_t;

    void set_source(Pattern& p);
    void set_source(Srgb rgb);
    void set_source(Srgba rgba);
    void set_source(Surface& src, Vec2<f64> origin);
    auto get_source() -> Pattern;

    void set_source_filter(Filter f);

    void paint();
    void fill();
    void stroke();

    void save();
    void restore();

    void translate(Vec2<f64> s);
    void scale(Vec2<f64> s);
    void rotate(f64 angle);

    void move_to(Vec2<f64> pos);
    void line_to(Vec2<f64> pos);
    void rectangle(Vec2<f64> origin, Vec2<f64> size);
    void arc(Vec2<f64> center, f64 radius, f64 angle1, f64 angle2);

    void select_font_face(char const* family, FontSlant slant, FontWeight weight);
    void set_font_size(f64 size);
    void show_text(char const* utf8);

private:
    gsl::owner<cairo_t*> m_raw;
};


class Surface {
public:
    Surface();
    Surface(gsl::owner<cairo_surface_t*> raw);
    Surface(Surface const& other);
    Surface(Surface&& other);
    ~Surface();

    void operator=(Surface const& rhs);
    void operator=(Surface&& rhs);

    auto raw() -> cairo_surface_t*;
    auto operator* () -> cairo_surface_t*;

    auto status() const -> status_t;

    void write_to_png(char const* filename);
    void write_to_png(std::filesystem::path const& p);

private:
    gsl::owner<cairo_surface_t*> m_raw;
};


class Pattern {
public:
    Pattern();
    Pattern(gsl::owner<cairo_pattern_t*> raw);
    Pattern(Pattern const& other);
    Pattern(Pattern&& other);
    ~Pattern();

    static auto create_for_surface(Surface &Surface) -> Pattern;

    void operator=(Pattern const& rhs);
    void operator=(Pattern&& rhs);

    auto raw() -> cairo_pattern_t*;
    auto operator* () -> cairo_pattern_t*;

    auto status() const -> status_t;

    void set_matrix(Matrix &m);
    void set_filter(Filter f);

private:
    gsl::owner<cairo_pattern_t*> m_raw;
};


class Matrix {
public:
    Matrix();
    Matrix(cairo_matrix_t m);

    static auto identity() -> Matrix;

    auto raw() -> cairo_matrix_t*;
    auto operator* () -> cairo_matrix_t*;

    void translate(Vec2<f64> v);
    void scale(Vec2<f64> v);

private:
    cairo_matrix_t m_raw;
};


inline Exception::Exception(status_t code)
    : m_code{code}
{}

inline auto Exception::what() const noexcept -> const char*
{
    return cairo_status_to_string(m_code);
}

inline auto Exception::code() const noexcept -> status_t
{
    return m_code;
}


inline Cairo::Cairo()
    : m_raw{nullptr}
{}

inline Cairo::Cairo(gsl::owner<cairo_t*> raw)
    : m_raw{raw}
{}

inline Cairo::Cairo(Cairo const& other)
    : m_raw{other.m_raw ? cairo_reference(other.m_raw) : nullptr}
{}

inline Cairo::Cairo(Cairo&& other)
    : m_raw{std::exchange(other.m_raw, nullptr)}
{}

inline Cairo::~Cairo()
{
    if (m_raw) {
        cairo_destroy(m_raw);
    }
}

inline auto Cairo::create(Surface& target) -> Cairo
{
    Cairo cr { cairo_create(target.raw()) };

    if (cr.status() != CAIRO_STATUS_SUCCESS) {
        throw Exception{cr.status()};
    }

    return cr;
}

inline auto Cairo::wrap_raw(cairo_t* raw) -> Cairo
{
    return { cairo_reference(raw) };
}

inline void Cairo::operator=(Cairo const& rhs)
{
    m_raw = rhs.m_raw ? cairo_reference(rhs.m_raw) : nullptr;
}

inline void Cairo::operator=(Cairo&& rhs)
{
    m_raw = std::exchange(rhs.m_raw, nullptr);
}

inline auto Cairo::raw() -> cairo_t*
{
    return m_raw;
}

inline auto Cairo::operator* () -> cairo_t*
{
    return m_raw;
}

inline auto Cairo::status() const -> cairo_status_t
{
    return cairo_status(m_raw);
}

inline void Cairo::set_source(Pattern& p)
{
    cairo_set_source(m_raw, *p);
}

inline void Cairo::set_source(Srgb c)
{
    cairo_set_source_rgb(m_raw, c.r, c.g, c.b);
}

inline void Cairo::set_source(Srgba c)
{
    cairo_set_source_rgba(m_raw, c.r, c.g, c.b, c.a);
}

inline void Cairo::set_source(Surface& src, Vec2<f64> origin)
{
    cairo_set_source_surface(m_raw, *src, origin.x, origin.y);
}

inline auto Cairo::get_source() -> Pattern
{
    return { cairo_pattern_reference(cairo_get_source(m_raw)) };
}

inline void Cairo::set_source_filter(Filter f)
{
    cairo_pattern_set_filter(cairo_get_source(m_raw), static_cast<cairo_filter_t>(f));
}

inline void Cairo::paint()
{
    cairo_paint(m_raw);
}

inline void Cairo::fill()
{
    cairo_fill(m_raw);
}

inline void Cairo::stroke()
{
    cairo_stroke(m_raw);
}

inline void Cairo::save()
{
    cairo_save(m_raw);
}

inline void Cairo::restore()
{
    cairo_restore(m_raw);
}

inline void Cairo::translate(Vec2<f64> v)
{
    cairo_translate(m_raw, v.x, v.y);
}

inline void Cairo::scale(Vec2<f64> s)
{
    cairo_scale(m_raw, s.x, s.y);
}

inline void Cairo::rotate(f64 angle)
{
    cairo_rotate(m_raw, angle);
}

inline void Cairo::move_to(Vec2<f64> pos)
{
    cairo_move_to(m_raw, pos.x, pos.y);
}

inline void Cairo::line_to(Vec2<f64> pos)
{
    cairo_line_to(m_raw, pos.x, pos.y);
}

inline void Cairo::rectangle(Vec2<f64> origin, Vec2<f64> size)
{
    cairo_rectangle(m_raw, origin.x, origin.y, size.x, size.y);
}

inline void Cairo::arc(Vec2<f64> center, f64 radius, f64 angle1, f64 angle2)
{
    cairo_arc(m_raw, center.x, center.y, radius, angle1, angle2);
}

inline void Cairo::select_font_face(char const* family, FontSlant slant, FontWeight weight)
{
    auto const s = static_cast<cairo_font_slant_t>(slant);
    auto const w = static_cast<cairo_font_weight_t>(weight);

    cairo_select_font_face(m_raw, family, s, w);
}

inline void Cairo::set_font_size(f64 size)
{
    cairo_set_font_size(m_raw, size);
}

inline void Cairo::show_text(char const* utf8)
{
    cairo_show_text(m_raw, utf8);
}


inline Surface::Surface()
    : m_raw{nullptr}
{}

inline Surface::Surface(gsl::owner<cairo_surface_t*> raw)
    : m_raw{raw}
{}

inline Surface::Surface(Surface const& other)
    : m_raw{other.m_raw ? cairo_surface_reference(other.m_raw) : nullptr}
{}

inline Surface::Surface(Surface&& other)
    : m_raw{std::exchange(other.m_raw, nullptr)}
{}

inline Surface::~Surface()
{
    if (m_raw) {
        cairo_surface_destroy(m_raw);
    }
}

inline void Surface::operator=(Surface const& rhs)
{
    m_raw = rhs.m_raw ? cairo_surface_reference(rhs.m_raw) : nullptr;
}

inline void Surface::operator=(Surface&& rhs)
{
    m_raw = std::exchange(rhs.m_raw, nullptr);
}

inline auto Surface::raw() -> cairo_surface_t*
{
    return m_raw;
}

inline auto Surface::operator* () -> cairo_surface_t*
{
    return m_raw;
}

inline auto Surface::status() const -> cairo_status_t
{
    return cairo_surface_status(m_raw);
}

inline void Surface::write_to_png(char const* filename)
{
    status_t status;

    status = cairo_surface_write_to_png(m_raw, filename);
    if (status) {
        throw Exception{ status };
    }
}

inline void Surface::write_to_png(std::filesystem::path const& p)
{
    write_to_png(p.c_str());
}


inline Pattern::Pattern()
    : m_raw{}
{}

inline Pattern::Pattern(gsl::owner<cairo_pattern_t*> raw)
    : m_raw{raw}
{}

inline Pattern::Pattern(Pattern const& other)
    : m_raw{other.m_raw ? cairo_pattern_reference(other.m_raw) : nullptr}
{}

inline Pattern::Pattern(Pattern&& other)
    : m_raw{std::exchange(other.m_raw, nullptr)}
{}

inline Pattern::~Pattern()
{
    if (m_raw) {
        cairo_pattern_destroy(m_raw);
    }
}

inline auto Pattern::create_for_surface(Surface &surface) -> Pattern
{
    return { cairo_pattern_create_for_surface(*surface) };
}

inline void Pattern::operator=(Pattern const& rhs)
{
    m_raw = rhs.m_raw ? cairo_pattern_reference(rhs.m_raw) : nullptr;
}

inline void Pattern::operator=(Pattern&& rhs)
{
    m_raw = std::exchange(rhs.m_raw, nullptr);
}

inline auto Pattern::raw() -> cairo_pattern_t*
{
    return m_raw;
}

inline auto Pattern::operator* () -> cairo_pattern_t*
{
    return m_raw;
}

inline auto Pattern::status() const -> status_t
{
    return cairo_pattern_status(m_raw);
}

inline void Pattern::set_matrix(Matrix &m)
{
    return cairo_pattern_set_matrix(m_raw, *m);
}

inline void Pattern::set_filter(Filter f)
{
    return cairo_pattern_set_filter(m_raw, static_cast<cairo_filter_t>(f));
}


inline Matrix::Matrix()
    : m_raw{}
{}

inline Matrix::Matrix(cairo_matrix_t m)
    : m_raw{m}
{}

inline auto Matrix::identity() -> Matrix
{
    auto m = Matrix{};
    cairo_matrix_init_identity(&m.m_raw);
    return m;
}

inline auto Matrix::raw() -> cairo_matrix_t*
{
    return &m_raw;
}

inline auto Matrix::operator* () -> cairo_matrix_t*
{
    return &m_raw;
}

inline void Matrix::translate(Vec2<f64> v)
{
    cairo_matrix_translate(&m_raw, v.x, v.y);
}

inline void Matrix::scale(Vec2<f64> s)
{
    cairo_matrix_scale(&m_raw, s.x, s.y);
}


template<class T>
constexpr auto pixel_format() -> Format;

template<>
inline constexpr auto pixel_format<Srgba>() -> Format
{
    return Format::Rgba128f;
}

template<>
inline constexpr auto pixel_format<Srgb>() -> Format
{
    return Format::Rgb96f;
}


inline auto image_surface_create(Format fmt, Vec2<i32> size) -> Surface
{
    Surface s { cairo_image_surface_create(static_cast<cairo_format_t>(fmt), size.x, size.y) };

    if (s.status() != CAIRO_STATUS_SUCCESS) {
        throw Exception{s.status()};
    }

    return s;
}

inline auto format_stride_for_width(Format fmt, int width) -> int
{
    int stride = cairo_format_stride_for_width(static_cast<cairo_format_t>(fmt), width);

    if (stride < 0) {
        throw Exception{CAIRO_STATUS_INVALID_STRIDE};
    }

    return stride;
}

template<class T>
inline auto image_surface_create(Image<T>& image) -> Surface
{
    auto const format = pixel_format<T>();
    auto const size = image.size();
    auto const data = reinterpret_cast<u8*>(image.data());

    auto const stride = format_stride_for_width(format, size.x);
    auto const cf = static_cast<cairo_format_t>(format);
    auto const ptr = cairo_image_surface_create_for_data(data, cf, size.x, size.y, stride);
    auto const s = Surface(ptr);

    if (s.status() != CAIRO_STATUS_SUCCESS) {
        throw Exception{s.status()};
    }

    return s;
}

} /* namespace iptsd::gfx::cairo */
