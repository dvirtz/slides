module;

#include <ranges>
#include <chrono>

export module calendar;

import closure;
import concat;

namespace detail {

template <typename Rng, typename T, int level>
consteval bool recursive_range_of() {
    if constexpr (level == 1)
        if constexpr (std::ranges::range<Rng>)
            return std::same_as<std::ranges::range_value_t<Rng>, T>;
        else
            return false;
    else
        return recursive_range_of<std::ranges::range_value_t<Rng>, T, level - 1>();
}

}  // namespace detail

template <typename Rng, typename T, int extent>
concept nested_range_of = detail::recursive_range_of<Rng, T, extent>();

template <typename Rng, typename T>
concept range_of = nested_range_of<Rng, T, 1>;

using date = std::chrono::year_month_day;

#if defined (__GNUC__) && !defined (__clang__)
using std::views::__adaptor::operator|;
#endif

namespace std {
// make sys_days incrementable

template <typename Clock, typename Duration>
struct incrementable_traits<std::chrono::time_point<Clock, Duration>> {
    using difference_type = typename Duration::rep;
};

#ifdef __clang__

template <typename Clock, typename Duration>
std::chrono::time_point<Clock, Duration> &operator++(
    std::chrono::time_point<Clock, Duration> &tp) {
    tp += Duration{1};
    return tp;
}

template <typename Clock, typename Duration>
std::chrono::time_point<Clock, Duration> operator++(
    std::chrono::time_point<Clock, Duration> &tp, int) {
    auto tmp = tp;
    ++tmp;
    return tmp;
}

#endif

}  // namespace std

namespace detail {

inline auto start_of_week(date d, std::chrono::weekday first_day) {
    const auto diff = std::chrono::weekday{d} - first_day;
    return std::chrono::sys_days{d} - diff;
}
}  // namespace detail

inline auto first_day(std::chrono::year year) {
    using namespace std::literals;
    using std::chrono::January;
    return std::chrono::sys_days{year / January / 1d};
}

export inline auto dates(std::chrono::year start, std::chrono::year stop) {
    return std::views::iota(first_day(start), first_day(stop)) |
           std::views::transform([](const auto day) { return date{day}; });
}

export inline auto dates_from(std::chrono::year year) {
    return std::views::iota(first_day(year)) |
           std::views::transform([](const auto day) { return date{day}; });
}

inline constexpr auto by_month = std::views::chunk_by(
        [](date a, date b) { return a.month() == b.month(); });

inline auto by_week(std::chrono::weekday first_day) {
    return std::views::chunk_by([first_day](date a, date b) {
        return detail::start_of_week(a, first_day) ==
               detail::start_of_week(b, first_day);
    });
}

export inline constexpr closure format_as_string = [](range_of<char> auto &&rng) {
#if __cpp_lib_format_ranges < 202207L
    return std::forward<decltype(rng)>(rng) | std::ranges::to<std::string>();
#else
    return rng;
#endif
};
  
export inline constexpr auto format_week = std::views::transform([](date d){ return std::format("{:3}", d.day()); }) | std::views::join;

// In:  range<range<date>>: month grouped by weeks.
// Out: range<std::string>: month with formatted weeks.
inline constexpr closure format_weeks =
    [](nested_range_of<date, 2> auto month)
{
  return concat(month | std::views::take(1) |
                    std::views::transform([](/* range_of<date> */ auto week)
                                     { return std::format(
                                           "{:>21s} ", week | format_week | format_as_string); }),
                month | std::views::drop(1) |
                    std::views::transform([](/* range_of<date> */ auto week)
                                     { return std::format(
                                           "{:22s}", week | format_week | format_as_string); }));
};

// Return a formatted string with the title of the month
// corresponding to a date.
inline std::string month_title(date d) { return std::format("{:^22%B}", d.month()); }

// In:  range<range<date>>: year of months of days
// Out: range<range<std::string>>: year of months of formatted wks
inline auto layout_months(std::chrono::weekday first_day) {
    return std::views::transform([first_day](range_of<date> auto month) {
        const auto week_count = std::ranges::distance(month | by_week(first_day));
        static const std::string empty_week(22, ' ');
        return concat(std::views::single(month_title(month.front())),
                      month | by_week(first_day) | format_weeks,
                      std::views::repeat(empty_week, 6 - week_count));
    });
}

// In:  range<range<range<string>>>
// Out: range<range<range<string>>>, transposing months.
inline auto transpose_months() {
    return std::views::transform(
        []<nested_range_of<std::string, 2> Rng>(Rng &&rng)
            requires std::ranges::forward_range<Rng>
        {
            auto begin = std::ranges::begin(rng);
#ifdef _MSC_VER
            // https://github.com/microsoft/STL/issues/4414
            return std::views::zip(*begin, *std::ranges::next(begin),
                              *std::ranges::next(begin, 2)) |
                   std::views::transform([](auto &&tuple) {
                       return concat(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple));
                   });
#else
            return std::views::zip_transform(concat, *begin, *std::ranges::next(begin),
                                        *std::ranges::next(begin, 2));
#endif
        });
}

// In:  range<date>
// Out: range<string>, lines of formatted output
export inline auto format_calendar(
    std::chrono::weekday first_day = std::chrono::Sunday) {
    return
        // Group the dates by month:
        by_month
        // Format the month into a range of strings:
        | layout_months(first_day)
        // Group the months that belong side-by-side:
        | std::views::chunk(3)
        // Transpose the rows and columns of the size-by-side months:
        | transpose_months()
        // Ungroup the side-by-side months:
        | std::views::join;
}
