#ifndef __STL_FUNCTION_H
#define __STL_FUNCTION_H

#include <functional>

template<typename T>
struct identity {
    constexpr T&& operator()(T&& x) const noexcept {
        return std::forward<T>(x);
    }

    constexpr const T& operator()(const T& x) const noexcept {
        return x;
    }
};

#endif
