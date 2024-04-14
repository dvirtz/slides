---
revealOptions:
  slideNumber: c/t
  compilerExplorer:
    compiler: "g131"
    options: -std=c++23 -O2 -march=haswell -Wall -Wextra -pedantic -Wno-unused-variable -Wno-unused-parameter
highlightTheme: atom-one-dark
---

<!-- .slide: data-background-image="resources/accu_title.png" -->

Note:  hello and welcome to my talk about C++ 23 ranges. 

---

https://dvirtz.github.io/slides/ranges_23/ranges.html

![QR code](resources/accu-qr-code.png)

<!-- .element: class="r-stretch" -->

Note: Before we start, slides are available online already on github. You can go to this link or scan this QR code to 
see the slides in your own device. 

My name is Dvir, I recently moved to live in Manchester here in the UK and I work for Roku.x

----

<!-- .slide: data-background-image="resources/EricTaDa.png" -->

Note:  now anyone here saw this talk by Eric Niebler from CppCon 2015? I personally was in awe after seeing this. In his talk Eric implements a calendar software using his range-v3 library and the elegance and readability of the code was simply amazing. Now, this was before ranges we're proposed for standardization but as you probably know ranges were standardized for C++ 20. However, a large number of useful utilities remained missing but the good news is C++ 23 will have many additions to ranges, and in fact, we can now finally implement the same calendar software using standard C++ and that's exactly what we're going to do today.

----

<!-- .slide: data-background-image="resources/recap.jpg" -->

## recap

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #859d34" -->

Source: https://discovernorthernireland.com/

<!-- .element: style="font-size: 0.5em" -->

Note: Let's first do a quick recap of what exactly ranges are in case anyone in the audience never used them. 



---

### what is a range

- A sequence of elements between two locations `i`, `k`.
- Often denoted by `[i, k)`.

Note: conceptually, a range is a sequence of all the elements between two locations, called `i` and `k` here, not including `k`.

---

> Although Concepts are constraints on types,
> you don’t find them by looking at the types in your system.
>
> You find them by studying the algorithms.

<p style="width: 100%; text-align: right;"> <cite> Eric Neibler </cite> </p>

Note: A famous quote by Eric says that "although concepts are constrained on types you don't find them by looking at the types 
but by studying the algorithms", so let's look start at the most simple algorithm...

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

Note: `std::for_each`. This is a conforming implementation of the C++ 17 version of `for_each`, which gets a range by the means of a pair of iterators, `first` and `last`,
iterates between them and call the given function `f` on each element.

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

Note: the way the implementation uses `first` is dereferencing it to get the current value and incrementing it to go to the next element 

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

Note: That brings us to the first concept which is an iterator (or is it called in standard `input_or_output_iterator`, because `std::iterator` was already taken), which is any type that can be dereferenced and incremented. From that we derive other types of iterators, by adding more and more capabilities, for example a bidirectional iterator can also go back using the decrement operator and so on and so forth. A new type of iterator introduced in C++ 20 is a contiguous iterator, which can be used to enable certain optimizations that are only possible if the elements are actually contiguous in memory.

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

Note: back to `for_each`, the only thing the algorithm does with the second argument, `last`, is to compare it to `first` to know when the end of the sequence
was reached. 

---

## Sentinels

- a semi-regular `S` is a `sentinel_for` an iterator `I` if for a range `[i, s)`
  - `i == s` is well defined.
  - If `i != s` then `i` is dereferenceable and `[++i, s)` denotes a range.

- Such an `s` is called a sentinel

Note: This brings us to the concept of sentinel, which is a semi regular type that can be compared to the iterator and if that comparison is false, then we know that we can increment the iterator and get "closer" to the sentinel. So basically sentinel is always tied to an iterator type. 

---

## range

a type we can feed to
- `ranges::begin` - to get an iterator
- `ranges::end` - to get a sentinel

A range `[i, s)` refers to the elements

`*i, *(i++), *((i++)++), …, j`

such that j == s.

Note: combining those two concepts gives us the range concept which is a type that can provide an iterator by calling `begin` and a sentinel by calling `end`.
The values of the range are what we get by repeatedly dereferencing and incrementing the iterator until it compares equal to the sentinel.

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

Note: The type of the iterator which we get from `begin` mandates the what concept the range satisfies, and you can see here examples from the standard library for the different concepts. 
Another important concept is a common range which is a range whose iterator and sentinel are the same type, like all those containers. Such a range can be used with the legacy code, which expect a pair of iterators.

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

Note: The C++ 20 ranges algorithms using the concepts we've seen to constrain the types they are able to get. Each algorithm has two signatures, one that takes an iterator
and a sentinel and one that takes a single range argument. Usually, you'll just need the latter one.

---

<!-- .slide: data-auto-animate -->

> The law of useful return: 
>
> A procedure should return all the potentially useful information it computed.

<p style="width: 100%; text-align: right;"> <cite> Alexander Stepanov </cite> </p>

Note: some of the algorithms also have a different return type than their C++17 counterparts and that's due to this law of useful return, coined by Alexander Stepanov,
that states that and data computed by a function which is potentially useful for the caller should be returned.

---

## range

a type we can feed to
- `ranges::begin` - to get an iterator
- `ranges::end` - to get a sentinel

A range `[i, s)` refers to the elements

`*i, *(i++), *((i++)++), …, j`

such that j == s.

Note: this `j` here, which is the end iterator is a potentially useful data which the caller doesn't have and so should be returned. Think on parsing for example, where we want to know where the current token ends.

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

Note: in addition to returning the function, `for_each` now returns the end iterator.

---

## views

A lightweight™ handle to a range.

- A range type that wraps a pair of iterators.
- A range type that holds its elements by `std::shared_ptr` and shares ownership with all its copies.
- A range type that generates its elements on demand.

Note: A special type of range which is cheap to pass around is called a view. For example, a range which is simply a pair of iterators, or a range which holds 
it's elements on the heap using a `shared_ptr`, or a range that simply generates its values on demand. We will talk about views in depth later on. 


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

Note: the most basic view which we have in the standard is the empty view, where begin and end are always the same.

---

## Factories & Adaptors

- Utility objects for creating views
  - adaptors (if transform an existing range) or 
  - factories (otherwise)

```cpp []
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

static_assert(std::ranges::empty(std::views::empty<int>));
```

Note: generally speaking, user code should not use the view types (like `empty_view`) directly. Instead, it should use range adaptors and factories, which reside in namespace `std::ranges::views` or its alias `std::views`. A range adaptor takes a range and transforms it in some way and factories generate new ranges.

