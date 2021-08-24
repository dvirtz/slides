#include <benchmark/benchmark.h>
#include <fmt/format.h>
#include <scn/scn.h>

#if __has_include(<charconv>)
#include <charconv>
#endif
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
#include <random>

// Computes a digest of data. It is used both to prevent compiler from
// optimizing away the benchmarked code and to verify that the results are
// correct. The overhead is less than 2.5% compared to just DoNotOptimize.
inline unsigned compute_digest(std::string_view data) {
  unsigned digest = 0;
  for (char c : data) digest += c;
  return digest;
}

struct Data {
  std::vector<int> values;
  unsigned digest;

  auto begin() const { return values.begin(); }
  auto end() const { return values.end(); }

  Data() : values(1'000'000) {
    // Similar data as in Boost Karma int generator test:
    // https://www.boost.org/doc/libs/1_63_0/libs/spirit/workbench/karma/real_generator.cpp
    // with rand replaced by uniform_int_distribution for consistent results
    // across platforms.
    std::mt19937 gen;
    std::uniform_int_distribution<int> dist(
        -RAND_MAX, RAND_MAX);
    std::generate(values.begin(), values.end(), [&]() {
      int scale = dist(gen) / 100 + 1;
      return static_cast<int>(dist(gen) * dist(gen)) / scale;
    });
    digest =
        std::accumulate(begin(), end(), unsigned(), [](unsigned lhs, int rhs) {
          return lhs + compute_digest(fmt::format(fmt::runtime("{}"), rhs));
        });
  }
} data;

struct DigestChecker {
  benchmark::State& state;
  unsigned digest = 0;

  explicit DigestChecker(benchmark::State& s) : state(s) {}

  ~DigestChecker() noexcept(false) {
    if (digest != static_cast<unsigned>(state.iterations()) * data.digest)
      throw std::logic_error("invalid length");
    state.SetItemsProcessed(state.iterations() * data.values.size());
    benchmark::DoNotOptimize(digest);
  }

  void add(std::string_view s) { digest += compute_digest(s); }
};

namespace imp {

namespace detail {

void reverse(char s[])
{
    int c, i, j;

    for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char s[]) /* convert `s` to integer */
{
  int i, sign;
  
  if ((sign = n) < 0) /* record sign */
    n = -n;           /* make n positive */
  i = 0;
  do {    /* generate digits in reverse order */
    s[i++] = n % 10 + '0';  /* get next digit */
  } while ((n /= 10) > 0); /* delete it */
  if (sign < 0)
    s[i++] = '-';
  s[i] = '\0';
  reverse(s);
}
  
}

struct {
  template<size_t N>
size_t operator()(int d, char(&result)[N]) {
  detail::itoa(d, result);
  return strlen(result);
}
} itoa;

struct {
template<size_t N>
size_t operator()(int d, char(&result)[N]) {
  return std::sprintf(result, "%d", d);
}
} sprintf;

struct {
template<size_t N>
size_t operator()(int d, char(&result)[N]) {
  std::ostringstream out;
  out << d;
  return out.str().copy(result, N);
}
} ostringstream;

struct {
template<size_t N>
size_t operator()(int d, char(&result)[N]) {
  std::ostringstream sst;
  using Facet = std::num_put<char, char*>;
  static std::locale loc{std::locale::classic(), new Facet};
  auto end = std::use_facet<Facet>(loc).put(result, sst, sst.fill(), static_cast<long>(d));
  return end-result;
}
} num_put;

struct {
template<size_t N>
size_t operator()(int d, char(&result)[N]) {
  return std::to_string(d).copy(result, N);
}
} to_string;

#if __has_include(<charconv>)
struct {
template<size_t N>
size_t operator()(int d, char(&result)[N]) {
  const auto [end, _] =
      std::to_chars(std::begin(result), std::end(result), d);
  return end - result;
}
} to_chars;
#endif

struct {
template<size_t N>
size_t operator()(int d, char(&result)[N]) {
  auto end = fmt::format_to(result, "{}", d);
  return end - result;
}
} format;

}

template<typename F>
void ToString(benchmark::State& state, F f) {
  auto dc = DigestChecker(state);
  for (auto s : state) {
    for (auto value : data) {
      const int n = std::numeric_limits<double>::digits10;
      char buf[n];
      auto size = f(value, buf);
      dc.add({buf, size});
    }
  }
}

#define BENCHMARK_RANDOM(Func) BENCHMARK_CAPTURE(ToString, Func, imp::Func)->Name(#Func)

BENCHMARK_RANDOM(itoa);
BENCHMARK_RANDOM(sprintf); 
BENCHMARK_RANDOM(ostringstream);
BENCHMARK_RANDOM(num_put);
BENCHMARK_RANDOM(to_string);
#if __has_include(<charconv>)
BENCHMARK_RANDOM(to_chars);
#endif
BENCHMARK_RANDOM(format);

static void Noop(benchmark::State& state) {
  for (auto _ : state) benchmark::DoNotOptimize(0);
}
BENCHMARK(Noop);
BENCHMARK_MAIN()`;