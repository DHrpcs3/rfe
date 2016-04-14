#pragma once
#include <list>
#include "thread_queue.h"
#include <mutex>
#include <atomic>
#include <utility>

namespace rfe
{
	inline namespace core
	{
		enum class event_result
		{
			skip, handled
		};

		inline thread_queue& events_thread()
		{
			static thread_queue result = make_direct_thread_queue();

			return result;
		}

		struct synchronized_t {} static constexpr synchronized;

		template<typename ...AT>
		class event
		{
			using func_t = std::function<event_result(const AT&...)>;
			using entry_t = typename std::list<func_t>::iterator;

			std::list<func_t> m_handlers;
			thread_queue m_queue;
			std::mutex m_mtx;

		public:
			event(thread_queue queue = events_thread()) : m_queue(queue)
			{
			}

			void invoke(AT... args)
			{
				std::list<func_t> handlers;

				{
					std::lock_guard<std::mutex> lock(m_mtx);
					handlers = m_handlers;
				}

				m_queue.async_invoke(ignore_result, [=, handlers = std::move(handlers)]
				{
					for (const auto &handler : handlers)
					{
						if (handler(args...) == event_result::handled)
						{
							break;
						}
					}
				});
			}

			event_result invoke(synchronized_t, AT... args)
			{
				std::list<func_t> handlers;

				{
					std::lock_guard<std::mutex> lock(m_mtx);
					handlers = m_handlers;
				}

				return m_queue.invoke([=, handlers = std::move(handlers)]
				{
					for (const auto &handler : handlers)
					{
						if (handler(args...) == event_result::handled)
						{
							return event_result::handled;
						}
					}

					return event_result::skip;
				});
			}

			void operator()(const AT&... args)
			{
				invoke(args...);
			}

			event_result operator()(synchronized_t, const AT&... args)
			{
				return invoke(synchronized, args...);
			}

			entry_t bind(func_t func)
			{
				std::lock_guard<std::mutex> lock(m_mtx);
				m_handlers.push_front(func);
				return m_handlers.begin();
			}

			template<typename T>
			entry_t bind(T *caller, event_result(T::*callback)(const AT&...))
			{
				return bind([=](const AT&... args) { return (caller->*callback)(args...); });
			}

			void unbind(entry_t iterator)
			{
				std::lock_guard<std::mutex> lock(m_mtx);
				m_handlers.erase(iterator);
			}

			template<typename FuntionType>
			auto operator +=(FuntionType function) -> std::enable_if_t<std::is_same<std::result_of_t<FuntionType(decltype(AT())...)>, event_result>::value, entry_t>
			{
				return bind(function);
			}

			template<typename FuntionType>
			auto operator +=(FuntionType function) -> std::enable_if_t<std::is_same<std::result_of_t<FuntionType(decltype(AT())...)>, void>::value, entry_t>
			{
				return bind([=](const AT&... args)
				{
					function(args...);
					return event_result::skip;
				});
			}

			void operator -=(entry_t what)
			{
				return unbind(what);
			}
		};

		template<typename T>
		class data_event_store
		{
		public:
			void set(T value)
			{
				throw std::logic_error("data_event_store::set undefined.");
			}

			T get() const
			{
				throw std::logic_error("data_event_store::get undefined.");
			}

			bool invoke_event(T)
			{
				return false;
			}

			bool equal(T value) const
			{
				return false;
			}
		};

		template<typename T>
		class combined_data;

		template<typename T>
		class local_data
		{
		public:
			using type = T;

		protected:
			mutable std::mutex m_mtx;

			type m_data;

			void set(const type &value)
			{
				std::lock_guard<std::mutex> lock(m_mtx);
				m_data = value;
			}

			const type &get() const
			{
				std::lock_guard<std::mutex> lock(m_mtx);
				return m_data;
			}

			bool equals(T value) const
			{
				return get() == value;
			}

			bool invoke_event(type value)
			{
				return false;
			}

			friend combined_data<T>;
		};

		template<typename T>
		class combined_data
		{
		public:
			using type = T;

		protected:
			mutable std::mutex m_get_mtx;
			mutable std::mutex m_invoke_mtx;

			local_data<type> m_local_data;
			std::function<void(type)> m_invoke_event_function;
			std::function<type()> m_get_function;

			bool invoke_event(const type &value)
			{
				auto func = (std::lock_guard<std::mutex>{ m_invoke_mtx }, m_invoke_event_function);

				if (func)
				{
					func(value);

					return true;
				}

				return false;
			}

			void set(const type &value)
			{
				m_local_data.set(value);
			}

			type get() const
			{
				auto func = (std::lock_guard<std::mutex>{ m_get_mtx }, m_get_function);

				if (func)
				{
					return func();
				}

				return m_local_data.get();
			}

			bool equals(T value) const
			{
				return get() == value;
			}

		public:
			void invoke_event_function(std::function<void(type)> function)
			{
				std::lock_guard<std::mutex> lock(m_invoke_mtx);

				m_invoke_event_function = function;
			}

			void get_function(std::function<type()> function)
			{
				std::lock_guard<std::mutex> lock(m_get_mtx);

				m_get_function = function;
			}
		};

		struct ignore_custom_invoker_t {} static constexpr ignore_custom_invoker;

		template<typename T, typename base_type_ = local_data<T>>
		class data_event : public base_type_
		{
		public:
			using type = T;
			using base_type = base_type_;

		protected:
			event_result dochange(const type &new_value)
			{
				type old_value = get();
				base_type::set(new_value);
				onchanged(old_value, new_value);

				return event_result::skip;
			}

		public:
			event<type> onchange;
			event<type, type> onchanged;

			data_event(T default_value, thread_queue queue = events_thread())
				: onchange(queue), onchanged(queue)
			{
				onchange.bind(this, &data_event::dochange);
				base_type::set(default_value);
			}

			data_event(thread_queue queue = events_thread())
				: data_event({}, queue)
			{
			}

			template<typename RType, typename = std::enable_if_t<std::is_convertible<RType, type>::value>>
			data_event(RType value) : data_event(static_cast<type>(value))
			{
			}

			type get() const
			{
				return base_type::get();
			}

			type operator()() const
			{
				return get();
			}

			void change(type &&value)
			{
				if (base_type::equals(value))
				{
					return;
				}

				if (!base_type::invoke_event(value))
				{
					onchange(synchronized, std::forward<type>(value));
				}
			}

			event_result change(ignore_custom_invoker_t, type &&value)
			{
				return onchange.invoke(synchronized, std::forward<type>(value));
			}

			operator const type() const
			{
				return get();
			}

			operator type()
			{
				return get();
			}

			data_event& operator = (type &&value)
			{
				change(std::forward<type>(value));
				return *this;
			}

			data_event& operator = (const type &value)
			{
				change((type)value);
				return *this;
			}

			template<typename aType> auto operator + (aType value) const { return get() + value; }
			template<typename aType> auto operator - (aType value) const { return get() - value; }
			template<typename aType> auto operator * (aType value) const { return get() * value; }
			template<typename aType> auto operator / (aType value) const { return get() / value; }
			template<typename aType> auto operator % (aType value) const { return get() % value; }
			template<typename aType> auto operator & (aType value) const { return get() & value; }
			template<typename aType> auto operator | (aType value) const { return get() | value; }
			template<typename aType> auto operator ^ (aType value) const { return get() ^ value; }

			template<typename aType> data_event& operator += (aType value) { return *this = get() + value; }
			template<typename aType> data_event& operator -= (aType value) { return *this = get() - value; }
			template<typename aType> data_event& operator *= (aType value) { return *this = get() * value; }
			template<typename aType> data_event& operator /= (aType value) { return *this = get() / value; }
			template<typename aType> data_event& operator %= (aType value) { return *this = get() % value; }
			template<typename aType> data_event& operator &= (aType value) { return *this = get() & value; }
			template<typename aType> data_event& operator |= (aType value) { return *this = get() | value; }
			template<typename aType> data_event& operator ^= (aType value) { return *this = get() ^ value; }

			data_event& operator ++()
			{
				type value = get();
				return *this = ++value;
			}

			type operator ++(int)
			{
				type value = get();
				type result = value;
				*this = value++;
				return result;
			}

			data_event& operator --()
			{
				type value = get();
				return *this = --value;
			}

			type operator --(int)
			{
				type value = get();
				type result = value;
				*this = value--;
				return result;
			}
		};
	}
}
