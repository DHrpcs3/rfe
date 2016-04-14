#pragma once
#include "types.h"
//#define WRAP_EXPR(expr) [&]{ return expr; }
//#define COPY_EXPR(expr) [=]{ return expr; }
//#define EXCEPTION(text, ...) fmt::exception(__FILE__, __LINE__, __FUNCTION__, text, ##__VA_ARGS__)
//#define VM_CAST(value) vm::impl_cast(value, __FILE__, __LINE__, __FUNCTION__)
#define IS_INTEGRAL(t) (std::is_integral<t>::value)
#define IS_INTEGER(t) (std::is_integral<t>::value || std::is_enum<t>::value)
#define IS_BINARY_COMPARABLE(t1, t2) (IS_INTEGER(t1) && IS_INTEGER(t2) && sizeof(t1) == sizeof(t2))

namespace rfe
{
	inline namespace core
	{
		static constexpr bool is_le_machine = true;
		template<typename T, std::size_t N, std::size_t M> class masked_array_t // array type accessed as (index ^ M)
		{
			T m_data[N];

		public:
			T& operator [](std::size_t index)
			{
				return m_data[index ^ M];
			}

			const T& operator [](std::size_t index) const
			{
				return m_data[index ^ M];
			}

			T& at(std::size_t index)
			{
				return (index ^ M) < N ? m_data[index ^ M] : throw std::out_of_range("Masked array");
			}

			const T& at(std::size_t index) const
			{
				return (index ^ M) < N ? m_data[index ^ M] : throw std::out_of_range("Masked array");
			}
		};

		template<typename T, std::size_t Size = sizeof(T)> struct se_storage
		{
			static_assert(!Size, "Bad se_storage<> type");
		};

		template<typename T> struct se_storage<T, 2>
		{
			using type = u16;

			[[deprecated]] static constexpr u16 swap(u16 src) // for reference
			{
				return (src >> 8) | (src << 8);
			}

			static inline u16 to(const T& src)
			{
				return _byteswap_ushort(reinterpret_cast<const u16&>(src));
			}

			static inline T from(u16 src)
			{
				const u16 result = _byteswap_ushort(src);
				return reinterpret_cast<const T&>(result);
			}
		};

		template<typename T> struct se_storage<T, 4>
		{
			using type = u32;

			[[deprecated]] static constexpr u32 swap(u32 src) // for reference
			{
				return (src >> 24) | (src << 24) | ((src >> 8) & 0x0000ff00) | ((src << 8) & 0x00ff0000);
			}

			static inline u32 to(const T& src)
			{
				return _byteswap_ulong(reinterpret_cast<const u32&>(src));
			}

			static inline T from(u32 src)
			{
				const u32 result = _byteswap_ulong(src);
				return reinterpret_cast<const T&>(result);
			}
		};

		template<typename T> struct se_storage<T, 8>
		{
			using type = u64;

			[[deprecated]] static constexpr u64 swap(u64 src) // for reference
			{
				return (src >> 56) | (src << 56) |
					((src >> 40) & 0x000000000000ff00) |
					((src >> 24) & 0x0000000000ff0000) |
					((src >> 8) & 0x00000000ff000000) |
					((src << 8) & 0x000000ff00000000) |
					((src << 24) & 0x0000ff0000000000) |
					((src << 40) & 0x00ff000000000000);
			}

			static inline u64 to(const T& src)
			{
				return _byteswap_uint64(reinterpret_cast<const u64&>(src));
			}

			static inline T from(u64 src)
			{
				const u64 result = _byteswap_uint64(src);
				return reinterpret_cast<const T&>(result);
			}
		};

		template<typename T> using se_storage_t = typename se_storage<T>::type;

		template<typename T1, typename T2> struct se_convert
		{
			using type_from = std::remove_cv_t<T1>;
			using type_to = std::remove_cv_t<T2>;
			using stype_from = se_storage_t<std::remove_cv_t<T1>>;
			using stype_to = se_storage_t<std::remove_cv_t<T2>>;
			using storage_from = se_storage<std::remove_cv_t<T1>>;
			using storage_to = se_storage<std::remove_cv_t<T2>>;

			static inline std::enable_if_t<std::is_same<type_from, type_to>::value, stype_to> convert(const stype_from& data)
			{
				return data;
			}

			static inline stype_to convert(const stype_from& data, ...)
			{
				return storage_to::to(storage_from::from(data));
			}
		};

		static struct se_raw_tag_t {} const se_raw{};

		template<typename T, bool Se = true> class se_t;

		// se_t with switched endianness
		template<typename T> class se_t<T, true>
		{
			using type = std::remove_cv_t<T>;
			using stype = se_storage_t<type>;
			using storage = se_storage<type>;

			stype m_data;

			static_assert(!std::is_union<type>::value && !std::is_class<type>::value, "se_t<> error: invalid type (struct or union)");
			static_assert(!std::is_pointer<type>::value, "se_t<> error: invalid type (pointer)");
			static_assert(!std::is_reference<type>::value, "se_t<> error: invalid type (reference)");
			static_assert(!std::is_array<type>::value, "se_t<> error: invalid type (array)");
			static_assert(!std::is_enum<type>::value, "se_t<> error: invalid type (enumeration), use integral type instead");
			static_assert(alignof(type) == alignof(stype), "se_t<> error: unexpected alignment");

			template<typename T2, bool = std::is_integral<T2>::value> struct bool_converter
			{
				static inline bool to_bool(const se_t<T2>& value)
				{
					return static_cast<bool>(value.value());
				}
			};

			template<typename T2> struct bool_converter<T2, true>
			{
				static inline bool to_bool(const se_t<T2>& value)
				{
					return value.m_data != 0;
				}
			};

		public:
			se_t() = default;

			se_t(const se_t& right) = default;

			inline se_t(type value)
				: m_data(storage::to(value))
			{
			}

			// construct directly from raw data (don't use)
			inline se_t(const stype& raw_value, const se_raw_tag_t&)
				: m_data(raw_value)
			{
			}

			inline type value() const
			{
				return storage::from(m_data);
			}

			// access underlying raw data (don't use)
			inline const stype& raw_data() const noexcept
			{
				return m_data;
			}

			se_t& operator =(const se_t&) = default;

			inline se_t& operator =(type value)
			{
				return m_data = storage::to(value), *this;
			}

			inline operator type() const
			{
				return storage::from(m_data);
			}

			// optimization
			explicit inline operator bool() const
			{
				return bool_converter<type>::to_bool(*this);
			}

			// optimization
			template<typename T2> inline std::enable_if_t<IS_BINARY_COMPARABLE(T, T2), se_t&> operator &=(const se_t<T2>& right)
			{
				return m_data &= right.raw_data(), *this;
			}

			// optimization
			template<typename CT> inline std::enable_if_t<IS_INTEGRAL(T) && std::is_convertible<CT, T>::value, se_t&> operator &=(CT right)
			{
				return m_data &= storage::to(right), *this;
			}

			// optimization
			template<typename T2> inline std::enable_if_t<IS_BINARY_COMPARABLE(T, T2), se_t&> operator |=(const se_t<T2>& right)
			{
				return m_data |= right.raw_data(), *this;
			}

			// optimization
			template<typename CT> inline std::enable_if_t<IS_INTEGRAL(T) && std::is_convertible<CT, T>::value, se_t&> operator |=(CT right)
			{
				return m_data |= storage::to(right), *this;
			}

			// optimization
			template<typename T2> inline std::enable_if_t<IS_BINARY_COMPARABLE(T, T2), se_t&> operator ^=(const se_t<T2>& right)
			{
				return m_data ^= right.raw_data(), *this;
			}

			// optimization
			template<typename CT> inline std::enable_if_t<IS_INTEGRAL(T) && std::is_convertible<CT, T>::value, se_t&> operator ^=(CT right)
			{
				return m_data ^= storage::to(right), *this;
			}
		};

		// se_t with native endianness
		template<typename T> class se_t<T, false>
		{
			using type = std::remove_cv_t<T>;

			type m_data;

			static_assert(!std::is_union<type>::value && !std::is_class<type>::value, "se_t<> error: invalid type (struct or union)");
			static_assert(!std::is_pointer<type>::value, "se_t<> error: invalid type (pointer)");
			static_assert(!std::is_reference<type>::value, "se_t<> error: invalid type (reference)");
			static_assert(!std::is_array<type>::value, "se_t<> error: invalid type (array)");
			static_assert(!std::is_enum<type>::value, "se_t<> error: invalid type (enumeration), use integral type instead");

		public:
			se_t() = default;

			se_t(const se_t&) = default;

			inline se_t(type value)
				: m_data(value)
			{
			}

			inline type value() const
			{
				return m_data;
			}

			se_t& operator =(const se_t& value) = default;

			inline se_t& operator =(type value)
			{
				return m_data = value, *this;
			}

			inline operator type() const
			{
				return m_data;
			}

			template<typename CT> inline std::enable_if_t<IS_INTEGRAL(T) && std::is_convertible<CT, T>::value, se_t&> operator &=(const CT& right)
			{
				return m_data &= right, *this;
			}

			template<typename CT> inline std::enable_if_t<IS_INTEGRAL(T) && std::is_convertible<CT, T>::value, se_t&> operator |=(const CT& right)
			{
				return m_data |= right, *this;
			}

			template<typename CT> inline std::enable_if_t<IS_INTEGRAL(T) && std::is_convertible<CT, T>::value, se_t&> operator ^=(const CT& right)
			{
				return m_data ^= right, *this;
			}
		};

		// se_t with native endianness (alias)
		template<typename T> using nse_t = se_t<T, false>;

		template<typename T, bool Se, typename T1> inline se_t<T, Se>& operator +=(se_t<T, Se>& left, const T1& right)
		{
			auto value = left.value();
			return left = (value += right);
		}

		template<typename T, bool Se, typename T1> inline se_t<T, Se>& operator -=(se_t<T, Se>& left, const T1& right)
		{
			auto value = left.value();
			return left = (value -= right);
		}

		template<typename T, bool Se, typename T1> inline se_t<T, Se>& operator *=(se_t<T, Se>& left, const T1& right)
		{
			auto value = left.value();
			return left = (value *= right);
		}

		template<typename T, bool Se, typename T1> inline se_t<T, Se>& operator /=(se_t<T, Se>& left, const T1& right)
		{
			auto value = left.value();
			return left = (value /= right);
		}

		template<typename T, bool Se, typename T1> inline se_t<T, Se>& operator %=(se_t<T, Se>& left, const T1& right)
		{
			auto value = left.value();
			return left = (value %= right);
		}

		template<typename T, bool Se, typename T1> inline se_t<T, Se>& operator <<=(se_t<T, Se>& left, const T1& right)
		{
			auto value = left.value();
			return left = (value <<= right);
		}

		template<typename T, bool Se, typename T1> inline se_t<T, Se>& operator >>=(se_t<T, Se>& left, const T1& right)
		{
			auto value = left.value();
			return left = (value >>= right);
		}

		template<typename T, bool Se> inline se_t<T, Se> operator ++(se_t<T, Se>& left, int)
		{
			auto value = left.value();
			auto result = value++;
			left = value;
			return result;
		}

		template<typename T, bool Se> inline se_t<T, Se> operator --(se_t<T, Se>& left, int)
		{
			auto value = left.value();
			auto result = value--;
			left = value;
			return result;
		}

		template<typename T, bool Se> inline se_t<T, Se>& operator ++(se_t<T, Se>& right)
		{
			auto value = right.value();
			return right = ++value;
		}

		template<typename T, bool Se> inline se_t<T, Se>& operator --(se_t<T, Se>& right)
		{
			auto value = right.value();
			return right = --value;
		}

		// optimization
		template<typename T1, typename T2> inline std::enable_if_t<IS_BINARY_COMPARABLE(T1, T2), bool> operator ==(const se_t<T1>& left, const se_t<T2>& right)
		{
			return left.raw_data() == right.raw_data();
		}

		// optimization
		template<typename T1, typename T2> inline std::enable_if_t<IS_INTEGRAL(T1) && IS_INTEGER(T2) && sizeof(T1) >= sizeof(T2), bool> operator ==(const se_t<T1>& left, T2 right)
		{
			return left.raw_data() == se_storage<T1>::to(right);
		}

		// optimization
		template<typename T1, typename T2> inline std::enable_if_t<IS_INTEGER(T1) && IS_INTEGRAL(T2) && sizeof(T1) <= sizeof(T2), bool> operator ==(T1 left, const se_t<T2>& right)
		{
			return se_storage<T2>::to(left) == right.raw_data();
		}

		// optimization
		template<typename T1, typename T2> inline std::enable_if_t<IS_BINARY_COMPARABLE(T1, T2), bool> operator !=(const se_t<T1>& left, const se_t<T2>& right)
		{
			return left.raw_data() != right.raw_data();
		}

		// optimization
		template<typename T1, typename T2> inline std::enable_if_t<IS_INTEGRAL(T1) && IS_INTEGER(T2) && sizeof(T1) >= sizeof(T2), bool> operator !=(const se_t<T1>& left, T2 right)
		{
			return left.raw_data() != se_storage<T1>::to(right);
		}

		// optimization
		template<typename T1, typename T2> inline std::enable_if_t<IS_INTEGER(T1) && IS_INTEGRAL(T2) && sizeof(T1) <= sizeof(T2), bool> operator !=(T1 left, const se_t<T2>& right)
		{
			return se_storage<T2>::to(left) != right.raw_data();
		}

		// optimization
		template<typename T1, typename T2> inline std::enable_if_t<IS_BINARY_COMPARABLE(T1, T2) && sizeof(T1) >= 4, se_t<decltype(T1() & T2())>> operator &(const se_t<T1>& left, const se_t<T2>& right)
		{
			return{ left.raw_data() & right.raw_data(), se_raw };
		}

		// optimization
		template<typename T1, typename T2> inline std::enable_if_t<IS_INTEGRAL(T1) && IS_INTEGER(T2) && sizeof(T1) >= sizeof(T2) && sizeof(T1) >= 4, se_t<decltype(T1() & T2())>> operator &(const se_t<T1>& left, T2 right)
		{
			return{ left.raw_data() & se_storage<T1>::to(right), se_raw };
		}

		// optimization
		template<typename T1, typename T2> inline std::enable_if_t<IS_INTEGER(T1) && IS_INTEGRAL(T2) && sizeof(T1) <= sizeof(T2) && sizeof(T2) >= 4, se_t<decltype(T1() & T2())>> operator &(T1 left, const se_t<T2>& right)
		{
			return{ se_storage<T2>::to(left) & right.raw_data(), se_raw };
		}

		// optimization
		template<typename T1, typename T2> inline std::enable_if_t<IS_BINARY_COMPARABLE(T1, T2) && sizeof(T1) >= 4, se_t<decltype(T1() | T2())>> operator |(const se_t<T1>& left, const se_t<T2>& right)
		{
			return{ left.raw_data() | right.raw_data(), se_raw };
		}

		// optimization
		template<typename T1, typename T2> inline std::enable_if_t<IS_INTEGRAL(T1) && IS_INTEGER(T2) && sizeof(T1) >= sizeof(T2) && sizeof(T1) >= 4, se_t<decltype(T1() | T2())>> operator |(const se_t<T1>& left, T2 right)
		{
			return{ left.raw_data() | se_storage<T1>::to(right), se_raw };
		}

		// optimization
		template<typename T1, typename T2> inline std::enable_if_t<IS_INTEGER(T1) && IS_INTEGRAL(T2) && sizeof(T1) <= sizeof(T2) && sizeof(T2) >= 4, se_t<decltype(T1() | T2())>> operator |(T1 left, const se_t<T2>& right)
		{
			return{ se_storage<T2>::to(left) | right.raw_data(), se_raw };
		}

		// optimization
		template<typename T1, typename T2> inline std::enable_if_t<IS_BINARY_COMPARABLE(T1, T2) && sizeof(T1) >= 4, se_t<decltype(T1() ^ T2())>> operator ^(const se_t<T1>& left, const se_t<T2>& right)
		{
			return{ left.raw_data() ^ right.raw_data(), se_raw };
		}

		// optimization
		template<typename T1, typename T2> inline std::enable_if_t<IS_INTEGRAL(T1) && IS_INTEGER(T2) && sizeof(T1) >= sizeof(T2) && sizeof(T1) >= 4, se_t<decltype(T1() ^ T2())>> operator ^(const se_t<T1>& left, T2 right)
		{
			return{ left.raw_data() ^ se_storage<T1>::to(right), se_raw };
		}

		// optimization
		template<typename T1, typename T2> inline std::enable_if_t<IS_INTEGER(T1) && IS_INTEGRAL(T2) && sizeof(T1) <= sizeof(T2) && sizeof(T2) >= 4, se_t<decltype(T1() ^ T2())>> operator ^(T1 left, const se_t<T2>& right)
		{
			return{ se_storage<T2>::to(left) ^ right.raw_data(), se_raw };
		}

		// optimization
		template<typename T> inline std::enable_if_t<IS_INTEGRAL(T) && sizeof(T) >= 4, se_t<decltype(~T())>> operator ~(const se_t<T>& right)
		{
			return{ ~right.raw_data(), se_raw };
		}

		template<typename T> using be_t = se_t<T, is_le_machine == true>;
		template<typename T> using le_t = se_t<T, is_le_machine == false>;

		template<typename T> struct is_be_t : public std::integral_constant<bool, false>
		{
		};

		template<typename T> struct is_be_t<be_t<T>> : public std::integral_constant<bool, true>
		{
		};

		template<typename T> struct is_be_t<const T> : public std::integral_constant<bool, is_be_t<T>::value>
		{
		};

		template<typename T> struct is_be_t<volatile T> : public std::integral_constant<bool, is_be_t<T>::value>
		{
		};

		// to_be_t helper struct
		template<typename T> struct to_be
		{
			using type = std::conditional_t<std::is_arithmetic<T>::value || std::is_enum<T>::value, be_t<T>, T>;
		};

		// be_t<T> if possible, T otherwise
		template<typename T> using to_be_t = typename to_be<T>::type;

		template<typename T> struct to_be<const T> // move const qualifier
		{
			using type = const to_be_t<T>;
		};

		template<typename T> struct to_be<volatile T> // move volatile qualifier
		{
			using type = volatile to_be_t<T>;
		};

		template<> struct to_be<void> { using type = void; };
		template<> struct to_be<bool> { using type = bool; };
		template<> struct to_be<char> { using type = char; };
		template<> struct to_be<u8> { using type = u8; };
		template<> struct to_be<s8> { using type = s8; };

		template<typename T> struct is_le_t : public std::integral_constant<bool, false>
		{
		};

		template<typename T> struct is_le_t<le_t<T>> : public std::integral_constant<bool, true>
		{
		};

		template<typename T> struct is_le_t<const T> : public std::integral_constant<bool, is_le_t<T>::value>
		{
		};

		template<typename T> struct is_le_t<volatile T> : public std::integral_constant<bool, is_le_t<T>::value>
		{
		};

		template<typename T> struct to_le
		{
			using type = std::conditional_t<std::is_arithmetic<T>::value || std::is_enum<T>::value, le_t<T>, T>;
		};

		// le_t<T> if possible, T otherwise
		template<typename T> using to_le_t = typename to_le<T>::type;

		template<typename T> struct to_le<const T> // move const qualifier
		{
			using type = const to_le_t<T>;
		};

		template<typename T> struct to_le<volatile T> // move volatile qualifier
		{
			using type = volatile to_le_t<T>;
		};

		template<> struct to_le<void> { using type = void; };
		template<> struct to_le<bool> { using type = bool; };
		template<> struct to_le<char> { using type = char; };
		template<> struct to_le<u8> { using type = u8; };
		template<> struct to_le<s8> { using type = s8; };

		// to_ne_t helper struct
		template<typename T> struct to_ne
		{
			using type = T;
		};

		template<typename T, bool Se> struct to_ne<se_t<T, Se>>
		{
			using type = T;
		};

		// restore native endianness for T: returns T for be_t<T> or le_t<T>, T otherwise
		template<typename T> using to_ne_t = typename to_ne<T>::type;

		template<typename T> struct to_ne<const T> // move const qualifier
		{
			using type = const to_ne_t<T>;
		};

		template<typename T> struct to_ne<volatile T> // move volatile qualifier
		{
			using type = volatile to_ne_t<T>;
		};

	}
}