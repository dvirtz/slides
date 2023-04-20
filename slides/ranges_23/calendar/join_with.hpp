#pragma once

#include <ranges>
#include <variant>

namespace join {

using namespace std;
using namespace ranges;

#ifdef __clang__
template <class _Rng, class _Pat>  // TRANSITION, LLVM-47414
concept _Can_const_join_with =
    forward_range<const _Rng> && forward_range<const _Pat> &&
    is_reference_v<range_reference_t<const _Rng>> &&
    input_range<range_reference_t<const _Rng>>;
#endif  // ^^^ workaround ^^^

template <input_range _Vw, forward_range _Pat>
    requires view<_Vw> && input_range<range_reference_t<_Vw>> && view<_Pat> &&
             _Compatible_joinable_ranges<range_reference_t<_Vw>, _Pat>
class join_with_view;

template <class _Vw, class _Pat>
class join_with_view_base : public view_interface<join::join_with_view<_Vw, _Pat>> {
   private:
    struct _Cache_wrapper {
        template <input_iterator _Iter>
        constexpr _Cache_wrapper(
            _Not_quite_object::_Construct_tag,
            const _Iter&
                _It) noexcept(noexcept(static_cast<decltype(_Val)>(*_It)))
            : _Val(*_It) {}

        remove_cv_t<range_reference_t<_Vw>> _Val;
    };

   protected:
    /* [[no_unique_address]] */ _Non_propagating_cache<_Cache_wrapper>
        _Inner{};
};

template <class _Vw, class _Pat>
    requires is_reference_v<range_reference_t<_Vw>>
class join_with_view_base<_Vw, _Pat>
    : public view_interface<join::join_with_view<_Vw, _Pat>> {};

template <class _Vw, class _Pat>
class _Join_with_view_outer_iter_base : public join_with_view_base<_Vw, _Pat> {
   protected:
    _Non_propagating_cache<iterator_t<_Vw>> _Outer_it;
};

template <forward_range _Vw, class _Pat>
class _Join_with_view_outer_iter_base<_Vw, _Pat>
    : public join_with_view_base<_Vw, _Pat> {};

template <input_range _Vw, forward_range _Pat>
    requires view<_Vw> && input_range<range_reference_t<_Vw>> && view<_Pat> &&
             _Compatible_joinable_ranges<range_reference_t<_Vw>, _Pat>
class join_with_view : public _Join_with_view_outer_iter_base<_Vw, _Pat> {
   private:
    template <bool _Const>
    using _InnerRng = range_reference_t<_Maybe_const<_Const, _Vw>>;

    /* [[no_unique_address]] */ _Vw _Range{};
    /* [[no_unique_address]] */ _Pat _Pattern{};

    template <bool _Const>
    struct _Category_base {};

    template <bool _Const>
        requires forward_range<_Maybe_const<_Const, _Vw>> &&
                 forward_range<_InnerRng<_Const>> &&
                 is_reference_v<_InnerRng<_Const>>
    struct _Category_base<_Const> {
        using _Outer = _Maybe_const<_Const, _Vw>;
        using _Inner = _InnerRng<_Const>;
        using _PatternBase = _Maybe_const<_Const, _Pat>;

        using iterator_category = conditional_t<
            !is_reference_v<common_reference_t<
                range_reference_t<_Inner>, range_reference_t<_PatternBase>>>,
            input_iterator_tag,
            conditional_t<
                common_range<_Inner> && common_range<_PatternBase> &&
                    derived_from<typename iterator_traits<
                                     iterator_t<_Outer>>::iterator_category,
                                 bidirectional_iterator_tag> &&
                    derived_from<typename iterator_traits<
                                     iterator_t<_Inner>>::iterator_category,
                                 bidirectional_iterator_tag> &&
                    derived_from<typename iterator_traits<iterator_t<
                                     _PatternBase>>::iterator_category,
                                 bidirectional_iterator_tag>,
                bidirectional_iterator_tag,
                conditional_t<
                    derived_from<typename iterator_traits<
                                     iterator_t<_Outer>>::iterator_category,
                                 forward_iterator_tag> &&
                        derived_from<typename iterator_traits<
                                         iterator_t<_Inner>>::iterator_category,
                                     forward_iterator_tag> &&
                        derived_from<typename iterator_traits<iterator_t<
                                         _PatternBase>>::iterator_category,
                                     forward_iterator_tag>,
                    forward_iterator_tag, input_iterator_tag>>>;
    };

    template <bool _Const>
    class _Iterator_base : public _Category_base<_Const> {};

