#include <source_location>
#include <expected>
#include <string_view>
#include <string>
#include <format>
#include <print>
#include <charconv>
#include <chrono>
#include <ranges>
#include <range/v3/view/concat.hpp>

import calendar;

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
            std::println("{:s}", calendar | std::views::join_with('\n') | format_as_string);
            return std::optional<int>{42};
        })
        .or_else([&]() { 
            auto calendar = dates_from(start) | format_calendar();
            // can't print_as_string an inifinite range
            for (auto&& line : calendar) {
                std::println("{:s}", std::forward<decltype(line)>(line) | format_as_string);
            }
            return std::optional<int>{42};
        });

  // clang-format on
}
///[calendar]
