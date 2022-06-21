#include <benchmark/benchmark.h>
#include <random>
#include <algorithm>
#include <fmt/format.h>
#include <scn/scn.h>

#if __has_include(<charconv>)
#define HAS_X_CHARS
#include <charconv>
#endif
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string_view>

namespace imp {

struct {
double operator()(const char* str, size_t /*len*/) {
  return std::atof(str);
}
} atof;

struct {
double operator()(const char* str, size_t /*len*/) {
  return std::strtod(str, nullptr);
}
} strtod;

struct {
double operator()(const char* str, size_t /*len*/) {
  double res;
  std::sscanf(str, "%lf", &res);
  return res;
}
} sscanf;

struct {
double operator()(const char* str, size_t len) {
  std::istringstream in{{str, len}};
  double res;
  in >> res;
  return res;
}
} istringstream;

struct {
double operator()(const char* str, size_t len) {
  auto read = []<typename It>(It from, It to, double& d) {
    std::ios_base::iostate err = std::ios_base::goodbit;
    std::istringstream sst;
    using Facet = std::num_get<char, It>;
    std::locale loc{sst.getloc(), new Facet};
    std::use_facet<Facet>(loc).get(from, to, sst, err, d);
  };

  double res;
  read(str, str + len, res);
  return res;
}
} num_get;

struct {
double operator()(const char* str, size_t /*len*/) {
  return std::stod(str);
}
} stod;

#if __has_include(<charconv>)
struct {
double operator()(const char* str, size_t len) {
  double res;
  std::from_chars(str, str + len, res);
  return res;
}
} from_chars;
#endif

struct {
double operator()(const char* str, size_t len) {
  double res;
  scn::scan(std::string_view{str, len}, "{}", res);
  return res;
}
} scan;

}

const unsigned kVerifyRandomCount = 100000;
const unsigned kIterationForRandom = 100;
const unsigned kIterationPerDigit = 10;
const unsigned kTrial = 10;
const unsigned kPrecision = 17;

template<typename T>
class Rng {
public:
  explicit Rng(unsigned seed = 0) : gen_{seed} {}

	T operator()() {
		return dist_(gen_);
	}

private:
  std::mt19937 gen_;
  using dist = std::conditional_t<std::is_integral_v<T>, std::uniform_int_distribution<T>, std::uniform_real_distribution<T>>; 
  dist dist_;
};

template<typename F>
void BenchSequential(benchmark::State& state, F f, const std::string_view name) {
	char buffer[256] = { '\0' };
    const auto digit = state.range(0);
	  const auto start = static_cast<int64_t>(std::pow(10, digit - 1));
		const auto end   = start * 10;

			int64_t v = start;
			Rng<int64_t> r;
			v += r() % start; 
			double sign = 1;

  for (auto&& _ : state) {
				double d = v * sign;
        const auto [ptr, ec] = std::to_chars(std::begin(buffer), std::end(buffer), d, std::chars_format::fixed, digit);
        *ptr = '\0';
				benchmark::DoNotOptimize(f(buffer, 256));
				sign = -sign;
				v += 1;
				if (v >= end)
					v = start;
		}
}

void Digits(benchmark::internal::Benchmark* b) {
	for (int64_t digit = 1; digit <= 17; digit++) {
    b->Arg(digit);
  }
}

#define BENCHMAKR_SEQUENTIAL(Func) BENCHMARK_CAPTURE(BenchSequential, Func, imp::Func, #Func)->Name(#Func)->Apply(Digits);

BENCHMAKR_SEQUENTIAL(atof);
BENCHMAKR_SEQUENTIAL(strtod);
BENCHMAKR_SEQUENTIAL(sscanf);
BENCHMAKR_SEQUENTIAL(istringstream);
BENCHMAKR_SEQUENTIAL(num_get);
BENCHMAKR_SEQUENTIAL(stod);
#ifdef HAS_X_CHARS
BENCHMAKR_SEQUENTIAL(from_chars);
#endif
BENCHMAKR_SEQUENTIAL(scan);

static void Noop(benchmark::State& state) {
  for (auto _ : state) benchmark::DoNotOptimize(0);
}
BENCHMARK(Noop);
BENCHMARK_MAIN()`;