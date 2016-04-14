#include <rfe/core/events_queue.h>
#include <rfe/core/thread_queue.h>
#include <thread>

namespace rfe
{
	inline namespace core
	{
		thread_local thread_queue_context_base* g_current_thread_queue_context = nullptr;

		void events_queue::invoke(std::function<void()> invoke_func)
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtx);
			m_invoke_funcs.push_front(invoke_func);
		}

		void events_queue::process_queue()
		{
			if (m_invoke_funcs.empty())
			{
				return;
			}

			while (!m_invoke_funcs.empty())
			{
				std::function<void()> call_event;
				{
					std::lock_guard<std::recursive_mutex> lock(m_mtx);
					call_event = m_invoke_funcs.back();
					m_invoke_funcs.pop_back();
				}

				call_event();
			}
		}

		void events_queue::operator()()
		{
			process_queue();
		}

		void events_queue::async_flush_queue(std::function<void()> ondone)
		{
			std::thread([=]
			{
				process_queue();

				if (ondone)
				{
					ondone();
				}

			}).detach();
		}

		void direct_events_queue_t::invoke(std::function<void()> invoke_func)
		{
			invoke_func();
		}

		void direct_events_queue_t::process_queue()
		{
		}

		direct_events_queue_t direct_events_queue;
	}
}