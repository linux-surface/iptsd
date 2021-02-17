#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <cmath>


namespace eval::perf {

using clock = std::chrono::high_resolution_clock;


class token {
private:
    friend class registry;

public:
    inline constexpr token(token const& other) = default;

    inline constexpr auto operator= (token const& rhs) -> token& = default;

private:
    inline constexpr token(std::size_t index_t);

private:
    std::size_t m_index;
};


class entry {
public:
    entry(std::string name);

    template<class D>
    auto total() const -> D;

    template<class D>
    auto min() const -> D;

    template<class D>
    auto max() const -> D;

    template<class D>
    auto mean() const -> D;

    template<class D>
    auto var() const -> D;

    template<class D>
    auto stddev() const -> D;

public:
    std::string name;

    unsigned int n_measurements;
    clock::duration duration;
    clock::duration minimum;
    clock::duration maximum;

    double r_mean_ns;
    double r_var_ns;
};


class measurement {
public:
    ~measurement();
    void stop();

private:
    friend class registry;

    measurement(entry& e, clock::time_point start);

private:
    entry& m_entry;
    clock::time_point m_start;
};


class registry {
public:
    auto create_entry(std::string name) -> token;

    auto record(token const& t) -> measurement;
    auto get_entry(token const& t) const -> entry const&;

    auto entries() const -> std::vector<entry> const&;

private:
    std::vector<entry> m_entries;
};


inline constexpr token::token(std::size_t i)
    : m_index{i}
{}


inline entry::entry(std::string name)
    : name{std::move(name)}
    , n_measurements{0}
    , duration{0}
    , minimum{clock::duration::max()}
    , maximum{0}
    , r_mean_ns{0.0f}
    , r_var_ns{0.0f}
{}

template<class D>
inline auto entry::total() const -> D
{
    return std::chrono::duration_cast<D>(this->duration);
}

template<class D>
inline auto entry::min() const -> D
{
    return std::chrono::duration_cast<D>(this->minimum);
}

template<class D>
inline auto entry::max() const -> D
{
    return std::chrono::duration_cast<D>(this->maximum);
}

template<class D>
inline auto entry::mean() const -> D
{
    auto const m = static_cast<typename D::rep>(std::round(r_mean_ns));

    return std::chrono::duration_cast<D>(std::chrono::nanoseconds(m));
}

template<class D>
inline auto entry::var() const -> D
{
    auto const v = n_measurements > 1 ?  r_var_ns / (n_measurements - 1) : 0.0;
    auto const d = static_cast<typename D::rep>(std::round(v));

    return std::chrono::duration_cast<D>(std::chrono::nanoseconds(d));
}

template<class D>
inline auto entry::stddev() const -> D
{
    auto const v = n_measurements > 1 ?  r_var_ns / (n_measurements - 1) : 0.0;
    auto const d = static_cast<typename D::rep>(std::round(std::sqrt(v)));

    return std::chrono::duration_cast<D>(std::chrono::nanoseconds(d));
}


inline measurement::measurement(entry& e, clock::time_point start)
    : m_entry{e}
    , m_start{start}
{}

inline measurement::~measurement()
{
    stop();
}

inline void measurement::stop()
{
    using ns = std::chrono::nanoseconds;

    auto const duration = clock::now() - m_start;

    if (m_start == clock::time_point::max())
        return;

    auto const d_ns = static_cast<double>(std::chrono::duration_cast<ns>(duration).count());

    if (m_entry.n_measurements == 0)
        m_entry.r_mean_ns = d_ns;

    m_entry.n_measurements += 1;
    m_entry.duration += duration;
    m_entry.minimum = std::min(m_entry.minimum, duration);
    m_entry.maximum = std::max(m_entry.maximum, duration);

    double const r_mean_old = m_entry.r_mean_ns;
    double const r_var_old = m_entry.r_var_ns;

    double const r_mean_new = r_mean_old + (d_ns - r_mean_old) / m_entry.n_measurements;
    double const r_var_new = r_var_old + (d_ns - r_mean_old) * (d_ns - r_mean_new);

    m_entry.r_mean_ns = r_mean_new;
    m_entry.r_var_ns = r_var_new;

    m_start = clock::time_point::max();
}


inline auto registry::create_entry(std::string name) -> token
{
    m_entries.emplace_back(std::move(name));
    return token { m_entries.size() - 1 };
}

inline auto registry::record(token const& t) -> measurement
{
    return measurement { m_entries[t.m_index], clock::now() };
}

inline auto registry::get_entry(token const& t) const -> entry const&
{
    return m_entries[t.m_index];
}

inline auto registry::entries() const -> std::vector<entry> const&
{
    return m_entries;
}

} /* namespace eval::perf */
