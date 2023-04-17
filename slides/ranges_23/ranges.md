---
revealOptions:
  slideNumber: c/t
  compilerExplorer:
    compiler: "gsnapshot"
    options: -std=c++23 -O2 -march=haswell -Wall -Wextra -pedantic -Wno-unused-variable -Wno-unused-parameter
highlightTheme: atom-one-dark
---

<!-- .slide: data-background-image="resources/title.png" -->

----

<!-- .slide: data-background-image="resources/EricTaDa.png" -->

----

<!-- .slide: data-background-image="resources/recap.jpg" -->

## recap

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #859d34" -->

Source: https://discovernorthernireland.com/

<!-- .element: style="font-size: 0.5em" -->

---

### what is a range

- A sequence of elements between two locations `i`, `k`.
- Often denoted by `[i, k)`.

---

<!-- .slide: data-auto-animate -->

## `std::for_each`

<pre style="font-size: 0.4em">
<code data-trim data-noescape class="lang-cpp">
template&lt;class InputIt, class UnaryFunction&gt;
constexpr UnaryFunction for_each(InputIt first, InputIt last, UnaryFunction f)
{
    for (; first != last; ++first) {
        f(*first);
    }
    return f;
}
</code>
</pre>

---

<!-- .slide: data-auto-animate -->

## `std::for_each`

<pre style="font-size: 0.4em">
<code data-trim data-noescape class="lang-cpp">
template&lt;class InputIt, class UnaryFunction&gt;
constexpr UnaryFunction for_each(<mark>InputIt first</mark>, InputIt last, UnaryFunction f)
{
    for (; first != last; <mark>++first</mark>) {
        f(<mark>*first</mark>);
    }
    return f;
}
</code>
</pre>

---

## iterator concepts

<table class="no-border" style="font-size: 0.6em">
<tbody>
  <tr>
    <td><code>input_or_output_iterator</code></td>
    <td>can be dereferenced (<code>*it</code>) and incremented (<code>++it</code>)</td>
  </tr>
  <tr class="fragment" data-fragment-index="1">
    <td><code>input_iterator</code></td>
    <td>referenced values can be read (<code>auto v = *it</code>)</td>
  </tr>
  <tr class="fragment" data-fragment-index="1">
    <td><code>output_iterator</code></td>
    <td>referenced values can be written to (<code>*it = v</code>)</td>
  </tr>
  <tr class="fragment" data-fragment-index="1">
    <td><code>forward_iterator</code></td>
    <td>input_iterator + comparable and multi-pass</td>
  </tr>
  <tr class="fragment" data-fragment-index="1">
    <td><code>bidirectional_iterator</code></td>
    <td>forward_iterator + decrementable (<code>--it</code>)</td>
  </tr>
  <tr class="fragment" data-fragment-index="1">
    <td><code>random_access_iterator</code></td>
    <td>bidirectional_iterator + random access (<code>it += n</code>)</td>
  </tr>
  <tr class="fragment" data-fragment-index="1">
    <td><code>contiguous_iterator</code></td>
    <td>random_access_iterator + contiguous in memory</td>
  </tr>
</tbody></table>

---

<!-- .slide: data-auto-animate -->

## `std::for_each`

<pre style="font-size: 0.4em">
<code data-trim data-noescape class="lang-cpp">
template&lt;class InputIt, class UnaryFunction&gt;
constexpr UnaryFunction for_each(InputIt first, <mark>InputIt last</mark>, UnaryFunction f)
{
    for (; <mark>first != last</mark>; ++first) {
        f(*first);
    }
    return f;
}
</code>
</pre>

---

## Sentinels

- a semi-regular `S` is a `sentinel_for` an iterator `I` if for a range `[i, s)`
  - `i == s` is well defined.
  - If `i != s` then `i` is dereferenceable and `[++i, s)` denotes a range.

- Such an `s` is called a sentinel

---

## range

a type we can feed to
- `ranges::begin` - to get an iterator
- `ranges::end` - to get a sentinel

A range `[i, s)` refers to the elements

`*i, *(i++), *((i++)++), …, j`

such that j == s.

---

## range concepts

|concept|example|
|-------|---------|
|`input_range` | e.g. a range over a `std::istream_iterator`|
|`output_range` | e.g. a range over a `std::back_insert_iterator`|
|`forward_range` | e.g. `std::forward_list`|
|`bidirectional_range` | e.g. `std::list`|
|`random_access_range` | e.g. `std::deque`|
|`contiguous_range` | e.g. `std::vector`|
|`common_range`|sentinel is same type as iterator|

<!-- .element: class="no-border no-header" style="font-size: 0.6em" -->

---

## algorithms

now constrained with concepts:

```cpp [|2,6,10,14|5,13]
///compiler=g102
///options=-std=c++2a
///hide
#include <concepts>
#include <iterator>
#include <ranges>

template<class I, class F>
struct for_each_result;

///unhide
namespace std::ranges {
template<input_iterator I, sentinel_for<I> S, 
         class Proj = identity,
         indirectly_unary_invocable<projected<I, Proj>> Fun>
constexpr for_each_result<I, Fun>
for_each(I first, S last, 
         Fun f, 
         Proj proj = {});

template<input_range R, 
         class Proj = identity,
         indirectly_unary_invocable<projected<iterator_t<R>, Proj>> Fun>
constexpr for_each_result<borrowed_iterator_t<R>, Fun>
for_each(R&& r, 
         Fun f, 
         Proj proj = {});
}
```

<!-- .element: style="font-size: 0.45em" -->

---

<!-- .slide: data-auto-animate -->

> The law of useful return: 
>
> A procedure should return all the potentially useful information it computed.

<p style="width: 100%; text-align: right;"> <cite> Alexander Stepanov </cite> </p>

---

## return value

```cpp [4,5]
///compiler=g102
///options=-std=c++2a
///hide
#include <concepts>
#include <utility>

///unhide
namespace std::ranges {
template<class I, class F>
struct for_each_result {
  [[no_unique_address]] I in;
  [[no_unique_address]] F fun;

  template<class I2, class F2>
  requires convertible_to<const I&, I2> && convertible_to<const F&, F2>
  operator for_each_result<I2, F2>() const & {
    return {in, fun};
  }

  template<class I2, class F2>
  requires convertible_to<I, I2> && convertible_to<F, F2>
  operator for_each_result<I2, F2>() && {
    return {std::move(in), std::move(fun)};
  }
};
}
```

<!-- .element: style="font-size: 0.4em" -->

Note: in addition to returning the function, it now returns the end iterator of the input range. This is the iterator that compared equal to the input sentinel.

---

## views

A lightweight™ handle to a range.

- A range type that wraps a pair of iterators.
- A range type that holds its elements by `std::shared_ptr` and shares ownership with all its copies.
- A range type that generates its elements on demand.


---

## Example

```cpp
///compiler=g102
///options=-std=c++2a
///hide
#include <type_traits>

template<typename T>
class view_interface;

///unhide
namespace std::ranges {
template<class T>
requires is_object_v<T>
class empty_view : public view_interface<empty_view<T>> {
public:
  static constexpr T* begin() noexcept { return nullptr; }
  static constexpr T* end() noexcept { return nullptr; }
  static constexpr T* data() noexcept { return nullptr; }
  static constexpr size_t size() noexcept { return 0; }
  static constexpr bool empty() noexcept { return true; }

  friend constexpr T* begin(empty_view) noexcept { return nullptr; }
  friend constexpr T* end(empty_view) noexcept { return nullptr; }
};
}
```

<!-- .element: style="font-size: 0.45em" -->

---

## Factories & Adaptors

- Utility objects for creating views
  - adaptors (if transform an existing range) or 
  - factories (otherwise)

```cpp [1-4,7]
///compiler=g102
///options=-std=c++2a
///hide
#include <ranges>

#if 0
///unhide
namespace std::ranges::views {
template<class T>
inline constexpr empty_view<T> empty{};
}
///hide
#endif
///unhide

using namespace std::ranges;
static_assert(empty(views::empty<int>));
```

---

<!-- .slide: data-auto-animate -->

## classic STL

```cpp []
///hide
#include <vector>
#include <numeric>
#include <algorithm>

///unhide
int sum_of_squares(int count) {
  using namespace std;

  vector<int> numbers(static_cast<size_t>(count));
  iota(numbers.begin(), numbers.end(), 1);
  transform(numbers.begin(), numbers.end(), numbers.begin(), 
                 [](int x) { return x * x; });
  return accumulate(numbers.begin(), numbers.end(), 0);
}
```

<!-- .element: data-id="code" style="font-size: 0.45em" -->

---

<!-- .slide: data-auto-animate -->

## function call syntax

```cpp []
///external
///libs=rangesv3:trunk
///hide
#include <range/v3/all.hpp>

///unhide
int sum_of_squares(int count) {
  using namespace ranges;

  return accumulate(
    views::transform(
      views::iota(1, count), 
      [](int x) { return x * x; }
    ), 0
  );
}
```

<!-- .element: data-id="code" style="font-size: 0.45em" -->

---

<!-- .slide: data-auto-animate -->

## piped syntax

```cpp []
///external
///libs=rangesv3:trunk
///hide
#include <range/v3/all.hpp>

///unhide
int sum_of_squares(int count) {
  using namespace ranges;
  
  auto squares = views::iota(1, count) 
               | views::transform([](int x) { return x * x; });
  return accumulate(squares, 0);
}
```

<!-- .element: data-id="code" style="font-size: 0.45em" -->

Note: this doesn't really compile with the C++20 accumulate.

---

<!-- .slide: data-auto-animate class="aside" -->

### listing the days

```cpp [1|3-12]
///hide
#include <chrono>
#include <ranges>

namespace views = std::views;

template <typename Clock, typename Duration>
struct std::incrementable_traits<std::chrono::time_point<Clock, Duration>> {
  using difference_type = typename Duration::rep;
};
///unhide
using date = std::chrono::year_month_day;

auto first_day(std::chrono::year year) {
  using namespace std::literals;
  using std::chrono::January;
  return std::chrono::sys_days{year / January / 1d};
}

auto dates(std::chrono::year start, std::chrono::year stop) {
  return views::iota(first_day(start), first_day(stop)) |
         views::transform([](const auto day) { return date{day}; });
}
```

<!-- .element: data-id="code" style="font-size: 0.45em" -->

---

<!-- .slide: data-auto-animate class="aside" -->

### listing the days

```cpp [3-12]
///hide
#include <chrono>
#include <ranges>

namespace views = std::views;

template <typename Clock, typename Duration>
struct std::incrementable_traits<std::chrono::time_point<Clock, Duration>> {
  using difference_type = typename Duration::rep;
};
///unhide
using date = std::chrono::year_month_day;

auto first_day(std::chrono::year year) {
  using namespace std::literals;
  using std::chrono::January;
  return std::chrono::sys_days{year / January / 1d};
}

auto dates_from(std::chrono::year year) {
  return views::iota(first_day(year)) |
         views::transform([](const auto day) { return date{day}; });
}
```

<!-- .element: data-id="code" style="font-size: 0.45em" -->

---

<!-- .slide: class="aside" -->

### `dates()`

<pre data-id="dates" style="font-size: 0.45em;">
<code data-trim class="text" style="white-space: pre-wrap;">
[2023-01-01, 2023-01-02, 2023-01-03, 2023-01-04, 2023-01-05, 2023-01-06, 2023-01-07, 2023-01-08, 2023-01-09, 2023-01-10, 2023-01-11, 2023-01-12, 2023-01-13, 2023-01-14, 2023-01-15, 2023-01-16, 2023-01-17, 2023-01-18, 2023-01-19, 2023-01-20, 2023-01-21, 2023-01-22, 2023-01-23, 2023-01-24, 2023-01-25, 2023-01-26, 2023-01-27, 2023-01-28, 2023-01-29, 2023-01-30, 2023-01-31, 2023-02-01, 2023-02-02, 2023-02-03, 2023-02-04, 2023-02-05, 2023-02-06, 2023-02-07, 2023-02-08, 2023-02-09, 2023-02-10, 2023-02-11, 2023-02-12, 2023-02-13, 2023-02-14, 2023-02-15, 2023-02-16, 2023-02-17, 2023-02-18, 2023-02-19, 2023-02-20, 2023-02-21, 2023-02-22, 2023-02-23, 2023-02-24, 2023-02-25, 2023-02-26, 2023-02-27, 2023-02-28, 2023-03-01, 2023-03-02, 2023-03-03, 2023-03-04, 2023-03-05, 2023-03-06, 2023-03-07, 2023-03-08, 2023-03-09, 2023-03-10, 2023-03-11, 2023-03-12, 2023-03-13, 2023-03-14, 2023-03-15, 2023-03-16, 2023-03-17, 2023-03-18, 2023-03-19, 2023-03-20, 2023-03-21, 2023-03-22, 2023-03-23, 2023-03-24, 2023-03-25, 2023-03-26, 2023-03-27, 2023-03-28, 2023-03-29, 2023-03-30, 2023-03-31, 2023-04-01, 2023-04-02, 2023-04-03, 2023-04-04, 2023-04-05, 2023-04-06, 2023-04-07, 2023-04-08, 2023-04-09, 2023-04-10, 2023-04-11, 2023-04-12, 2023-04-13, 2023-04-14, 2023-04-15, 2023-04-16, 2023-04-17, 2023-04-18, 2023-04-19, 2023-04-20, 2023-04-21, 2023-04-22, 2023-04-23, 2023-04-24, 2023-04-25, 2023-04-26, 2023-04-27, 2023-04-28, 2023-04-29, 2023-04-30, 2023-05-01, 2023-05-02, 2023-05-03, 2023-05-04, 2023-05-05, 2023-05-06, 2023-05-07, 2023-05-08, 2023-05-09, 2023-05-10, 2023-05-11, 2023-05-12, 2023-05-13, 2023-05-14, 2023-05-15, 2023-05-16, 2023-05-17, 2023-05-18, 2023-05-19, 2023-05-20, 2023-05-21, 2023-05-22, 2023-05-23, 2023-05-24, 2023-05-25, 2023-05-26, 2023-05-27, 2023-05-28, 2023-05-29, 2023-05-30, 2023-05-31, 2023-06-01, 2023-06-02, 2023-06-03, 2023-06-04, 2023-06-05, 2023-06-06, 2023-06-07, 2023-06-08, 2023-06-09, 2023-06-10, 2023-06-11, 2023-06-12, 2023-06-13, 2023-06-14, 2023-06-15, 2023-06-16, 2023-06-17, 2023-06-18, 2023-06-19, 2023-06-20, 2023-06-21, 2023-06-22, 2023-06-23, 2023-06-24, 2023-06-25, 2023-06-26, 2023-06-27, 2023-06-28, 2023-06-29, 2023-06-30, 2023-07-01, 2023-07-02, 2023-07-03, 2023-07-04, 2023-07-05, 2023-07-06, 2023-07-07, 2023-07-08, 2023-07-09, 2023-07-10, 2023-07-11, 2023-07-12, 2023-07-13, 2023-07-14, 2023-07-15, 2023-07-16, 2023-07-17, 2023-07-18, 2023-07-19, 2023-07-20, 2023-07-21, 2023-07-22, 2023-07-23, 2023-07-24, 2023-07-25, 2023-07-26, 2023-07-27, 2023-07-28, 2023-07-29, 2023-07-30, 2023-07-31, 2023-08-01, 2023-08-02, 2023-08-03, 2023-08-04, 2023-08-05, 2023-08-06, 2023-08-07, 2023-08-08, 2023-08-09, 2023-08-10, 2023-08-11, 2023-08-12, 2023-08-13, 2023-08-14, 2023-08-15, 2023-08-16, 2023-08-17, 2023-08-18, 2023-08-19, 2023-08-20, 2023-08-21, 2023-08-22, 2023-08-23, 2023-08-24, 2023-08-25, 2023-08-26, 2023-08-27, 2023-08-28, 2023-08-29, 2023-08-30, 2023-08-31, 2023-09-01, 2023-09-02, 2023-09-03, 2023-09-04, 2023-09-05, 2023-09-06, 2023-09-07, 2023-09-08, 2023-09-09, 2023-09-10, 2023-09-11, 2023-09-12, 2023-09-13, 2023-09-14, 2023-09-15, 2023-09-16, 2023-09-17, 2023-09-18, 2023-09-19, 2023-09-20, 2023-09-21, 2023-09-22, 2023-09-23, 2023-09-24, 2023-09-25, 2023-09-26, 2023-09-27, 2023-09-28, 2023-09-29, 2023-09-30, 2023-10-01, 2023-10-02, 2023-10-03, 2023-10-04, 2023-10-05, 2023-10-06, 2023-10-07, 2023-10-08, 2023-10-09, 2023-10-10, 2023-10-11, 2023-10-12, 2023-10-13, 2023-10-14, 2023-10-15, 2023-10-16, 2023-10-17, 2023-10-18, 2023-10-19, 2023-10-20, 2023-10-21, 2023-10-22, 2023-10-23, 2023-10-24, 2023-10-25, 2023-10-26, 2023-10-27, 2023-10-28, 2023-10-29, 2023-10-30, 2023-10-31, 2023-11-01, 2023-11-02, 2023-11-03, 2023-11-04, 2023-11-05, 2023-11-06, 2023-11-07, 2023-11-08, 2023-11-09, 2023-11-10, 2023-11-11, 2023-11-12, 2023-11-13, 2023-11-14, 2023-11-15, 2023-11-16, 2023-11-17, 2023-11-18, 2023-11-19, 2023-11-20, 2023-11-21, 2023-11-22, 2023-11-23, 2023-11-24, 2023-11-25, 2023-11-26, 2023-11-27, 2023-11-28, 2023-11-29, 2023-11-30, 2023-12-01, 2023-12-02, 2023-12-03, 2023-12-04, 2023-12-05, 2023-12-06, 2023-12-07, 2023-12-08, 2023-12-09, 2023-12-10, 2023-12-11, 2023-12-12, 2023-12-13, 2023-12-14, 2023-12-15, 2023-12-16, 2023-12-17, 2023-12-18, 2023-12-19, 2023-12-20, 2023-12-21, 2023-12-22, 2023-12-23, 2023-12-24, 2023-12-25, 2023-12-26, 2023-12-27, 2023-12-28, 2023-12-29, 2023-12-30, 2023-12-31]
</code>
</pre>

