#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <cmath>


namespace iptsd::eval::perf {

using clock = std::chrono::high_resolution_clock;


class Token {
private:
    friend class Registry;

public:
    inline constexpr Token(Token const& other) = default;

    inline constexpr auto operator= (Token const& rhs) -> Token& = default;

private:
    inline constexpr Token(std::size_t index_t);

private:
    std::size_t m_index;
};


class Entry {
public:
    Entry(std::string name);

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
    friend class Registry;

    measurement(Entry& e, clock::time_point start);

private:
    Entry& m_entry;
    clock::time_point m_start;
};


class Registry {
public:
    auto create_entry(std::string name) -> Token;

    auto record(Token const& t) -> measurement;
    auto get_entry(Token const& t) const -> Entry const&;

    auto entries() const -> std::vector<Entry> const&;

private:
    std::vector<Entry> m_entries;
};


inline constexpr Token::Token(std::size_t i)
    : m_index{i}
{}


inline Entry::Entry(std::string name)
    : name{std::move(name)}
    , n_measurements{0}
    , duration{0}
    , minimum{clock::duration::max()}
    , maximum{0}
    , r_mean_ns{0.0f}
    , r_var_ns{0.0f}
{}

template<class D>
inline auto Entry::total() const -> D
{
    return std::chrono::duration_cast<D>(this->duration);
}

template<class D>
inline auto Entry::min() const -> D
{
    return std::chrono::duration_cast<D>(this->minimum);
}

template<class D>
inline auto Entry::max() const -> D
{
    return std::chrono::duration_cast<D>(this->maximum);
}

template<class D>
inline auto Entry::mean() const -> D
{
    auto const m = static_cast<typename D::rep>(std::round(r_mean_ns));

    return std::chrono::duration_cast<D>(std::chrono::nanoseconds(m));
}

template<class D>
inline auto Entry::var() const -> D
{
    auto const v = n_measurements > 1 ?  r_var_ns / (n_measurements - 1) : 0.0;
    auto const d = static_cast<typename D::rep>(std::round(v));

    return std::chrono::duration_cast<D>(std::chrono::nanoseconds(d));
}

template<class D>
inline auto Entry::stddev() const -> D
{
    auto const v = n_measurements > 1 ?  r_var_ns / (n_measurements - 1) : 0.0;
    auto const d = static_cast<typename D::rep>(std::round(std::sqrt(v)));

    return std::chrono::duration_cast<D>(std::chrono::nanoseconds(d));
}


inline measurement::measurement(Entry& e, clock::time_point start)
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


inline auto Registry::create_entry(std::string name) -> Token
{
    m_entries.emplace_back(std::move(name));
    return Token { m_entries.size() - 1 };
}

inline auto Registry::record(Token const& t) -> measurement
{
    return measurement { m_entries[t.m_index], clock::now() };
}

inline auto Registry::get_entry(Token const& t) const -> Entry const&
{
    return m_entries[t.m_index];
}

inline auto Registry::entries() const -> std::vector<Entry> const&
{
    return m_entries;
}

} /* namespace iptsd::eval::perf */
