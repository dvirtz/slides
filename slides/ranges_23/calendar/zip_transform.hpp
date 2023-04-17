#pragma once

#include <ranges>

#if __cpp_lib_ranges_zip < 202110L

namespace std::ranges::views
{
  inline constexpr auto zip_transform = []<typename F, std::ranges::range... Rngs>(F f, Rngs&& ...rngs)
  {
    return zip(std::forward<Rngs>(rngs)...) | transform([f = std::move(f)]<typename Tuple>(Tuple &&tuple)
                                                             { return std::apply(f, std::forward<Tuple>(tuple)); });
  };
}

#endif