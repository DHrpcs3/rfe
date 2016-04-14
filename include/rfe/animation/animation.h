#pragma once
#include <rfe/core/types.h>
#include <rfe/animation/easing.h>
#include <chrono>
#include <functional>
#include <list>
#include <deque>
#include <mutex>

namespace rfe
{
	namespace animation
	{
		using clock = std::chrono::high_resolution_clock;

		using namespace std::chrono_literals;

		class element
		{
		public:
			using function_type = std::function<bool(double x)>;

		private:
			std::function<void()> m_onstart = nullptr;
			std::function<void()> m_onstop = nullptr;

			function_type m_function;
			easing_function m_easing = default_easing_function;

			clock::duration m_length;
			clock::time_point m_start_time;
			bool m_animating = false;

		public:
			element(function_type function, const clock::duration &length = 1s);

			element& easing(easing_function function);
			element& onstart(std::function<void()> function);
			element& onstop(std::function<void()> function);

			void start(const clock::time_point &time = clock::now());
			void stop();
			bool animating() const;
			bool update(const clock::time_point &time = clock::now());
		};

		class page
		{
		public:
			using function_type = element::function_type;

		private:
			std::list<element> m_elements;

		public:
			page() = default;
			page(const element &function);
			page(element &&function);

			page& and_(element &&function);

			void start(const clock::time_point &time = clock::now());
			void stop();
			bool update(const clock::time_point &time = clock::now());
		};

		struct queue
		{
			bool animating = false;
			std::deque<page> pages;

			queue() = default;
			queue(page &&page_);

			queue& then(page &&page);

			void append(const queue &rhs);
			void stop();
			bool update(const clock::time_point &time = clock::now());
		};

		enum class insert_mode
		{
			parallel,
			append,
			replace
		};

		class animable
		{
			std::deque<queue> m_queues;
			using clock = std::chrono::high_resolution_clock;
			std::mutex m_mtx;

		public:
			void play(queue&& animation, insert_mode mode = insert_mode::parallel);
			bool empty() const;
			std::size_t count() const;
			void update(const clock::time_point &time = clock::now());
		};
	}
}