    template <bool _Const>
        requires forward_range<_Maybe_const<_Const, _Vw>>
    class _Iterator_base<_Const> : public _Category_base<_Const> {
       protected:
        using _OuterIter = iterator_t<_Maybe_const<_Const, _Vw>>;

        _Iterator_base() = default;
        constexpr explicit _Iterator_base(_OuterIter&& _Outer_it_)
            : _Outer_it(std::move(_Outer_it_)) {}

        /* [[no_unique_address]] */ _OuterIter _Outer_it{};
    };

    template <bool _Const>
    class _Iterator : public _Iterator_base<_Const> {
       private:
        friend join_with_view;

        using _Mybase = _Iterator_base<_Const>;
        using _Parent_t = _Maybe_const<_Const, join_with_view>;
        using _Base = _Maybe_const<_Const, _Vw>;
        using _PatternBase = _Maybe_const<_Const, _Pat>;

        using _OuterIter = iterator_t<_Base>;
        using _InnerIter = iterator_t<_InnerRng<_Const>>;
        using _PatternIter = iterator_t<_PatternBase>;

        // True if and only if the expression *i, where i is an iterator from
        // the outer range, is a glvalue:
        static constexpr bool _Deref_is_glvalue =
            is_reference_v<_InnerRng<_Const>>;

        _Parent_t* _Parent{};
        variant<_PatternIter, _InnerIter> _Inner_it{};

        constexpr _Iterator(_Parent_t& _Parent_, _OuterIter _Outer_)
            requires forward_range<_Base>
            : _Mybase(std::move(_Outer_)), _Parent(std::addressof(_Parent_)) {
            if (this->_Outer_it != ranges::end(_Parent->_Range)) {
                _Inner_it.template emplace<1>(ranges::begin(_Update_inner()));
                _Satisfy();
            }
        }

        constexpr explicit _Iterator(_Parent_t& _Parent_)
            requires(!forward_range<_Base>)
            : _Parent{std::addressof(_Parent_)} {
            if (*_Parent->_Outer_it != ranges::end(_Parent->_Range)) {
                _Inner_it.template emplace<1>(ranges::begin(_Update_inner()));
                _Satisfy();
            }
        }

        [[nodiscard]] constexpr _OuterIter& _Get_outer() noexcept {
            if constexpr (forward_range<_Base>) {
                return this->_Outer_it;
            } else {
                return *_Parent->_Outer_it;
            }
        }

        [[nodiscard]] constexpr const _OuterIter& _Get_outer() const noexcept {
            if constexpr (forward_range<_Base>) {
                return this->_Outer_it;
            } else {
                return *_Parent->_Outer_it;
            }
        }

        [[nodiscard]] constexpr auto& _Update_inner() {
            if constexpr (_Deref_is_glvalue) {
                return (*_Get_outer());
            } else {
                return _Parent->_Inner
                    ._Emplace(_Not_quite_object::_Construct_tag{}, _Get_outer())
                    ._Val;
            }
        }

        [[nodiscard]] constexpr auto& _Get_inner() noexcept {
            if constexpr (_Deref_is_glvalue) {
                return (*_Get_outer());
            } else {
                return (*_Parent->_Inner)._Val;
            }
        }

        template <class _Ret = void>
        constexpr _Ret _Visit_inner_it(auto&& _Func) const {
            if (_Inner_it.index() == 0) {
                return _Func(std::get<0>(_Inner_it));
            } else if (_Inner_it.index() == 1) {
                return _Func(std::get<1>(_Inner_it));
            } else {
                throw std::bad_variant_access();
            }
        }

        constexpr void _Satisfy() {
            for (;;) {
                if (_Inner_it.index() == 0) {
                    if (std::get<0>(_Inner_it) != ranges::end(_Parent->_Pattern)) {
                        break;
                    }

                    _Inner_it.template emplace<1>(ranges::begin(_Update_inner()));
                } else {
                    assert(_Inner_it.index() == 1);

                    if (std::get<1>(_Inner_it) != ranges::end(_Get_inner())) {
                        break;
                    }

                    auto& _Outer_it = _Get_outer();
                    ++_Outer_it;
                    if (_Outer_it == ranges::end(_Parent->_Range)) {
                        if constexpr (_Deref_is_glvalue) {
                            _Inner_it.template emplace<1>();
                        }
                        break;
                    }

                    _Inner_it.template emplace<0>(ranges::begin(_Parent->_Pattern));
                }
            }
        }

