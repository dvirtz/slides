module;

#include <range/v3/view/concat.hpp>
#include <ranges>
#include <memory>

export module concat;

// needed because range-v3 doesn't support move-only ranges
template <ranges::range Rng>
struct shared_view : public ranges::view_interface<shared_view<Rng>>
{
    using value_type = ranges::range_value_t<Rng>;
    shared_view() = default;
    shared_view(Rng rng) : ptr_{std::make_shared<Rng>(std::move(rng))} {
        static_assert(ranges::viewable_range<shared_view<Rng>>);
    }

    auto begin() const { return ptr_->begin(); }
    auto end() const { return ptr_->end(); }

private:
    std::shared_ptr<Rng> ptr_;
};

auto to_rangev3_view = [](auto rng) {
    if constexpr (ranges::viewable_range<decltype(rng)>)
    {
        return rng;
    }
    else
    {
        return shared_view{std::move(rng)};
    }
};

export inline constexpr auto concat = [](std::ranges::range auto... rngs)
    requires(sizeof...(rngs) >= 1)
{
        return ranges::views::concat(to_rangev3_view(std::move(rngs))...);
};
