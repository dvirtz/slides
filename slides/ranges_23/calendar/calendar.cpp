// Range v3 library
//
//  Copyright Eric Niebler 2013-present
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//

///[calendar]

// Usage:
//     calendar 2023
//
// Output:
/*
       January               February               March
 01 02 03 04 05 06 07           01 02 03 04           01 02 03 04
 08 09 10 11 12 13 14  05 06 07 08 09 10 11  05 06 07 08 09 10 11
 15 16 17 18 19 20 21  12 13 14 15 16 17 18  12 13 14 15 16 17 18
 22 23 24 25 26 27 28  19 20 21 22 23 24 25  19 20 21 22 23 24 25
             29 30 31              26 27 28     26 27 28 29 30 31

        April                  May                   June
                   01     01 02 03 04 05 06              01 02 03
 02 03 04 05 06 07 08  07 08 09 10 11 12 13  04 05 06 07 08 09 10
 09 10 11 12 13 14 15  14 15 16 17 18 19 20  11 12 13 14 15 16 17
 16 17 18 19 20 21 22  21 22 23 24 25 26 27  18 19 20 21 22 23 24
 23 24 25 26 27 28 29           28 29 30 31     25 26 27 28 29 30
                   30
         July                 August              September
                   01        01 02 03 04 05                 01 02
 02 03 04 05 06 07 08  06 07 08 09 10 11 12  03 04 05 06 07 08 09
 09 10 11 12 13 14 15  13 14 15 16 17 18 19  10 11 12 13 14 15 16
 16 17 18 19 20 21 22  20 21 22 23 24 25 26  17 18 19 20 21 22 23
 23 24 25 26 27 28 29        27 28 29 30 31  24 25 26 27 28 29 30
                30 31
       October               November              December
 01 02 03 04 05 06 07           01 02 03 04                 01 02
 08 09 10 11 12 13 14  05 06 07 08 09 10 11  03 04 05 06 07 08 09
 15 16 17 18 19 20 21  12 13 14 15 16 17 18  10 11 12 13 14 15 16
 22 23 24 25 26 27 28  19 20 21 22 23 24 25  17 18 19 20 21 22 23
             29 30 31        26 27 28 29 30  24 25 26 27 28 29 30
                                                               31
// */

// Credits:
//   Thanks to H. S. Teoh for the article that served as the
//     inspiration for this example:
//     <http://wiki.dlang.org/Component_programming_with_ranges>
//   Thanks to github's Arzar for bringing date::week_number
//     to my attention.

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <format>
#include <functional>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <utility>
#include <array>
#include <source_location>

#include "expected.hpp"
#include "closure.hpp"
#include "concat.hpp"
#include "print.hpp"
#include "repeat.hpp"
#include "zip_transform.hpp"

namespace ranges = std::ranges;
namespace views = std::views;

namespace detail {

template <typename Rng, typename T, int level>
consteval bool recursive_range_of() {
    if constexpr (level == 1)
        if constexpr (ranges::range<Rng>)
            return std::same_as<ranges::range_value_t<Rng>, T>;
        else
            return false;
    else
        return recursive_range_of<ranges::range_value_t<Rng>, T, level - 1>();
}

}  // namespace detail

template <typename Rng, typename T, int extent>
concept nested_range_of = detail::recursive_range_of<Rng, T, extent>();

template <typename Rng, typename T>
concept range_of = nested_range_of<Rng, T, 1>;

using date = std::chrono::year_month_day;

namespace std {
// make sys_days incrementable

template <typename Clock, typename Duration>
struct incrementable_traits<std::chrono::time_point<Clock, Duration>> {
    using difference_type = typename Duration::rep;
};

#ifdef __clang__

template <typename Clock, typename Duration>
std::chrono::time_point<Clock, Duration>& operator++(
    std::chrono::time_point<Clock, Duration>& tp) {
    tp += Duration{1};
    return tp;
}

template <typename Clock, typename Duration>
std::chrono::time_point<Clock, Duration> operator++(
    std::chrono::time_point<Clock, Duration>& tp, int) {
    auto tmp = tp;
    ++tmp;
    return tmp;
}

#endif

}  // namespace std

namespace detail {
template <typename Rng>
auto format_as_string(const std::string_view fmt, Rng&& rng) {
  return std::vformat(fmt, std::make_format_args(
#if __cpp_lib_format_ranges < 202207L
    std::forward<Rng>(rng) | ranges::to<std::string>()
#else
  std::forward<Rng>(rng)
#endif
  ));
}

template <typename Rng>
auto print_as_string(Rng&& rng) {
  return std::println("{:s}",
#if __cpp_lib_format_ranges < 202207L
                      std::forward<Rng>(rng) | ranges::to<std::string>()
#else
                      std::forward<Rng>(rng)
#endif
  );
}

auto start_of_week(date d) {
  const auto from_sunday = std::chrono::weekday{d} - std::chrono::Sunday;
  return std::chrono::sys_days{d} - from_sunday;
}
}  // namespace detail

auto first_day(std::chrono::year year) {
  using namespace std::literals;
  using std::chrono::January;
  return std::chrono::sys_days{year / January / 1d};
}

auto dates(std::chrono::year start, std::chrono::year stop) {
  return views::iota(first_day(start), first_day(stop)) |
         views::transform([](const auto day) { return date{day}; });
}

auto dates_from(std::chrono::year year) {
  return views::iota(first_day(year)) |
         views::transform([](const auto day) { return date{day}; });
}