       public:
        using iterator_concept =  //
            conditional_t<
                _Deref_is_glvalue && bidirectional_range<_Base>  //
                    && _Bidi_common_range<_InnerRng<_Const>> &&
                    _Bidi_common_range<_PatternBase>,
                bidirectional_iterator_tag,
                conditional_t<_Deref_is_glvalue && forward_range<_Base> &&
                                  forward_range<_InnerRng<_Const>>,
                              forward_iterator_tag, input_iterator_tag>>;
        using value_type =
            common_type_t<iter_value_t<_InnerIter>, iter_value_t<_PatternIter>>;
        using difference_type =
            _Common_diff_t<_OuterIter, _InnerIter, _PatternIter>;

        // clang-format off
            _Iterator() requires default_initializable<_OuterIter> = default;
        // clang-format on

        constexpr _Iterator(_Iterator<!_Const> _It)
            requires _Const                                              //
                         && convertible_to<iterator_t<_Vw>, _OuterIter>  //
                         && convertible_to<iterator_t<_InnerRng<false>>,
                                           _InnerIter>                      //
                         && convertible_to<iterator_t<_Pat>, _PatternIter>  //
            : _Mybase(std::move(_It._Outer_it)), _Parent(_It._Parent) {
            switch (_It._Inner_it.index()) {
                case 0:
                    _Inner_it.emplace<0>(std::move(std::get<0>(_It._Inner_it)));
                    break;
                case 1:
                    _Inner_it.emplace<1>(std::move(std::get<1>(_It._Inner_it)));
                    break;
                default:
                    throw std::bad_variant_access();
            }
        }

        [[nodiscard]] constexpr decltype(auto) operator*() const {
            using _Ref = common_reference_t<iter_reference_t<_InnerIter>,
                                            iter_reference_t<_PatternIter>>;
            return _Visit_inner_it<_Ref>(
                [](auto&& _It) -> _Ref { return *_It; });
        }

        constexpr _Iterator& operator++() {
            switch (_Inner_it.index()) {
                case 0:
                    ++std::get<0>(_Inner_it);
                    break;
                case 1:
                    ++std::get<1>(_Inner_it);
                    break;
                default:
                    throw std::bad_variant_access();
            }
            _Satisfy();
            return *this;
        }

        constexpr void operator++(int) { ++*this; }

        constexpr _Iterator operator++(int)
            requires _Deref_is_glvalue && forward_iterator<_OuterIter> &&
                     forward_iterator<_InnerIter>
        {
            auto _Tmp = *this;
            ++*this;
            return _Tmp;
        }

        constexpr _Iterator& operator--()
            requires _Deref_is_glvalue && bidirectional_range<_Base>  //
                     && _Bidi_common_range<_InnerRng<_Const>> &&
                     _Bidi_common_range<_PatternBase>
        {
            auto& _Outer_it = _Get_outer();
            if (_Outer_it == ranges::end(_Parent->_Range)) {
                --_Outer_it;
                _Inner_it.emplace<1>(ranges::end(_Get_inner()));
            }

            for (;;) {
                if (_Inner_it.index() == 0) {
                    auto& _It = std::get<0>(_Inner_it);
                    if (_It == ranges::begin(_Parent->_Pattern)) {
                        --_Outer_it;
                        _Inner_it.emplace<1>(ranges::end(_Get_inner()));
                    } else {
                        break;
                    }
                } else if (_Inner_it.index() == 1) {
                    auto& _It = std::get<1>(_Inner_it);
                    if (_It == ranges::begin(_Get_inner())) {
                        _Inner_it.emplace<0>(
                            ranges::end(_Parent->_Pattern));
                    } else {
                        break;
                    }
                } else {
                    throw std::bad_variant_access();
                }
            }

            switch (_Inner_it.index()) {
                case 0:
                    --std::get<0>(_Inner_it);
                    break;
                case 1:
                    --std::get<1>(_Inner_it);
                    break;
                default:
                    throw std::bad_variant_access();
            }
            return *this;
        }

        constexpr _Iterator operator--(int)
            requires _Deref_is_glvalue && bidirectional_range<_Base>  //
                     && _Bidi_common_range<_InnerRng<_Const>> &&
                     _Bidi_common_range<_PatternBase>
        {
            auto _Tmp = *this;
            --*this;
            return _Tmp;
        }

        [[nodiscard]] friend constexpr bool operator==(const _Iterator& _Left,
                                                    const _Iterator& _Right)
            requires _Deref_is_glvalue
                     && forward_range<_Base> && equality_comparable<_InnerIter>
        {
            if (_Left._Outer_it != _Right._Outer_it) {
                return false;
            }

            if (_Left._Inner_it.index() != _Right._Inner_it.index()) {
                return false;
            }

            switch (_Left._Inner_it.index()) {
                case 0:
                    return std::get<0>(_Left._Inner_it) == std::get<0>(_Right._Inner_it);
                case 1:
                    return std::get<1>(_Left._Inner_it) == std::get<1>(_Right._Inner_it);
                default:
                    return true;
            }

            _STL_UNREACHABLE;
        }

