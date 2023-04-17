////////////////////////////////////////////////////////////////
// Reference implementation of repeat_view proposal P2474
// https://github.com/microsoft/STL/pull/3142
//
#pragma once


#include <ranges>

#if __cpp_lib_ranges_repeat < 202207L

_STD_BEGIN
namespace ranges {

    template <move_constructible _Ty, semiregular _Bo = unreachable_sentinel_t>
        requires (is_object_v<_Ty> && same_as<_Ty, remove_cv_t<_Ty>>
                  && (_Integer_like<_Bo> || same_as<_Bo, unreachable_sentinel_t>) )
    class repeat_view : public view_interface<repeat_view<_Ty, _Bo>> {
    private:
        friend views::_Take_fn;
        friend views::_Drop_fn;

        class _Iterator {
        private:
            friend repeat_view;
            using _Index_type = conditional_t<same_as<_Bo, unreachable_sentinel_t>, ptrdiff_t, _Bo>;

            const _Ty* _Value{};
            /* [[no_unique_address]] */ _Index_type _Current{};

            constexpr explicit _Iterator(const _Ty* _Val, _Index_type _Bo_ = _Index_type{}) noexcept // strengthened
                : _Value(_Val), _Current(_Bo_) {}

        public:
            using iterator_concept  = random_access_iterator_tag;
            using iterator_category = random_access_iterator_tag;
            using value_type        = _Ty;
            using difference_type =
                conditional_t<_Signed_integer_like<_Index_type>, _Index_type, _Iota_diff_t<_Index_type>>;

            _Iterator() = default;

            _NODISCARD constexpr const _Ty& operator*() const noexcept {
                return *_Value;
            }

            constexpr _Iterator& operator++() noexcept /* strengthened */ {
#if _ITERATOR_DEBUG_LEVEL != 0
                _STL_VERIFY(_Current < (numeric_limits<_Index_type>::max)(),
                    "cannot increment repeat_view iterator past end (integer overflow)");
#endif
                ++_Current;
                return *this;
            }
            constexpr _Iterator operator++(int) noexcept /* strengthened */ {
                auto _Tmp = *this;
                ++*this;
                return _Tmp;
            }

            constexpr _Iterator& operator--() noexcept /* strengthened */ {
#if _ITERATOR_DEBUG_LEVEL != 0
                if constexpr (!is_same_v<_Bo, unreachable_sentinel_t>) {
                    _STL_VERIFY(_Current > 0, "cannot decrement below 0");
                } else {
                    _STL_VERIFY(_Current > (numeric_limits<_Index_type>::min)(),
                        "cannot decrement repeat_view iterator past end (integer overflow)");
                }
#endif
                --_Current;
                return *this;
            }
            constexpr _Iterator operator--(int) noexcept /* strengthened */ {
                auto _Tmp = *this;
                --*this;
                return _Tmp;
            }

            constexpr _Iterator& operator+=(difference_type _Off) noexcept /* strengthened */ {
#if _ITERATOR_DEBUG_LEVEL != 0
                if constexpr (!is_same_v<_Bo, unreachable_sentinel_t>) {
                    _STL_VERIFY(_Current + _Off >= 0, "cannot subtract below 0");
                }
                if (_Off > 0) {
                    _STL_VERIFY(_Current <= (numeric_limits<_Index_type>::max)() - static_cast<_Index_type>(_Off),
                        "cannot advance repeat_view iterator past end (integer overflow)");
                } else {
                    _STL_VERIFY(_Current >= (numeric_limits<_Index_type>::min)() - static_cast<_Index_type>(_Off),
                        "cannot advance repeat_view iterator past end (integer overflow)");
                }
#endif //_ITERATOR_DEBUG_LEVEL != 0
                _Current += _Off;
                return *this;
            }
            constexpr _Iterator& operator-=(difference_type _Off) noexcept /* strengthened */ {
#if _ITERATOR_DEBUG_LEVEL != 0
                if constexpr (!is_same_v<_Bo, unreachable_sentinel_t>) {
                    _STL_VERIFY(_Current - _Off >= 0, "cannot subtract below 0");
                }
                if (_Off < 0) {
                    _STL_VERIFY(_Current <= (numeric_limits<_Index_type>::max)() + static_cast<_Index_type>(_Off),
                        "cannot advance repeat_view iterator past end (integer overflow)");
                } else {
                    _STL_VERIFY(_Current >= (numeric_limits<_Index_type>::min)() + static_cast<_Index_type>(_Off),
                        "cannot advance repeat_view iterator past end (integer overflow)");
                }
#endif // _ITERATOR_DEBUG_LEVEL != 0
                _Current -= _Off;
                return *this;
            }
            _NODISCARD constexpr const _Ty& operator[](difference_type _Idx) const noexcept {
                return *(*this + _Idx);
            }

            _NODISCARD_FRIEND constexpr bool operator==(const _Iterator& _Left, const _Iterator& _Right) noexcept
            /* strengthened */ {
                return _Left._Current == _Right._Current;
            }
            _NODISCARD_FRIEND constexpr auto operator<=>(const _Iterator& _Left, const _Iterator& _Right) noexcept
            /* strengthened */ {
                return _Left._Current <=> _Right._Current;
            }