In this example, we use the `views::empty` factory to generate an empty range of `int`s and then verify (at compile time!) it is indeed empty.

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

Note: to illustrate the way ranges change the way we write code, let's look at this example of calculating the sum of squares of the numbers 0 to `count`.
Besides all those annoying calls to `begin` and `end` we have to run each step in its own statement. The code is not composable.  

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

Note: range adaptors are composable. we can pass the result of one adaptor to the next.
Using function calls, however, have the problem that to read the code we should go from inside out.

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

Note: using the pipe operator, enables us to read the code top down and makes it much more readable. 
note that this doesn't really compile with the C++20 accumulate because a rangified version of that was not standardized in C++20 but that also changed in C++23 as we will see.

---

<!-- .slide: data-auto-animate class="aside" -->

### listing the days

```cpp [1|9-12|3-7|9-12]
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

Note: We can now start to implement our calendar. We'll use the C++20 `chrono` dates library. To simplify the code, we'll call `std::chrono::year_month_day` a `date`. Given 
a range of years to generate a calender of, we produce a list of all the dates between the first day of the start year and the first day of the last one (remember we're not including the stop year). Since `date` itself is not incrementable we use the `chrono::sys_days` which is a `time_point` with granularity of days which is incrementable and so can be used with iota view and then we cast the resulting list back to dates. 

---

<!-- .slide: data-auto-animate class="aside" -->

### listing the days

```cpp [9-12]
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

Note: we will also support generating an infinite calendar starting from a given year and we use the infinite version of iota for that.

---

<!-- .slide: class="aside" -->

### `dates()`

<pre data-id="dates" style="font-size: 0.45em;">
<code data-trim class="text" style="white-space: pre-wrap;">
[2023-01-01, 2023-01-02, 2023-01-03, 2023-01-04, 2023-01-05, 2023-01-06, 2023-01-07, 2023-01-08, 2023-01-09, 2023-01-10, 2023-01-11, 2023-01-12, 2023-01-13, 2023-01-14, 2023-01-15, 2023-01-16, 2023-01-17, 2023-01-18, 2023-01-19, 2023-01-20, 2023-01-21, 2023-01-22, 2023-01-23, 2023-01-24, 2023-01-25, 2023-01-26, 2023-01-27, 2023-01-28, 2023-01-29, 2023-01-30, 2023-01-31, 2023-02-01, 2023-02-02, 2023-02-03, 2023-02-04, 2023-02-05, 2023-02-06, 2023-02-07, 2023-02-08, 2023-02-09, 2023-02-10, 2023-02-11, 2023-02-12, 2023-02-13, 2023-02-14, 2023-02-15, 2023-02-16, 2023-02-17, 2023-02-18, 2023-02-19, 2023-02-20, 2023-02-21, 2023-02-22, 2023-02-23, 2023-02-24, 2023-02-25, 2023-02-26, 2023-02-27, 2023-02-28, 2023-03-01, 2023-03-02, 2023-03-03, 2023-03-04, 2023-03-05, 2023-03-06, 2023-03-07, 2023-03-08, 2023-03-09, 2023-03-10, 2023-03-11, 2023-03-12, 2023-03-13, 2023-03-14, 2023-03-15, 2023-03-16, 2023-03-17, 2023-03-18, 2023-03-19, 2023-03-20, 2023-03-21, 2023-03-22, 2023-03-23, 2023-03-24, 2023-03-25, 2023-03-26, 2023-03-27, 2023-03-28, 2023-03-29, 2023-03-30, 2023-03-31, 2023-04-01, 2023-04-02, 2023-04-03, 2023-04-04, 2023-04-05, 2023-04-06, 2023-04-07, 2023-04-08, 2023-04-09, 2023-04-10, 2023-04-11, 2023-04-12, 2023-04-13, 2023-04-14, 2023-04-15, 2023-04-16, 2023-04-17, 2023-04-18, 2023-04-19, 2023-04-20, 2023-04-21, 2023-04-22, 2023-04-23, 2023-04-24, 2023-04-25, 2023-04-26, 2023-04-27, 2023-04-28, 2023-04-29, 2023-04-30, 2023-05-01, 2023-05-02, 2023-05-03, 2023-05-04, 2023-05-05, 2023-05-06, 2023-05-07, 2023-05-08, 2023-05-09, 2023-05-10, 2023-05-11, 2023-05-12, 2023-05-13, 2023-05-14, 2023-05-15, 2023-05-16, 2023-05-17, 2023-05-18, 2023-05-19, 2023-05-20, 2023-05-21, 2023-05-22, 2023-05-23, 2023-05-24, 2023-05-25, 2023-05-26, 2023-05-27, 2023-05-28, 2023-05-29, 2023-05-30, 2023-05-31, 2023-06-01, 2023-06-02, 2023-06-03, 2023-06-04, 2023-06-05, 2023-06-06, 2023-06-07, 2023-06-08, 2023-06-09, 2023-06-10, 2023-06-11, 2023-06-12, 2023-06-13, 2023-06-14, 2023-06-15, 2023-06-16, 2023-06-17, 2023-06-18, 2023-06-19, 2023-06-20, 2023-06-21, 2023-06-22, 2023-06-23, 2023-06-24, 2023-06-25, 2023-06-26, 2023-06-27, 2023-06-28, 2023-06-29, 2023-06-30, 2023-07-01, 2023-07-02, 2023-07-03, 2023-07-04, 2023-07-05, 2023-07-06, 2023-07-07, 2023-07-08, 2023-07-09, 2023-07-10, 2023-07-11, 2023-07-12, 2023-07-13, 2023-07-14, 2023-07-15, 2023-07-16, 2023-07-17, 2023-07-18, 2023-07-19, 2023-07-20, 2023-07-21, 2023-07-22, 2023-07-23, 2023-07-24, 2023-07-25, 2023-07-26, 2023-07-27, 2023-07-28, 2023-07-29, 2023-07-30, 2023-07-31, 2023-08-01, 2023-08-02, 2023-08-03, 2023-08-04, 2023-08-05, 2023-08-06, 2023-08-07, 2023-08-08, 2023-08-09, 2023-08-10, 2023-08-11, 2023-08-12, 2023-08-13, 2023-08-14, 2023-08-15, 2023-08-16, 2023-08-17, 2023-08-18, 2023-08-19, 2023-08-20, 2023-08-21, 2023-08-22, 2023-08-23, 2023-08-24, 2023-08-25, 2023-08-26, 2023-08-27, 2023-08-28, 2023-08-29, 2023-08-30, 2023-08-31, 2023-09-01, 2023-09-02, 2023-09-03, 2023-09-04, 2023-09-05, 2023-09-06, 2023-09-07, 2023-09-08, 2023-09-09, 2023-09-10, 2023-09-11, 2023-09-12, 2023-09-13, 2023-09-14, 2023-09-15, 2023-09-16, 2023-09-17, 2023-09-18, 2023-09-19, 2023-09-20, 2023-09-21, 2023-09-22, 2023-09-23, 2023-09-24, 2023-09-25, 2023-09-26, 2023-09-27, 2023-09-28, 2023-09-29, 2023-09-30, 2023-10-01, 2023-10-02, 2023-10-03, 2023-10-04, 2023-10-05, 2023-10-06, 2023-10-07, 2023-10-08, 2023-10-09, 2023-10-10, 2023-10-11, 2023-10-12, 2023-10-13, 2023-10-14, 2023-10-15, 2023-10-16, 2023-10-17, 2023-10-18, 2023-10-19, 2023-10-20, 2023-10-21, 2023-10-22, 2023-10-23, 2023-10-24, 2023-10-25, 2023-10-26, 2023-10-27, 2023-10-28, 2023-10-29, 2023-10-30, 2023-10-31, 2023-11-01, 2023-11-02, 2023-11-03, 2023-11-04, 2023-11-05, 2023-11-06, 2023-11-07, 2023-11-08, 2023-11-09, 2023-11-10, 2023-11-11, 2023-11-12, 2023-11-13, 2023-11-14, 2023-11-15, 2023-11-16, 2023-11-17, 2023-11-18, 2023-11-19, 2023-11-20, 2023-11-21, 2023-11-22, 2023-11-23, 2023-11-24, 2023-11-25, 2023-11-26, 2023-11-27, 2023-11-28, 2023-11-29, 2023-11-30, 2023-12-01, 2023-12-02, 2023-12-03, 2023-12-04, 2023-12-05, 2023-12-06, 2023-12-07, 2023-12-08, 2023-12-09, 2023-12-10, 2023-12-11, 2023-12-12, 2023-12-13, 2023-12-14, 2023-12-15, 2023-12-16, 2023-12-17, 2023-12-18, 2023-12-19, 2023-12-20, 2023-12-21, 2023-12-22, 2023-12-23, 2023-12-24, 2023-12-25, 2023-12-26, 2023-12-27, 2023-12-28, 2023-12-29, 2023-12-30, 2023-12-31]
</code>
</pre>