----

<!-- .slide: data-background-image="resources/formatting.png" -->

## Formatting ranges

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #84d4f6" -->

Source: [pennaspillo.it](https://pennaspillo.it/it/londra/casette-pastello-londra-dove-trovarle/)

<!-- .element: style="font-size: 0.5em" -->

Note: Perhaps the most useful addition to C++ is the ability to format and print ranges, as part
of the new format library, and most of the code examples we'll look at today will use that ability,
so we'll start with that.


---

### Standard

<pre>
<code class="lang-cpp" data-trim data-line-numbers="|1|2|3|4" data-fragment-index="0">
///compiler=clang_trunk
///options+=-std=c++2b -stdlib=libc++ -fexperimental-library
///output=[1, 2, 3]\n{1, 2, 3}\n(42, 16)\n{1: 2, 3: 4}
///hide
#include &lt;https://godbolt.org/z/Yv18W39YM/code/1&gt; // print
#include &lt;vector&gt;
#include &lt;set&gt;
#include &lt;map&gt;

int main() {
///unhide
std::println("{}", std::vector{1, 2, 3});
std::println("{}", std::set{1, 2, 3});
std::println("{}", std::pair{42, 16});
std::println("{}", std::map&lt;int, int&gt;{{1, 2}, {3, 4}});
///hide
}
</code>
</pre>

<pre class="fragment" data-fragment-index="0">
<code class="lang-HTML" data-trim data-line-numbers="1|2|3|4" data-fragment-index="1">
[1, 2, 3]
{1, 2, 3}
(42, 16)
{1: 2, 3: 4}
</code>
</pre>

---

<!-- .slide: data-auto-animate -->

### format specifiers

```nohighlight
"{ arg-id (optional) }"
"{ arg-id (optional) : format-spec }"
```

<!-- .element: data-id="formats" -->

---

<!-- .slide: data-auto-animate -->

### nested
### format specifiers

```nohighlight
"{ arg-id (optional) }"
"{ arg-id (optional) : format-spec }"
"{ arg-id (optional) : format-spec : format-spec }"
"{ arg-id (optional) : format-spec : format-spec : format-spec }"
```

<!-- .element: data-id="formats" -->

---

### Examples

<pre>
<code class="lang-cpp" data-trim data-line-numbers="1|2|3|4" data-fragment-index="1">
///compiler=clang_trunk
///options+=-std=c++2b -stdlib=libc++ -fexperimental-library
///output=[1, 2, 3]\n[1, 2, 3]\n###########[1, 2, 3]\n#####[0x1, 0x2, 0x3]
///hide
#include &lt;https://godbolt.org/z/Yv18W39YM/code/1&gt; // print
#include &lt;vector&gt;

int main() {
///unhide
std::println("{}",         std::vector{1, 2, 3});
std::println("{:}",        std::vector{1, 2, 3});
std::println("{:#>20}",    std::vector{1, 2, 3});
std::println("{:#>20:#x}", std::vector{1, 2, 3});
///hide
}
</code>
</pre>

<pre>
<code class="lang-HTML" data-trim data-line-numbers="1|2|3|4" data-fragment-index="1">
[1, 2, 3]
[1, 2, 3]
##########[1, 2, 3]
####[0x1, 0x2, 0x3]
</code>
</pre>


---

### Customization

<pre>
<code class="lang-cpp" data-trim data-line-numbers="1|2|3" data-fragment-index="1">
///compiler=clang_trunk
///options+=-std=c++2b -stdlib=libc++ -fexperimental-library
///output="A\tCCU"\n"C++": 23\n1, 2, 3
///hide
#include &lt;https://godbolt.org/z/Yv18W39YM/code/1&gt; // print
#include &lt;tuple&gt;
#include &lt;set&gt;

int main() {
  using namespace std::literals;
///unhide
std::println("{:?}",  "A\tCCU"s);
std::println("{:m}",  std::pair{"C++", 23});
std::println("{:n}",  std::set{1, 2, 3});
///hide
}
</code>
</pre>

<pre>
<code class="lang-HTML" data-trim data-line-numbers="1|2|3" data-fragment-index="1">
"A\tCCU"
"C++": 23
1, 2, 3
</code>
</pre>

----

<!-- .slide: data-background-image="resources/windowing.jpg" -->


## Windowing views

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #faf1be" -->

Source: [Mark Waugh](https://www.markwaugh.net/-/galleries/portfolios/travel/uk/manchester/manchester-skyline-at-night/-/medias/556a2fb0-19ce-4b1e-af43-ddb688731c23-manchester-city-centre-skyline/)

<!-- .element: style="font-size: 0.5em" -->

---

<!-- .slide: data-auto-animate -->

### `views::chunk`

<pre>
<code data-trim data-noescape class="lang-cpp">
///libs=fmt:trunk
///output=[[1, 2], [3, 4], [5]]
///hide
#include &lt;https://godbolt.org/z/K884c4hza/code/1&gt; // print
#include &lt;ranges&gt;
#include &lt;vector&gt;

int main() {
///unhide
std::vector v = {<mark>1, 2,</mark> 3, 4, 5};
std::println("{}", v | std::views::chunk(2));
///hide
}
</code>
</pre>

---

<!-- .slide: data-auto-animate -->

### `views::chunk`

<pre>
<code data-trim data-noescape class="lang-cpp">
///libs=fmt:trunk
///output=[[1, 2], [3, 4], [5]]
///hide
#include &lt;https://godbolt.org/z/K884c4hza/code/1&gt; // print
#include &lt;ranges&gt;
#include &lt;vector&gt;

int main() {
///unhide
std::vector v = {1, 2, <mark>3, 4,</mark> 5};
std::println("{}", v | std::views::chunk(2));
///hide
}
</code>
</pre>

---

<!-- .slide: data-auto-animate -->

### `views::chunk`

<pre>
<code data-trim data-noescape class="lang-cpp">
///libs=fmt:trunk
///output=[[1, 2], [3, 4], [5]]
///hide
#include &lt;https://godbolt.org/z/K884c4hza/code/1&gt; // print
#include &lt;ranges&gt;
#include &lt;vector&gt;

int main() {
///unhide
std::vector v = {1, 2, 3, 4, <mark>5</mark>};
std::println("{}", v | std::views::chunk(2));
///hide
}
</code>
</pre>

```
[[1, 2], [3, 4], [5]]
```

---

<!-- .slide: data-auto-animate -->

### `views::slide`

<pre>
<code data-trim data-noescape class="lang-cpp">
///libs=fmt:trunk
///output=[[1, 2], [2, 3], [3, 4], [4, 5]]
///hide
#include &lt;https://godbolt.org/z/K884c4hza/code/1&gt; // print
#include &lt;ranges&gt;
#include &lt;vector&gt;

int main() {
///unhide
std::vector v = {<mark>1, 2,</mark> 3, 4, 5};
std::println("{}", v | std::views::slide(2));
///hide
}
</code>
</pre>

---

<!-- .slide: data-auto-animate -->

### `views::slide`

<pre>
<code data-trim data-noescape class="lang-cpp">
///libs=fmt:trunk
///output=[[1, 2], [2, 3], [3, 4], [4, 5]]
///hide
#include &lt;https://godbolt.org/z/K884c4hza/code/1&gt; // print
#include &lt;ranges&gt;
#include &lt;vector&gt;

int main() {
///unhide
std::vector v = {1, <mark>2, 3,</mark> 4, 5};
std::println("{}", v | std::views::slide(2));
///hide
}
</code>
</pre>

---

<!-- .slide: data-auto-animate -->

### `views::slide`

<pre>
<code data-trim data-noescape class="lang-cpp">
///libs=fmt:trunk
///output=[[1, 2], [2, 3], [3, 4], [4, 5]]
///hide
#include &lt;https://godbolt.org/z/K884c4hza/code/1&gt; // print
#include &lt;ranges&gt;
#include &lt;vector&gt;

int main() {
///unhide
std::vector v = {1, 2, <mark>3, 4,</mark> 5};
std::println("{}", v | std::views::slide(2));
///hide
}
</code>
</pre>

---

<!-- .slide: data-auto-animate -->

### `views::slide`

<pre>
<code data-trim data-noescape class="lang-cpp">
///libs=fmt:trunk
///output=[[1, 2], [2, 3], [3, 4], [4, 5]]
///hide
#include &lt;https://godbolt.org/z/K884c4hza/code/1&gt; // print
#include &lt;ranges&gt;
#include &lt;vector&gt;

int main() {
///unhide
std::vector v = {1, 2, 3, <mark>4, 5</mark>};
std::println("{}", v | std::views::slide(2));
///hide
}
</code>
</pre>

```
[[1, 2], [2, 3], [3, 4], [4, 5]]
```

---

<!-- .slide: data-auto-animate -->

### `views::stride`

<pre>
<code data-trim data-noescape class="lang-cpp">
///libs=fmt:trunk
///output=[1, 3, 5]
///hide
#include &lt;https://godbolt.org/z/K884c4hza/code/1&gt; // print
#include &lt;ranges&gt;
#include &lt;vector&gt;

int main() {
///unhide
std::vector v = {<mark>1,</mark> 2, 3, 4, 5};
std::println("{}", v | std::views::stride(2));
///hide
}
</code>
</pre>

---

<!-- .slide: data-auto-animate -->

### `views::stride`

<pre>
<code data-trim data-noescape class="lang-cpp">
///libs=fmt:trunk
///output=[1, 3, 5]
///hide
#include &lt;https://godbolt.org/z/K884c4hza/code/1&gt; // print
#include &lt;ranges&gt;
#include &lt;vector&gt;

int main() {
///unhide
std::vector v = {1, 2, <mark>3,</mark> 4, 5};
std::println("{}", v | std::views::stride(2));
///hide
}
</code>
</pre>

---

<!-- .slide: data-auto-animate -->

### `views::stride`

<pre>
<code data-trim data-noescape class="lang-cpp">
///libs=fmt:trunk
///output=[1, 3, 5]
///hide
#include &lt;https://godbolt.org/z/K884c4hza/code/1&gt; // print
#include &lt;ranges&gt;
#include &lt;vector&gt;

int main() {
///unhide
std::vector v = {1, 2, 3, 4, <mark>5</mark>};
std::println("{}", v | std::views::stride(2));
///hide
}
</code>
</pre>

```
[1, 3, 5]
```

---

### generic?

| Algorithm | Step | Size | Partial |
|-----------|------|------|-----|
| generic	| n |	k	 | b |
| chunk	| k	| k	| true |
| slide	| 1	| k	| false |
| stride	| k |	1 |	N/A |

Notes: why not generic?

- no named arguments
- views would have redundant members


---

<!-- .slide: data-auto-animate -->

### windowed

```cpp
///libs=fmt:trunk
///output=[[0, 1, 2, 3, 4], [3, 4, 5, 6, 7], [6, 7, 8, 9, 10], [9, 10, 11, 12, 13], [12, 13, 14, 15, 16], [15, 16, 17, 18, 19]]
///hide
#include <https://godbolt.org/z/K884c4hza/code/1> // print
#include <ranges>

///unhide
namespace args {
struct windowed {
    std::size_t size;
    std::size_t stride;
};
}  // namespace args
constexpr auto windowed(args::windowed args) {
    using namespace std::views;
    return slide(args.size) | stride(args.stride);
}

///hide
int main() {
///unhide
std::println("{}",
              std::views::iota(0, 20) 
              | windowed({.size = 5, .stride = 3}));
///hide
}
```

---

<!-- .slide: data-auto-animate -->

### windowed

```cpp
///libs=fmt:trunk
///output=[[0, 1, 2, 3, 4], [3, 4, 5, 6, 7], [6, 7, 8, 9, 10], [9, 10, 11, 12, 13], [12, 13, 14, 15, 16], [15, 16, 17, 18, 19]]
///hide
#include <https://godbolt.org/z/K884c4hza/code/1> // print
#include <ranges>

///unhide
namespace args {
struct windowed {
    std::size_t size;
    std::size_t stride;
};
}  // namespace args
constexpr auto windowed(args::windowed args) {
    using namespace std::views;
    return slide(args.size) | stride(args.stride);
}

///hide
int main() {
///unhide
std::println("{}",
              std::views::iota(0, 20) 
              | windowed({.size = 5, .stride = 3}));
///hide
}
```

```
[[0, 1, 2, 3, 4], [3, 4, 5, 6, 7], [6, 7, 8, 9, 10], ...
```

---

<!-- .slide: data-auto-animate -->

### `views::chunk_by`

```cpp
///libs=fmt:trunk
///output=[[1, 2, 2, 3], [1, 2], [0, 4, 5], [2]]
///hide
#include <https://godbolt.org/z/K884c4hza/code/1> // print
#include <ranges>
#include <vector>

int main() {
///unhide
using namespace std::ranges;
std::vector v = {1, 2, 2, 3, 1, 2, 0, 4, 5, 2};
std::println("{}", v | views::chunk_by(less_equal{}));
///hide
}
```

<!-- .slide: data-auto-animate -->

---

<!-- .slide: data-auto-animate -->

### `views::chunk_by`

```cpp
///libs=fmt:trunk
///output=[[1, 2, 2, 3], [1, 2], [0, 4, 5], [2]]
///hide
#include <https://godbolt.org/z/K884c4hza/code/1> // print
#include <ranges>
#include <vector>

int main() {
///unhide
using namespace std::ranges;
std::vector v = {1, 2, 2, 3, 1, 2, 0, 4, 5, 2};
std::println("{}", v | views::chunk_by(less_equal{}));
///hide
}
```

```
[[1, 2, 2, 3], [1, 2], [0, 4, 5], [2]]
```

---

### unlike `range-v3`'s `group_by`


```cpp
///libs=rangesv3:trunk,fmt:trunk
///output=[[1, 2, 2, 3, 1, 2], [0, 4, 5, 2]]
///hide
#include <https://godbolt.org/z/K884c4hza/code/1> // print
#include <range/v3/view/group_by.hpp>
#include <vector>

int main() {
///unhide
using namespace ranges;
std::vector v = {1, 2, 2, 3, 1, 2, 0, 4, 5, 2};
std::println("{}", v | views::group_by(less_equal{}));
///hide
}
```

```
[[1, 2, 2, 3, 1, 2], [0, 4, 5, 2]]
```

Note: `group_by` is deprecated and replaced by `chunk_by`

---

<!-- .slide: class="aside" -->

### split months

```cpp
///hide
#include <chrono>
#include <ranges>

namespace views = std::views;
using date = std::chrono::year_month_day;

///unhide
auto by_month() {
  return views::chunk_by([](date a, date b) { 
    return a.month() == b.month(); 
  });
}
```

---

<!-- .slide: data-auto-animate class="aside" -->

### `dates()`

<pre data-id="dates" style="font-size: 0.45em;">
<code data-trim class="text" style="white-space: pre-wrap;">
[2023-01-01, 2023-01-02, 2023-01-03, 2023-01-04, 2023-01-05, 2023-01-06, 2023-01-07, 2023-01-08, 2023-01-09, 2023-01-10, 2023-01-11, 2023-01-12, 2023-01-13, 2023-01-14, 2023-01-15, 2023-01-16, 2023-01-17, 2023-01-18, 2023-01-19, 2023-01-20, 2023-01-21, 2023-01-22, 2023-01-23, 2023-01-24, 2023-01-25, 2023-01-26, 2023-01-27, 2023-01-28, 2023-01-29, 2023-01-30, 2023-01-31, 2023-02-01, 2023-02-02, 2023-02-03, 2023-02-04, 2023-02-05, 2023-02-06, 2023-02-07, 2023-02-08, 2023-02-09, 2023-02-10, 2023-02-11, 2023-02-12, 2023-02-13, 2023-02-14, 2023-02-15, 2023-02-16, 2023-02-17, 2023-02-18, 2023-02-19, 2023-02-20, 2023-02-21, 2023-02-22, 2023-02-23, 2023-02-24, 2023-02-25, 2023-02-26, 2023-02-27, 2023-02-28, 2023-03-01, 2023-03-02, 2023-03-03, 2023-03-04, 2023-03-05, 2023-03-06, 2023-03-07, 2023-03-08, 2023-03-09, 2023-03-10, 2023-03-11, 2023-03-12, 2023-03-13, 2023-03-14, 2023-03-15, 2023-03-16, 2023-03-17, 2023-03-18, 2023-03-19, 2023-03-20, 2023-03-21, 2023-03-22, 2023-03-23, 2023-03-24, 2023-03-25, 2023-03-26, 2023-03-27, 2023-03-28, 2023-03-29, 2023-03-30, 2023-03-31, 2023-04-01, 2023-04-02, 2023-04-03, 2023-04-04, 2023-04-05, 2023-04-06, 2023-04-07, 2023-04-08, 2023-04-09, 2023-04-10, 2023-04-11, 2023-04-12, 2023-04-13, 2023-04-14, 2023-04-15, 2023-04-16, 2023-04-17, 2023-04-18, 2023-04-19, 2023-04-20, 2023-04-21, 2023-04-22, 2023-04-23, 2023-04-24, 2023-04-25, 2023-04-26, 2023-04-27, 2023-04-28, 2023-04-29, 2023-04-30, 2023-05-01, 2023-05-02, 2023-05-03, 2023-05-04, 2023-05-05, 2023-05-06, 2023-05-07, 2023-05-08, 2023-05-09, 2023-05-10, 2023-05-11, 2023-05-12, 2023-05-13, 2023-05-14, 2023-05-15, 2023-05-16, 2023-05-17, 2023-05-18, 2023-05-19, 2023-05-20, 2023-05-21, 2023-05-22, 2023-05-23, 2023-05-24, 2023-05-25, 2023-05-26, 2023-05-27, 2023-05-28, 2023-05-29, 2023-05-30, 2023-05-31, 2023-06-01, 2023-06-02, 2023-06-03, 2023-06-04, 2023-06-05, 2023-06-06, 2023-06-07, 2023-06-08, 2023-06-09, 2023-06-10, 2023-06-11, 2023-06-12, 2023-06-13, 2023-06-14, 2023-06-15, 2023-06-16, 2023-06-17, 2023-06-18, 2023-06-19, 2023-06-20, 2023-06-21, 2023-06-22, 2023-06-23, 2023-06-24, 2023-06-25, 2023-06-26, 2023-06-27, 2023-06-28, 2023-06-29, 2023-06-30, 2023-07-01, 2023-07-02, 2023-07-03, 2023-07-04, 2023-07-05, 2023-07-06, 2023-07-07, 2023-07-08, 2023-07-09, 2023-07-10, 2023-07-11, 2023-07-12, 2023-07-13, 2023-07-14, 2023-07-15, 2023-07-16, 2023-07-17, 2023-07-18, 2023-07-19, 2023-07-20, 2023-07-21, 2023-07-22, 2023-07-23, 2023-07-24, 2023-07-25, 2023-07-26, 2023-07-27, 2023-07-28, 2023-07-29, 2023-07-30, 2023-07-31, 2023-08-01, 2023-08-02, 2023-08-03, 2023-08-04, 2023-08-05, 2023-08-06, 2023-08-07, 2023-08-08, 2023-08-09, 2023-08-10, 2023-08-11, 2023-08-12, 2023-08-13, 2023-08-14, 2023-08-15, 2023-08-16, 2023-08-17, 2023-08-18, 2023-08-19, 2023-08-20, 2023-08-21, 2023-08-22, 2023-08-23, 2023-08-24, 2023-08-25, 2023-08-26, 2023-08-27, 2023-08-28, 2023-08-29, 2023-08-30, 2023-08-31, 2023-09-01, 2023-09-02, 2023-09-03, 2023-09-04, 2023-09-05, 2023-09-06, 2023-09-07, 2023-09-08, 2023-09-09, 2023-09-10, 2023-09-11, 2023-09-12, 2023-09-13, 2023-09-14, 2023-09-15, 2023-09-16, 2023-09-17, 2023-09-18, 2023-09-19, 2023-09-20, 2023-09-21, 2023-09-22, 2023-09-23, 2023-09-24, 2023-09-25, 2023-09-26, 2023-09-27, 2023-09-28, 2023-09-29, 2023-09-30, 2023-10-01, 2023-10-02, 2023-10-03, 2023-10-04, 2023-10-05, 2023-10-06, 2023-10-07, 2023-10-08, 2023-10-09, 2023-10-10, 2023-10-11, 2023-10-12, 2023-10-13, 2023-10-14, 2023-10-15, 2023-10-16, 2023-10-17, 2023-10-18, 2023-10-19, 2023-10-20, 2023-10-21, 2023-10-22, 2023-10-23, 2023-10-24, 2023-10-25, 2023-10-26, 2023-10-27, 2023-10-28, 2023-10-29, 2023-10-30, 2023-10-31, 2023-11-01, 2023-11-02, 2023-11-03, 2023-11-04, 2023-11-05, 2023-11-06, 2023-11-07, 2023-11-08, 2023-11-09, 2023-11-10, 2023-11-11, 2023-11-12, 2023-11-13, 2023-11-14, 2023-11-15, 2023-11-16, 2023-11-17, 2023-11-18, 2023-11-19, 2023-11-20, 2023-11-21, 2023-11-22, 2023-11-23, 2023-11-24, 2023-11-25, 2023-11-26, 2023-11-27, 2023-11-28, 2023-11-29, 2023-11-30, 2023-12-01, 2023-12-02, 2023-12-03, 2023-12-04, 2023-12-05, 2023-12-06, 2023-12-07, 2023-12-08, 2023-12-09, 2023-12-10, 2023-12-11, 2023-12-12, 2023-12-13, 2023-12-14, 2023-12-15, 2023-12-16, 2023-12-17, 2023-12-18, 2023-12-19, 2023-12-20, 2023-12-21, 2023-12-22, 2023-12-23, 2023-12-24, 2023-12-25, 2023-12-26, 2023-12-27, 2023-12-28, 2023-12-29, 2023-12-30, 2023-12-31]
</code>
</pre>

---

<!-- .slide: data-auto-animate class="aside" -->

### `| by_month`

<pre data-id="dates" style="font-size: 0.45em;">
<code data-trim class="text" style="white-space: pre-wrap;">
[
  [2023-01-01, 2023-01-02, 2023-01-03, 2023-01-04, 2023-01-05, 2023-01-06, 2023-01-07, 2023-01-08, 2023-01-09, 2023-01-10, 2023-01-11, 2023-01-12, 2023-01-13, 2023-01-14, 2023-01-15, 2023-01-16, 2023-01-17, 2023-01-18, 2023-01-19, 2023-01-20, 2023-01-21, 2023-01-22, 2023-01-23, 2023-01-24, 2023-01-25, 2023-01-26, 2023-01-27, 2023-01-28, 2023-01-29, 2023-01-30, 2023-01-31], 
  [2023-02-01, 2023-02-02, 2023-02-03, 2023-02-04, 2023-02-05, 2023-02-06, 2023-02-07, 2023-02-08, 2023-02-09, 2023-02-10, 2023-02-11, 2023-02-12, 2023-02-13, 2023-02-14, 2023-02-15, 2023-02-16, 2023-02-17, 2023-02-18, 2023-02-19, 2023-02-20, 2023-02-21, 2023-02-22, 2023-02-23, 2023-02-24, 2023-02-25, 2023-02-26, 2023-02-27, 2023-02-28], 
  [2023-03-01, 2023-03-02, 2023-03-03, 2023-03-04, 2023-03-05, 2023-03-06, 2023-03-07, 2023-03-08, 2023-03-09, 2023-03-10, 2023-03-11, 2023-03-12, 2023-03-13, 2023-03-14, 2023-03-15, 2023-03-16, 2023-03-17, 2023-03-18, 2023-03-19, 2023-03-20, 2023-03-21, 2023-03-22, 2023-03-23, 2023-03-24, 2023-03-25, 2023-03-26, 2023-03-27, 2023-03-28, 2023-03-29, 2023-03-30, 2023-03-31], 
  [2023-04-01, 2023-04-02, 2023-04-03, 2023-04-04, 2023-04-05, 2023-04-06, 2023-04-07, 2023-04-08, 2023-04-09, 2023-04-10, 2023-04-11, 2023-04-12, 2023-04-13, 2023-04-14, 2023-04-15, 2023-04-16, 2023-04-17, 2023-04-18, 2023-04-19, 2023-04-20, 2023-04-21, 2023-04-22, 2023-04-23, 2023-04-24, 2023-04-25, 2023-04-26, 2023-04-27, 2023-04-28, 2023-04-29, 2023-04-30], 
  [2023-05-01, 2023-05-02, 2023-05-03, 2023-05-04, 2023-05-05, 2023-05-06, 2023-05-07, 2023-05-08, 2023-05-09, 2023-05-10, 2023-05-11, 2023-05-12, 2023-05-13, 2023-05-14, 2023-05-15, 2023-05-16, 2023-05-17, 2023-05-18, 2023-05-19, 2023-05-20, 2023-05-21, 2023-05-22, 2023-05-23, 2023-05-24, 2023-05-25, 2023-05-26, 2023-05-27, 2023-05-28, 2023-05-29, 2023-05-30, 2023-05-31], 
  [2023-06-01, 2023-06-02, 2023-06-03, 2023-06-04, 2023-06-05, 2023-06-06, 2023-06-07, 2023-06-08, 2023-06-09, 2023-06-10, 2023-06-11, 2023-06-12, 2023-06-13, 2023-06-14, 2023-06-15, 2023-06-16, 2023-06-17, 2023-06-18, 2023-06-19, 2023-06-20, 2023-06-21, 2023-06-22, 2023-06-23, 2023-06-24, 2023-06-25, 2023-06-26, 2023-06-27, 2023-06-28, 2023-06-29, 2023-06-30], 
  [2023-07-01, 2023-07-02, 2023-07-03, 2023-07-04, 2023-07-05, 2023-07-06, 2023-07-07, 2023-07-08, 2023-07-09, 2023-07-10, 2023-07-11, 2023-07-12, 2023-07-13, 2023-07-14, 2023-07-15, 2023-07-16, 2023-07-17, 2023-07-18, 2023-07-19, 2023-07-20, 2023-07-21, 2023-07-22, 2023-07-23, 2023-07-24, 2023-07-25, 2023-07-26, 2023-07-27, 2023-07-28, 2023-07-29, 2023-07-30, 2023-07-31], 
  [2023-08-01, 2023-08-02, 2023-08-03, 2023-08-04, 2023-08-05, 2023-08-06, 2023-08-07, 2023-08-08, 2023-08-09, 2023-08-10, 2023-08-11, 2023-08-12, 2023-08-13, 2023-08-14, 2023-08-15, 2023-08-16, 2023-08-17, 2023-08-18, 2023-08-19, 2023-08-20, 2023-08-21, 2023-08-22, 2023-08-23, 2023-08-24, 2023-08-25, 2023-08-26, 2023-08-27, 2023-08-28, 2023-08-29, 2023-08-30, 2023-08-31], 
  [2023-09-01, 2023-09-02, 2023-09-03, 2023-09-04, 2023-09-05, 2023-09-06, 2023-09-07, 2023-09-08, 2023-09-09, 2023-09-10, 2023-09-11, 2023-09-12, 2023-09-13, 2023-09-14, 2023-09-15, 2023-09-16, 2023-09-17, 2023-09-18, 2023-09-19, 2023-09-20, 2023-09-21, 2023-09-22, 2023-09-23, 2023-09-24, 2023-09-25, 2023-09-26, 2023-09-27, 2023-09-28, 2023-09-29, 2023-09-30], 
  [2023-10-01, 2023-10-02, 2023-10-03, 2023-10-04, 2023-10-05, 2023-10-06, 2023-10-07, 2023-10-08, 2023-10-09, 2023-10-10, 2023-10-11, 2023-10-12, 2023-10-13, 2023-10-14, 2023-10-15, 2023-10-16, 2023-10-17, 2023-10-18, 2023-10-19, 2023-10-20, 2023-10-21, 2023-10-22, 2023-10-23, 2023-10-24, 2023-10-25, 2023-10-26, 2023-10-27, 2023-10-28, 2023-10-29, 2023-10-30, 2023-10-31], 
  [2023-11-01, 2023-11-02, 2023-11-03, 2023-11-04, 2023-11-05, 2023-11-06, 2023-11-07, 2023-11-08, 2023-11-09, 2023-11-10, 2023-11-11, 2023-11-12, 2023-11-13, 2023-11-14, 2023-11-15, 2023-11-16, 2023-11-17, 2023-11-18, 2023-11-19, 2023-11-20, 2023-11-21, 2023-11-22, 2023-11-23, 2023-11-24, 2023-11-25, 2023-11-26, 2023-11-27, 2023-11-28, 2023-11-29, 2023-11-30], 
  [2023-12-01, 2023-12-02, 2023-12-03, 2023-12-04, 2023-12-05, 2023-12-06, 2023-12-07, 2023-12-08, 2023-12-09, 2023-12-10, 2023-12-11, 2023-12-12, 2023-12-13, 2023-12-14, 2023-12-15, 2023-12-16, 2023-12-17, 2023-12-18, 2023-12-19, 2023-12-20, 2023-12-21, 2023-12-22, 2023-12-23, 2023-12-24, 2023-12-25, 2023-12-26, 2023-12-27, 2023-12-28, 2023-12-29, 2023-12-30, 2023-12-31]
]
</code>
</pre>


----

<!-- .slide: data-background-image="resources/generators.jpg" -->


## Generator

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #7f5030" -->

Source: [Albanpix/Rex Features](https://www.theguardian.com/environment/gallery/2014/may/02/the-beauty-of-windfarms-in-pictures#img-11)

<!-- .element: style="font-size: 0.5em" -->

---

### `std::generator`

```cpp [1-8|10-11]
///hide
#if __has_include(<generator>)
///unhide
#include <generator>
///hide
#else
#include <https://godbolt.org/z/YrdWqrMTv/code/1> // generator
#endif
///unhide

std::generator<int> fib() {
    auto a = 0, b = 1;
    while (true) {
        co_yield std::exchange(a, std::exchange(b, a + b));
    }
}

///hide
int main() {
///unhide
auto rng = fib() | std::views::drop(6) | std::views::take(3);
return std::ranges::fold_left(std::move(rng), 0, std::plus{});
///hide
}
```

<!-- .element: style="font-size: 0.5em" -->

Note: `std::generator` is a move-only view which models `input_range` and has move-only iterators. This is because the coroutine state is a unique resource

---

<!-- .slide: data-auto-animate -->

### `std::generator`

```cpp [1-6|8-12]
///hide
#include <memory_resource>

///unhide
template<
    class Ref,
    class V = void,
    class Allocator = void
>
class generator;

namespace pmr {
template< class Ref, class V = void >
using generator =
    generator<Ref, V, std::pmr::polymorphic_allocator<>>;
}
```

<!-- .element: data-id="code" style="font-size: 0.4em" -->

---

<!-- .slide: data-auto-animate -->

### `std::generator`

```cpp [8-10]
///hide
#include <memory_resource>
#include <type_traits>

using std::is_void_v;
using std::remove_cvref_t;
using std::is_reference_v;
using std::conditional_t;
///unhide
template<
    class Ref,
    class V = void,
    class Allocator = void
>
class generator {

using Value = conditional_t<is_void_v<V>, remove_cvref_t<Ref>, V>; 
using Reference = conditional_t<is_void_v<V>, Ref&&, Ref>; 
using Yielded = conditional_t<is_reference_v<Reference>, Reference, const Reference&>;

};
```

<!-- .element: data-id="code" style="font-size: 0.4em" -->

Note: 
- `Yielded`: the type that should be passed to `co_yield`.
- `Reference`: the type that's returned when iterating
- `Value`: used primarily by `ranges::to`

`T&&` is the default mainly for performance. See [p2529](https://wg21.link/p2529)


---

### yielding ranges

```cpp
///libs=fmt:trunk
///output=["Hello", "Elements", "Of"]
///hide
#include <https://godbolt.org/z/YrdWqrMTv/code/1> // generator
#include <https://godbolt.org/z/1jWT1a5KT/code/1> // print
#include <vector>

///unhide
std::generator<std::string_view> elements() { 
  std::vector<std::string> v = {"Hello", "Elements", "Of"}; 
  co_yield std::ranges::elements_of(v); 
}
///hide
int main() {
  std::println("{}", elements());
}
```

---

### recursive generator

```cpp
///hide
#include <https://godbolt.org/z/YrdWqrMTv/code/1> // generator

///unhide
struct Tree {
    Tree* left;
    Tree* right;
    int value;
};

std::generator<int> visit(const Tree& tree) {
    if (tree.left) 
      co_yield std::ranges::elements_of(visit(*tree.left));
    co_yield tree.value;
    if (tree.right) 
      co_yield std::ranges::elements_of(visit(*tree.right));
}
```

---

### implementing new adaptors

```cpp [1-12|14-17]
///libs=fmt:trunk
///hide
#include <https://godbolt.org/z/YrdWqrMTv/code/1> // generator
#include <https://godbolt.org/z/K884c4hza/code/1> // print
#include <array>
#include <vector>

///unhide
template <std::ranges::range Head, std::ranges::range... Tail>
inline std::generator<
    std::common_type_t<std::ranges::range_reference_t<Head>,
                            std::ranges::range_reference_t<Tail>...>,
    std::common_type_t<std::ranges::range_value_t<Head>,
                       std::ranges::range_value_t<Tail>...>>
concat(Head head, Tail... tail) {
    co_yield std::ranges::elements_of(std::move(head));
    if constexpr (sizeof...(tail) > 0) {
        co_yield std::ranges::elements_of(concat(std::move(tail)...));
    }
};

///hide
int main() {  
///unhide
std::vector<int> v1{1, 2, 3}, v2{4, 5}, v3{};
std::array<const int,3> a{6, 7, 8};
auto s = std::views::single(9);
std::println("{}", concat(v1, v2, v3, a, s));
///hide
}
```

<!-- .element: style="font-size: 0.45em" -->

----

<!-- .slide: data-background-image="resources/pov.jpg" -->

## changing point of view

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #aab283" -->

Source: [Mark Sutcliffe](https://www.countryfile.com/go-outdoors/walks/top-10-easy-mountains-for-beginners/)

<!-- .element: style="font-size: 0.5em" -->

---

### Is this a view

```cpp
///hide
#include <https://godbolt.org/z/YrdWqrMTv/code/1> // generator

///unhide
template <std::ranges::range Head, std::ranges::range... Tail>
inline std::generator<
    std::common_type_t<std::ranges::range_reference_t<Head>,
                            std::ranges::range_reference_t<Tail>...>,
    std::common_type_t<std::ranges::range_value_t<Head>,
                       std::ranges::range_value_t<Tail>...>>
concat(Head head, Tail... tail) {
    co_yield std::ranges::elements_of(std::move(head));
    if constexpr (sizeof...(tail) > 0) {
        co_yield std::ranges::elements_of(concat(std::move(tail)...));
    }
};
```

<!-- .element: style="font-size: 0.45em" -->

---

<!-- .slide: data-auto-animate -->

### what is a view?

```cpp
///fails=concept definition syntax is 'concept <name> = <expr>'
concept Range<typename T> =
    Iterable<T> && 
    Semiregular<T> && 
    is_range<T>::value;

// Copying and assignment execute in constant time
// is_range detects shallow constness
```

<!-- .element: data-id="code" -->
    
[N4128](https:wg21.link//N4128): Ranges for the Standard Library

---

<!-- .slide: data-auto-animate -->

### what is a view?

```cpp
///hide
#include <ranges>

template<typename T>
concept Range = std::ranges::range<T>;
template<typename T>
concept Semiregular = std::semiregular<T>;
using std::ranges::enable_view;

///unhide
template<class T> 
concept View = 
    Range<T> && 
    Semiregular<T> && 
    enable_view<T>;

// has constant time copy, move and assignment operators
// enable_view detects shallow constness
```

<!-- .element: data-id="code" -->
    
[P0896](https://wg21.link/P0896): The One Ranges Proposal 

---

<!-- .slide: data-auto-animate -->

### what is a view?

```cpp
///hide
#include <ranges>

using std::ranges::range;
using std::ranges::enable_view;
using std::movable;
using std::default_initializable;

///unhide
template<class T>
concept view =
    range<T> && 
    movable<T> && 
    default_initializable<T> && 
    enable_view<T>;

// has constant time move construction, 
// move assignment and destruction operators
// enable_view detects shallow constness
```

<!-- .element: data-id="code" -->
    
[P1456](https://wg21.link/P1456): Move-only views 

---

<!-- .slide: data-auto-animate -->

### what is a view?

```cpp
///hide
#include <ranges>

using std::ranges::range;
using std::ranges::enable_view;
using std::movable;
using std::default_initializable;

///unhide
template<class T>
concept view =
    range<T> && 
    movable<T> && 
    default_initializable<T> && 
    enable_view<T>;

// has constant time move construction, 
// move assignment and destruction operators
```

<!-- .element: data-id="code" -->
    
[LWG3326](https://wg21.link/LWG3326): `enable_view` has false positives

---

<!-- .slide: data-auto-animate -->

### what is a view?

```cpp
///hide
#include <ranges>

using std::ranges::range;
using std::ranges::enable_view;
using std::movable;

///unhide
template<class T>
concept view =
    range<T> && 
    movable<T> && 
    enable_view<T>;

// has constant time move construction, 
// move assignment and destruction operators
```

<!-- .element: data-id="code" -->
    
[P2325](https://wg21.link/P2325): Views should not be required to be default constructible 

---

### viewing r-value containers

<!-- .slide: data-auto-animate -->

```cpp
///hide
#include <vector>
#include <ranges>

namespace views = std::views;

///unhide
std::vector<int> get_ints();

auto rng = get_ints()
         | views::filter([](int i){ return i > 0; })
         | views::transform([](int i){ return i * i; });
```

<!-- .element: data-id="code" -->

---

### viewing r-value containers

<!-- .slide: data-auto-animate -->

```cpp
///hide
#include <vector>
#include <ranges>

namespace views = std::views;

///unhide
std::vector<int> get_ints();

auto ints = get_ints();
auto rng = ints
         | views::filter([](int i){ return i > 0; })
         | views::transform([](int i){ return i * i; });
```

<!-- .element: data-id="code" -->

---

### `owning_view`

```cpp
///hide
#include <ranges>

///unhide
template<std::ranges::range T>
struct owning_view {
    T t;
    
    owning_view(T t) : t(std::move(t)) { }
    
    owning_view(owning_view const&) = delete;
    owning_view(owning_view&&) = default;
    owning_view& operator=(owning_view const&) = delete;
    owning_view& operator=(owning_view&&) = default;
    
    auto begin() { return begin(t); }
    auto end()   { return end(t); }
};
```

---

### viewing r-value containers

<!-- .slide: data-auto-animate -->

```cpp
///hide
#include <vector>
#include <ranges>

using std::ranges::owning_view;

namespace views = std::views;

///unhide
std::vector<int> get_ints();

auto rng = owning_view{get_ints()}
         | views::filter([](int i){ return i > 0; })
         | views::transform([](int i){ return i * i; });
```

<!-- .element: data-id="code" -->

---

### `views::all`

> Given a subexpression `E`, the expression `views​::​all(E)` is expression-equivalent to:
> - `decay-copy(E)` if the decayed type of `E` models view.
> - Otherwise, `ref_­view{E}` if that expression is well-formed.
> - Otherwise, <mark>`owning_­view{E}`</mark>.

<!-- .element: style="text-align: left; width: 100%; font-size: 0.8em" -->

---

### what is a view?

```cpp
///hide
#include <ranges>

using std::ranges::range;
using std::ranges::enable_view;
using std::movable;

///unhide
template<class T>
concept view =
    range<T> && 
    movable<T> && 
    enable_view<T>;

// 1. has O(1) move construction
// 2. move assignment is no more complex than 
//    destruction followed by move construction
// 3. N copies of M elements have O(N+M) destruction 
//    [implies that a moved-from object has O(1) destruction]
// 4. if copy construcible, has O(1) copy construction
// 5. if copyable, copy assignment is no more complex than 
//    destruction followed by copy construction.
```
    
[P2415](https://wg21.link/P2415): What is a `view`? 

---

### viewing r-value containers

<!-- .slide: data-auto-animate -->

```cpp
///hide
#include <vector>
#include <ranges>

namespace views = std::views;

///unhide
std::vector<int> get_ints();

auto rng = get_ints()
         | views::filter([](int i){ return i > 0; })
         | views::transform([](int i){ return i * i; });
```

----

<!-- .slide: data-background-image="resources/piping.jpg" -->

## Piping user-defined range adaptors

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #fea98e" -->


Source: [mybestplace.com](https://www.mybestplace.com/en/article/singing-ringing-tree-the-tree-that-sings-with-the-blowing-wind)

<!-- .element: style="font-size: 0.5em" -->

---

### range adaptor object

`$$
 adaptor(range, args...) \equiv \\
 adaptor(args...)(range) \equiv \\
 range\:|\:adaptor(args...) 
$$`

<div class="fragment">

e.g. `views::transform`

</div>

---

### range adaptor closure object

<div class="r-stack">

<div class="fragment fade-out" data-fragment-index="1" style="width: 100%">

`$$ C(R) \equiv R\:|\:C $$`

<div class="fragment fade-in-then-out" data-fragment-index="0">

e.g. `views::reverse`, `views::transform(f)`

</div>

</div>

<div class="fragment fade-in" data-fragment-index="1" style="width: 100%">

`$$ R\:|\:C\:|\:D \equiv R\:|\:(C\:|\:D) $$`

<div class="fragment fade-in" data-fragment-index="2">

```cpp
///hide
#include <ranges>
namespace views = std::views;

///unhide
auto reverse_trasform(auto f) {
  return views::reverse | views::transform(f);
}
```

</div>

</div>

</div>


Note: a unary function object that accepts a `viewable_­range` argument and returns a view 
  such that the above equation hold


---

## more complex example

```cpp [1 -4|6]
///libs=fmt:trunk
///fails=no match for 'operator|'
///hide
#include <ranges>
#include <vector>
#include "https://godbolt.org/z/1vfTWKdcf/code/1" // generator
#include "https://godbolt.org/z/n43nMfj58/code/1" // concat
#include "https://godbolt.org/z/K884c4hza/code/1" // print
namespace ranges = std::ranges;
namespace views = std::views;

///unhide
auto reverse_tail = [](ranges::forward_range auto&& R) {
  return concat(R | views::take(1), 
                R | views::drop(1) | views::reverse);
};

///hide
int main() {
///unhide
std::println("{}", std::vector{1, 2, 3, 4} | reverse_tail);
///hide
}
```

---

### `ranges::range_adaptor_closure`

```cpp [1-7|2]
///libs=fmt:trunk
///fails=no match for 'operator|'
///hide
#include <ranges>
#include <vector>
#include "https://godbolt.org/z/1vfTWKdcf/code/1" // generator
#include "https://godbolt.org/z/n43nMfj58/code/1" // concat
#include "https://godbolt.org/z/K884c4hza/code/1" // print
#include "https://godbolt.org/z/9KE5MoMKc/code/1" // closure
namespace ranges = std::ranges;
namespace views = std::views;

///unhide
struct reverse_tail_fn 
  : std::ranges::range_adaptor_closure<reverse_tail_fn> {
  auto operator()(ranges::forward_range auto&& R) {
    return concat(R | views::take(1), 
                  R | views::drop(1) | views::reverse);
  };
}  reverse_tail;

///hide
int main() {
///unhide
std::println("{}", std::vector{1, 2, 3, 4} | reverse_tail);
///hide
}
```

---

### back to lambda

```c++ [1-13|15-20]
///libs=fmt:trunk
///hide
#include <ranges>
#include <vector>
#include "https://godbolt.org/z/1vfTWKdcf/code/1" // generator
#include "https://godbolt.org/z/n43nMfj58/code/1" // concat
#include "https://godbolt.org/z/K884c4hza/code/1" // print
namespace ranges = std::ranges;
namespace views = std::views;

namespace std::ranges {

template <typename D>
using range_adaptor_closure = views::__adaptor::_RangeAdaptorClosure;
}

///unhide
template <typename F>
class closure 
  : public std::ranges::range_adaptor_closure<closure<F>> {
    F f;
public:
    constexpr closure(F f) : f(f) { }

    template <std::ranges::viewable_range R>
        requires std::invocable<F const&, R>
    constexpr auto operator()(R&& r) const {
        return f(std::forward<R>(r));
    }
};

closure reverse_tail = [](ranges::forward_range auto&& R) {
  return concat(R | views::take(1), 
                R | views::drop(1) | views::reverse);
};

///hide
int main() {
///unhide
std::println("{}", std::vector{1, 2, 3, 4} | reverse_tail);
///hide
}
```

---

### case for adaptor

```cpp [18-24|1-16|26-27]
///libs=fmt:trunk
///hide
#include <ranges>
#include <vector>
#include "https://godbolt.org/z/1vfTWKdcf/code/1" // generator
#include "https://godbolt.org/z/n43nMfj58/code/1" // concat
#include "https://godbolt.org/z/K884c4hza/code/1" // print
namespace ranges = std::ranges;
namespace views = std::views;

namespace std {

constexpr inline auto bind_back(auto &&f, auto &&...args) {
    return [=](auto &&...args2) { return f(args2..., args...); };
}

namespace ranges {

template <typename D>
using range_adaptor_closure = views::__adaptor::_RangeAdaptorClosure;
}

}  // namespace std

template <typename F>
class closure 
  : public std::ranges::range_adaptor_closure<closure<F>> {
    F f;
public:
    constexpr closure(F f) : f(f) { }

    template <std::ranges::viewable_range R>
        requires std::invocable<F const&, R>
    constexpr auto operator()(R&& r) const {
        return f(std::forward<R>(r));
    }
};

///unhide
template <typename F>
class adaptor {
    F f;
public:
    constexpr adaptor(F f) : f(f) { }

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const {
        if constexpr (std::invocable<F const&, Args...>) {
            return f(std::forward<Args>(args)...);
        } else {
            return closure(std::bind_back(f, 
              std::forward<Args>(args)...));
        }
    }
};

adaptor reverse_transform_tail = 
  [](ranges::forward_range auto&& r, auto&& f) {
  return concat(r | views::take(1), 
                r | views::drop(1) 
                  | views::reverse 
                  | views::transform(f));
};

///hide
int main() {
///unhide
std::println("{}", std::vector{1, 2, 3, 4} 
  | reverse_transform_tail(std::identity{}));
///hide
}
```

---

<!-- .slide: class="aside" -->

### formatting months

```cpp [30-39|34-37|28|1-5|9-24]
///hide
#include <ranges>
#include <string>
#include <chrono>

#include <https://godbolt.org/z/MfhP19KfT/code/1> // closure
#include <https://godbolt.org/z/o46E4a9ds/code/1> // to

namespace ranges = std::ranges;
namespace views = std::views;

namespace detail {

template <typename Rng, typename T, int level>
consteval bool recursive_range_of() {
  if constexpr (level == 1)
    if constexpr (ranges::range<Rng>)
      return std::same_as<ranges::range_value_t<Rng>, T>;
    else
      return false;
  else
    return recursive_range_of<ranges::range_value_t<Rng>, T, level - 1>();
}

}  // namespace detail

template <typename Rng, typename T, int extent>
concept nested_range_of = detail::recursive_range_of<Rng, T, extent>();

template <typename Rng, typename T>
concept range_of = nested_range_of<Rng, T, 1>;

using date = std::chrono::year_month_day;

namespace detail {
template <typename Rng>
auto format_as_string(const std::string_view fmt, Rng&& rng) {
  return std::vformat(fmt, std::make_format_args(std::forward<Rng>(rng) |
                                                 ranges::to<std::string>()));
}

auto start_of_week(date d) {
  const auto from_sunday = std::chrono::weekday{d} - std::chrono::Sunday;
  return std::chrono::sys_days{d} - from_sunday;
}
}  // namespace detail

using std::views::chunk_by;

///unhide
auto by_week() {
  return chunk_by([](date a, date b) {
    return detail::start_of_week(a) == detail::start_of_week(b);
  });
}

// In:  range<range<date>>: month grouped by weeks.
// Out: range<std::string>: month with formatted weeks.
inline constexpr closure format_weeks =
    [](nested_range_of<date, 2> auto month) {
      const auto format_day = [](date d) {
        return std::format("{:3}", d.day());
      };
      const auto format_week = views::transform(format_day) | views::join;
      return concat(
          month | views::take(1) |
              views::transform([&](range_of<date> auto week) {
                return detail::format_as_string("{:>21} ", week | format_week);
              }),
          month | views::drop(1) |
              views::transform([&](range_of<date> auto week) {
                return detail::format_as_string("{:22}", week | format_week);
              }));
    };

// Return a formatted string with the title of the month
// corresponding to a date.
std::string month_title(date d) { return std::format("{:^22%B}", d.month()); }

auto layout_months() {
  return views::transform([](range_of<date> auto month) {
    const auto week_count = ranges::distance(month | by_week());
    static const std::string empty_week(22, ' ');
    return concat(
        views::single(month_title(month.front())),
        month | by_week() | format_weeks,
        views::repeat(empty_week, 6 - week_count) | ranges::to<std::vector>());
  });
}
```

<!-- .element: style="font-size: 0.4em" -->

---

<!-- .slide: data-auto-animate class="aside" -->

### `| by_month`

<pre data-id="dates" style="font-size: 0.45em;">
<code data-trim class="text" style="white-space: pre-wrap;">
[
  [2023-01-01, 2023-01-02, 2023-01-03, 2023-01-04, 2023-01-05, 2023-01-06, 2023-01-07, 2023-01-08, 2023-01-09, 2023-01-10, 2023-01-11, 2023-01-12, 2023-01-13, 2023-01-14, 2023-01-15, 2023-01-16, 2023-01-17, 2023-01-18, 2023-01-19, 2023-01-20, 2023-01-21, 2023-01-22, 2023-01-23, 2023-01-24, 2023-01-25, 2023-01-26, 2023-01-27, 2023-01-28, 2023-01-29, 2023-01-30, 2023-01-31], 
  [2023-02-01, 2023-02-02, 2023-02-03, 2023-02-04, 2023-02-05, 2023-02-06, 2023-02-07, 2023-02-08, 2023-02-09, 2023-02-10, 2023-02-11, 2023-02-12, 2023-02-13, 2023-02-14, 2023-02-15, 2023-02-16, 2023-02-17, 2023-02-18, 2023-02-19, 2023-02-20, 2023-02-21, 2023-02-22, 2023-02-23, 2023-02-24, 2023-02-25, 2023-02-26, 2023-02-27, 2023-02-28], 
  [2023-03-01, 2023-03-02, 2023-03-03, 2023-03-04, 2023-03-05, 2023-03-06, 2023-03-07, 2023-03-08, 2023-03-09, 2023-03-10, 2023-03-11, 2023-03-12, 2023-03-13, 2023-03-14, 2023-03-15, 2023-03-16, 2023-03-17, 2023-03-18, 2023-03-19, 2023-03-20, 2023-03-21, 2023-03-22, 2023-03-23, 2023-03-24, 2023-03-25, 2023-03-26, 2023-03-27, 2023-03-28, 2023-03-29, 2023-03-30, 2023-03-31], 
  [2023-04-01, 2023-04-02, 2023-04-03, 2023-04-04, 2023-04-05, 2023-04-06, 2023-04-07, 2023-04-08, 2023-04-09, 2023-04-10, 2023-04-11, 2023-04-12, 2023-04-13, 2023-04-14, 2023-04-15, 2023-04-16, 2023-04-17, 2023-04-18, 2023-04-19, 2023-04-20, 2023-04-21, 2023-04-22, 2023-04-23, 2023-04-24, 2023-04-25, 2023-04-26, 2023-04-27, 2023-04-28, 2023-04-29, 2023-04-30], 
  [2023-05-01, 2023-05-02, 2023-05-03, 2023-05-04, 2023-05-05, 2023-05-06, 2023-05-07, 2023-05-08, 2023-05-09, 2023-05-10, 2023-05-11, 2023-05-12, 2023-05-13, 2023-05-14, 2023-05-15, 2023-05-16, 2023-05-17, 2023-05-18, 2023-05-19, 2023-05-20, 2023-05-21, 2023-05-22, 2023-05-23, 2023-05-24, 2023-05-25, 2023-05-26, 2023-05-27, 2023-05-28, 2023-05-29, 2023-05-30, 2023-05-31], 
  [2023-06-01, 2023-06-02, 2023-06-03, 2023-06-04, 2023-06-05, 2023-06-06, 2023-06-07, 2023-06-08, 2023-06-09, 2023-06-10, 2023-06-11, 2023-06-12, 2023-06-13, 2023-06-14, 2023-06-15, 2023-06-16, 2023-06-17, 2023-06-18, 2023-06-19, 2023-06-20, 2023-06-21, 2023-06-22, 2023-06-23, 2023-06-24, 2023-06-25, 2023-06-26, 2023-06-27, 2023-06-28, 2023-06-29, 2023-06-30], 
  [2023-07-01, 2023-07-02, 2023-07-03, 2023-07-04, 2023-07-05, 2023-07-06, 2023-07-07, 2023-07-08, 2023-07-09, 2023-07-10, 2023-07-11, 2023-07-12, 2023-07-13, 2023-07-14, 2023-07-15, 2023-07-16, 2023-07-17, 2023-07-18, 2023-07-19, 2023-07-20, 2023-07-21, 2023-07-22, 2023-07-23, 2023-07-24, 2023-07-25, 2023-07-26, 2023-07-27, 2023-07-28, 2023-07-29, 2023-07-30, 2023-07-31], 
  [2023-08-01, 2023-08-02, 2023-08-03, 2023-08-04, 2023-08-05, 2023-08-06, 2023-08-07, 2023-08-08, 2023-08-09, 2023-08-10, 2023-08-11, 2023-08-12, 2023-08-13, 2023-08-14, 2023-08-15, 2023-08-16, 2023-08-17, 2023-08-18, 2023-08-19, 2023-08-20, 2023-08-21, 2023-08-22, 2023-08-23, 2023-08-24, 2023-08-25, 2023-08-26, 2023-08-27, 2023-08-28, 2023-08-29, 2023-08-30, 2023-08-31], 
  [2023-09-01, 2023-09-02, 2023-09-03, 2023-09-04, 2023-09-05, 2023-09-06, 2023-09-07, 2023-09-08, 2023-09-09, 2023-09-10, 2023-09-11, 2023-09-12, 2023-09-13, 2023-09-14, 2023-09-15, 2023-09-16, 2023-09-17, 2023-09-18, 2023-09-19, 2023-09-20, 2023-09-21, 2023-09-22, 2023-09-23, 2023-09-24, 2023-09-25, 2023-09-26, 2023-09-27, 2023-09-28, 2023-09-29, 2023-09-30], 
  [2023-10-01, 2023-10-02, 2023-10-03, 2023-10-04, 2023-10-05, 2023-10-06, 2023-10-07, 2023-10-08, 2023-10-09, 2023-10-10, 2023-10-11, 2023-10-12, 2023-10-13, 2023-10-14, 2023-10-15, 2023-10-16, 2023-10-17, 2023-10-18, 2023-10-19, 2023-10-20, 2023-10-21, 2023-10-22, 2023-10-23, 2023-10-24, 2023-10-25, 2023-10-26, 2023-10-27, 2023-10-28, 2023-10-29, 2023-10-30, 2023-10-31], 
  [2023-11-01, 2023-11-02, 2023-11-03, 2023-11-04, 2023-11-05, 2023-11-06, 2023-11-07, 2023-11-08, 2023-11-09, 2023-11-10, 2023-11-11, 2023-11-12, 2023-11-13, 2023-11-14, 2023-11-15, 2023-11-16, 2023-11-17, 2023-11-18, 2023-11-19, 2023-11-20, 2023-11-21, 2023-11-22, 2023-11-23, 2023-11-24, 2023-11-25, 2023-11-26, 2023-11-27, 2023-11-28, 2023-11-29, 2023-11-30], 
  [2023-12-01, 2023-12-02, 2023-12-03, 2023-12-04, 2023-12-05, 2023-12-06, 2023-12-07, 2023-12-08, 2023-12-09, 2023-12-10, 2023-12-11, 2023-12-12, 2023-12-13, 2023-12-14, 2023-12-15, 2023-12-16, 2023-12-17, 2023-12-18, 2023-12-19, 2023-12-20, 2023-12-21, 2023-12-22, 2023-12-23, 2023-12-24, 2023-12-25, 2023-12-26, 2023-12-27, 2023-12-28, 2023-12-29, 2023-12-30, 2023-12-31]
]
</code>
</pre>

---

<!-- .slide: data-auto-animate class="aside" -->

### `| layout_months`

<pre data-id="dates" style="font-size: 0.45em;">
<code data-trim class="text" style="white-space: pre-wrap;">
[
  ["       January        ", "01 02 03 04 05 06 07  ", "08 09 10 11 12 13 14  ", "15 16 17 18 19 20 21  ", "22 23 24 25 26 27 28  ", "29 30 31              ", "                      "], 
  ["       February       ", "         01 02 03 04  ", "05 06 07 08 09 10 11  ", "12 13 14 15 16 17 18  ", "19 20 21 22 23 24 25  ", "26 27 28              ", "                      "], 
  ["        March         ", "         01 02 03 04  ", "05 06 07 08 09 10 11  ", "12 13 14 15 16 17 18  ", "19 20 21 22 23 24 25  ", "26 27 28 29 30 31     ", "                      "], 
  ["        April         ", "                  01  ", "02 03 04 05 06 07 08  ", "09 10 11 12 13 14 15  ", "16 17 18 19 20 21 22  ", "23 24 25 26 27 28 29  ", "30                    "], 
  ["         May          ", "   01 02 03 04 05 06  ", "07 08 09 10 11 12 13  ", "14 15 16 17 18 19 20  ", "21 22 23 24 25 26 27  ", "28 29 30 31           ", "                      "], 
  ["         June         ", "            01 02 03  ", "04 05 06 07 08 09 10  ", "11 12 13 14 15 16 17  ", "18 19 20 21 22 23 24  ", "25 26 27 28 29 30     ", "                      "], 
  ["         July         ", "                  01  ", "02 03 04 05 06 07 08  ", "09 10 11 12 13 14 15  ", "16 17 18 19 20 21 22  ", "23 24 25 26 27 28 29  ", "30 31                 "], 
  ["        August        ", "      01 02 03 04 05  ", "06 07 08 09 10 11 12  ", "13 14 15 16 17 18 19  ", "20 21 22 23 24 25 26  ", "27 28 29 30 31        ", "                      "], 
  ["      September       ", "               01 02  ", "03 04 05 06 07 08 09  ", "10 11 12 13 14 15 16  ", "17 18 19 20 21 22 23  ", "24 25 26 27 28 29 30  ", "                      "], 
  ["       October        ", "01 02 03 04 05 06 07  ", "08 09 10 11 12 13 14  ", "15 16 17 18 19 20 21  ", "22 23 24 25 26 27 28  ", "29 30 31              ", "                      "], 
  ["       November       ", "         01 02 03 04  ", "05 06 07 08 09 10 11  ", "12 13 14 15 16 17 18  ", "19 20 21 22 23 24 25  ", "26 27 28 29 30        ", "                      "], 
  ["       December       ", "               01 02  ", "03 04 05 06 07 08 09  ", "10 11 12 13 14 15 16  ", "17 18 19 20 21 22 23  ", "24 25 26 27 28 29 30  ", "31                    "]
]
</code>
</pre>

---

<!-- .slide: data-auto-animate class="aside" -->

### `| chunk(3)`

<pre data-id="dates" style="font-size: 0.45em;">
<code data-trim class="text" style="white-space: pre-wrap;">
[
  [
    ["       January        ", " 01 02 03 04 05 06 07 ", " 08 09 10 11 12 13 14 ", " 15 16 17 18 19 20 21 ", " 22 23 24 25 26 27 28 ", " 29 30 31             ", "                      "], 
    ["       February       ", "          01 02 03 04 ", " 05 06 07 08 09 10 11 ", " 12 13 14 15 16 17 18 ", " 19 20 21 22 23 24 25 ", " 26 27 28             ", "                      "], 
    ["        March         ", "          01 02 03 04 ", " 05 06 07 08 09 10 11 ", " 12 13 14 15 16 17 18 ", " 19 20 21 22 23 24 25 ", " 26 27 28 29 30 31    ", "                      "]
  ], 
  [
    ["        April         ", "                   01 ", " 02 03 04 05 06 07 08 ", " 09 10 11 12 13 14 15 ", " 16 17 18 19 20 21 22 ", " 23 24 25 26 27 28 29 ", " 30                   "], 
    ["         May          ", "    01 02 03 04 05 06 ", " 07 08 09 10 11 12 13 ", " 14 15 16 17 18 19 20 ", " 21 22 23 24 25 26 27 ", " 28 29 30 31          ", "                      "], 
    ["         June         ", "             01 02 03 ", " 04 05 06 07 08 09 10 ", " 11 12 13 14 15 16 17 ", " 18 19 20 21 22 23 24 ", " 25 26 27 28 29 30    ", "                      "]
  ], 
  [
    ["         July         ", "                   01 ", " 02 03 04 05 06 07 08 ", " 09 10 11 12 13 14 15 ", " 16 17 18 19 20 21 22 ", " 23 24 25 26 27 28 29 ", " 30 31                "], 
    ["        August        ", "       01 02 03 04 05 ", " 06 07 08 09 10 11 12 ", " 13 14 15 16 17 18 19 ", " 20 21 22 23 24 25 26 ", " 27 28 29 30 31       ", "                      "], 
    ["      September       ", "                01 02 ", " 03 04 05 06 07 08 09 ", " 10 11 12 13 14 15 16 ", " 17 18 19 20 21 22 23 ", " 24 25 26 27 28 29 30 ", "                      "]
  ], 
  [
    ["       October        ", " 01 02 03 04 05 06 07 ", " 08 09 10 11 12 13 14 ", " 15 16 17 18 19 20 21 ", " 22 23 24 25 26 27 28 ", " 29 30 31             ", "                      "], 
    ["       November       ", "          01 02 03 04 ", " 05 06 07 08 09 10 11 ", " 12 13 14 15 16 17 18 ", " 19 20 21 22 23 24 25 ", " 26 27 28 29 30       ", "                      "], 
    ["       December       ", "                01 02 ", " 03 04 05 06 07 08 09 ", " 10 11 12 13 14 15 16 ", " 17 18 19 20 21 22 23 ", " 24 25 26 27 28 29 30 ", " 31                   "]
  ]
] 
</code>
</pre>

----

<!-- .slide: data-background-image="resources/zipping.jpg" -->

## zipping

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #6b5a52" -->


Source: [Simon Emmett Photography](https://www.countryfile.com/go-outdoors/best-fossil-hunting-destinations-uk/)

<!-- .element: style="font-size: 0.5em" -->

---

<!-- .slide: data-auto-animate -->

### grading

```cpp
///libs=fmt:trunk
///opts+=-Wno-array-bounds -Wno-stringop-overflow
///output=[("Joan", 8.5), ("Ben", 7.1), ("Gina", 9), ("Tim", 9.5)]
///hide
#include <vector>
#include <ranges>
#include <string>
#include "https://godbolt.org/z/K884c4hza/code/1" // print

int main() {
using namespace std::literals;
///unhide
std::vector names{"Joan"s, "Ben"s, "Gina"s, "Tim"s};
std::vector grades{8.5, 7.1, 9.0, 9.5};

std::println("student grades are: {}", 
  std::views::zip(names, grades));
///hide
}
```

<!-- .element: data-id="code" -->

Note: useful for "Structure of arrays" technic

---

<!-- .slide: data-auto-animate -->

### transforming

```cpp
///libs=fmt:trunk
///opts+=-Wno-array-bounds -Wno-stringop-overflow
///output=["Joan got 8.5", "Ben got 7.1", "Gina got 9", "Tim got 9.5"]
///hide
#include <vector>
#include <ranges>
#include <string>
#include <format>
#include "https://godbolt.org/z/K884c4hza/code/1" // print

int main() {
using namespace std::literals;
///unhide
std::vector names{"Joan"s, "Ben"s, "Gina"s, "Tim"s};
std::vector grades{8.5, 7.1, 9.0, 9.5};

std::println("{}", std::views::zip_transform(
  [](const std::string_view name, float grade) {
    return std::format("{} got {}", name, grade);
  }, 
  names, grades));
///hide
}
```

<!-- .element: data-id="code" -->


---

<!-- .slide: data-auto-animate -->

### reference and value type


```cpp
///compiler=vcpp_v19_latest_x64
///options=/std:c++latest
///hide
#include <vector>
#include <ranges>
#include <string>
#include "https://godbolt.org/z/6Tx7d4fc3/code/1" // zip

void zip() {
using namespace std::literals;
///unhide
std::vector names{"Joan"s, "Ben"s, "Gina"s, "Tim"s};
std::vector grades{8.5, 7.1, 9.0, 9.5};

using zipped = decltype(std::views::zip(names, grades));
static_assert(std::same_as<
  std::ranges::range_reference_t<zipped>,
  std::tuple<std::string&, double&>>);
static_assert(std::same_as<
  std::ranges::range_value_t<zipped>,
  std::tuple<std::string, double>>);
///hide
}
```

<!-- .element: data-id="code" -->

---

### Sorting

<!-- .slide: data-auto-animate -->

```cpp
///libs=fmt:vcpkg
///compiler=vcpp_v19_latest_x64
///options=/std:c++latest
///hide
#include <vector>
#include <ranges>
#include <string>
#include <format>
#include <algorithm>
#include "https://godbolt.org/z/6Tx7d4fc3/code/1" // zip
#include "https://godbolt.org/z/K884c4hza/code/1" // print

void sort() {
using namespace std::literals;
///unhide
std::vector names{"Joan"s, "Ben"s, "Gina"s, "Tim"s};
std::vector grades{8.5, 7.1, 9.0, 9.5};

std::ranges::sort(std::views::zip(grades, names), 
                  std::ranges::greater{});

std::println("{}", names);
///hide
}
```

<!-- .element: data-id="code" -->

---

### rvalue sorting

```cpp
///libs=fmt:trunk
///fails=no match for call
///hide
#include <ranges>
#include <string>
#include <algorithm>
#include <vector>
#include "https://godbolt.org/z/K884c4hza/code/1" // print

int main() {
///unhide
std::vector<int> v{4, 2, 3, 1};
std::ranges::sort(v | std::views::transform([](int i){
  return std::to_string(i);
}));
std::println("{}", v);
///hide
}
```

Note: It was considered to fix by requiring a reference type to be an lvalue reference but that would break proxy references. 
Instead they used the fact that proxy references have shallow constness so it should be possible to assign
to a const instance.

---

### `indirectly_writable`

```cpp [2, 6-8]
///hide
#include <utility>
#include <iterator>

///unhide
template< class Out, class T >
  concept indirectly_writable =
    requires(Out&& o, T&& t) {
      *o = std::forward<T>(t);
      *std::forward<Out>(o) = std::forward<T>(t);
      const_cast<const std::iter_reference_t<Out>&&>(*o) = std::forward<T>(t);
      const_cast<const std::iter_reference_t<Out>&&>(*std::forward<Out>(o)) =
        std::forward<T>(t);
    };
```

<!-- .element: style="font-size: 0.43em" -->

---

### more assignment operators

```diff
  // [tuple.assign], tuple assignment
  constexpr tuple& operator=(const tuple&);
+ constexpr tuple& operator=(const tuple&) const;
  constexpr tuple& operator=(tuple&&) noexcept(see below);
+ constexpr tuple& operator=(tuple&&) const noexcept(see below);   

  // [tuple.special], specialized algorithms
  template<class... Types>
  constexpr void swap(tuple<Types...>& x, tuple<Types...>& y) 
    noexcept(see below);
+ <class... Types>
+ constexpr void swap(const tuple<Types...>& x, const tuple<Types...>& y) 
+   noexcept(see below);
```

<!-- .element: style="font-size: 0.45em" -->

Note: same was added to `pair`

---

### `unique_copy`

```cpp [3|7,10,12-13]
///hide
#include <functional>
#include <iterator>
template<typename I, typename O> using unique_copy_result = std::pair<I, O>;

///unhide
template<typename I, typename S, typename O, typename C>
constexpr unique_copy_result<I, O> 
unique_copy(I first, S last, O out, C pred)
{
  if(first != last)
  {
    std::iter_value_t<I> value = *first;
    *out = value;
    ++out;
    while(++first != last)
    {
        auto && x = *first;
        if(!invoke(pred, value, x))
        {
            value = (decltype(x) &&)x;
            *out = value;
            ++out;
        }
    }
  }
  return {first, out};
}
```

<!-- .element: style="font-size: 0.5em" -->


Source: [range-v3](https://github.com/ericniebler/range-v3/blob/541b06320b89c16787cc6f785f749f8847cf2cd1/include/range/v3/algorithm/unique_copy.hpp)

<!-- .element: style="font-size: 0.5em" -->

---

<!-- .slide: data-auto-animate -->

### `unique_copy`

```cpp
///libs=rangesv3:0120
///fails=no match for call
///hide
#include <vector>
#include <iostream>
#include <iterator>
#include <range/v3/algorithm/unique_copy.hpp>
int main() {
///unhide
std::vector<bool> vb{true, true, false, false};
using R = std::vector<bool>::reference;
ranges::unique_copy(vb,
  std::ostream_iterator<bool>{std::cout, " "},
  [](R b1, R b2) { return b1 == b2; } );
///hide
}
```

<!-- .element: data-id="code" style="font-size: 0.5em" -->

---

<!-- .slide: data-auto-animate -->

### `unique_copy`

```cpp
///libs=rangesv3:0120
///fails=no match for call
///hide
#include <vector>
#include <iostream>
#include <iterator>
#include <range/v3/algorithm/unique_copy.hpp>
int main() {
///unhide
std::vector<bool> vb{true, true, false, false};
///hide
#if 1
using R = std::iter_common_reference_t<std::ranges::iterator_t<std::vector<bool>>>;
#else
///unhide
using R = std::ranges::range_common_reference_t<std::vector<bool>>;
///hide
#endif
///unhide
ranges::unique_copy(vb,
  std::ostream_iterator<bool>{std::cout, " "},
  [](R b1, R b2) { return b1 == b2; } );
///hide
}
```

<!-- .element: data-id="code" style="font-size: 0.5em" -->

---

### `indirectly_readable`

```cpp [10-12]
///hide
#include <iterator>
#include <ranges>

namespace ranges = std::ranges;

///unhide
template< class In >
concept indirectly_readable =
  requires(const In in) {
    typename std::iter_value_t<In>;
    typename std::iter_reference_t<In>;
    typename std::iter_rvalue_reference_t<In>;
    { *in } -> std::same_as<std::iter_reference_t<In>>;
    { ranges::iter_move(in) } -> std::same_as<std::iter_rvalue_reference_t<In>>;
  } &&
  std::common_reference_with<
    std::iter_reference_t<In>&&, std::iter_value_t<In>&
  > &&
  std::common_reference_with<
    std::iter_reference_t<In>&&, std::iter_rvalue_reference_t<In>&&
  > &&
  std::common_reference_with<
    std::iter_rvalue_reference_t<In>&&, const std::iter_value_t<In>&
  >;
```

<!-- .element: style="font-size: 0.42em" -->

---

### more constructors

```diff
     // [tuple.cnstr], tuple construction
     constexpr explicit(see below) tuple();
     constexpr explicit(see below) tuple(const Types&...);
     template<class... UTypes>
       constexpr explicit(see below) tuple(UTypes&&...);

     tuple(const tuple&) = default;
     tuple(tuple&&) = default;

+    template<class... UTypes>
+      constexpr explicit(see below) tuple(tuple<UTypes...>&);
     template<class... UTypes>
       constexpr explicit(see below) tuple(const tuple<UTypes...>&);
     template<class... UTypes>
       constexpr explicit(see below) tuple(tuple<UTypes...>&&);
+    template<class... UTypes>
+      constexpr explicit(see below) tuple(const tuple<UTypes...>&&);
```

<!-- .element: style="font-size: 0.45em" -->


---

<!-- .slide: data-auto-animate -->

### more tuple views

```cpp
///libs=fmt:trunk
///hide
#include <array>
#include <ranges>
#include <string>
#include <tuple>
#include "https://godbolt.org/z/K884c4hza/code/1" // print
 
int main()
{
///unhide
constexpr std::array v {1, 2, 3, 4, 5, 6};
std::println("v = {}", v); 
for (auto const [index, window]: 
  std::views::enumerate(v | std::views::adjacent<3>))
{
  std::println("w = {:>{}}{}", "", 3*index, window);
}
///hide
}
```

<!-- .element: data-id="code" -->

---

<!-- .slide: data-auto-animate -->

### more tuple views

```cpp
///libs=fmt:trunk
///hide
#include <array>
#include <ranges>
#include <string>
#include <tuple>
#include "https://godbolt.org/z/K884c4hza/code/1" // print
 
int main()
{
///unhide
constexpr std::array v {1, 2, 3, 4, 5, 6};
std::println("v = {}", v); 
for (auto const [index, window]: 
  std::views::enumerate(v | std::views::pairwise))
{
  std::println("w = {:>{}}{}", "", 3*index, window);
}
///hide
}
```

<!-- .element: data-id="code" -->

---

<!-- .slide: data-auto-animate -->

### more tuple views

```cpp
///libs=fmt:trunk
///hide
#include <array>
#include <ranges>
#include <string>
#include <tuple>
#include "https://godbolt.org/z/K884c4hza/code/1" // print
 
int main()
{
///unhide
constexpr std::array v {1, 2, 3, 4, 5, 6};
std::println("all pairs = {}", 
    std::views::cartesian_product(v, v)); 
///hide
}
```

<!-- .element: data-id="code" -->

---

<!-- .slide: class="aside" -->

### transposing months

```cpp [|7-9]
///hide
#include <ranges>
#include <string>

#include <https://godbolt.org/z/1vfTWKdcf/code/1> // generator
#include <https://godbolt.org/z/fceWKG4KE/code/1> // concat

namespace ranges = std::ranges;
namespace views = std::views;

namespace detail {

template <typename Rng, typename T, int level>
consteval bool recursive_range_of() {
  if constexpr (level == 1)
    if constexpr (ranges::range<Rng>)
      return std::same_as<ranges::range_value_t<Rng>, T>;
    else
      return false;
  else
    return recursive_range_of<ranges::range_value_t<Rng>, T, level - 1>();
}

}  // namespace detail

template <typename Rng, typename T, int extent>
concept nested_range_of = detail::recursive_range_of<Rng, T, extent>();

template <typename Rng, typename T>
concept range_of = nested_range_of<Rng, T, 1>;

///unhide
// In:  range<range<range<string>>>
// Out: range<range<range<string>>>, transposing months.
auto transpose_months() {
  return views::transform(
    []<nested_range_of<std::string, 2> Rng>(Rng&& rng) {
      const auto begin = ranges::begin(rng);
      return views::zip_transform(
          concat, *begin, *ranges::next(begin),
          *ranges::next(begin, 2));
    }
  );
}
```

---

<!-- .slide: data-auto-animate class="aside" -->

### `| chunk(3)`

<pre data-id="dates" style="font-size: 0.45em;">
<code data-trim class="text" style="white-space: pre-wrap;">
[
  [
    ["       January        ", " 01 02 03 04 05 06 07 ", " 08 09 10 11 12 13 14 ", " 15 16 17 18 19 20 21 ", " 22 23 24 25 26 27 28 ", " 29 30 31             ", "                      "], 
    ["       February       ", "          01 02 03 04 ", " 05 06 07 08 09 10 11 ", " 12 13 14 15 16 17 18 ", " 19 20 21 22 23 24 25 ", " 26 27 28             ", "                      "], 
    ["        March         ", "          01 02 03 04 ", " 05 06 07 08 09 10 11 ", " 12 13 14 15 16 17 18 ", " 19 20 21 22 23 24 25 ", " 26 27 28 29 30 31    ", "                      "]
  ], 
  [
    ["        April         ", "                   01 ", " 02 03 04 05 06 07 08 ", " 09 10 11 12 13 14 15 ", " 16 17 18 19 20 21 22 ", " 23 24 25 26 27 28 29 ", " 30                   "], 
    ["         May          ", "    01 02 03 04 05 06 ", " 07 08 09 10 11 12 13 ", " 14 15 16 17 18 19 20 ", " 21 22 23 24 25 26 27 ", " 28 29 30 31          ", "                      "], 
    ["         June         ", "             01 02 03 ", " 04 05 06 07 08 09 10 ", " 11 12 13 14 15 16 17 ", " 18 19 20 21 22 23 24 ", " 25 26 27 28 29 30    ", "                      "]
  ], 
  [
    ["         July         ", "                   01 ", " 02 03 04 05 06 07 08 ", " 09 10 11 12 13 14 15 ", " 16 17 18 19 20 21 22 ", " 23 24 25 26 27 28 29 ", " 30 31                "], 
    ["        August        ", "       01 02 03 04 05 ", " 06 07 08 09 10 11 12 ", " 13 14 15 16 17 18 19 ", " 20 21 22 23 24 25 26 ", " 27 28 29 30 31       ", "                      "], 
    ["      September       ", "                01 02 ", " 03 04 05 06 07 08 09 ", " 10 11 12 13 14 15 16 ", " 17 18 19 20 21 22 23 ", " 24 25 26 27 28 29 30 ", "                      "]
  ], 
  [
    ["       October        ", " 01 02 03 04 05 06 07 ", " 08 09 10 11 12 13 14 ", " 15 16 17 18 19 20 21 ", " 22 23 24 25 26 27 28 ", " 29 30 31             ", "                      "], 
    ["       November       ", "          01 02 03 04 ", " 05 06 07 08 09 10 11 ", " 12 13 14 15 16 17 18 ", " 19 20 21 22 23 24 25 ", " 26 27 28 29 30       ", "                      "], 
    ["       December       ", "                01 02 ", " 03 04 05 06 07 08 09 ", " 10 11 12 13 14 15 16 ", " 17 18 19 20 21 22 23 ", " 24 25 26 27 28 29 30 ", " 31                   "]
  ]
] 
</code>
</pre>

---

<!-- .slide: data-auto-animate class="aside" -->

### `| transpose_months`

<pre data-id="dates" style="font-size: 0.45em;">
<code data-trim class="text" style="white-space: pre-wrap;">
[
  ["       January               February               March         ", " 01 02 03 04 05 06 07           01 02 03 04           01 02 03 04 ", " 08 09 10 11 12 13 14  05 06 07 08 09 10 11  05 06 07 08 09 10 11 ", " 15 16 17 18 19 20 21  12 13 14 15 16 17 18  12 13 14 15 16 17 18 ", " 22 23 24 25 26 27 28  19 20 21 22 23 24 25  19 20 21 22 23 24 25 ", " 29 30 31              26 27 28              26 27 28 29 30 31    ", "                                                                  "], 
  ["        April                  May                   June         ", "                   01     01 02 03 04 05 06              01 02 03 ", " 02 03 04 05 06 07 08  07 08 09 10 11 12 13  04 05 06 07 08 09 10 ", " 09 10 11 12 13 14 15  14 15 16 17 18 19 20  11 12 13 14 15 16 17 ", " 16 17 18 19 20 21 22  21 22 23 24 25 26 27  18 19 20 21 22 23 24 ", " 23 24 25 26 27 28 29  28 29 30 31           25 26 27 28 29 30    ", " 30                                                               "], 
  ["         July                 August              September       ", "                   01        01 02 03 04 05                 01 02 ", " 02 03 04 05 06 07 08  06 07 08 09 10 11 12  03 04 05 06 07 08 09 ", " 09 10 11 12 13 14 15  13 14 15 16 17 18 19  10 11 12 13 14 15 16 ", " 16 17 18 19 20 21 22  20 21 22 23 24 25 26  17 18 19 20 21 22 23 ", " 23 24 25 26 27 28 29  27 28 29 30 31        24 25 26 27 28 29 30 ", " 30 31                                                            "], 
  ["       October               November              December       ", " 01 02 03 04 05 06 07           01 02 03 04                 01 02 ", " 08 09 10 11 12 13 14  05 06 07 08 09 10 11  03 04 05 06 07 08 09 ", " 15 16 17 18 19 20 21  12 13 14 15 16 17 18  10 11 12 13 14 15 16 ", " 22 23 24 25 26 27 28  19 20 21 22 23 24 25  17 18 19 20 21 22 23 ", " 29 30 31              26 27 28 29 30        24 25 26 27 28 29 30 ", "                                             31                   "]
]
</code>
</pre>

---

<!-- .slide: data-auto-animate class="aside" -->

### `| join`

<pre data-id="dates" style="font-size: 0.45em;">
<code data-trim class="text" style="white-space: pre-wrap;">
[
  "       January               February               March         ",
  " 01 02 03 04 05 06 07           01 02 03 04           01 02 03 04 ",
  " 08 09 10 11 12 13 14  05 06 07 08 09 10 11  05 06 07 08 09 10 11 ",
  " 15 16 17 18 19 20 21  12 13 14 15 16 17 18  12 13 14 15 16 17 18 ",
  " 22 23 24 25 26 27 28  19 20 21 22 23 24 25  19 20 21 22 23 24 25 ",
  " 29 30 31              26 27 28              26 27 28 29 30 31    ",
  "                                                                  ",
  "        April                  May                   June         ",
  "                   01     01 02 03 04 05 06              01 02 03 ",
  " 02 03 04 05 06 07 08  07 08 09 10 11 12 13  04 05 06 07 08 09 10 ",
  " 09 10 11 12 13 14 15  14 15 16 17 18 19 20  11 12 13 14 15 16 17 ",
  " 16 17 18 19 20 21 22  21 22 23 24 25 26 27  18 19 20 21 22 23 24 ",
  " 23 24 25 26 27 28 29  28 29 30 31           25 26 27 28 29 30    ",
  " 30                                                               ",
  "         July                 August              September       ",
  "                   01        01 02 03 04 05                 01 02 ",
  " 02 03 04 05 06 07 08  06 07 08 09 10 11 12  03 04 05 06 07 08 09 ",
  " 09 10 11 12 13 14 15  13 14 15 16 17 18 19  10 11 12 13 14 15 16 ",
  " 16 17 18 19 20 21 22  20 21 22 23 24 25 26  17 18 19 20 21 22 23 ",
  " 23 24 25 26 27 28 29  27 28 29 30 31        24 25 26 27 28 29 30 ",
  " 30 31                                                            ",
  "       October               November              December       ",
  " 01 02 03 04 05 06 07           01 02 03 04                 01 02 ",
  " 08 09 10 11 12 13 14  05 06 07 08 09 10 11  03 04 05 06 07 08 09 ",
  " 15 16 17 18 19 20 21  12 13 14 15 16 17 18  10 11 12 13 14 15 16 ",
  " 22 23 24 25 26 27 28  19 20 21 22 23 24 25  17 18 19 20 21 22 23 ",
  " 29 30 31              26 27 28 29 30        24 25 26 27 28 29 30 ",
  "                                             31                   "
]
</code>
</pre>

---

<!-- .slide: data-auto-animate class="aside" -->

### `| join_with('\n')`

<pre data-id="dates" style="font-size: 0.45em;">
<code data-trim class="text" style="white-space: pre-wrap;">
       January               February               March         
 01 02 03 04 05 06 07           01 02 03 04           01 02 03 04 
 08 09 10 11 12 13 14  05 06 07 08 09 10 11  05 06 07 08 09 10 11 
 15 16 17 18 19 20 21  12 13 14 15 16 17 18  12 13 14 15 16 17 18 
 22 23 24 25 26 27 28  19 20 21 22 23 24 25  19 20 21 22 23 24 25 
 29 30 31              26 27 28              26 27 28 29 30 31    
                                                                  
        April                  May                   June         
                   01     01 02 03 04 05 06              01 02 03 
 02 03 04 05 06 07 08  07 08 09 10 11 12 13  04 05 06 07 08 09 10 
 09 10 11 12 13 14 15  14 15 16 17 18 19 20  11 12 13 14 15 16 17 
 16 17 18 19 20 21 22  21 22 23 24 25 26 27  18 19 20 21 22 23 24 
 23 24 25 26 27 28 29  28 29 30 31           25 26 27 28 29 30    
 30                                                               
         July                 August              September       
                   01        01 02 03 04 05                 01 02 
 02 03 04 05 06 07 08  06 07 08 09 10 11 12  03 04 05 06 07 08 09 
 09 10 11 12 13 14 15  13 14 15 16 17 18 19  10 11 12 13 14 15 16 
 16 17 18 19 20 21 22  20 21 22 23 24 25 26  17 18 19 20 21 22 23 
 23 24 25 26 27 28 29  27 28 29 30 31        24 25 26 27 28 29 30 
 30 31                                                            
       October               November              December       
 01 02 03 04 05 06 07           01 02 03 04                 01 02 
 08 09 10 11 12 13 14  05 06 07 08 09 10 11  03 04 05 06 07 08 09 
 15 16 17 18 19 20 21  12 13 14 15 16 17 18  10 11 12 13 14 15 16 
 22 23 24 25 26 27 28  19 20 21 22 23 24 25  17 18 19 20 21 22 23 
 29 30 31              26 27 28 29 30        24 25 26 27 28 29 30 
                                             31                   
</code>
</pre>

----

<!-- .slide: data-background-image="resources/materialize.jpg" -->

## materialize

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #e2c4b8" -->


Source: [pmtoday.co.uk](https://www.pmtoday.co.uk/battersea-power-station-the-project-view/)

<!-- .element: style="font-size: 0.5em" -->

---

<!-- .slide: data-auto-animate -->

### to vector

```cpp
///compiler=vcpp_v19_latest_x64
///options=/std:c++latest
///hide
#include <ranges>
#include <vector> 

int main() {
///unhide
auto numbers = std::views::iota(1, 10);

auto vec = std::ranges::to<std::vector<int>>(numbers);
///hide
}
```

<!-- .element: data-id="code" style="font-size: 0.5em" -->

---

<!-- .slide: data-auto-animate -->

### Allocator anyone?

```cpp
///compiler=vcpp_v19_latest_x64
///options=/std:c++latest
///hide
#include <ranges>
#include <vector>

int main() {
using Alloc = std::allocator<int>;
Alloc alloc;
///unhide
auto numbers = std::views::iota(1, 10);

auto vec = std::ranges::to<std::vector<int, Alloc>>(numbers, alloc);
///hide
}
```

<!-- .element: data-id="code" style="font-size: 0.5em" -->

---

<!-- .slide: data-auto-animate -->

### type deduction

```cpp
///compiler=vcpp_v19_latest_x64
///options=/std:c++latest
///hide
#include <ranges>
#include <vector>

int main() {
///unhide
auto numbers = std::views::iota(1, 10);

auto vec = std::ranges::to<std::vector>(numbers);
///hide
}
```

<!-- .element: data-id="code" style="font-size: 0.5em" -->

---

<!-- .slide: data-auto-animate -->

### range constructor

```cpp
///compiler=vcpp_v19_latest_x64
///options=/std:c++latest
///hide
#include <ranges>
#include <vector>

int main() {
///unhide
auto numbers = std::views::iota(1, 10);

auto vec = std::vector{std::from_range, numbers};
///hide
}
```

<!-- .element: data-id="code" style="font-size: 0.5em" -->

---

<!-- .slide: data-auto-animate -->

### composable

```cpp
///compiler=vcpp_v19_latest_x64
///options=/std:c++latest
///hide
#include <ranges>
#include <vector>

int main() {
///unhide
auto numbers = std::views::iota(1, 10);

auto vec = numbers | std::ranges::to<std::vector>();
///hide
}
```

<!-- .element: data-id="code" style="font-size: 0.5em" -->

Note: nasty error when you forget the parentheses

---

<!-- .slide: data-auto-animate -->

### value type conversion

```cpp
///compiler=vcpp_v19_latest_x64
///options=/std:c++latest
///hide
#include <ranges>
#include <vector>

int main() {
///unhide
auto numbers = std::views::iota(1, 10);

auto vec = numbers | std::ranges::to<std::vector<double>>();
///hide
}
```

<!-- .element: data-id="code" style="font-size: 0.5em" -->

---

<!-- .slide: data-auto-animate -->

### associative containers

```cpp
///compiler=vcpp_v19_latest_x64
///options=/std:c++latest
///hide
#include <ranges>
#include <vector>
#include <unordered_map>
#include "https://godbolt.org/z/6Tx7d4fc3/code/1" // zip

int main() {
using namespace std::literals;
///unhide
std::vector names{"Joan"s, "Ben"s, "Gina"s, "Tim"s};
std::vector grades{8.5, 7.1, 9.0, 9.5};

auto map = std::views::zip(names, grades) 
///hide
  // until P2165
  | std::views::transform([]<typename T>(T&& t) {
    return std::make_pair(std::get<0>(std::forward<T>(t)), std::get<1>(std::forward<T>(t)));
  })
///unhide
  | std::ranges::to<std::unordered_map>();
///hide
}
```

<!-- .element: data-id="code" style="font-size: 0.5em" -->

---

<!-- .slide: data-auto-animate -->

### nested ranges

```cpp
///compiler=vcpp_v19_latest_x64
///options=/std:c++latest
///hide
#include <ranges>
#include <vector>
#include <list>
#include <deque>
#include <forward_list>

int main() {
///unhide
std::list<std::forward_list<int>> lst = {{0, 1, 2, 3}, {4, 5, 6, 7}}; 
auto vec = std::ranges::to<std::vector<std::deque<double>>>(lst);
///hide
}
```

<!-- .element: data-id="code" style="font-size: 0.5em" -->

---

<!-- .slide: data-auto-animate -->

### User defined types

```cpp
///compiler=vcpp_v19_latest_x64
///options=/std:c++latest
///hide
#include <ranges>

///unhide
template<typename T>
class MyContainer {
public:
  MyContainer(std::from_range_t, std::ranges::range auto&& r);

  T* begin() const;
  T* end() const;
};

template<std::ranges::range Rng>
MyContainer(std::from_range_t, Rng&& r) 
  -> MyContainer<std::ranges::range_value_t<Rng>>;

///hide
template<typename T>
MyContainer<T>::MyContainer(std::from_range_t, std::ranges::range auto&& r) {}

int main() {
///unhide
auto numbers = std::views::iota(1, 10);
auto cont = std::ranges::to<MyContainer>(numbers);
///hide
}
```

<!-- .element: data-id="code" style="font-size: 0.5em" -->

---

### algorithm

`ranges::to<C>(r, args...)` 

- constructs `C` using the first valid from
  - `C{r, args...}`
  - `C{std::from_range, r, args...}`
  - `C{ranges::begin(r), ranges::end(r), args...}`
  - `C c{args...}; c.reserve(std::ranges::size(r)); std::ranges::copy(r, std::back_inserter(c));`
- recurse if needed.

<!-- .element: style="font-size: 0.75em" -->

Note: calls distance only for supporting containers

---

### why `from_range_t`?

```cpp
///compiler=vcpp_v19_latest_x64
///options=/std:c++latest
///hide
#include <ranges>
#include <vector>
#include <list>

int main() {
///unhide
std::list<int> l; 
std::vector v{l};
///hide
}
```

Note: Should `v` be `std::vector<int>` or `std::vector<std::list<int>>` ?

---

### more methods

```cpp
///compiler=vcpp_v19_latest_x64
///options=/std:c++latest
///hide
#include <ranges>
#include <vector>
#include <list>

int main() {
///unhide
std::vector<int> v;
v.insert_range(v.end(), std::views::iota(1, 10));
v.append_range(std::views::iota(20, 30));
v.assign_range(std::views::iota(30, 100));
///hide
}
```

Note: no `std::array` support yet

----

<!-- .slide: data-background-image="resources/folding.png" -->


## folding

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #d3e9fa" -->

Source: [wessexcoastgeology.soton.ac.uk](https://wessexcoastgeology.soton.ac.uk/Stair-Hole-Lulworth.htm)

<!-- .element: style="font-size: 0.5em" -->

---

### `std::fold_left`

```cpp [1-3]
///hide
#include <ranges>
using std::input_iterator;
using std::sentinel_for;
using std::ranges::input_range;
using std::ranges::iterator_t;

template<typename...>
concept indirectly_binary_left_foldable = true;

///unhide
template<input_iterator I, sentinel_for<I> S, class T,
         indirectly_binary_left_foldable<T, I> F>
constexpr auto fold_left(I first, S last, T init, F f);

template<input_range R, class T, 
         indirectly_binary_left_foldable<T, iterator_t<R>> F>
constexpr auto fold_left(R&& r, T init, F f);
```

`$$ f(f(f(f(init, x_1), x_2), ...), x_n) $$`

Note: function is not defaulted to `std::plus`

---

### examples

```cpp [1|3-6|3,8-9]
///libs=fmt:trunk
///output=sum: 36\nmul: 40320
#include <algorithm>
#include <vector>

///hide
#include <https://godbolt.org/z/1jWT1a5KT/code/1> // print

int main() {
///unhide
std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8};

int sum = std::ranges::fold_left(v, 0, std::plus<int>());
std::println("sum: {}", sum);

int mul = std::ranges::fold_left(v, 1, std::multiplies<int>());
std::println("mul: {}", mul);
///hide
}
```

<!-- .element: style="font-size: 0.52em" -->

---

### examples

```cpp [2-4,9|5-9]
///libs=fmt:trunk
///output=f(f(f(f(f(f(f(f(f(f(init,x₀),x₁),x₂),x₃),x₄),x₅),x₆),x₇),x₈),x₉)
///hide
#include <https://godbolt.org/z/ovqY7K8oa/code/1> // left_fold
#include <https://godbolt.org/z/1jWT1a5KT/code/1> // print
#include <https://godbolt.org/z/E6rYKGKWY/code/2> // utf8
#include <ranges>

auto utf32_to_str(char32_t cp) {
    std::string s;
    utf8::append(cp, s);
    return s;
}

int main() {
using namespace std::literals;
///unhide
std::println("{}", 
  std::ranges::fold_left(
    std::views::iota(0, 10), 
    "init"s, 
    [](std::string&& accum, const int index){
      return std::format("f({},x{})", 
                         std::move(accum), 
                         utf32_to_str(U'₀' + index));
    }));
///hide
}
```

---

### examples

```cpp [1-7,15|8-14]
///hide
#include <https://godbolt.org/z/ovqY7K8oa/code/1> // left_fold

#define FWD(X) std::forward<decltype(X)>(X)

///unhide
template<
  std::input_iterator I, std::sentinel_for<I> S,
  class Proj = std::identity,
  std::indirect_unary_predicate<std::projected<I, Proj>> Pred
>
constexpr std::iter_difference_t<I>
count_if( I first, S last, Pred pred, Proj proj = {} ) {
  return std::ranges::fold_left(first, last, 
    std::iter_difference_t<I>{0}, 
    [&](auto accum, auto&& val) {
      return std::invoke(pred, std::invoke(proj, FWD(val))) 
        ? accum + 1 
        : accum;
  });
}
```

<!-- .element: style="font-size: 0.5em" -->

---

### `std::fold_right`

```cpp [1-3]
///hide
#include <ranges>
using std::bidirectional_iterator;
using std::sentinel_for;
using std::ranges::bidirectional_range;
using std::ranges::iterator_t;

template<typename...>
concept indirectly_binary_right_foldable = true;

///unhide
template<bidirectional_iterator I, sentinel_for<I> S, class T,
         indirectly_binary_right_foldable<T, I> F>
constexpr auto fold_right(I first, S last, T init, F f);

template<bidirectional_range R, class T,
         indirectly_binary_right_foldable<T, iterator_t<R>> F>
constexpr auto fold_right(R&& r, T init, F f);;
```

<!-- .element: style="font-size: 0.5em" -->

`$$ f(x_1, f(x_2, ...f(x_n, init))) $$`

---

### examples

```cpp [2-4,9|5-9]
///libs=fmt:trunk
///output=f(x₀,f(x₁,f(x₂,f(x₃,f(x₄,f(x₅,f(x₆,f(x₇,f(x₈,f(x₉,init))))))))))
///hide
#include <https://godbolt.org/z/ovqY7K8oa/code/1> // left_fold
#include <https://godbolt.org/z/1jWT1a5KT/code/1> // print
#include <https://godbolt.org/z/E6rYKGKWY/code/2> // utf8
#include <ranges>

auto utf32_to_str(char32_t cp) {
    std::string s;
    utf8::append(cp, s);
    return s;
}

namespace std::ranges {

inline constexpr auto flipped = []<typename F>(F&& f) {
    return [&f]<typename T, typename U>(T&& t, U&& u) {
        return invoke(std::forward<F>(f), std::forward<U>(u), std::forward<T>(t));
    };
};

template< class F, class T, class I >
    concept indirectly_binary_right_foldable =      // exposition only
            indirectly_binary_left_foldable<decltype(flipped(std::declval<F>())), T, I>;

template<
    std::bidirectional_iterator I, std::sentinel_for<I> S, class T,
    indirectly_binary_right_foldable<T, I> F
>
constexpr auto fold_right( I first, S last, T init, F f )
{
    return fold_left(subrange(first, last) | views::reverse, init, flipped(std::move(f)));
}

template<
    ranges::bidirectional_range R, class T,
    indirectly_binary_right_foldable<T, ranges::iterator_t<R>> F
>
constexpr auto fold_right( R&& r, T init, F f ) {
    return fold_right(begin(r), end(r), init, std::move(f));
}
}

int main() {
using namespace std::literals;
///unhide
std::println("{}", 
  std::ranges::fold_right(
    std::views::iota(0, 10), 
    "init"s, 
    [](const int index, std::string&& accum){
      return std::format("f(x{},{})", 
                         utf32_to_str(U'₀' + index), 
                         std::move(accum));
    }));
///hide
}
```

---

<!-- .slide: data-auto-animate -->

### `std::ranges::min`

```cpp [1-2,8|3-7]
///hide
#include <https://godbolt.org/z/ovqY7K8oa/code/1> // left_fold
#include <ranges>

#define FWD(X) std::forward<decltype(X)>(X)

///unhide
template< std::ranges::input_range R >
constexpr std::ranges::range_value_t<R> min( R&& r ) {
  return std::ranges::fold_left(FWD(r), 
///hide
    *std::ranges::begin(r),
// \
///unhide
    ???, 
    [&](auto&& min, auto&& next) {
      return next < min ? next : min;
    });
}
```

<!-- .element: data-id="code" -->

---

<!-- .slide: data-auto-animate -->

### `std::ranges::min`

```cpp [4]
///hide
#include <https://godbolt.org/z/ovqY7K8oa/code/1> // left_fold
#include <ranges>

#define FWD(X) std::forward<decltype(X)>(X)

///unhide
template< std::ranges::input_range R >
constexpr std::ranges::range_value_t<R> min( R&& r ) {
  return std::ranges::fold_left(FWD(r), 
    *std::ranges::begin(r),
    [&](auto&& min, auto&& next) {
      return next < min ? next : min;
    });
}
```

<!-- .element: data-id="code" -->

---

<!-- .slide: data-auto-animate -->

### `std::ranges::min`

```cpp [3-6]
///hide
#include <https://godbolt.org/z/ovqY7K8oa/code/1> // left_fold
#include <ranges>

#define FWD(X) std::forward<decltype(X)>(X)

///unhide
template< std::ranges::input_range R >
constexpr std::ranges::range_value_t<R> min( R&& r ) {
  return std::ranges::fold_left_first(FWD(r),
    [&](auto&& min, auto&& next) {
      return next < min ? next : min;
    });
}
```

<!-- .element: data-id="code" -->

---

<!-- .slide: data-auto-animate -->

### `std::ranges::min`

<pre>
<code class="lang-cpp" data-trim data-noescape data-line-numbers="3">
///hide
#include &lt;https://godbolt.org/z/ovqY7K8oa/code/1&gt; // left_fold
#include &lt;ranges&gt;

#define FWD(X) std::forward&lt;decltype(X)&gt;(X)

///unhide
template&lt; std::ranges::input_range R &gt;
constexpr std::ranges::range_value_t&lt;R&gt; min( R&& r ) {
  return <mark>*</mark>std::ranges::fold_left_first(FWD(r),
    [&](auto&& min, auto&& next) {
      return next < min ? next : min;
    });
}
</code>
</pre>

<!-- .element: data-id="code" -->

---

<!-- .slide: data-auto-animate -->

> The law of useful return: 
>
> A procedure should return all the potentially useful information it computed.

<p style="width: 100%; text-align: right;"> <cite> Alexander Stepanov </cite> </p>

```cpp
///fails=cannot decompose non-array non-class type 'int'
///hide
#include <https://godbolt.org/z/ovqY7K8oa/code/1> // left_fold
#include <ranges>

int main() {
namespace ranges = std::ranges;
ranges::empty_view<int> numbers;
///unhide
auto [_, total] = ranges::fold_left(numbers, 0, std::plus<>{});
///hide
}
```

<!-- .element: class="fragment" data-id="code" style="font-size: 0.45em" -->

---

<!-- .slide: data-auto-animate -->

> The law of useful return: 
>
> A procedure should return all the potentially useful information it computed.

<p style="width: 100%; text-align: right;"> <cite> Alexander Stepanov </cite> </p>

```cpp
///hide
#include <https://godbolt.org/z/zjqqYG7hh/code/1> // left_fold_with_iter
#include <ranges>

int main() {
namespace ranges = std::ranges;
ranges::empty_view<int> numbers;
///unhide
auto [it, total] = ranges::fold_left_with_iter(numbers, 0, std::plus<>{});
///hide
}
```

<!-- .element: data-id="code" style="font-size: 0.45em" -->

---

### all the folds

| algorithm | direction | init | return |
|-|-|-|-|
| `fold_left` | left | + | `T` |
| `fold_left_first` | left | - | `T` |
| `fold_right` | right | + | `T` |
| `fold_right_last` | right | - | `T` |
| `fold_left_with_iter` | left | + | (`it`, `T`) |
| `fold_left_first_with_iter` | left | - | (`it`, `T`) |

<!-- .element: style="font-size: 0.7em" -->

----

<div style="display: grid; grid-template-columns: repeat(3, 1fr); grid-template-rows: 50% 20% 30%; font-size: 75%">

# Thank you

<!-- .element: style="grid-column: 1 / 4; font-size: calc(2*var(--r-heading1-size));" -->

![gmail](resources/Gmail_icon_(2020).png)

![github](resources/github-mark-white.png)

![twitter](resources/twitter.png)

[dvirtz@gmail.com](mailto:dvirtz@gmail.com)

[github.com/dvirtz](https://github.com/dvirtz)

[@dvirtzwastaken](https://twitter.com/dvirtzwastaken)

</div>