        [[nodiscard]] friend constexpr decltype(auto) iter_move(
            const _Iterator& _It) {
            using _Rvalue_ref =
                common_reference_t<iter_rvalue_reference_t<_InnerIter>,
                                   iter_rvalue_reference_t<_PatternIter>>;
            return _It._Visit_inner_it<_Rvalue_ref>(ranges::iter_move);
        }

    //     friend constexpr void iter_swap(const _Iterator& _Left,
    //                                     const _Iterator& _Right)
    //         requires indirectly_swappable<_InnerIter, _PatternIter>
    //     {
    //         switch (_Left._Inner_it.index()) {
    //             case 0:
    //                 switch (_Right._Inner_it.index()) {
    //                     case 0:
    //                         return ranges::iter_swap(std::get<0>(_Left._Inner_it),
    //                                                  std::get<0>(_Right._Inner_it));
    //                     case 1:
    //                         return ranges::iter_swap(std::get<0>(_Left._Inner_it),
    //                                                  std::get<1>(_Right._Inner_it));
    //                     default:
    //                         break;
    //                 }
    //                 break;
    //             case 0:
    //                 switch (_Right._Inner_it.index()) {
    //                     case 0:
    //                         return ranges::iter_swap(std::get<1>(_Left._Inner_it),
    //                                                  std::get<0>(_Right._Inner_it));
    //                     case 1:
    //                         return ranges::iter_swap(std::get<1>(_Left._Inner_it),
    //                                                  std::get<1>(_Right._Inner_it));
    //                     default:
    //                         break;
    //                 }
    //                 break;
    //             default:
    //                 break;
    //         }

    //         throw std::bad_variant_access();
    //     }
    };

    template <bool _Const>
    class _Sentinel {
       private:
        friend join_with_view;

        using _Parent_t = _Maybe_const<_Const, join_with_view>;
        using _Base = _Maybe_const<_Const, _Vw>;

        /* [[no_unique_address]] */ sentinel_t<_Base> _Last{};

        constexpr explicit _Sentinel(_Parent_t& _Parent) noexcept(
            noexcept(ranges::end(_Parent._Range)) &&
            is_nothrow_move_constructible_v<sentinel_t<_Base>>)  // strengthened
            : _Last(ranges::end(_Parent._Range)) {}

        template <bool _OtherConst>
        [[nodiscard]] constexpr bool _Equal(const _Iterator<_OtherConst>& _It)
            const noexcept(noexcept(_Fake_copy_init<bool>(_It._Get_outer() ==
                                                          _Last))) {
            static_assert(
                sentinel_for<sentinel_t<_Base>,
                             iterator_t<_Maybe_const<_OtherConst, _Vw>>>);
            return _It._Get_outer() == _Last;
        }

       public:
        _Sentinel() = default;

        constexpr _Sentinel(_Sentinel<!_Const> _Se) noexcept(
            is_nothrow_constructible_v<sentinel_t<_Base>,
                                       sentinel_t<_Vw>>)  // strengthened
            requires _Const
                     && convertible_to<sentinel_t<_Vw>, sentinel_t<_Base>>
            : _Last(std::move(_Se._Last)) {}

        template <bool _OtherConst>
            requires sentinel_for<sentinel_t<_Base>,
                                  iterator_t<_Maybe_const<_OtherConst, _Vw>>>
        [[nodiscard]] friend constexpr bool operator==(
            const _Iterator<_OtherConst>& _Left,
            const _Sentinel&
                _Right) noexcept(noexcept(_Right
                                              ._Equal(
                                                  _Left))) /* strengthened */ {
            return _Right._Equal(_Left);
        }
    };

   public:
    // clang-format off
        join_with_view() requires default_initializable<_Vw> && default_initializable<_Pat> = default;
    // clang-format on

    constexpr explicit join_with_view(_Vw _Range_, _Pat _Pattern_) noexcept(
        is_nothrow_move_constructible_v<_Vw>&&
            is_nothrow_move_constructible_v<_Pat>)  // strengthened
        : _Range{std::move(_Range_)}, _Pattern{std::move(_Pattern_)} {}

