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

// https://github.com/dspinellis/unix-history-repo/blob/Research-V6/usr/source/iolib/ftoa.c
void ftoa(double x, char* str, int prec, int format) {
  /* converts a floating point number to an ascii string */
  /* x is stored into str, which should be at least 30 chars long */
  int ie, i, k, ndig, fstyle;
  double y;
  // if (nargs() != 7)
  //   IEHzap("ftoa  ");
  ndig = (prec <= 0) ? 7 : (prec > 22 ? 23 : prec + 1);
  if (format == 'f' || format == 'F')
    fstyle = 1;
  else
    fstyle = 0;
  /* print in e format unless last arg is 'f' */
  ie = 0;
  /* if x negative, write minus and reverse */
  if (x < 0) {
    *str++ = '-';
    x = -x;
  }

  /* put x in range 1 <= x < 10 */
  if (x > 0.0)
    while (x < 1.0) {
      x *= 10.0;
      ie--;
    }
  while (x >= 10.0) {
    x = x / 10.0;
    ie++;
  }

  /* in f format, number of digits is related to size */
  if (fstyle) ndig += ie;

  /* round. x is between 1 and 10 and ndig will be printed to
     right of decimal point so rounding is ... */
  for (y = i = 1; i < ndig; i++) y = y / 10.;
  x += y / 2.;
  if (x >= 10.0) {
    x = 1.0;
    ie++;
  } /* repair rounding disasters */
  /* now loop.  put out a digit (obtain by multiplying by
    10, truncating, subtracting) until enough digits out */
  /* if fstyle, and leading zeros, they go out special */
  if (fstyle && ie < 0) {
    *str++ = '0';
    *str++ = '.';
    if (ndig < 0) ie = ie - ndig; /* limit zeros if underflow */
    for (i = -1; i > ie; i--) *str++ = '0';
  }
  for (i = 0; i < ndig; i++) {
    k = x;
    *str++ = k + '0';
    if (i == (fstyle ? ie : 0)) /* where is decimal point */
      *str++ = '.';
    x -= (y = k);
    x *= 10.0;
  }

  /* now, in estyle,  put out exponent if not zero */
  if (!fstyle && ie != 0) {
    *str++ = 'E';
    if (ie < 0) {
      ie = -ie;
      *str++ = '-';
    }
    for (k = 100; k > ie; k /= 10)
      ;
    for (; k > 0; k /= 10) {
      *str++ = ie / k + '0';
      ie = ie % k;
    }
  }
  *str = '\0';
  return;
}

struct {
template<size_t N>
void operator()(double d, char (&result)[N], int precision) {
  ftoa(d, result, precision, 'f');
}
} dtoa;

struct {
template<size_t N>
void operator()(double d, char (&result)[N], int precision) {
  [[maybe_unused]] auto res = ::gcvt(d, precision, result);
}
} gcvt;

struct {
template<size_t N>
void operator()(double d, char (&result)[N], int precision) {
  std::snprintf(result, N, "%.*lf", precision, d);
}
} sprintf;

struct {
template<size_t N>
void operator()(double d, char (&result)[N], int precision) {
  std::stringstream out;
  out.rdbuf()->pubsetbuf(result, N);
  out << std::setprecision(precision) << std::fixed << d;
}
} ostringstream;

struct {
template<size_t N>
void operator()(double d, char (&result)[N], int precision) {
    std::ostringstream sst;  
    sst << std::setprecision(static_cast<int>(precision)) << std::fixed;
    using Facet = std::num_put<char, char*>;
    static std::locale loc{std::locale::classic(), new Facet};
    std::use_facet<Facet>(loc).put(result, sst, sst.fill(), d);
}
} num_put;

struct {
template<size_t N>
void operator()(double d, char (&result)[N], int precision) {
  std::to_string(d).copy(result, N);
}
} to_string;

#ifdef HAS_X_CHARS
struct {
template<size_t N>
void operator()(double d, char (&result)[N], int precision) {
  std::to_chars(std::begin(result), std::end(result), d,
                std::chars_format::fixed, precision);
}
} to_chars;
#endif

struct {
template<size_t N>
void operator()(double d, char (&result)[N], int precision) {
  fmt::format_to(result, "{:.{}f}", d, precision);
}
} fmt;

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

class RandomData {
public:
	static auto GetData() {
		static RandomData singleton;
		return singleton.mData;
	}

	static const size_t kCount = 1000;

private:
	RandomData()
	{
		Rng<double> r;

    mData.reserve(kCount);
    std::generate_n(std::back_inserter(mData), kCount, [&r]{
      double d;
      do {
        d = r();
      } while (std::isnan(d) || std::isinf(d));
      return d;
    });
	}

	std::vector<double> mData;
};

template<typename F>
void BenchRandom(benchmark::State& state,  F f, const std::string_view fname) {
	char buffer[256];
	const auto data = RandomData::GetData();
	size_t n = RandomData::kCount;

  for (auto&& _ : state) {
    for (auto&& d : data) {
      f(d, buffer, state.range(0));
    }
  }
}

void Precision(benchmark::internal::Benchmark* b) {
	for (int64_t precision = 1; precision <= 17; precision++) {
    b->Arg(precision);
  }
}

#define BENCHMARK_RANDOM(Func) BENCHMARK_CAPTURE(BenchRandom, Func, imp::Func, #Func)->Name(#Func)->Apply(Precision)

BENCHMARK_RANDOM(dtoa);
BENCHMARK_RANDOM(gcvt);
BENCHMARK_RANDOM(sprintf);
BENCHMARK_RANDOM(ostringstream);
BENCHMARK_RANDOM(num_put);
BENCHMARK_RANDOM(to_string);
#ifdef HAS_X_CHARS
BENCHMARK_RANDOM(to_chars);
#endif
BENCHMARK_RANDOM(fmt);

static void Noop(benchmark::State& state) {
  for (auto _ : state) benchmark::DoNotOptimize(0);
}
BENCHMARK(Noop);
BENCHMARK_MAIN()`;