#pragma once

#include <ranges>
#include "generator.hpp"

namespace detail {

template <std::ranges::range Head, std::ranges::range... Tail>
inline std::generator<
    std::common_type_t<std::ranges::range_reference_t<Head>,
                       std::ranges::range_reference_t<Tail>...>,
    std::common_type_t<std::ranges::range_value_t<Head>,
                       std::ranges::range_value_t<Tail>...>
    >
concat(Head head, Tail... tail) {
    co_yield std::ranges::elements_of(std::move(head));
    if constexpr (sizeof...(tail) > 0) {
        co_yield std::ranges::elements_of(concat(std::move(tail)...));
    }
};

}  // namespace detail

inline constexpr auto concat = []<std::ranges::range... Rng>(Rng&&... rng) {
    return detail::concat(std::forward<Rng>(rng)...);
};