    template <input_range _Rng>
        requires constructible_from<_Vw, views::all_t<_Rng>> &&
                     constructible_from<
                         _Pat, single_view<range_value_t<_InnerRng<false>>>>
    constexpr explicit join_with_view(_Rng&& _Range_, range_value_t<_InnerRng<false>> _Elem) noexcept(
        noexcept(_Vw(views::all(std::forward<_Rng>(_Range_)))) && noexcept(
            _Pat(views::single(std::move(_Elem)))))  // strengthened
        : _Range(views::all(std::forward<_Rng>(_Range_))),
          _Pattern(views::single(std::move(_Elem))) {}

    [[nodiscard]] constexpr _Vw base() const& noexcept(
        is_nothrow_copy_constructible_v<_Vw>) /* strengthened */
        requires copy_constructible<_Vw>
    {
        return _Range;
    }
    [[nodiscard]] constexpr _Vw base() && noexcept(
        is_nothrow_move_constructible_v<_Vw>) /* strengthened */ {
        return std::move(_Range);
    }

    [[nodiscard]] constexpr auto begin() {
        if constexpr (forward_range<_Vw>) {
            constexpr bool _Use_const = _Simple_view<_Vw> &&
                                        is_reference_v<_InnerRng<false>> &&
                                        _Simple_view<_Pat>;
            return _Iterator<_Use_const>{*this, ranges::begin(_Range)};
        } else {
            this->_Outer_it._Emplace(ranges::begin(_Range));
            return _Iterator<false>{*this};
        }
    }

    [[nodiscard]] constexpr auto begin() const
#ifdef __clang__  // TRANSITION, LLVM-47414
        requires _Can_const_join_with<_Vw, _Pat>
#else   // ^^^ workaround / no workaround vvv
        requires forward_range<const _Vw> && forward_range<const _Pat> &&
                 is_reference_v<_InnerRng<true>> && input_range<_InnerRng<true>>
#endif  // TRANSITION, LLVM-47414
    {
        return _Iterator<true>{*this, ranges::begin(_Range)};
    }

    [[nodiscard]] constexpr auto end() {
        constexpr bool _Both_simple = _Simple_view<_Vw> && _Simple_view<_Pat>;
        if constexpr (forward_range<_Vw>  //
                      && is_reference_v<_InnerRng<false>> &&
                      forward_range<_InnerRng<false>>  //
                      && common_range<_Vw> && common_range<_InnerRng<false>>) {
            return _Iterator<_Both_simple>{*this, ranges::end(_Range)};
        } else {
            return _Sentinel<_Both_simple>{*this};
        }
    }

    [[nodiscard]] constexpr auto end() const
#ifdef __clang__  // TRANSITION, LLVM-47414
        requires _Can_const_join_with<_Vw, _Pat>
#else   // ^^^ workaround / no workaround vvv
        requires forward_range<const _Vw> && forward_range<const _Pat> &&
                 is_reference_v<_InnerRng<true>> && input_range<_InnerRng<true>>
#endif  // TRANSITION, LLVM-47414
    {
        if constexpr (forward_range<_InnerRng<true>> && common_range<_Vw> &&
                      common_range<_InnerRng<true>>) {
            return _Iterator<true>{*this, ranges::end(_Range)};
        } else {
            return _Sentinel<true>{*this};
        }
    }
};

template <class _Rng, class _Pat>
join::join_with_view(_Rng&&, _Pat&&)
    -> join::join_with_view<views::all_t<_Rng>, views::all_t<_Pat>>;

template <input_range _Rng>
join::join_with_view(_Rng&&, range_value_t<range_reference_t<_Rng>>)
    -> join::join_with_view<views::all_t<_Rng>,
                      single_view<range_value_t<range_reference_t<_Rng>>>>;

namespace views {
struct _Join_with_fn {
    // clang-format off
            template <viewable_range _Rng, class _Pat>
            [[nodiscard]] constexpr auto operator()(_Rng&& _Range, _Pat&& _Pattern) const noexcept(
                noexcept(join::join_with_view(std::forward<_Rng>(_Range), std::forward<_Pat>(_Pattern)))) requires requires {
                join::join_with_view(std::forward<_Rng>(_Range), std::forward<_Pat>(_Pattern));
            }
            {  // clang-format on
        return join::join_with_view(std::forward<_Rng>(_Range),
                              std::forward<_Pat>(_Pattern));
    }

    template <class _Delim>
        requires constructible_from<decay_t<_Delim>, _Delim>
    [[nodiscard]] constexpr auto operator()(_Delim&& _Delimiter) const
        noexcept(is_nothrow_constructible_v<decay_t<_Delim>, _Delim>) {
            return _Range_closure<_Join_with_fn, decay_t<_Delim>>{_STD forward<_Delim>(_Delimiter)};
    }
};

inline constexpr _Join_with_fn join_with;
}  // namespace views

}