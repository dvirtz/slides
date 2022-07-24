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
#include <numeric>

// Computes a digest of data. It is used both to prevent compiler from
// optimizing away the benchmarked code and to verify that the results are
// correct. The overhead is less than 2.5% compared to just DoNotOptimize.
inline unsigned compute_digest(int data) {
  unsigned digest = 0;
  data = std::abs(data);
  do {
    digest += data % 10;
  } while (data /= 10);
  return digest;
}

struct Data {
  std::vector<std::string> values;
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
        0, RAND_MAX);
    std::generate(values.begin(), values.end(), [&]() {
      int scale = dist(gen) / 100 + 1;
      return fmt::format("{}", static_cast<int>(dist(gen) * dist(gen)) / scale);
    });
    digest =
        std::accumulate(begin(), end(), unsigned(), [](unsigned lhs, const std::string& rhs) {
          return lhs + compute_digest(std::stoi(rhs));
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

  void add(int i) { digest += compute_digest(i); }
};

namespace imp {

struct {
int operator()(const char* str, size_t /*len*/) {
  return std::atoi(str);
}
} atoi;

struct {
int operator()(const char* str, size_t /*len*/) {
  return static_cast<int>(std::strtol(str, nullptr, 10));
}
} strtol;

struct {
int operator()(const char* str, size_t /*len*/) {
  int res;
  std::sscanf(str, "%d", &res);
  return res;
}
} sscanf;

struct {
int operator()(const char* str, size_t len) {
  std::istringstream in{{str, len}};
  int res;
  in >> res;
  return res;
}
} istringstream;

struct {
int operator()(const char* str, size_t len) {
  auto read = []<typename It>(It from, It to, long& d) {
    std::ios_base::iostate err = std::ios_base::goodbit;
    std::istringstream sst;
    using Facet = std::num_get<char, It>;
    static std::locale loc{std::locale::classic(), new Facet};
    std::use_facet<Facet>(loc).get(from, to, sst, err, d);
  };

  long res;
  read(str, str + len, res);
  return static_cast<int>(res);
}
} num_get;

struct {
int operator()(const char* str, size_t len) {
  return std::stoi({str, len});
}
} stoi;

#if __has_include(<charconv>)
struct {
int operator()(const char* str, size_t len) {
  int res;
  std::from_chars(str, str + len, res);
  return res;
}
} from_chars;
#endif

struct {
int operator()(const char* str, size_t len) {
  int res;
  scn::scan(std::string_view{str, len}, "{}", res);
  return res;
}
} scan;

}

template<typename F>
void FromString(benchmark::State& state, F f) {
  auto dc = DigestChecker(state);
  for (auto s : state) {
    for (auto value : data) {
      dc.add(f(value.c_str(), value.size()));
    }
  }
}

#define BENCHMAKR_ATOI(Func) BENCHMARK_CAPTURE(FromString, Func, imp::Func)->Name(#Func)

BENCHMAKR_ATOI(atoi);
BENCHMAKR_ATOI(strtol);
BENCHMAKR_ATOI(sscanf); 
BENCHMAKR_ATOI(istringstream);
BENCHMAKR_ATOI(num_get);
BENCHMAKR_ATOI(stoi);
#if __has_include(<charconv>)
BENCHMAKR_ATOI(from_chars);
#endif
BENCHMAKR_ATOI(scan);

static void Noop(benchmark::State& state) {
  for (auto _ : state) benchmark::DoNotOptimize(0);
}
BENCHMARK(Noop);
BENCHMARK_MAIN();