auto by_month() {
  return views::chunk_by([](date a, date b) { return a.month() == b.month(); });
}

auto by_week() {
  return views::chunk_by([](date a, date b) {
      return detail::start_of_week(a) == detail::start_of_week(b);
  });
}

// In:  range<range<date>>: month grouped by weeks.
// Out: range<std::string>: month with formatted weeks.
inline constexpr closure format_weeks =
    [](nested_range_of<date, 2> auto month) {
        const auto format_day = [](date d) {
            return std::format("{:3}", d.day());
        };
        const auto format_week = views::transform(format_day) | views::join;
        return concat(month | views::take(1) |
                          views::transform([&](range_of<date> auto week) {
                              return detail::format_as_string(
                                  "{:>21s} ", week | format_week);
                          }),
                      month | views::drop(1) |
                          views::transform([&](range_of<date> auto week) {
                              return detail::format_as_string(
                                  "{:22s}", week | format_week);
                          }));
    };

// Return a formatted string with the title of the month
// corresponding to a date.
std::string month_title(date d) { return std::format("{:^22%B}", d.month()); }

// In:  range<range<date>>: year of months of days
// Out: range<range<std::string>>: year of months of formatted wks
auto layout_months() {
  return views::transform([](range_of<date> auto month) {
      const auto week_count = ranges::distance(month | by_week());
      static const std::string empty_week(22, ' ');
      return concat(views::single(month_title(month.front())),
                    month | by_week() | format_weeks,
                    views::repeat(empty_week, 6 - week_count));
  });
}

// In:  range<range<range<string>>>
// Out: range<range<range<string>>>, transposing months.
auto transpose_months() {
  return views::transform([]<nested_range_of<std::string, 2> Rng>(Rng&& rng)
                              requires ranges::forward_range<Rng>
                          {
                              const auto begin = ranges::begin(rng);
                              return views::zip_transform(
                                  concat, *begin, *ranges::next(begin),
                                  *ranges::next(begin, 2));
                          });
}

// In:  range<date>
// Out: range<string>, lines of formatted output
auto format_calendar() {
  return
      // Group the dates by month:
      by_month()
      // Format the month into a range of strings:
      | layout_months()
      // Group the months that belong side-by-side:
      | views::chunk(3)
      // Transpose the rows and columns of the size-by-side months:
      | transpose_months()
      // Ungroup the side-by-side months:
      | views::join;
}

class Error {
 public:
  Error(std::string_view message,
        std::source_location location = std::source_location::current())
      : message_{std::format("{}:{} {}", location.file_name(), location.line(),
                             message)} {}

  std::string_view message() const { return message_; }

 private:
  std::string message_;
};

std::unexpected<Error> unexpected(
    std::string_view message,
    std::source_location location = std::source_location::current()) {
  return std::unexpected(Error{message, location});
}

template <typename T>
using expected = std::expected<T, Error>;

template <std::integral T = int>
constexpr inline expected<T> convert(const std::string_view str) {
  T t;
  const auto [_, ec] = std::from_chars(str.data(), str.data() + str.size(), t);
  if (ec == std::errc{}) {
      return t;
  }

  return unexpected(std::error_condition{ec}.message());
};

int main(int argc, char* argv[]) {
  using std::chrono::year;
  using std::chrono::years;
  using namespace std::literals;

  const auto usage = [exe = argv[0]](const Error& error) {
      std::println(stderr, "{}", error.message());
      std::println(stderr, "usage: {} start [stop|never] [per-line]", exe);
      exit(1);
  };
  const auto arg = [&](int index, std::optional<std::string> default_value =
                                      std::nullopt) -> expected<std::string> {
      if (index < argc) {
          return argv[index];
      }

      if (default_value) {
          return *std::move(default_value);
      }

      return unexpected(std::format("index {} out of bounds", index));
  };
  const auto to_year = [](std::string_view str) {
      return convert(str).transform([](int i) { return year{i}; });
  };

  // clang-format off

    const auto start = 
        arg(1)
        .and_then(to_year)
        .or_else([&](const Error& error) {
            usage(error);
            return expected<year>{};
        })
        .value();
    const auto maybe_stop = 
        [&]() -> expected<std::optional<year>> {
            if (argc < 3) {
                return start + years{1};
            }

            return arg(2)
                .and_then([&](std::string_view str) -> expected<std::optional<year>> {
                    if (str == "never"sv) {
                        return std::nullopt;
                    }

                    return to_year(str)
                        .and_then([&](year stop) -> expected<std::optional<year>> {
                            if (start <= stop)
                                return stop;
                                
                            return unexpected("The stop year must not be smaller than the start");
                        })
                        .transform_error([](const Error&) {
                            return Error{"stop can be either a year or \"never\""};
                        });
                });
        }()
        .or_else([&](const Error& error) {
            usage(error);
            return expected<std::optional<year>>{};
        })
        .value();

    maybe_stop
        .and_then([&](year stop) {
            auto calendar = dates(start, stop) | format_calendar();
            detail::print_as_string(calendar | views::join_with('\n'));
            return std::optional<int>{42};
        })
        .or_else([&]() { 
            auto calendar = dates_from(start) | format_calendar();
            // can't print_as_string an inifinite range
            for (auto&& line : calendar) {
                detail::print_as_string(std::forward<decltype(line)>(line));
            }
            return std::optional<int>{42};
        });

  // clang-format on
}
///[calendar]
