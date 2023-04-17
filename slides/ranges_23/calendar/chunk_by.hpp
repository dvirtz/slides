#pragma once

#include <ranges>

#include "closure.hpp"

template <typename Pred>
auto chunk_by(Pred&& pred) {
#ifdef _MSC_VER
    return std::ranges::views::chunk_by(std::forward<Pred>(pred));
#else
    return closure{
        [not_pred =
             std::not_fn(std::forward<Pred>(pred))]<typename Rng>(Rng rng)
            -> std::generator<std::ranges::subrange<
                std::ranges::iterator_t<Rng>, std::ranges::iterator_t<Rng> > > {
            auto end = std::ranges::end(rng);
            for (auto
                     it = std::ranges::begin(rng),
                     next = std::ranges::next(
                         std::ranges::adjacent_find(it, end, [](auto, auto){ return false; }), 1, end);
                 it != end; it = next) {
                co_yield std::ranges::subrange{it, next};
            }
        }};
#endif
}
