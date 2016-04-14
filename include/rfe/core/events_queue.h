#pragma once
#include <deque>
#include <mutex>

namespace rfe
{
	inline namespace core
	{
		class events_queue
		{
			std::recursive_mutex m_mtx;
			std::deque<std::function<void()>> m_invoke_funcs;

		public:
			virtual void invoke(std::function<void()> invoke_func);
			virtual void process_queue();

			void operator()();
			void async_flush_queue(std::function<void()> ondone = nullptr);
		};

		class direct_events_queue_t : public events_queue
		{
		public:
			void invoke(std::function<void()> invoke_func) override;
			void process_queue() override;
		};

		extern direct_events_queue_t direct_events_queue;
	}
}