Note: this is how the result looks like for 2023

----

<!-- .slide: data-background-image="resources/formatting.png" -->

## Formatting ranges

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #84d4f6" -->

Source: [pennaspillo.it](https://pennaspillo.it/it/londra/casette-pastello-londra-dove-trovarle/)

<!-- .element: style="font-size: 0.5em" -->

Note: Next thing we want to do is take the list of dates and format them as we want to calendar to look like. To that end we will use what is perhaps the most important addition to C++23 and that is the ability to format and print ranges, as part of the new format library.


---

### Standard

<pre>
<code class="lang-cpp" data-trim data-line-numbers="|1|2|3|4" data-fragment-index="0">
///compiler=clang1810
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
std::println("{}", std::tuple{42, 16});
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

Note: A really nice addition, not directly related to ranges is `std::println` which, as you probably guessed, prints the data followed by a new line.
We now have the ability to print standard containers. Different types use different formatting by default. `vector` is formatted using square brackets. 
Associative containers like `set`, use curly braces. `tuple`s and `pair`s have parentheses and `std::map` is printed as a list of key colon value.
---

<!-- .slide: data-auto-animate -->

### format specifiers

```nohighlight
"{ arg-id (optional) }"
"{ arg-id (optional) : format-spec }"
```

<!-- .element: data-id="formats" -->

Note: to customize the formatting, use use what's called a format specifier, which in C++20 was a pair of curly braces, optionally containing an index for the
argument you want to be put in this place, followed by colon and some text which tells the format library exactly how to format this argument, like width and 
precision.

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

Note: for formatting a range's elements we add an additional colon format-spec. and if that's a range of ranges we can add more format specs this way as needed.

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

Note: The first and second line print vector according to the default formatting. on the third line we specify to print that vector right aligned in a 20 character space,
padded by hashes. On the last line, in addition, we specify that the elements should be formatted as hexadecimal numbers. The width, alignment and padding of the range
are kept.  

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

Note: there are new format specifies like question mark for escaping a string, `m` for printing a pair as a key colon value and `n` for omitting the outer delimiters.

----

<!-- .slide: data-background-image="resources/windowing.jpg" -->


## Windowing views

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #faf1be" -->

Source: [Mark Waugh](https://www.markwaugh.net/-/galleries/portfolios/travel/uk/manchester/manchester-skyline-at-night/-/medias/556a2fb0-19ce-4b1e-af43-ddb688731c23-manchester-city-centre-skyline/)

<!-- .element: style="font-size: 0.5em" -->

Note: Another thing we need to do is split to list of dates to months to be able to print them side by side. C++23 also gives us this capability and other similar windowing views.

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

Note: chunk slices the input range to sub-ranges of a fixed size, 2 in this example.

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

Note: the last chunk might be smaller then the chunk size.

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

Note: slide provides a running window to the range, again of a fixed size, stepping one element at a time.

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

Note: Here, all the chunks have the same size.

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

Note: stride generates a range of elements at equal distance.

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

Note: You might notice that in fact all those adaptors are a specialization of a generic windowing view which can have arbitrary size, step and whether we want partial chunks at the end, which brings up the question of why the standard provides this generic version and implement all the others using it and the answer is that it would make each of them use more space for the unneeded parameters and also that calling this hypothetical algorithms would require two numbers and a bool and since we don't have named arguments would obfuscate calling code.


---

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

Note: it is strait forward, however, to implement a generic windowing adaptor by combining slide and stride like this. supporting the third parameter is left as an exercise for the viewers. Notice the usage of designated initializers to emulate named arguments.

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

Note: the last windowing view is `chunk_by` which gets a binary predicate and splits the range whenever the predicate returns false on adjacent elements.

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

Note: in this example, we produce a list of the monotonically increasing sub-sequences of the input range.

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

Note: if you are a range-v3 user, it used to have a `group_by` adaptor which has a slightly different behavior. 
This is now deprecated and replaced by `chunk_by` as well.

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

Note: going back to the calendar, we chunk the list of dates we generated previously to months using `chunk_by`.

---

<!-- .slide: data-auto-animate class="aside" -->

### `dates()`

<pre data-id="dates" style="font-size: 0.45em;">
<code data-trim class="text" style="white-space: pre-wrap;">
[2023-01-01, 2023-01-02, 2023-01-03, 2023-01-04, 2023-01-05, 2023-01-06, 2023-01-07, 2023-01-08, 2023-01-09, 2023-01-10, 2023-01-11, 2023-01-12, 2023-01-13, 2023-01-14, 2023-01-15, 2023-01-16, 2023-01-17, 2023-01-18, 2023-01-19, 2023-01-20, 2023-01-21, 2023-01-22, 2023-01-23, 2023-01-24, 2023-01-25, 2023-01-26, 2023-01-27, 2023-01-28, 2023-01-29, 2023-01-30, 2023-01-31, 2023-02-01, 2023-02-02, 2023-02-03, 2023-02-04, 2023-02-05, 2023-02-06, 2023-02-07, 2023-02-08, 2023-02-09, 2023-02-10, 2023-02-11, 2023-02-12, 2023-02-13, 2023-02-14, 2023-02-15, 2023-02-16, 2023-02-17, 2023-02-18, 2023-02-19, 2023-02-20, 2023-02-21, 2023-02-22, 2023-02-23, 2023-02-24, 2023-02-25, 2023-02-26, 2023-02-27, 2023-02-28, 2023-03-01, 2023-03-02, 2023-03-03, 2023-03-04, 2023-03-05, 2023-03-06, 2023-03-07, 2023-03-08, 2023-03-09, 2023-03-10, 2023-03-11, 2023-03-12, 2023-03-13, 2023-03-14, 2023-03-15, 2023-03-16, 2023-03-17, 2023-03-18, 2023-03-19, 2023-03-20, 2023-03-21, 2023-03-22, 2023-03-23, 2023-03-24, 2023-03-25, 2023-03-26, 2023-03-27, 2023-03-28, 2023-03-29, 2023-03-30, 2023-03-31, 2023-04-01, 2023-04-02, 2023-04-03, 2023-04-04, 2023-04-05, 2023-04-06, 2023-04-07, 2023-04-08, 2023-04-09, 2023-04-10, 2023-04-11, 2023-04-12, 2023-04-13, 2023-04-14, 2023-04-15, 2023-04-16, 2023-04-17, 2023-04-18, 2023-04-19, 2023-04-20, 2023-04-21, 2023-04-22, 2023-04-23, 2023-04-24, 2023-04-25, 2023-04-26, 2023-04-27, 2023-04-28, 2023-04-29, 2023-04-30, 2023-05-01, 2023-05-02, 2023-05-03, 2023-05-04, 2023-05-05, 2023-05-06, 2023-05-07, 2023-05-08, 2023-05-09, 2023-05-10, 2023-05-11, 2023-05-12, 2023-05-13, 2023-05-14, 2023-05-15, 2023-05-16, 2023-05-17, 2023-05-18, 2023-05-19, 2023-05-20, 2023-05-21, 2023-05-22, 2023-05-23, 2023-05-24, 2023-05-25, 2023-05-26, 2023-05-27, 2023-05-28, 2023-05-29, 2023-05-30, 2023-05-31, 2023-06-01, 2023-06-02, 2023-06-03, 2023-06-04, 2023-06-05, 2023-06-06, 2023-06-07, 2023-06-08, 2023-06-09, 2023-06-10, 2023-06-11, 2023-06-12, 2023-06-13, 2023-06-14, 2023-06-15, 2023-06-16, 2023-06-17, 2023-06-18, 2023-06-19, 2023-06-20, 2023-06-21, 2023-06-22, 2023-06-23, 2023-06-24, 2023-06-25, 2023-06-26, 2023-06-27, 2023-06-28, 2023-06-29, 2023-06-30, 2023-07-01, 2023-07-02, 2023-07-03, 2023-07-04, 2023-07-05, 2023-07-06, 2023-07-07, 2023-07-08, 2023-07-09, 2023-07-10, 2023-07-11, 2023-07-12, 2023-07-13, 2023-07-14, 2023-07-15, 2023-07-16, 2023-07-17, 2023-07-18, 2023-07-19, 2023-07-20, 2023-07-21, 2023-07-22, 2023-07-23, 2023-07-24, 2023-07-25, 2023-07-26, 2023-07-27, 2023-07-28, 2023-07-29, 2023-07-30, 2023-07-31, 2023-08-01, 2023-08-02, 2023-08-03, 2023-08-04, 2023-08-05, 2023-08-06, 2023-08-07, 2023-08-08, 2023-08-09, 2023-08-10, 2023-08-11, 2023-08-12, 2023-08-13, 2023-08-14, 2023-08-15, 2023-08-16, 2023-08-17, 2023-08-18, 2023-08-19, 2023-08-20, 2023-08-21, 2023-08-22, 2023-08-23, 2023-08-24, 2023-08-25, 2023-08-26, 2023-08-27, 2023-08-28, 2023-08-29, 2023-08-30, 2023-08-31, 2023-09-01, 2023-09-02, 2023-09-03, 2023-09-04, 2023-09-05, 2023-09-06, 2023-09-07, 2023-09-08, 2023-09-09, 2023-09-10, 2023-09-11, 2023-09-12, 2023-09-13, 2023-09-14, 2023-09-15, 2023-09-16, 2023-09-17, 2023-09-18, 2023-09-19, 2023-09-20, 2023-09-21, 2023-09-22, 2023-09-23, 2023-09-24, 2023-09-25, 2023-09-26, 2023-09-27, 2023-09-28, 2023-09-29, 2023-09-30, 2023-10-01, 2023-10-02, 2023-10-03, 2023-10-04, 2023-10-05, 2023-10-06, 2023-10-07, 2023-10-08, 2023-10-09, 2023-10-10, 2023-10-11, 2023-10-12, 2023-10-13, 2023-10-14, 2023-10-15, 2023-10-16, 2023-10-17, 2023-10-18, 2023-10-19, 2023-10-20, 2023-10-21, 2023-10-22, 2023-10-23, 2023-10-24, 2023-10-25, 2023-10-26, 2023-10-27, 2023-10-28, 2023-10-29, 2023-10-30, 2023-10-31, 2023-11-01, 2023-11-02, 2023-11-03, 2023-11-04, 2023-11-05, 2023-11-06, 2023-11-07, 2023-11-08, 2023-11-09, 2023-11-10, 2023-11-11, 2023-11-12, 2023-11-13, 2023-11-14, 2023-11-15, 2023-11-16, 2023-11-17, 2023-11-18, 2023-11-19, 2023-11-20, 2023-11-21, 2023-11-22, 2023-11-23, 2023-11-24, 2023-11-25, 2023-11-26, 2023-11-27, 2023-11-28, 2023-11-29, 2023-11-30, 2023-12-01, 2023-12-02, 2023-12-03, 2023-12-04, 2023-12-05, 2023-12-06, 2023-12-07, 2023-12-08, 2023-12-09, 2023-12-10, 2023-12-11, 2023-12-12, 2023-12-13, 2023-12-14, 2023-12-15, 2023-12-16, 2023-12-17, 2023-12-18, 2023-12-19, 2023-12-20, 2023-12-21, 2023-12-22, 2023-12-23, 2023-12-24, 2023-12-25, 2023-12-26, 2023-12-27, 2023-12-28, 2023-12-29, 2023-12-30, 2023-12-31]
</code>
</pre>

Note: we go from this

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

Note: to that


----

<!-- .slide: data-background-image="resources/generators.jpg" -->


## Generator

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #7f5030" -->

Source: [Albanpix/Rex Features](https://www.theguardian.com/environment/gallery/2014/may/02/the-beauty-of-windfarms-in-pictures#img-11)

<!-- .element: style="font-size: 0.5em" -->

Note: We mentioned range factories which generate their elements on the fly. A new type of this category is `std::generator`. I'm not going to delve deep
into coroutines in this talk. For that you gave other talks in this conference but will focus on the range aspects.

---

### `std::generator`

```cpp [1-8|10-11]
///compiler=clang1810
///options=-std=c++2b -stdlib=libc++
///hide
#include <utility>
#include <print>
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

Note: this is a classic fibonacci sequence implemented using coroutines. the resulting range is a move-only input range. This is because the coroutine state, held by the generator, is a unique resource.

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

Note: generators are parameterized on three types, `Ref`, `V` and allocator, There is also a polymorphic version in the `pmr` namespace.

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

See [P2529](https://wg21.link/p2529) for details.

Note: 
Those are exposition only types but the show the reference and value types of the range.
- `Yielded`: the type that should be passed to `co_yield`.
- `Reference`: the type that's returned when iterating
- `Value`: used primarily by `ranges::to`

In particular, the default reference type is an rvalue reference which means the value is moved out when the iterator is dereferenced. `T&&` is the default mainly for performance. 


---

### yielding ranges

```cpp
///compiler=clang1810
///options=-std=c++2b -stdlib=libc++
///output=["Hello", "Elements", "Of"]
///hide
#include "https://godbolt.org/z/1vfTWKdcf/code/1" // generator
#include <print>
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

Note: for yielding a sequence, instead of calling `co_yield` in a loop, you can use `elements_of` utility.

---

### recursive generator

```cpp
///hide
#include <generator>

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

Note: the same utility can be used to generate the elements of a recursive generator. 
See how elegant this in-order binary tree traversal algorithm looks using `std::generator`.

---

### implementing new adaptors

```cpp [1-12|14-17]
///compiler=clang1810
///options=-std=c++2b -stdlib=libc++
///hide
#include "https://godbolt.org/z/1vfTWKdcf/code/1" // generator
#include <array>
#include <vector>
#include <print>

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

Note: to me, the most useful thing `std::generator` enables is to easily implement new range adaptors. Here we implement `concat` by generating the elements of the
first sequence and recursively the elements of all the other ranges. There are of course down sides for going in this route. First, we only get an input range which a
direct implementation can usually do better. Secondly, there's the overhead of the coroutine mechanism which the compiler is not always able to optimize out. But for
the sake of this talk, it will work fine. 

----

<!-- .slide: data-background-image="resources/pov.jpg" -->

## changing point of view

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #aab283" -->

Source: [Mark Sutcliffe](https://www.countryfile.com/go-outdoors/walks/top-10-easy-mountains-for-beginners/)

<!-- .element: style="font-size: 0.5em" -->

Note: As promised, we now come to an important change recently made and that is the definition of the view concept, which as you recall, should be 
a lightweight range which can be passed around freely between adaptors.

---

### Is this a view

```cpp
///hide
#include <generator>

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

Note: this adaptor does own all the elements of the given ranges (passed by value), at least until they are moved outside during iteration, so can it be considered a view?

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

Note: this is the definition of view from the first paper that introduced ranges to the standard library. Notice the slightly different syntax as this was before
concepts were finalized. Also, at first, ranges were called iterables and views were, confusingly, called ranges.  

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

Note: Let's move to the paper which finally got merged into C++20. It defines that a view is a range that is semi-regular (that is default initializable and copyable),
has constant time copy, move and assignment and has shallow constness which usually means non-ownership.

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

// move assignment and destruction operators
// has constant time move construction, 
// enable_view detects shallow constness
```

<!-- .element: data-id="code" -->
    
[P1456](https://wg21.link/P1456): Move-only views 

Note: from then, a series of papers started relaxing the requirements to support more types of views, like move only views, for viewing unique types like input streams.
This version also fixes the lack of destruction complexity. Note that this implies that the example we mentioned of a ranging holding its elements by a `shared_ptr` is not a view actually because the last destructor will not have constant time complexity.

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

Note: it turned out that trying to auto detect views by types having shallow constness had to many false positives, and it was changed to be opt in but specializing the `enable_view` template.

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

Note: finally, the last change before C++20 dropped the requirement for default constructiblity because there's really no need to force that.

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

Note: this code tries to create a view of a temporary vector. It doesn't compile in C++20 because the view can not own its elements, otherwise its destructor wouldn't
have constant time complexity.

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

Note: the alternative is to save the vector in a variable which the view can the reference.

---

### `owning_view`

```cpp
///hide
#include <ranges>

namespace ranges = std::ranges;

///unhide
template<std::ranges::range T>
struct owning_view : ranges::view_interface<owning_view<T>> {
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

Note: but what would actually go wrong if we defined a (move only) owning view?

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

Note: Has the complexity of the code worsened? We still construct the vector once, move it inside the owning view and destructing it once. 
The most we added is a couple of moves and avoiding reference semantics might even enable the compiler to make that faster than the lvalue
version.

---

### `views::all`

> Given a subexpression `E`, the expression `views​::​all(E)` is expression-equivalent to:
> - `decay-copy(E)` if the decayed type of `E` models view.
> - Otherwise, `ref_­view{E}` if that expression is well-formed.
> - Otherwise, <mark>`owning_­view{E}`</mark>.

<!-- .element: style="text-align: left; width: 100%; font-size: 0.8em" -->

Note: indeed, in C++23 (in fact it was applied to C++20 as a defect report) `views::all`, which is the adaptor that always runs first in a pipeline to convert the input range to a view, now creates an owning view when given an rvalue range.

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

Note: the way the view concept is now defined by the standard is a fairly complex way to tell that using views should not increase the complexity of the code. 

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

Note: the first version of this example now compiles and a range holding its elements by a shared_ptr is now indeed a view as well as concat.   

----

<!-- .slide: data-background-image="resources/piping.jpg" -->

## Piping user-defined range adaptors

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #fea98e" -->


Source: [mybestplace.com](https://www.mybestplace.com/en/article/singing-ringing-tree-the-tree-that-sings-with-the-blowing-wind)

<!-- .element: style="font-size: 0.5em" -->

Note: One last C++23 addition we are going to use for the next step in the calendar software is the ability to easily make
custom views composable using the pipe operator.
Enabling users to write their own range adaptors that inter-operate well with standard library adaptors, will remove the urgency of adding 
more adaptors to the standard library. 

---

### range adaptor object

`$$
 adaptor(range, args...) \equiv \\
 adaptor(args...)(range) \equiv \\
 range\:|\:adaptor(args...) 
$$`

e.g. `views::transform`

Note: the world of range adaptors have two important concepts. The first is a range adaptor object, such as views::transform that accepts a viewable
range and optionally more arguments and produces a view over that range. We interact with the adaptor by calling it while passing the range and the 
other arguments (the first line in the definition) or by only passing the other arguments and calling the result with the range or, more frequently, 
piping the range in.
The object resulted from passing only the other arguments has a name...

---

### range adaptor closure object

<div class="r-stack">

<div class="fragment fade-out" data-fragment-index="0" style="width: 100%">

`$$ C(R) \equiv R\:|\:C $$`

e.g. `views::reverse`, `views::transform(f)`

</div>

<div class="fragment fade-in" data-fragment-index="0" style="width: 100%">

`$$ R\:|\:C\:|\:D \equiv R\:|\:(C\:|\:D) $$`

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


Note: and that is a range adaptor closure object, so a unary function object that accepts a `viewable_­range` argument and returns a view 
  such that the above equation holds. 

A closure composition is itself a closure, so the result of reverse_transform is still composable.


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

Note: A more complex adaptor, however, is not automatically a closure 

and so trying to pipe a range to it would fail to compile.  

---

### `ranges::range_adaptor_closure`

```cpp [1-7|2,9]
///compiler=clang1810
///options=-std=c++2b -stdlib=libc++
///fails=no match for 'operator|'
///hide
#include <ranges>
#include <vector>
#include <print>
#include "https://godbolt.org/z/1vfTWKdcf/code/1" // generator
#include "https://godbolt.org/z/n43nMfj58/code/1" // concat

#if defined(__clang__)

namespace std::ranges {

template <typename D>
using range_adaptor_closure = __range_adaptor_closure<D>;

}  // namespace std::ranges

#endif

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

Note: C++23 adds a utility type, range_adaptor_closure, that when deriving from makes a user defined type a closure

so replacing the lambda with a proper function object deriving from `range_adaptor_object` makes this snippet well formed.

---

### back to lambda

```c++ [1-13|15-20]
///compiler=clang1810
///options=-std=c++2b -stdlib=libc++
///hide
#include <ranges>
#include <vector>
#include <print>
#include "https://godbolt.org/z/1vfTWKdcf/code/1" // generator
#include "https://godbolt.org/z/n43nMfj58/code/1" // concat

#if defined(__clang__)

namespace std::ranges {

template <typename D>
using range_adaptor_closure = __range_adaptor_closure<D>;

}  // namespace std::ranges

#endif

namespace ranges = std::ranges;
namespace views = std::views;

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

Note: of course, we don't want to go back to the pre C++11 days where we had to define a function object before calling an algorithm
so we can have a generic closure object, wrapping a lambda and forwarding calls into it 

and when defining the lambda, we use `CTAD` to automatically deduce the closure type and we can pipe into it again. 

---

### case for adaptor

```cpp [18-24,26-27|1-16]
///compiler=clang1810
///options=-std=c++2b -stdlib=libc++
///hide
#include <ranges>
#include <vector>
#include <print>
#include "https://godbolt.org/z/1vfTWKdcf/code/1" // generator
#include "https://godbolt.org/z/n43nMfj58/code/1" // concat
namespace ranges = std::ranges;
namespace views = std::views;

namespace std {

constexpr inline auto bind_back(auto &&f, auto &&...args) {
    return [=](auto &&...args2) { return f(args2..., args...); };
}

#if defined(__clang__)

namespace ranges {

template <typename D>
using range_adaptor_closure = __range_adaptor_closure<D>;

#endif

}  // namespace ranges

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

Note: if our adaptor takes additional arguments as in this example, than we can wrap it with an adaptor object to make it composable.

We define adaptor as seen here. When called, it checks (at compile time) if it is already invocable, which will happen when the range is passed along the other
arguments, otherwise it returns a closure to be called later on. 

Note that `closure` and `adaptor`, unlike `range_adaptro_closure` are not standardized because the author, Berry Revzin, wasn't sure their the optimal solution
but its possible they will be in a future standard.

---

<!-- .slide: class="aside" -->

### formatting months

```cpp [30-39|34-37|28|1-5|9-24]
///hide
#include <ranges>
#include <string>
#include <chrono>

#include <https://godbolt.org/z/WG1caxrxn/code/1> // closure

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

///unhide
auto by_week() {
  return views::chunk_by([](date a, date b) {
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
        views::repeat(empty_week, 6 - week_count));
  });
}
```

<!-- .element: style="font-size: 0.4em" -->

Note: we can now continue developing the calender. we stopped with having a list of months which are themselves a list of dates.

To layout each month we concatenate the month title, followed by each week in that month, and padding with empty weeks in case there's less
than 6, for which we use the C++23 repeat adaptor.

The month title will be the center aligned month name.

To get the weeks we use `chunk_by` adaptor again comparing the first day of the week as there is no standard function to get the week number.

Then to format the list of weeks we right align the first week, and keep the rest left aligned. Here is where we used the closure utility
for making `format_weeks` composable. We will go back to `format_as_string` later on.

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

Note: Thus we go from this list of months

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

to a list of formatted months.

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

Note: we then want to print each 3 months side by side, so we chunk the months to groups of size 3. The way we will then tile them is to take
the first element of each month, which are the titles, and concatenate them. Then we'll take the second elements, which are the first weeks 
and concatenate them, and so on and so forth.

----

<!-- .slide: data-background-image="resources/zipping.jpg" -->

## zipping

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #6b5a52" -->


Source: [Simon Emmett Photography](https://www.countryfile.com/go-outdoors/best-fossil-hunting-destinations-uk/)

<!-- .element: style="font-size: 0.5em" -->

Note: The algorithm I just described is called zipping. If you've ever written in python I'm sure you ran into it.
But even if you never heard of it, this is quiet intuitive. Let's look at a simple example.

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

Note: Assume we have a list of student names and a list of their grades. We can easily pair a student with their grade using the zip adaptor.

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

```
[("Joan", 8.5), ("Ben", 7.1), ("Gina", 9), ("Tim", 9.5)]
```

Note: As you can see the in the output, each element is a tuple of a `string` and a double. This is very useful for "Structure of arrays" style of programming.

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

```
["Joan got 8.5", "Ben got 7.1", "Gina got 9", "Tim got 9.5"]
```


Note: many times we want to do additional processing over the tuples and for that we can use the `zip_transform` adaptor.

---

<!-- .slide: data-auto-animate -->

### reference and value type


```cpp
///compiler=vcpp_v19_35_x64
///options=/std:c++latest
///hide
#include <vector>
#include <ranges>
#include <string>

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

Note: As we said the elements of a zip view are tuples over the elements of the source ranges and the reference type is a tuple of all the reference types. 
Interestingly, it is not a reference by itself. 

---

### Sorting

<!-- .slide: data-auto-animate -->

```cpp
///libs=fmt:vcpkg
///compiler=vcpp_v19_35_x64
///options=/std:c++latest
///hide
#include <vector>
#include <ranges>
#include <string>
#include <format>
#include <algorithm>
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

```
["Tim", "Gina", "Joan", "Ben"]
```

Note: those reference semantics allows us, for example, to sort the list of student names by their grades, descending.

---

<!-- .slide: data-visibility="hidden" -->

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

<!-- .slide: data-visibility="hidden" -->

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

### more `tuple` assignment operators

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

Note: to enable that sorting, they had in fact to make some changes to tuple so it would play nicely with all the range concepts
and so more assignment operators were added

---

<!-- .slide: data-visibility="hidden" -->

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

<!-- .slide: data-visibility="hidden" data-auto-animate -->

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

<!-- .slide: data-visibility="hidden" data-auto-animate -->

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

<!-- .element: data-visibility="hidden"data-id="code" style="font-size: 0.5em" -->

---

<!-- .slide: data-visibility="hidden" -->

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

### more `tuple` constructors

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

Note: as well as more constructors, as if that list wasn't long enough, 
but I guess that's the price you have to pay to make user code cleaner.


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
std::println("{}", v); 
for (auto const [index, window]: 
  std::views::enumerate(v | std::views::adjacent<3>))
{
  std::println("{:>{}}{}", "", 3*index, window);
}
///hide
}
```

<!-- .element: data-id="code" -->

```
[1, 2, 3, 4, 5, 6]
(1, 2, 3)
   (2, 3, 4)
      (3, 4, 5)
         (4, 5, 6)
```

<!-- .element: data-id="output" -->

Note: adjacent is like slide, only for compile time size. 
We also use the new enumerate view which produces pair of elements along with their index.

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
std::println("{}", v); 
for (auto const [index, window]: 
  std::views::enumerate(v | std::views::pairwise))
{
  std::println("{:>{}}{}", "", 3*index, window);
}
///hide
}
```

<!-- .element: data-id="code" -->

```
[1, 2, 3, 4, 5, 6]
(1, 2)
   (2, 3)
      (3, 4)
         (4, 5)
            (5, 6)
```

<!-- .element: data-id="output" -->

Note: adjacent view with a size of two has an alias called `pairwise`.

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

```
all pairs = [(1, 1), (1, 2), (1, 3), (1, 4), (1, 5), (1, 6), (2, 1), ...
```

<!-- .element: data-id="output" style="font-size: 0.45em" -->

Note: finally, cartesian produce produces all the possible combinations of elements of the source ranges

---

<!-- .slide: class="aside" -->

### transposing months

```cpp [|7-9]
///hide
#include <ranges>
#include <string>
#include <generator>
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

Note: going back to building the calendar, we use `zip_transform` along with `concat` to tile the three months of each chunk side by side as we described.
In eric's implementation, this is called transpose and he used a custom adaptor called `transpose` which is in fact a run-time version of zip, the same way
that join is a runtime version of concat. This enables his implementation to support customizing the number of months in each row but had I followed the
same path I wouldn't be able to show `zip` view. 

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

Note: to visualize what we did, this step brings us from this list of groups of 3 months

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

Note: to this. This now shows almost like the requested output. The only thing left is

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

Note: to join the chunks to get a list of all the lines

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

Note: and the join the lines using the new `join_with` adaptor entering a new line character in between

----

<!-- .slide: data-background-image="resources/materialize.jpg" -->

## materialize

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #e2c4b8" -->


Source: [pmtoday.co.uk](https://www.pmtoday.co.uk/battersea-power-station-the-project-view/)

<!-- .element: style="font-size: 0.5em" -->

Note: remember `format_as_string`? The reason it's needed is that not all standard library implement range formatting yet 
so to print the range of characters we ended up with we need to convert that to a string. Luckily, we now have an adaptor for that.

---

<!-- .slide: data-auto-animate -->

### to vector

```cpp
///compiler=vcpp_v19_35_x64
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

Note: this adaptor is called `to` and it converts any range to a container, `vector` of `int`s in this example.

---

<!-- .slide: data-auto-animate -->

### Allocator anyone?

```cpp
///compiler=vcpp_v19_35_x64
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

Note: it's possible to pass an allocator if you're so inclined.

---

<!-- .slide: data-auto-animate -->

### type deduction

```cpp
///compiler=vcpp_v19_35_x64
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

Note: it also has the nice ability of deducing the element type from the source range's `value_type`

---

<!-- .slide: data-auto-animate -->

### range constructor

```cpp
///compiler=vcpp_v19_35_x64
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

Note: to enable this adaptor, all standard containers got new constructors accepting a tag object called `std::from_range` as the first argument.

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

Note: this is the way `to` works. It first tries to call a constructor accepting the same set of arguments it gets, if that fails to compile, it tries a constructor
taking `std::from_range`, the third priority is a passing begin and end separately finally resulting on reserve (if the container supports that) followed by repeated push backs. If it gets a nested range it recurses down.

---

<!-- .slide: data-auto-animate -->

### why `from_range_t`?

```cpp
///compiler=vcpp_v19_35_x64
///options=/std:c++latest
///hide
#include <ranges>
#include <vector>
#include <list>

int main() {
///unhide
std::list<int> l; 
std::vector v{l};
static_assert(std::same_as<decltype(v), std::vector<std::list<int>>>);
///hide
}
```

<!-- .element: data-id="code" style="font-size: 0.45em" -->

Note: This code deduces a vector of lists of size 1.

---

<!-- .slide: data-auto-animate -->

### why `from_range_t`?

```cpp
///compiler=vcpp_v19_35_x64
///options=/std:c++latest
///hide
#include <ranges>
#include <vector>
#include <list>

int main() {
///unhide
std::list<int> l; 
std::vector v{std::from_range, l};
static_assert(std::same_as<decltype(v), std::vector<int>>);
///hide
}
```

<!-- .element: data-id="code" style="font-size: 0.45em" -->

Note: now it is a range of `int`s copied from the list.


---

<!-- .slide: data-auto-animate -->

### composable

```cpp
///compiler=vcpp_v19_35_x64
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

Note: as any adaptor, it is composable using the pipe operator. 
It requires the empty parentheses though, and you can expect nasty errors when you forget them.

---

<!-- .slide: data-auto-animate -->

### value type conversion

```cpp
///compiler=vcpp_v19_35_x64
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

Note: we can request a container of a different type assuming the `value_type` is convertible 

---

<!-- .slide: data-auto-animate -->

### associative containers

```cpp
///compiler=vcpp_v19_35_x64
///options=/std:c++latest
///hide
#include <ranges>
#include <vector>
#include <unordered_map>

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

Note: a very convenient usage of `to` is to convert a zipped pair to an associative container, using the first sequence as keys and the second as values.

---

<!-- .slide: data-auto-animate -->

### nested ranges

```cpp
///compiler=vcpp_v19_35_x64
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

Note: `to` also supports range of ranges and can be useful for converting between different containers.

---

<!-- .slide: data-auto-animate -->

### User defined types

```cpp
///compiler=vcpp_v19_35_x64
///options=/std:c++latest
///hide
#include <ranges>

///unhide
template<typename T>
class MyContainer {
public:
  MyContainer(std::ranges::range auto&& r);

  T* begin() const;
  T* end() const;
};

template<std::ranges::range Rng>
MyContainer(Rng&& r) 
  -> MyContainer<std::ranges::range_value_t<Rng>>;

///hide
template<typename T>
MyContainer<T>::MyContainer(std::ranges::range auto&& r) {}

int main() {
///unhide
auto numbers = std::views::iota(1, 10);
auto cont = std::ranges::to<MyContainer>(numbers);
///hide
}
```

<!-- .element: data-id="code" style="font-size: 0.5em" -->

Note: to support your own custom containers, just make sure one of the appropriate constructors is defined.

---

### more methods

```cpp
///compiler=vcpp_v19_35_x64
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

Note: besides additional constructors, we also got more convenience methods to insert ranges to containers. unfortunately, `std::array` is not supported yet.

---

<!-- .slide: class="aside" -->

### `format_as_string`

```cpp
///hide
#include <format>
#include <ranges>

///unhide
namespace detail {
template <typename Rng>
auto format_as_string(const std::string_view fmt, Rng&& rng) {
  return std::vformat(fmt, std::make_format_args(
#if __cpp_lib_format_ranges < 202207L
    std::forward<Rng>(rng) | std::ranges::to<std::string>()
#else
  std::forward<Rng>(rng)
#endif
  ));
}
}  // namespace detail
```

Note: here is the implementation of `format_as_string`. If ranges formatting is supported I use that, otherwise, I convert the range to a string.

---

<!-- .slide: class="aside" -->

# live demo

----

<!-- .slide: data-background-image="resources/folding.png" -->


## folding

<!-- .element: class="r-stretch" style="text-shadow: 2px 2px 2px black; color: #d3e9fa" -->

Source: [wessexcoastgeology.soton.ac.uk](https://wessexcoastgeology.soton.ac.uk/Stair-Hole-Lulworth.htm)

<!-- .element: style="font-size: 0.5em" -->

Note: remember how `std::accumulate` didn't have a ranges version in C++20? now we can forget about accumulate because C++23 
has an even better algorithms for folding a sequence, which is what accumulate actually does. We didn't need it for calendar
but I think it's too important to not mention it in this talk.

---

### `std::fold_left`

```cpp [5-7]
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

Note: this is in fact a whole family of algorithms, starting with `fold_left` that takes an initial value and a binary callable and executes the callable on the initial value and the first element of the sequence. It then takes the result of that and plugs it into the callable along with the second element, and so on. 

---

### examples

```cpp [1,4-6|1,4,8-9]
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

Note: those algorithms live in the `algorithm` header, not `numeric`.

If you pass `std::plus` to `fold_left` you get accumulate back, but the function is not defaulted to `std::plus` because this algorithm is useful for much more than addition for example multiplication.

---

### examples

```cpp [2-9]
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

Note: but not just math operations. we can use it to format the range in interesting ways. anyone want to guess what this prints?

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

Note: in fact, many of the standard algorithms could be implemented using `fold`. Let's look at `count_if` for example. we want to count how many times the
predicate returns true, so the initial value is 0 and we increment the accumulator every time the predicate returns true, otherwise, we keep it the same.

---

### `std::fold_right`

```cpp [5-7]
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

Note: symmetrically, we have `fold_right`, that pass `init` as the right argument and also iterates the sequence in reverse order.

---

### examples

```cpp [2-9]
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

Note: I couldn't think of any obvious case where you'd need `fold_right` but we can do the same trick as before to print its definition.

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

Note: let's now try implement the `min` algorithm using fold_left. what would be the initial value?

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

Note: the first element. but now we compare the first element to itself, which is redundant and also limit ourselves to forward ranges because we call begin
twice.

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

Note: that the motivation for an additional algorithm in this family, `fold_left_first` which takes no initial value and just starts calling
the function on the first two elements. 

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

Note: to support empty sequences, it has to return an optional, so we actually need to dereference it to get the value.
This will result in undefined behavior if the range is empty but that's how the standard min is defined.

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

Note: remember this law? to follow it we'd need to return the end iterator along with the result of folding but that would make every call site cumbersome,
because in 99% of the cases the caller has no use for that. 

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

Note: for that the standard provides `fold_left_with_iter` which returns the end iterator.

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

Note: this table shows all the fold family in C++ 23.

----

## resources

- C++23 calendar: https://godbolt.org/z/qvqW8vYrq
- range-v3 calendar: https://github.com/ericniebler/range-v3/blob/master/example/calendar.cpp
- Eric's 2015 talk: https://youtu.be/mFUXNMfaciE
- Berry's 2022 talk about ranges formatting: https://youtu.be/EQELdyecZlU
- A Plan for C++23 Ranges: https://wg21.link/p2214

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
