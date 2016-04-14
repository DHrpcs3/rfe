#pragma once
#include "event.h"
#include <functional>
#include <vector>

namespace rfe
{
	inline namespace core
	{
		template<typename... T>
		class unbinder;

		class event_binder_t
		{
		public:
			std::vector<std::function<void()>> unbind_funcs;

		public:
			~event_binder_t();
			void unbind_all();

			template<typename... T>
			unbinder<T...> operator()(event<T...>& evt);
		};

		template<typename... AT>
		class unbinder
		{
			typedef std::function<event_result(AT...)> func_t;

			event_binder_t *m_parent;
			event<AT...>& m_event;

		public:
			unbinder(event_binder_t *parent, event<AT...>& evt)
				: m_parent(parent)
				, m_event(evt)
			{
			}

			void bind(func_t func)
			{
				auto remove_me = m_event.bind(func);

				m_parent->unbind_funcs.emplace_back(std::bind([=](event<AT...>* evt)
				{
					evt->unbind(remove_me);
				}, &m_event));
			}

			template<typename T>
			void bind(T *caller, event_result(T::*callback)(AT...))
			{
				bind([=](AT... args) { return (caller->*callback)(args...); });
			}

			template<typename FuntionType>
			auto operator +=(FuntionType function) -> std::enable_if_t<std::is_same<std::result_of_t<FuntionType(decltype(AT())...)>, event_result>::value>
			{
				bind(function);
			}

			template<typename FuntionType>
			auto operator +=(FuntionType function) -> std::enable_if_t<std::is_same<std::result_of_t<FuntionType(decltype(AT())...)>, void>::value>
			{
				bind([=](AT... args)
				{
					function(args...);
					return event_result::skip;
				});
			}
		};

		template<typename... T>
		unbinder<T...> event_binder_t::operator()(event<T...>& evt)
		{
			return{ this, evt };
		}
	}
}