            _NODISCARD_FRIEND constexpr _Iterator operator+(_Iterator _Iter, difference_type _Off) noexcept
            /* strengthened */ {
                _Iter += _Off;
                return _Iter;
            }
            _NODISCARD_FRIEND constexpr _Iterator operator+(difference_type _Off, _Iterator _Iter) noexcept
            /* strengthened */ {
                _Iter += _Off;
                return _Iter;
            }

            _NODISCARD_FRIEND constexpr _Iterator operator-(_Iterator _Iter, difference_type _Off) noexcept
            /* strengthened */ {
                _Iter -= _Off;
                return _Iter;
            }
            _NODISCARD_FRIEND constexpr difference_type operator-(
                const _Iterator& _Left, const _Iterator& _Right) noexcept /* strengthened */ {
                return static_cast<difference_type>(
                    static_cast<difference_type>(_Left._Current) - static_cast<difference_type>(_Right._Current));
            }
        };

        /* [[no_unique_address]] */ _Movable_box<_Ty> _Value{};
        /* [[no_unique_address]] */ _Bo _Bound{};

        template <class _Tuple, size_t... _Indices>
        _NODISCARD_CTOR constexpr repeat_view(_Tuple& _Val, index_sequence<_Indices...>, _Bo _Bound_) noexcept(
            is_nothrow_constructible_v<_Ty, tuple_element_t<_Indices, _Tuple>...>)
            : _Value(in_place, _Tuple_get<_Indices>(_STD move(_Val))...), _Bound(_Bound_) {}

    public:
        // clang-format off
        repeat_view() requires default_initializable<_Ty> = default;
        // clang-format on

        _NODISCARD_CTOR constexpr explicit repeat_view(const _Ty& _Value_, _Bo _Bound_ = _Bo{}) noexcept(
            is_nothrow_copy_constructible_v<_Ty>) // strengthened
            requires copy_constructible<_Ty>
            : _Value(in_place, _Value_), _Bound(_Bound_) {
#if _CONTAINER_DEBUG_LEVEL > 0
            if constexpr (!is_same_v<_Bo, unreachable_sentinel_t>) {
                _STL_VERIFY(_Bound >= 0, "Bound must be >= 0");
            }
#endif
        }
        _NODISCARD_CTOR constexpr explicit repeat_view(_Ty&& _Value_, _Bo _Bound_ = _Bo{}) noexcept(
            is_nothrow_move_constructible_v<_Ty>) // strengthened
            : _Value(in_place, _STD move(_Value_)), _Bound(_Bound_) {
#if _CONTAINER_DEBUG_LEVEL > 0
            if constexpr (!is_same_v<_Bo, unreachable_sentinel_t>) {
                _STL_VERIFY(_Bound >= 0, "Bound must be >= 0");
            }
#endif
        }
        template <class... _TArgs, class... _BArgs>
            requires constructible_from<_Ty, _TArgs...> && constructible_from<_Bo, _BArgs...>
        // clang-format off
        _NODISCARD_CTOR constexpr explicit repeat_view(piecewise_construct_t,
            tuple<_TArgs...> _Val_args, tuple<_BArgs...> _Bound_args = tuple<>{})
            noexcept(is_nothrow_constructible_v<_Ty, _TArgs...>) // strengthened
            // clang-format on
            : repeat_view(_Val_args, index_sequence_for<_TArgs...>{}, _STD make_from_tuple<_Bo>(_Bound_args)) {}

        _NODISCARD constexpr _Iterator begin() const noexcept /* strengthened */ {
            return _Iterator{_STD addressof(*_Value)};
        }
        _NODISCARD constexpr _Iterator end() const noexcept // strengthened
            requires (!same_as<_Bo, unreachable_sentinel_t>)
        {
            return _Iterator{_STD addressof(*_Value), _Bound};
        }
        _NODISCARD constexpr unreachable_sentinel_t end() const noexcept {
            return unreachable_sentinel;
        }

        _NODISCARD constexpr auto size() const noexcept // strengthened
            requires (!same_as<_Bo, unreachable_sentinel_t>)
        {
            return _STD _To_unsigned_like(_Bound);
        }
    };

    template <class _Ty, class _Bo>
    repeat_view(_Ty, _Bo) -> repeat_view<_Ty, _Bo>;

    namespace views {
        struct _Repeat_fn {
            template <class _Ty>
            _NODISCARD constexpr auto operator()(_Ty&& _Value) const
                noexcept(noexcept(repeat_view(_STD forward<_Ty>(_Value))))
                requires requires { repeat_view(_STD forward<_Ty>(_Value)); }
            {
                return repeat_view(_STD forward<_Ty>(_Value));
            }

            template <class _Ty1, class _Ty2>
            _NODISCARD constexpr auto operator()(_Ty1&& _Val1, _Ty2&& _Val2) const
                noexcept(noexcept(repeat_view(_STD forward<_Ty1>(_Val1), _STD forward<_Ty2>(_Val2))))
                requires requires { repeat_view(_STD forward<_Ty1>(_Val1), _STD forward<_Ty2>(_Val2)); }
            {
                return repeat_view(_STD forward<_Ty1>(_Val1), _STD forward<_Ty2>(_Val2));
            }
        };

        inline constexpr _Repeat_fn repeat;
    } // namespace views

}

_STD_END

#endif
