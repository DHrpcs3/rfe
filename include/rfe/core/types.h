#pragma once

#include <typeinfo>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <array>
#include <cassert>
#include <type_traits>
#include <mutex>

#if defined(_MSC_VER) && !defined(__GNUG__)
#define EXTCONSTEXPR
#else
#define EXTCONSTEXPR constexpr
#endif

namespace rfe
{
	inline namespace core
	{
		inline namespace basic_types
		{
			using uchar = unsigned char;
			using ushort = unsigned short;
			using uint = unsigned int;
			using ulong = unsigned long;
			using ullong = unsigned long long;

			using byte = char;
			using ubyte = uchar;
			using llong = long long;

			using u8 = uint8_t;
			using u16 = uint16_t;
			using u32 = uint32_t;
			using u64 = uint64_t;

			using s8 = int8_t;
			using s16 = int16_t;
			using s32 = int32_t;
			using s64 = int64_t;

			using f32 = float;
			using f64 = double;

			union alignas(2) f16
			{
				u16 _u16;
				u8 _u8[2];

				f16() = default;
				f16(f32 value);

				operator f32() const;

			private:
				static f16 make(u16 raw);
				static f16 f32_to_f16(f32 value);
				static f32 f16_to_f32(f16 value);
			};
		}

		inline namespace helper_types
		{
			struct ignore
			{
				template<typename T>
				ignore(T)
				{
				}
			};

			template<typename Type>
			class enum_field
			{
				using enum_type = Type;

				using und_type = std::underlying_type_t<enum_type>;
				und_type m_value;

			public:
				constexpr enum_field(und_type value) : m_value(static_cast<und_type>(value))
				{
				}

				constexpr operator enum_type() const
				{
					return static_cast<enum_type>(m_value);
				}

				explicit constexpr operator und_type() const
				{
					return m_value;
				}

				constexpr enum_field operator |(const enum_field& rhs) const
				{
					return{ m_value | rhs.m_value };
				}

				constexpr enum_field operator &(const enum_field& rhs) const
				{
					return{ m_value & rhs.m_value };
				}

				constexpr enum_field operator ~() const
				{
					return{ ~m_value };
				}
			};

			struct fnv_1a_hasher
			{
				template<std::size_t Size = sizeof(std::size_t)>
				struct fnv_1a;

				template<>
				struct fnv_1a<4>
				{
					static const std::size_t offset_basis = size_t(14695981039346656037ULL);
					static const std::size_t prime = size_t(1099511628211ULL);
				};

				template<>
				struct fnv_1a<8>
				{
					static const std::size_t offset_basis = size_t(2166136261);
					static const std::size_t prime = size_t(16777619);
				};

				static std::size_t hash(const u8* raw, std::size_t size)
				{
					std::size_t result = fnv_1a<>::offset_basis;

					for (std::size_t byte = 0; byte < size; ++byte)
					{
						result ^= (std::size_t)raw[byte];
						result *= fnv_1a<>::prime;
					}

					return result;
				}

				template<typename Type>
				static std::size_t hash(const Type& value)
				{
					return hash((const u8*)&value, sizeof(Type));
				}

				template<typename Type>
				std::size_t operator()(const Type& value) const
				{
					return hash(value);
				}
			};

			struct self_hasher
			{
				template<typename Type, typename = std::enable_if_t<std::is_integral<Type>::value>>
				std::size_t operator()(Type value) const
				{
					return (std::size_t)value;
				}
			};

			struct binary_equals
			{
				template<typename T>
				bool operator()(const T& a, const T& b)
				{
					return memcmp(&a, &b, sizeof(T)) == 0;
				}
			};

			struct nullref_t
			{
				constexpr nullref_t() = default;
			} static const nullref{};

			struct null_t
			{
				constexpr null_t() = default;
				constexpr null_t(nullptr_t) {};
				constexpr null_t(nullref_t) {};

				operator nullref_t() const { return nullref; };
				operator nullptr_t() const { return nullptr; };
			} static const null{};

			template<typename Type>
			struct ref
			{
				using type = Type;

			private:
				type *m_data_ptr;

			public:
				ref() : m_data_ptr(nullptr)
				{
				}

				ref(type& raw_ref) : m_data_ptr(&raw_ref)
				{
				}

				type& get()
				{
					return m_data_ptr;
				}

				const type& get() const
				{
					return m_data_ptr;
				}

				type *operator&()
				{
					return &get();
				}

				const type *operator&() const
				{
					return &get();
				}

				operator type&()
				{
					return get();
				}

				operator const type&() const
				{
					return get();
				}

				bool empty() const
				{
					return m_data_ptr == nullptr;
				}

				void value(const type &value_)
				{
					*m_data_ptr = value_;
				}

				type value() const
				{
					return *m_data_ptr;
				}

				bool operator == (nullref_t) const
				{
					return empty();
				}

				bool operator != (nullref_t) const
				{
					return !empty();
				}

				ref& operator = (nullref_t)
				{
					m_data_ptr = nullptr;
					return *this;
				}

				ref& operator = (type &raw_ref)
				{
					m_data_ptr = &raw_ref;
					return *this;
				}
			};

			template<typename Type>
			struct cref
			{
				using type = Type;

			private:
				const type *m_data_ptr;

			public:
				cref() : m_data_ptr(nullptr)
				{
				}

				cref(const ref<type> &data) : m_data_ptr(data.m_data_ptr)
				{
				}

				cref(const type& raw_ref) : m_data_ptr(&raw_ref)
				{
				}

				const type& get() const
				{
					return m_data_ptr;
				}

				const type *operator&() const
				{
					return &get();
				}

				operator const type&() const
				{
					return get();
				}

				bool empty() const
				{
					return m_data_ptr == nullptr;
				}

				type value() const
				{
					return get();
				}

				bool operator == (nullref_t) const
				{
					return empty();
				}

				bool operator != (nullref_t) const
				{
					return !empty();
				}

				cref& operator = (nullref_t)
				{
					m_data_ptr = nullptr;
					return *this;
				}

				cref& operator = (const type &raw_ref)
				{
					m_data_ptr = &raw_ref;
					return *this;
				}

				cref& operator = (const ref<type> &data)
				{
					m_data_ptr = &data.m_data_ptr;
					return *this;
				}
			};

			template<typename Type>
			struct not_null
			{
				using type = Type;

			private:
				type *m_pointer;

			public:
				not_null(type *pointer) : m_pointer(pointer)
				{
					assert(pointer != nullptr);
				}

				type* get() const
				{
					return m_pointer;
				}

				explicit operator type*() const
				{
					return get();
				}

				type* operator ->() const
				{
					return get();
				}
			};

			struct none_t {};

			template<typename Type>
			Type clone(const Type& object)
			{
				return (Type)object;
			}

			template<typename Type, typename MutexType>
			Type clone(const Type& object, MutexType& mutex)
			{
				std::lock_guard<MutexType> lock(mutex);
				return (Type)object;
			}
		}

		inline namespace geometry_types
		{
			template<typename T = float>
			inline T pi()
			{
				return T(3.141592653589793238462643383279502884197169399375105820974944592307816406286);
			};

			template<int Dimension, typename Type>
			struct vector
			{
				static const int dimension = Dimension;
				using type = std::decay_t<Type>;

			protected:
				type m_data[dimension];

			public:
				using iterator = type *;
				using const_iterator = const type *;

				template<int OtherDimension, typename OtherType, typename = std::enable_if_t<(OtherDimension < Dimension)>>
				EXTCONSTEXPR vector(const vector<OtherDimension, OtherType>& other)
				{
					for (int i = 0; i < OtherDimension; ++i)
					{
						m_data[i] = static_cast<Type>(other[i]);
					}

					for (int i = OtherDimension; i < Dimension; ++i)
					{
						m_data[i] = {};
					}
				}

				vector(type (&data)[dimension])
				{
					for (int i = 0; i < dimension; ++i)
					{
						m_data[i] = data[i];
					}
				}

			private:
				template<typename Type = void, typename... Args>
				struct test_args_impl
				{
					static const bool value =
						std::is_convertible<std::decay_t<Type>, type>::value &&
						test_args_impl<Args...>::value;
				};

				template<>
				struct test_args_impl<void>
				{
					static const bool value = true;
				};

			public:
				vector(std::array<Type, Dimension> data)
				{
					for (int i = 0; i < Dimension; ++i)
					{
						m_data[i] = data[i];
					}
				}

				template<typename... Args, typename = std::enable_if_t<test_args_impl<Args...>::value>>
				EXTCONSTEXPR vector(Args... args) : m_data{ static_cast<type>(args)... }
				{
				}

				EXTCONSTEXPR bool operator >(const vector& rhs) const
				{
					for (int i = 0; i < dimension; ++i)
					{
						if (m_data[i] <= rhs.m_data[i])
							return false;
					}
					return true;
				}
				EXTCONSTEXPR bool operator >(Type rhs) const
				{
					for (int i = 0; i < dimension; ++i)
					{
						if (m_data[i] <= rhs)
							return false;
					}
					return true;
				}
				EXTCONSTEXPR bool operator <(const vector& rhs) const
				{
					for (int i = 0; i < dimension; ++i)
					{
						if (m_data[i] >= rhs.m_data[i])
							return false;
					}
					return true;
				}
				EXTCONSTEXPR bool operator <(Type rhs) const
				{
					for (int i = 0; i < dimension; ++i)
					{
						if (m_data[i] >= rhs)
							return false;
					}
					return true;
				}
				EXTCONSTEXPR bool operator >=(const vector& rhs) const
				{
					for (int i = 0; i < dimension; ++i)
					{
						if (m_data[i] < rhs.m_data[i])
							return false;
					}
					return true;
				}
				EXTCONSTEXPR bool operator >=(Type rhs) const
				{
					for (int i = 0; i < dimension; ++i)
					{
						if (m_data[i] < rhs)
							return false;
					}
					return true;
				}
				EXTCONSTEXPR bool operator <=(const vector& rhs) const
				{
					for (int i = 0; i < dimension; ++i)
					{
						if (m_data[i] > rhs.m_data[i])
							return false;
					}
					return true;
				}
				EXTCONSTEXPR bool operator <=(Type rhs) const
				{
					for (int i = 0; i < dimension; ++i)
					{
						if (m_data[i] > rhs)
							return false;
					}
					return true;
				}

				EXTCONSTEXPR vector operator -(const vector& rhs) const
				{
					vector result;

					for (int i = 0; i < dimension; ++i)
					{
						result.m_data[i] = m_data[i] - rhs.m_data[i];
					}

					return result;
				}
				EXTCONSTEXPR vector operator -(Type rhs) const
				{
					vector result;

					for (int i = 0; i < dimension; ++i)
					{
						result.m_data[i] = m_data[i] - rhs;
					}

					return result;
				}
				EXTCONSTEXPR vector operator +(const vector& rhs) const
				{
					vector result;

					for (int i = 0; i < dimension; ++i)
					{
						result.m_data[i] = m_data[i] + rhs.m_data[i];
					}

					return result;
				}
				EXTCONSTEXPR vector operator +(Type rhs) const
				{
					vector result;

					for (int i = 0; i < dimension; ++i)
					{
						result.m_data[i] = m_data[i] + rhs;
					}

					return result;
				}

				EXTCONSTEXPR vector operator *(Type rhs) const
				{
					vector result;

					for (int i = 0; i < dimension; ++i)
					{
						result.m_data[i] = m_data[i] * rhs;
					}

					return result;
				}
				EXTCONSTEXPR vector operator *(const vector& rhs) const
				{
					vector result;

					for (int i = 0; i < dimension; ++i)
					{
						result.m_data[i] = m_data[i] * rhs.m_data[i];
					}

					return result;
				}
				EXTCONSTEXPR vector operator /(Type rhs) const
				{
					vector result;

					for (int i = 0; i < dimension; ++i)
					{
						result.m_data[i] = m_data[i] / rhs;
					}

					return result;
				}
				EXTCONSTEXPR vector operator /(const vector& rhs) const
				{
					vector result;

					for (int i = 0; i < dimension; ++i)
					{
						result.m_data[i] = m_data[i] / rhs.m_data[i];
					}

					return result;
				}

				vector& operator -=(const vector& rhs)
				{
					for (int i = 0; i < dimension; ++i)
					{
						m_data[i] -= rhs.m_data[i];
					}

					return *this;
				}
				vector& operator -=(Type rhs)
				{
					for (int i = 0; i < dimension; ++i)
					{
						m_data[i] -= rhs;
					}
					return *this;
				}
				vector& operator +=(const vector& rhs)
				{
					for (int i = 0; i < dimension; ++i)
					{
						m_data[i] += rhs.m_data[i];
					}
					return *this;
				}
				vector& operator +=(Type rhs)
				{
					for (int i = 0; i < dimension; ++i)
					{
						m_data[i] += rhs;
					}
					return *this;
				}

				vector& operator *=(Type rhs)
				{
					for (int i = 0; i < dimension; ++i)
					{
						m_data[i] *= rhs;
					}
					return *this;
				}
				vector& operator *=(const vector& rhs)
				{
					for (int i = 0; i < dimension; ++i)
					{
						m_data[i] *= rhs.m_data[i];
					}
					return *this;
				}
				vector& operator /=(Type rhs)
				{
					for (int i = 0; i < dimension; ++i)
					{
						m_data[i] /= rhs;
					}
					return *this;
				}
				vector& operator /=(const vector& rhs)
				{
					for (int i = 0; i < dimension; ++i)
					{
						m_data[i] /= rhs.m_data[i];
					}
					return *this;
				}

				EXTCONSTEXPR bool operator ==(const vector& rhs) const
				{
					for (int i = 0; i < dimension; ++i)
					{
						if (m_data[i] != rhs.m_data[i])
							return false;
					}

					return true;
				}

				EXTCONSTEXPR bool operator ==(Type rhs) const
				{
					for (int i = 0; i < dimension; ++i)
					{
						if (m_data[i] != rhs)
							return false;
					}

					return true;
				}

				EXTCONSTEXPR bool operator !=(const vector& rhs) const
				{
					return !(*this == rhs);
				}

				EXTCONSTEXPR bool operator !=(Type rhs) const
				{
					return !(*this == rhs);
				}

				EXTCONSTEXPR const type& operator[](std::size_t index) const
				{
					return m_data[index];
				}

				type& operator[](std::size_t index)
				{
					return m_data[index];
				}

				EXTCONSTEXPR vector abs() const
				{
					vector result;

					for (int i = 0; i < dimension; ++i)
					{
						result[i] = std::abs(m_data[i]);
					}

					return result;
				}

				EXTCONSTEXPR vector clamp(type min, type max) const
				{
					vector result;

					for (int i = 0; i < dimension; ++i)
					{
						type value = m_data[i];

						if (value < min)
						{
							result[i] = min;
						}
						else if (value > max)
						{
							result[i] = max;
						}
						else
						{
							result[i] = value;
						}
					}

					return result;
				}

				template<typename T>
				EXTCONSTEXPR operator vector<Dimension, T>() const
				{
					vector<Dimension, T> result;

					for (int i = 0; i < dimension; ++i)
					{
						result[i] = static_cast<T>(m_data[i]);
					}

					return result;
				}

				EXTCONSTEXPR double distance(const vector& to) const
				{
					double value = 0.0;

					for (int i = 0; i < dimension; ++i)
					{
						value += (m_data[i] - to.m_data[i]) * (m_data[i] - to.m_data[i]);
					}

					return std::sqrt(value);
				}

				iterator begin()
				{
					return m_data;
				}

				EXTCONSTEXPR const_iterator begin() const
				{
					return m_data;
				}

				iterator end()
				{
					return m_data;
				}

				EXTCONSTEXPR const_iterator end() const
				{
					return m_data;
				}

				EXTCONSTEXPR std::size_t size() const
				{
					return dimension;
				}
			};

			template<int Dimension>
			struct vector<Dimension, bool>
			{
				static const int dimension = Dimension;
				using type = bool;

			protected:
				type m_data[dimension];

			public:
				using iterator = type *;
				using const_iterator = const type *;

				vector(const type(&data)[dimension])
				{
					for (int i = 0; i < dimension; ++i)
					{
						m_data[i] = data[i];
					}
				}

				template<typename... Args>
				constexpr vector(Args... args) : m_data{ args... }
				{
				}

				EXTCONSTEXPR bool all() const
				{
					for (bool value : m_data)
					{
						if (!value)
						{
							return false;
						}
					}

					return true;
				}

				EXTCONSTEXPR bool any() const
				{
					for (bool value : m_data)
					{
						if (value)
						{
							return true;
						}
					}

					return false;
				}

				EXTCONSTEXPR const type& operator[](std::size_t index) const
				{
					return m_data[index];
				}

				type& operator[](std::size_t index)
				{
					return m_data[index];
				}

				iterator begin()
				{
					return m_data;
				}

				EXTCONSTEXPR const_iterator begin() const
				{
					return m_data;
				}

				iterator end()
				{
					return m_data;
				}

				EXTCONSTEXPR const_iterator end() const
				{
					return m_data;
				}

				EXTCONSTEXPR std::size_t size() const
				{
					return dimension;
				}
			};

			template<typename Type, int RowCount, int ColumnCount = RowCount>
			struct matrix
			{
				static const int row_count = RowCount;
				static const int column_count = ColumnCount;
				static const int count = row_count;

				using type = Type;
				using vector_t = vector<column_count, type>;

			private:
				vector_t m_data[row_count];

			public:
				using iterator = vector_t *;
				using const_iterator = const vector_t *;

				matrix(const vector_t (&data)[row_count])
				{
					for (int i = 0; i < row_count; ++i)
					{
						m_data[i] = data[i];
					}
				}

				constexpr matrix() : m_data{}
				{
				}

				EXTCONSTEXPR matrix(type value)
				{
					for (int i = 0; i < std::min(row_count, column_count); ++i)
					{
						m_data[i][i] = value;
					}
				}

				EXTCONSTEXPR matrix(std::array<vector_t, row_count> data)
				{
					for (int i = 0; i < row_count; ++i)
					{
						m_data[i] = data[i];
					}
				}

				EXTCONSTEXPR const vector_t &operator[](std::size_t index) const
				{
					return m_data[index];
				}

				vector_t& operator[](std::size_t index)
				{
					return m_data[index];
				}

				iterator begin()
				{
					return m_data;
				}

				EXTCONSTEXPR const_iterator begin() const
				{
					return m_data;
				}

				iterator end()
				{
					return m_data;
				}

				EXTCONSTEXPR const_iterator end() const
				{
					return m_data;
				}
			};

			template<typename Type, int ColumnCount>
			struct matrix<Type, 1, ColumnCount>
			{
				static const int row_count = 1;
				static const int column_count = ColumnCount;
				static const int count = row_count;

				using type = Type;
				using vector_t = vector<column_count, type>;

			private:
				vector_t m_data;

			public:
				using iterator = vector_t *;
				using const_iterator = const vector_t *;

				matrix() : m_data{}
				{
				}

				matrix(const vector_t& data) : m_data(data)
				{
				}

				EXTCONSTEXPR const vector_t &operator[](std::size_t index) const
				{
					return m_data[index];
				}

				vector_t& operator[](std::size_t index)
				{
					if (index)
					{
						throw std::out_of_range("");
					}

					return m_data;
				}

				iterator begin()
				{
					return &m_data;
				}

				EXTCONSTEXPR const_iterator begin() const
				{
					return &m_data;
				}

				iterator end()
				{
					return &m_data + 1;
				}

				EXTCONSTEXPR const_iterator end() const
				{
					return &m_data + 1;
				}
			};

			template<typename Type, int LRowCount, int LColumnCount, int RRowCount, int RColumnCount>
			matrix<Type, LRowCount, RColumnCount> operator *(const matrix<Type, LRowCount, LColumnCount> &lhs, const matrix<Type, RRowCount, RColumnCount> &rhs)
			{
				static_assert(LColumnCount == RRowCount, "Incompatible matrices");

				matrix<Type, LRowCount, RColumnCount> result;

				for (int i = 0; i < LRowCount; ++i)
				{
					for (int j = 0; j < RColumnCount; ++j)
					{
						for (int k = 0; k < LColumnCount; ++k)
						{
							result[i][j] += lhs[i][k] * rhs[k][j];
						}
					}
				}

				return result;
			}

			template<int Dimension, typename Type>
			struct point : vector<Dimension, Type>
			{
				using base = vector<Dimension, Type>;

				EXTCONSTEXPR point(const base& rhs) : base(rhs)
				{
				}

				EXTCONSTEXPR point() = default;

				using base::operator==;
				using base::operator!=;

				using base::operator>;
				using base::operator<;
				using base::operator>=;
				using base::operator<=;

				using base::operator+;
				using base::operator-;
				using base::operator/;
				using base::operator*;

				using base::operator+=;
				using base::operator-=;
				using base::operator/=;
				using base::operator*=;

				using base::operator[];
			};

			template<typename Type>
			struct point<1, Type> : vector<1, Type>
			{
				using base = vector<1, Type>;

				EXTCONSTEXPR point(const base& rhs) : base(rhs)
				{
				}
				constexpr point(Type x_ = {}) : base(x_)
				{
				}

				constexpr Type x() const { return base::m_data[0]; }
				void x(Type value) { base::m_data[0] = value; }

				using base::operator==;
				using base::operator!=;

				using base::operator>;
				using base::operator<;
				using base::operator>=;
				using base::operator<=;

				using base::operator+;
				using base::operator-;
				using base::operator/;
				using base::operator*;

				using base::operator+=;
				using base::operator-=;
				using base::operator/=;
				using base::operator*=;

				using base::operator[];
			};

			template<typename Type>
			struct point<2, Type> : vector<2, Type>
			{
				using base = vector<2, Type>;

				EXTCONSTEXPR point(const base& rhs) : base(rhs)
				{
				}

				template<int OtherDimension, typename OtherType, typename = std::enable_if_t<(OtherDimension < 2)>>
				EXTCONSTEXPR point(const vector<OtherDimension, OtherType> &other)
				{
					for (int i = 0; i < OtherDimension; ++i)
					{
						m_data[i] = static_cast<Type>(other[i]);
					}

					for (int i = OtherDimension; i < 2; ++i)
					{
						m_data[i] = {};
					}
				}

				constexpr point(Type x_ = {}, Type y_ = {}) : base(x_, y_)
				{
				}

				constexpr Type x() const { return base::m_data[0]; }
				void x(Type value) { base::m_data[0] = value; }
				constexpr Type y() const { return base::m_data[1]; }
				void y(Type value) { base::m_data[1] = value; }

				using base::operator==;
				using base::operator!=;

				using base::operator>;
				using base::operator<;
				using base::operator>=;
				using base::operator<=;

				using base::operator+;
				using base::operator-;
				using base::operator/;
				using base::operator*;

				using base::operator+=;
				using base::operator-=;
				using base::operator/=;
				using base::operator*=;

				using base::operator[];

				EXTCONSTEXPR std::size_t at_plane(const base& size) const
				{
					return x() + y() * size[0];
				}
			};

			template<typename Type>
			struct point<3, Type> : vector<3, Type>
			{
				using base = vector<3, Type>;

				EXTCONSTEXPR point(const base& rhs) : base(rhs)
				{
				}
				template<int OtherDimension, typename OtherType, typename = std::enable_if_t<(OtherDimension < 3)>>
				EXTCONSTEXPR point(const vector<OtherDimension, OtherType> &other)
				{
					for (int i = 0; i < OtherDimension; ++i)
					{
						m_data[i] = static_cast<Type>(other[i]);
					}

					for (int i = OtherDimension; i < 3; ++i)
					{
						m_data[i] = {};
					}
				}
				constexpr point(Type x_ = {}, Type y_ = {}, Type z_ = {}) : base(x_, y_, z_)
				{
				}

				EXTCONSTEXPR Type x() const { return base::m_data[0]; }
				void x(Type value) { base::m_data[0] = value; }
				EXTCONSTEXPR Type y() const { return base::m_data[1]; }
				void y(Type value) { base::m_data[1] = value; }
				EXTCONSTEXPR Type z() const { return base::m_data[2]; }
				void z(Type value) { base::m_data[2] = value; }

				using base::operator==;
				using base::operator!=;

				using base::operator>;
				using base::operator<;
				using base::operator>=;
				using base::operator<=;

				using base::operator+;
				using base::operator-;
				using base::operator/;
				using base::operator*;

				using base::operator+=;
				using base::operator-=;
				using base::operator/=;
				using base::operator*=;

				using base::operator[];
			};

			template<typename Type>
			struct point<4, Type> : vector<4, Type>
			{
				using base = vector<4, Type>;

				EXTCONSTEXPR point(const base& rhs) : base(rhs)
				{
				}
				template<int OtherDimension, typename OtherType, typename = std::enable_if_t<(OtherDimension < 4)>>
				EXTCONSTEXPR point(const vector<OtherDimension, OtherType> &other)
				{
					for (int i = 0; i < OtherDimension; ++i)
					{
						m_data[i] = static_cast<Type>(other[i]);
					}

					for (int i = OtherDimension; i < 3; ++i)
					{
						m_data[i] = {};
					}

					m_data[3] = static_cast<Type>(1);
				}
				constexpr point(Type x_ = {}, Type y_ = {}, Type z_ = {}, Type w_ = { 1 })
					: base(x_, y_, z_, w_)
				{
				}

				constexpr Type x() const { return base::m_data[0]; }
				void x(Type value) { base::m_data[0] = value; }
				constexpr Type y() const { return base::m_data[1]; }
				void y(Type value) { base::m_data[1] = value; }
				constexpr Type z() const { return base::m_data[2]; }
				void z(Type value) { base::m_data[2] = value; }
				constexpr Type w() const { return base::m_data[3]; }
				void w(Type value) { base::m_data[3] = value; }

				using base::operator==;
				using base::operator!=;

				using base::operator>;
				using base::operator<;
				using base::operator>=;
				using base::operator<=;

				using base::operator+;
				using base::operator-;
				using base::operator/;
				using base::operator*;

				using base::operator+=;
				using base::operator-=;
				using base::operator/=;
				using base::operator*=;

				using base::operator[];
			};


			template<int Dimension, typename Type>
			struct size : vector<Dimension, Type>
			{
				using base = vector<Dimension, Type>;

				EXTCONSTEXPR size(const base& rhs) : base(rhs)
				{
				}
				using base::operator==;
				using base::operator!=;

				using base::operator>;
				using base::operator<;
				using base::operator>=;
				using base::operator<=;

				using base::operator+;
				using base::operator-;
				using base::operator/;
				using base::operator*;

				using base::operator+=;
				using base::operator-=;
				using base::operator/=;
				using base::operator*=;

				using base::operator[];
			};

			template<typename Type>
			struct size<1, Type> : vector<1, Type>
			{
				using base = vector<1, Type>;

				EXTCONSTEXPR size(const base& rhs) : base(rhs)
				{
				}
				constexpr size(Type width_ = {}) : base(width_)
				{
				}
				constexpr Type width() const { return base::m_data[0]; }
				void width(Type value) { base::m_data[0] = value; }

				using base::operator==;
				using base::operator!=;

				using base::operator>;
				using base::operator<;
				using base::operator>=;
				using base::operator<=;

				using base::operator+;
				using base::operator-;
				using base::operator/;
				using base::operator*;

				using base::operator+=;
				using base::operator-=;
				using base::operator/=;
				using base::operator*=;

				using base::operator[];
			};

			template<typename Type>
			struct size<2, Type> : vector<2, Type>
			{
				using base = vector<2, Type>;

				EXTCONSTEXPR size(const base& rhs) : base(rhs)
				{
				}
				constexpr size(Type width_ = {}, Type height_ = {}) : base(width_, height_)
				{
				}

				constexpr Type width() const { return base::m_data[0]; }
				void width(Type value) { base::m_data[0] = value; }
				constexpr Type height() const { return base::m_data[1]; }
				void height(Type value) { base::m_data[1] = value; }

				using base::operator==;
				using base::operator!=;

				using base::operator>;
				using base::operator<;
				using base::operator>=;
				using base::operator<=;

				using base::operator+;
				using base::operator-;
				using base::operator/;
				using base::operator*;

				using base::operator+=;
				using base::operator-=;
				using base::operator/=;
				using base::operator*=;

				using base::operator[];
			};

			template<typename Type>
			struct size<3, Type> : vector<3, Type>
			{
				using base = vector<3, Type>;

				EXTCONSTEXPR size(const base& rhs) : base(rhs)
				{
				}
				constexpr size(Type width_ = {}, Type height_ = {}, Type depth_ = {})
					: base(width_, height_, depth_)
				{
				}

				constexpr Type width() const { return base::m_data[0]; }
				void width(Type value) { base::m_data[0] = value; }
				constexpr Type height() const { return base::m_data[1]; }
				void height(Type value) { base::m_data[1] = value; }
				constexpr Type depth() const { return base::m_data[2]; }
				void depth(Type value) { base::m_data[2] = value; }

				using base::operator==;
				using base::operator!=;

				using base::operator>;
				using base::operator<;
				using base::operator>=;
				using base::operator<=;

				using base::operator+;
				using base::operator-;
				using base::operator/;
				using base::operator*;

				using base::operator+=;
				using base::operator-=;
				using base::operator/=;
				using base::operator*=;

				using base::operator[];
			};

			template<int Dimension, typename Type>
			struct color : vector<Dimension, Type>
			{
				using base = vector<Dimension, Type>;

				EXTCONSTEXPR color(const base& rhs) : base(rhs)
				{
				}
				using base::operator==;
				using base::operator!=;

				using base::operator>;
				using base::operator<;
				using base::operator>=;
				using base::operator<=;

				using base::operator+;
				using base::operator-;
				using base::operator/;
				using base::operator*;

				using base::operator+=;
				using base::operator-=;
				using base::operator/=;
				using base::operator*=;

				using base::operator[];
			};

			template<typename Type>
			struct color<1, Type> : vector<1, Type>
			{
				using base = vector<1, Type>;

				EXTCONSTEXPR color(const base& rhs) : base(rhs)
				{
				}
				constexpr color(Type r_ = {}) : base(r_)
				{
				}

				constexpr Type r() const { return base::m_data[0]; }
				void r(Type value) { base::base::m_data[0] = value; }

				using base::operator==;
				using base::operator!=;

				using base::operator>;
				using base::operator<;
				using base::operator>=;
				using base::operator<=;

				using base::operator+;
				using base::operator-;
				using base::operator/;
				using base::operator*;

				using base::operator+=;
				using base::operator-=;
				using base::operator/=;
				using base::operator*=;

				using base::operator[];
			};

			template<typename Type>
			struct color<2, Type> : vector<2, Type>
			{
				using base = vector<2, Type>;

				EXTCONSTEXPR color(const base& rhs) : base(rhs)
				{
				}
				constexpr color(Type r_ = {}, Type g_ = {}) : base(r_, g_)
				{
				}

				constexpr Type r() const { return base::m_data[0]; }
				void r(Type value) { base::m_data[0] = value; }
				constexpr Type g() const { return base::m_data[1]; }
				void g(Type value) { base::m_data[1] = value; }

				constexpr const Type* rg() const { return base::m_data; }

				using base::operator==;
				using base::operator!=;

				using base::operator>;
				using base::operator<;
				using base::operator>=;
				using base::operator<=;

				using base::operator+;
				using base::operator-;
				using base::operator/;
				using base::operator*;

				using base::operator+=;
				using base::operator-=;
				using base::operator/=;
				using base::operator*=;

				using base::operator[];
			};

			template<typename Type>
			struct color<3, Type> : vector<3, Type>
			{
				using base = vector<3, Type>;

				EXTCONSTEXPR color(const base& rhs) : base(rhs)
				{
				}
				constexpr color(Type r_ = {}, Type g_ = {}, Type b_ = {}) : base(r_, g_, b_)
				{
				}

				constexpr Type r() const { return base::m_data[0]; }
				void r(Type value) { base::m_data[0] = value; }
				constexpr Type g() const { return base::m_data[1]; }
				void g(Type value) { base::m_data[1] = value; }
				constexpr Type b() const { return base::m_data[2]; }
				void b(Type value) { base::m_data[2] = value; }

				constexpr const Type* rgb() const { return base::m_data; }

				using base::operator==;
				using base::operator!=;

				using base::operator>;
				using base::operator<;
				using base::operator>=;
				using base::operator<=;

				using base::operator+;
				using base::operator-;
				using base::operator/;
				using base::operator*;

				using base::operator+=;
				using base::operator-=;
				using base::operator/=;
				using base::operator*=;

				using base::operator[];
			};

			template<typename Type>
			struct color<4, Type> : vector<4, Type>
			{
				using base = vector<4, Type>;

				EXTCONSTEXPR color(const base& rhs) : base(rhs)
				{
				}
				constexpr color(Type r_ = {}, Type g_ = {}, Type b_ = {}, Type a_ = { 1 })
					: base(r_, g_, b_, a_)
				{
				}

				constexpr Type r() const { return base::m_data[0]; }
				void r(Type value) { base::m_data[0] = value; }
				constexpr Type g() const { return base::m_data[1]; }
				void g(Type value) { base::m_data[1] = value; }
				constexpr Type b() const { return base::m_data[2]; }
				void b(Type value) { base::m_data[2] = value; }
				constexpr Type a() const { return base::m_data[3]; }
				void a(Type value) { base::m_data[3] = value; }

				constexpr const Type* rgba() const { return base::m_data; }

				using base::operator==;
				using base::operator!=;

				using base::operator>;
				using base::operator<;
				using base::operator>=;
				using base::operator<=;

				using base::operator+;
				using base::operator-;
				using base::operator/;
				using base::operator*;

				using base::operator+=;
				using base::operator-=;
				using base::operator/=;
				using base::operator*=;

				using base::operator[];
			};

			template<int Dimension, typename Type>
			struct coord;

			template<typename ValueType, typename RangeType>
			static bool in_range(const ValueType& test, RangeType min, RangeType max)
			{
				return test >= min && test <= max;
			}

			template<int Dimension, typename Type>
			struct area
			{
				static const int dimension = Dimension;
				using type = Type;

				point<dimension, type> p1;
				point<dimension, type> p2;

				EXTCONSTEXPR operator coord<dimension, type>() const
				{
					point<dimension, type> pos;
					size<dimension, type> size;

					for (int i = 0; i < dimension; ++i)
					{
						if (p1.m_data[i] < p2.m_data[i])
						{
							pos.m_data[i] = p1.m_data[i];
							size.m_data[i] = p2.m_data[i] - p1.m_data[i];
						}
						else
						{
							pos.m_data[i] = p2.m_data[i];
							size.m_data[i] = p1.m_data[i] - p2.m_data[i];
						}
					}

					return{ pos, size };
				}

				EXTCONSTEXPR bool operator ==(const area& rhs) const
				{
					return p1 == rhs.p1 && p2 == rhs.p2;
				}

				EXTCONSTEXPR bool operator !=(const area& rhs) const
				{
					return !(*this == rhs);
				}

				EXTCONSTEXPR area flipped_vertical() const
				{
					return area{ { p1.x(), p2.y() },{ p2.x(), p1.y() } };
				}
			};

			template<int Dimension, typename Type>
			struct coord
			{
				static const int dimension = Dimension;
				using type = Type;

				point<dimension, type> position;
				size<dimension, type> size;

				EXTCONSTEXPR operator area<dimension, type>() const
				{
					return{ position, position + size };
				}

				EXTCONSTEXPR bool operator ==(const coord& rhs) const
				{
					return position == rhs.position && size == rhs.size;
				}

				EXTCONSTEXPR bool operator !=(const coord& rhs) const
				{
					return !(*this == rhs);
				}

				EXTCONSTEXPR bool intersect(const coord& r) const
				{
					type tw = size.width();
					type th = size.height();
					type rw = r.size.width();
					type rh = r.size.height();

					if (rw <= type{} || rh <= type{} || tw <= type{} || th <= type{})
						return false;

					type tx = position.x();
					type ty = position.y();
					type rx = r.position.x();
					type ry = r.position.y();

					rw += rx;
					rh += ry;
					tw += tx;
					th += ty;

					return
						((rw < rx || rw > tx) &&
							(rh < ry || rh > ty) &&
							(tw < tx || tw > rx) &&
							(th < ty || th > ry));
				}
			};

			using point1i = point<1, int>;
			using point1f = point<1, float>;
			using point1d = point<1, double>;

			using point2i = point<2, int>;
			using point2f = point<2, float>;
			using point2d = point<2, double>;

			using point3i = point<3, int>;
			using point3f = point<3, float>;
			using point3d = point<3, double>;

			using point4i = point<4, int>;
			using point4f = point<4, float>;
			using point4d = point<4, double>;

			using size1i = size<1, int>;
			using size1f = size<1, float>;
			using size1d = size<1, double>;

			using size2i = size<2, int>;
			using size2f = size<2, float>;
			using size2d = size<2, double>;

			using size3i = size<3, int>;
			using size3f = size<3, float>;
			using size3d = size<3, double>;

			using color1i = color<1, int>;
			using color1f = color<1, float>;
			using color1d = color<1, double>;

			using color2i = color<2, int>;
			using color2f = color<2, float>;
			using color2d = color<2, double>;

			using color3i = color<3, int>;
			using color3f = color<3, float>;
			using color3d = color<3, double>;

			using color4i = color<4, int>;
			using color4f = color<4, float>;
			using color4d = color<4, double>;

			using area2i = area<2, int>;
			using area2f = area<2, float>;
			using area2d = area<2, double>;

			using coord2i = coord<2, int>;
			using coord2f = coord<2, float>;
			using coord2d = coord<2, double>;

			using vector2i = vector<2, int>;
			using vector3i = vector<3, int>;
			using vector4i = vector<4, int>;

			using vector2f = vector<2, float>;
			using vector3f = vector<3, float>;
			using vector4f = vector<4, float>;

			using vector2d = vector<2, double>;
			using vector3d = vector<3, double>;
			using vector4d = vector<4, double>;

			using matrix2i = matrix<int, 2>;
			using matrix3i = matrix<int, 3>;
			using matrix4i = matrix<int, 4>;

			using matrix2f = matrix<float, 2>;
			using matrix3f = matrix<float, 3>;
			using matrix4f = matrix<float, 4>;

			using matrix2d = matrix<double, 2>;
			using matrix3d = matrix<double, 3>;
			using matrix4d = matrix<double, 4>;
		}
	}

	using namespace core::basic_types;
	using namespace core::helper_types;
	using namespace core::geometry_types;

	namespace mtx
	{
		static core::matrix4f scale_offset(const core::vector3f &scale, const core::vector3f &offset)
		{
			return
			{ {
				{ scale[0], 0.f, 0.f, 0.f },
				{ 0.f, scale[1], 0.f, 0.f },
				{ 0.f, 0.f, scale[2], 0.f },
				{ offset[0], offset[1], offset[2], 1.f },
			} };
		}
	}
}
