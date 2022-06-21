#include <fmt/format.h>
#include <scn/scn.h>

#include <cassert>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <numbers>
#include <sstream>
#include <string_view>

// parse the 2 strings as numbers, add the numbers and return the result as a
// string String add(String lhs, String rhs);

// https://github.com/dspinellis/unix-history-repo/blob/Research-V6/usr/source/iolib/ftoa.c
void ftoa(double x, char *str, int prec, int format)
{
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
  if (x < 0)
  {
    *str++ = '-';
    x = -x;
  }

  /* put x in range 1 <= x < 10 */
  if (x > 0.0)
    while (x < 1.0)
    {
      x *= 10.0;
      ie--;
    }
  while (x >= 10.0)
  {
    x = x / 10.0;
    ie++;
  }

  /* in f format, number of digits is related to size */
  if (fstyle)
    ndig += ie;

  /* round. x is between 1 and 10 and ndig will be printed to
     right of decimal point so rounding is ... */
  for (y = i = 1; i < ndig; i++)
    y = y / 10.;
  x += y / 2.;
  if (x >= 10.0)
  {
    x = 1.0;
    ie++;
  } /* repair rounding disasters */
  /* now loop.  put out a digit (obtain by multiplying by
    10, truncating, subtracting) until enough digits out */
  /* if fstyle, and leading zeros, they go out special */
  if (fstyle && ie < 0)
  {
    *str++ = '0';
    *str++ = '.';
    if (ndig < 0)
      ie = ie - ndig; /* limit zeros if underflow */
    for (i = -1; i > ie; i--)
      *str++ = '0';
  }
  for (i = 0; i < ndig; i++)
  {
    k = x;
    *str++ = k + '0';
    if (i == (fstyle ? ie : 0)) /* where is decimal point */
      *str++ = '.';
    x -= (y = k);
    x *= 10.0;
  }

  /* now, in estyle,  put out exponent if not zero */
  if (!fstyle && ie != 0)
  {
    *str++ = 'E';
    if (ie < 0)
    {
      ie = -ie;
      *str++ = '-';
    }
    for (k = 100; k > ie; k /= 10)
      ;
    for (; k > 0; k /= 10)
    {
      *str++ = ie / k + '0';
      ie = ie % k;
    }
  }
  *str = '\0';
  return;
}

struct
{
  void operator()(const char *lhs, const char *rhs, char *result,
                  int precision) const
  {
    const double l = std::atof(lhs);
    const double r = std::atof(rhs);
    ftoa(l + r, result, precision, 'f');
  }
} XtoY;

struct
{
  void operator()(const char *lhs, const char *rhs, char *result,
                  int precision) const
  {
    const double l = std::strtod(lhs, nullptr);
    const double r = std::strtod(rhs, nullptr);
    gcvt(l + r, precision, result);
  }
} strtoX_gcvt;

struct
{
  void operator()(const char *lhs, const char *rhs, char *result,
                  int precision) const
  {
    double l, r;
    std::sscanf(lhs, "%lg", &l);
    std::sscanf(rhs, "%lg", &r);
    std::sprintf(result, "%.*lf", precision, l + r);
  }
} sXf;

struct
{
  template <typename T = double>
  std::string operator()(std::string_view lhs, std::string_view rhs,
                         int precision) const
  {
    T l, r;
    std::stringstream in;
    in << lhs << ' ' << rhs;
    in >> l >> r;

    std::stringstream out;
    out << std::setprecision(precision) << std::fixed << l + r;
    return out.str();
  }
} stringstream;

struct
{
  template <typename T = double>
  std::string operator()(std::string_view lhs, std::string_view rhs,
                         int precision) const
  {
    auto read = []<typename It>(It from, It to, T &d)
    {
      std::ios_base::iostate err = std::ios_base::goodbit;
      std::istringstream sst;
      using Facet = std::num_get<char, It>;
      std::locale loc{sst.getloc(), new Facet};
      std::use_facet<Facet>(loc).get(from, to, sst, err, d);
    };

    T l, r;
    read(lhs.begin(), lhs.end(), l);
    read(rhs.begin(), rhs.end(), r);

    auto write = [precision]<typename It>(It out, T d)
    {
      std::ostringstream sst;
      sst << std::setprecision(static_cast<int>(precision)) << std::fixed;
      using Facet = std::num_put<char, It>;
      std::locale loc{sst.getloc(), new Facet};
      std::use_facet<Facet>(loc).put(out, sst, sst.fill(), d);
    };

    std::string res;
    write(std::back_inserter(res), l + r);
    return res;
  }
} num_X;

struct
{
  std::string operator()(const std::string &lhs, const std::string &rhs,
                         int precision) const
  {
    return std::to_string(std::stod(lhs) + std::stod(rhs));
  }
} stoX_to_string;

struct
{
  template <typename T = double>
  std::string operator()(std::string_view lhs, std::string_view rhs,
                         int precision) const
  {
    T l, r;
    std::from_chars(lhs.data(), lhs.data() + lhs.size(), l);
    std::from_chars(rhs.data(), rhs.data() + rhs.size(), r);
    std::string res(std::numeric_limits<T>::max_exponent10 + 20, 0);
    const auto [end, _] =
        std::to_chars(res.data(), res.data() + res.size(), l + r,
                      std::chars_format::fixed, precision);
    res.resize(end - res.data());
    return res;
  }
} X_chars;

struct
{
  template <typename T = double>
  std::string operator()(std::string_view lhs, std::string_view rhs,
                         int precision) const
  {
    T l, r;
    scn::scan(lhs, "{}", l);
    scn::scan(rhs, "{}", r);
    return fmt::format("{:.{}f}", l + r, precision);
  }
} scan_format;

int main()
{
  static const int DEFAULT_PRECISION =
      std::numeric_limits<double>::max_digits10;

  auto test =
      [lhs = fmt::format("{:.{}f}", std::numbers::pi - 0.2, DEFAULT_PRECISION),
       rhs = fmt::format("{:.{}f}", 0.2, DEFAULT_PRECISION)](
          const std::string_view name, auto method)
  {
    fmt::print("{:20}", name);
    if constexpr (std::is_invocable_v<decltype(method), const char *,
                                      const char *, char *, int>)
    {
      constexpr int n = 1 /*'-'*/ +
                        (std::numeric_limits<double>::max_exponent10 +
                         1) /*exponent+1 digits*/
                        + 1 /*'.'*/ + DEFAULT_PRECISION /* precision*/ +
                        1 /*terminating null*/;
      char buf[n];
      method(lhs.c_str(), rhs.c_str(), buf, DEFAULT_PRECISION);
      fmt::print("{}\n", buf);
    }
    else
    {
      fmt::print("{}\n", method(lhs, rhs, DEFAULT_PRECISION));
    }
  };

  fmt::print("{:20}{:.{}f}\n", "π", std::numbers::pi, DEFAULT_PRECISION);
  test("XtoY", XtoY);
  test("strtoX_gcvt", strtoX_gcvt);
  test("sXf", sXf);
  test("stringstream", stringstream);
  test("num_X", num_X);
  test("stoX_to_string", stoX_to_string);
  test("X_chars", X_chars);
  test("scan_format", scan_format);
}