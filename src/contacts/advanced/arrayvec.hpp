#pragma once
#include <stdexcept>

template <typename T, size_t SIZE>
class ArrayVec {
    T impl[SIZE];
    size_t len{};

// Not bothering to destroy objects when removing
public:
    using value_type = T;

    T* data() {
        return impl;
    }
    void push_back(const T& val) {
        impl[len++] = val;
    }
    void push_back(T&& val) {
        impl[len++] = val;
    }
    T* begin() {
        return impl;
    }
    T* end() {
        return impl + len;
    }
    T& at(size_t i) {
        return impl[i];
    }
    T& operator[](size_t i) {
        return impl[i];
    }
    void clear() {
        len = 0;
    }
    void reserve(size_t n) {
        if (n > SIZE) throw std::runtime_error("arrayvec reserve");
    }
    size_t size() {
        return len;
    }
    bool empty() {
        return !len;
    }
    void assign(size_t count, const T& x) {
        reserve(count);
        for (size_t i = 0; i < count; ++i) {
            impl[i] = x;
        }
    }
    void resize(size_t count, const T& x) {
        for (; len < count; ++len) {
            impl[len] = x;
        }
    }
};
