#pragma once

#if __cpp_lib_format >= 202207L && __cpp_lib_format_ranges >= 202207L
#include <format>
#else

#include <fmt/format.h>
#include <fmt/ranges.h>

namespace std
{
  using fmt::print;
#if FMT_VERSION > 90100
  using fmt::println;
#else
  template<typename... Args>
  auto println(fmt::format_string<Args...> fmt, Args&&... args) {
    print(fmt, std::forward<Args>(args)...);
    print("\n");
  }
  
  template<typename... Args>
  auto println(std::FILE* out, fmt::format_string<Args...> fmt, Args&&... args) {
    print(out, fmt, std::forward<Args>(args)...);
    print("\n");
  }
#endif
} // namespace std

#endif // __cpp_lib_format

