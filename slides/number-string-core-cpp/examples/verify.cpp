#include <fmt/format.h>
#include <scn/scn.h>

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <cassert>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <numbers>
#include <random>
#include <sstream>
#include <string_view>

constexpr int DEFAULT_PRECISION = 17;

constexpr int BUF_SIZE =
    1 /*'-'*/ +
    (std::numeric_limits<double>::max_exponent10 + 1) /*exponent+1 digits*/
    + 1 /*'.'*/ + DEFAULT_PRECISION /* precision*/ + 1 /*terminating null*/;

template <typename Format, typename... Args>
void error(Format &&format, Args &&...args) {
  throw std::runtime_error(
      fmt::format(format, std::forward<Args>(args)...));
}

template <typename T>
class Rng {
 public:
  explicit Rng(unsigned seed = 0) : gen_{seed} {}

  T operator()() { return dist_(gen_); }

 private:
  std::mt19937 gen_;
  using dist = std::conditional_t<std::is_integral_v<T>,
                                  std::uniform_int_distribution<T>,
                                  std::uniform_real_distribution<T>>;
  dist dist_;
};

template <typename Method>
static size_t verifyValue(double value, Method method,
                          const std::string_view expect = "") {
  auto str = method(value);

  if (not expect.empty() && str != expect) {
    error("Error: expect {} but actual {}", expect, str);
  }

  auto [roundtrip, processed] = method(str);

  if (processed < 0) {
    return 0;
  }

  if (str.size() != static_cast<size_t>(processed)) {
    error("Error: some extra character {} -> '{}'", value, str);
  }

  if (value != roundtrip) {
    error("Error: roundtrip fail {:.17g} -> '{}' -> {:.17g}", value, str,
          roundtrip);
  }

  return str.size();
}

template <typename Method>
static void verify(const std::string_view fname, Method method) try {
  fmt::print("Verifying {:20} ... ", fname);

  // Boundary and simple cases
  verifyValue(0, method);
  verifyValue(0.1, method, "0.1");
  verifyValue(0.12, method, "0.12");
  verifyValue(0.123, method, "0.123");
  verifyValue(0.1234, method, "0.1234");
  verifyValue(1.2345, method, "1.2345");
  verifyValue(1.0 / 3.0, method);
  verifyValue(2.0 / 3.0, method);
  verifyValue(10.0 / 3.0, method);
  verifyValue(20.0 / 3.0, method);
  verifyValue(std::numeric_limits<double>::min(), method);
  verifyValue(std::numeric_limits<double>::max(), method);
  verifyValue(std::numeric_limits<double>::denorm_min(), method);

  Rng<double> r;

  constexpr unsigned kVerifyRandomCount = 100000;

  uint64_t lenSum = 0;
  size_t lenMax = 0;
  for (unsigned i = 0; i < kVerifyRandomCount; i++) {
    double d;
    do {
      d = r();
    } while (std::isnan(d) || std::isinf(d));
    size_t len = verifyValue(d, method);
    lenSum += len;
    lenMax = std::max(lenMax, len);
  }

  double lenAvg = double(lenSum) / kVerifyRandomCount;
  fmt::print("OK. Length Avg = {:2.3f}, Max = {}\n", lenAvg, lenMax);
} catch (const std::exception &ex) {
  fmt::print("{}\n", ex.what());
}

// parse the 2 strings as numbers, add the numbers and return the result as a
// string String add(String lhs, String rhs);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wconversion"

