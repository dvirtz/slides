#include <fmt/format.h>

#include <boost/convert.hpp>
#include <boost/convert/lexical_cast.hpp>
#include <boost/convert/printf.hpp>
#include <boost/convert/spirit.hpp>
#include <boost/convert/stream.hpp>
#include <boost/convert/strtol.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/locale.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>

struct
{
  template <typename T = double>
  std::string operator()(std::string_view lhs, std::string_view rhs, int /*precision*/) const
  {
    const auto l = boost::lexical_cast<T>(lhs);
    const auto r = boost::lexical_cast<T>(rhs);
    return boost::lexical_cast<std::string>(l + r);
  }
} lexical_cast;

struct
{
  template <typename T = double>
  std::string operator()(std::string_view lhs, std::string_view rhs, int precision) const
  {
    const auto l = boost::lexical_cast<T>(lhs);
    const auto r = boost::lexical_cast<T>(rhs);
    return (boost::format("%.17f") % (l + r)).str();
  }
} boost_format;

struct
{
  template <typename Num>
  struct precision_policy : boost::spirit::karma::real_policies<Num>
  {
    precision_policy(int precision) : precision_{precision} {}
    unsigned precision(Num /*n*/) const { return precision_; }
    int precision_;
  };

  template <typename T = double>
  std::string operator()(std::string_view lhs, std::string_view rhs, int precision) const
  {
    const auto parse_double = [](std::string_view str, double &d)
    {
      namespace qi = boost::spirit::qi;

      using qi::double_;

      const auto action = [&d](double v)
      { d = v; };
      qi::parse(str.begin(), str.end(), double_[action]);
    };

    double l, r;
    parse_double(lhs, l);
    parse_double(rhs, r);

    namespace karma = boost::spirit::karma;
    using karma::double_;
    using precision_double_ =
        karma::real_generator<double, precision_policy<T>>;

    std::string res;
    karma::generate(std::back_inserter(res), precision_double_{precision}, l + r);
    return res;
  }
} qi_karma;

struct
{
  template <typename Num>
  struct precision_policy : boost::spirit::karma::real_policies<Num>
  {
    precision_policy(int precision) : precision_{precision} {}
    unsigned precision(Num /*n*/) const { return precision_; }
    int precision_;
  };

  template <typename T = double>
  std::string operator()(std::string_view lhs, std::string_view rhs, int precision) const
  {
    const auto parse_double = [](std::string_view str, double &d)
    {
      namespace x3 = boost::spirit::x3;

      using x3::double_;
      using x3::_attr;

      const auto action = [&d](auto &ctx)
      { d = _attr(ctx); };
      x3::parse(str.begin(), str.end(), double_[action]);
    };

    double l, r;
    parse_double(lhs, l);
    parse_double(rhs, r);

    namespace karma = boost::spirit::karma;
    using karma::double_;
    using precision_double_ =
        karma::real_generator<double, precision_policy<T>>;

    std::string res;
    karma::generate(std::back_inserter(res), precision_double_{precision}, l + r);
    return res;
  }
} x3_karma;

template <typename Converter>
struct convert_
{
  template <typename T = double>
  std::string operator()(std::string_view lhs, std::string_view rhs,
                         int precision)
  {
    Converter ccnv;
    namespace arg = boost::cnv::parameter;

    // https://github.com/boostorg/convert/issues/53
    using T_ = std::conditional_t<std::is_same_v<Converter, boost::cnv::printf>, float, double>;

    const auto l = boost::convert<T_>(lhs, ccnv).value();
    const auto r = boost::convert<T_>(rhs, ccnv).value();
    if constexpr (std::is_invocable_v<Converter, decltype(arg::precision)>)
    {
      return boost::convert<std::string>(l + r,
                                         ccnv(arg::precision = precision))
          .value();
    }
    else
    {
      return boost::convert<std::string>(l + r, ccnv).value();
    }
  }
};

template <typename Converter>
convert_<Converter> convert;

int main()
{
  static const int DEFAULT_PRECISION =
      std::numeric_limits<double>::max_digits10 + 1;

  auto test =
      [lhs = fmt::format("{:.{}f}", std::numbers::pi - 0.2, DEFAULT_PRECISION),
       rhs = fmt::format("{:.{}f}", 0.2, DEFAULT_PRECISION)](
          const std::string_view name, auto method)
  {
    fmt::print("{:25}", name);
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

  fmt::print("{:25}{:.{}f}\n", "π", std::numbers::pi, DEFAULT_PRECISION);
  test("lexical_cast", lexical_cast);
  test("boost_format", boost_format);
  test("qi_karma", qi_karma);
  test("x3_karma", x3_karma);
  test("convert<cstream>", convert<boost::cnv::cstream>);
  test("convert<strtol>", convert<boost::cnv::strtol>);
  test("convert<lexical_cast>", convert<boost::cnv::lexical_cast>);
  test("convert<printf>", convert<boost::cnv::printf>);
  test("convert<spirit>", convert<boost::cnv::spirit>);
}