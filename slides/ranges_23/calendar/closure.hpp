#pragma once

#include <ranges>

#ifndef _MSC_VER
namespace std {

constexpr inline auto bind_back(auto &&f, auto &&...args) {
    return [=](auto &&...args2) { return f(args2..., args...); };
}

namespace ranges {

template <typename D>
using range_adaptor_closure = views::__adaptor::_RangeAdaptorClosure;
}

}  // namespace std
#endif

template <typename F>
class closure : public std::ranges::range_adaptor_closure<closure<F>> {
    F f;

   public:
    constexpr closure(F f) : f(f) {}

    template <std::ranges::viewable_range R>
        requires std::invocable<F const &, R>
    constexpr auto operator()(R &&r) const {
        return f(std::forward<R>(r));
    }
};

template <typename F>
class adaptor {
    F f;

   public:
    constexpr adaptor(F f) : f(f) {}

    template <typename... Args>
    constexpr auto operator()(Args &&...args) const {
        if constexpr (std::invocable<F const &, Args...>) {
            return f(std::forward<Args>(args)...);
        } else {
            return closure(std::bind_back(f, std::forward<Args>(args)...));
        }
    }
};