// https://github.com/dspinellis/unix-history-repo/blob/Research-V6/usr/source/iolib/ftoa.c
void ftoa(double x, char *str, int prec, int format) {
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

#pragma GCC diagnostic pop

struct to_double_res {
  double res = 0;
  ptrdiff_t processed;
};

[[maybe_unused]] struct {
  to_double_res operator()(const std::string &str) const {
    return {std::atof(str.c_str()), static_cast<ptrdiff_t>(str.size())};
  }

  std::string operator()(const double d) const {
    char buf[BUF_SIZE];
    ftoa(d, buf, DEFAULT_PRECISION, 'f');
    return buf;
  }
} XtoY;

[[maybe_unused]] struct {
  to_double_res operator()(const std::string &str) const {
    char *end;
    auto res = std::strtod(str.c_str(), &end);
    return {res, errno != 0 ? -1 : (end - str.c_str())};
  }

  std::string operator()(const double d) const {
    char buf[BUF_SIZE];
    gcvt(d, DEFAULT_PRECISION, buf);
    return buf;
  }
} strtoX_gcvt;

[[maybe_unused]] struct {
  to_double_res operator()(const std::string &str) const {
    double res = 0;
    ptrdiff_t processed = -1;
    std::sscanf(str.c_str(), "%lf%tn", &res, &processed);
    return {res, processed};
  }

  std::string operator()(const double d) const {
    char buf[BUF_SIZE];
    std::sprintf(buf, "%g", d);
    return buf;
  }
} sXf;

[[maybe_unused]] struct {
  to_double_res operator()(const std::string &str) const {
    std::istringstream in{str};
    double res = 0;
    in >> res;
    auto processed = [&]() -> ptrdiff_t {
      if (in.eof()) {
        return std::ssize(str);
      }
      return in.tellg();
    }();
    return {res, processed};
  }

  std::string operator()(const double d) const {
    std::ostringstream out;
    out << std::defaultfloat << d;
    return out.str();
  }
} stringstream;

[[maybe_unused]] struct {
  to_double_res operator()(const std::string &str) const {
    std::ios_base::iostate err = std::ios_base::goodbit;
    std::istringstream sst;
    using Facet = std::num_get<char, std::string::const_iterator>;
    static std::locale loc{std::locale::classic(), new Facet};
    double res = 0;
    auto end =
        std::use_facet<Facet>(loc).get(str.begin(), str.end(), sst, err, res);
    return {res, (err & std::ios_base::failbit) ? -1 : (end - str.begin())};
  }

  std::string operator()(const double d) const {
    std::ostringstream sst;
    sst << std::defaultfloat;
    std::string res;
    res.reserve(BUF_SIZE);
    using Facet = std::num_put<char, decltype(std::back_inserter(res))>;
    std::locale loc{sst.getloc(), new Facet};
    std::use_facet<Facet>(loc).put(std::back_inserter(res), sst, sst.fill(), d);
    return res;
  }
} num_X;

[[maybe_unused]] struct {
  to_double_res operator()(const std::string &str) const {
    return {std::stod(str), static_cast<ptrdiff_t>(str.size())};
  }

  std::string operator()(const double d) const { return std::to_string(d); }
} stoX_to_string;

[[maybe_unused]] struct {
  to_double_res operator()(const std::string &str) const {
    double res = 0;
    const auto [end, ec] =
        std::from_chars(str.data(), str.data() + str.size(), res);
    return {res, std::error_condition{ec} ? -1 : (end - str.data())};
  }

  std::string operator()(const double d) const {
    std::string res(BUF_SIZE, 0);
    const auto [end, _] = std::to_chars(res.data(), res.data() + res.size(), d);
    res.resize(static_cast<size_t>(end - res.data()));
    return res;
  }
} X_chars;

[[maybe_unused]] struct {
  to_double_res operator()(const std::string &str) const {
    double res = 0;
    auto scan_result = scn::scan(str, "{}", res);
    return {res, !scan_result ? -1 : (scan_result.begin() - str.data())};
  }

  std::string operator()(const double d) const { return fmt::format("{}", d); }
} scan_format;

[[maybe_unused]] struct {
  template <typename Num>
  struct precision_policy : boost::spirit::karma::real_policies<Num>
  {
    precision_policy(unsigned precision) : precision_{precision} {}
    unsigned precision(Num /*n*/) const { return precision_; }
    unsigned precision_;
  };

  to_double_res operator()(const std::string &str) const {
    namespace qi = boost::spirit::qi;

    using boost::phoenix::ref;
    using qi::_1;
    using qi::double_;

    double res = 0;
    auto first = str.begin();
    auto success = qi::parse(first, str.end(), double_[ref(res) = _1]);
    return { res, !success ? -1 : first - str.begin() };
  }

  std::string operator()(const double d) const {
    namespace karma = boost::spirit::karma;
    using karma::double_;

    std::string res;
    using precision_double_ =
        karma::real_generator<double, precision_policy<double>>;
    karma::generate(std::back_inserter(res), precision_double_{DEFAULT_PRECISION}, d);
    return res;
  }
} qi_karma;

int main() {
  verify("XtoY", XtoY);
  verify("strtoX_gcvt", strtoX_gcvt);
  verify("sXf", sXf);
  verify("stringstream", stringstream);
  verify("num_X", num_X);
  verify("stoX_to_string", stoX_to_string);
  verify("X_chars", X_chars);
  verify("scan_format", scan_format);
  verify("qi_karma", qi_karma);
}
