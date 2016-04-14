#include <rfe/animation/animation.h>

namespace rfe
{
	namespace animation
	{
		element::element(function_type function, const clock::duration &length)
			: m_function(function)
			, m_length(length)
		{
		}

		element& element::easing(easing_function function)
		{
			m_easing = function;
			return *this;
		}

		element& element::onstart(std::function<void()> function)
		{
			m_onstart = function;
			return *this;
		}

		element& element::onstop(std::function<void()> function)
		{
			m_onstop = function;
			return *this;
		}

		void element::start(const clock::time_point &time)
		{
			assert(m_animating == false);

			m_animating = true;

			if (m_onstart)
				m_onstart();

			m_start_time = time;
		}

		void element::stop()
		{
			assert(m_animating == true);

			if (m_onstop)
				m_onstop();

			m_animating = false;
		}

		bool element::animating() const
		{
			return m_animating;
		}

		bool element::update(const clock::time_point &time)
		{
			if (!m_animating)
			{
				return false;
			}

			if (time >= (m_start_time + m_length))
			{
				m_function(1.0f);
				stop();
				return false;
			}

			if (!m_function(m_easing(double((time - m_start_time).count()) / m_length.count())))
			{
				stop();
				return false;
			}

			return true;
		}

		page::page(const element &function)
		{
			m_elements.emplace_back(std::move(function));
		}

		page::page(element &&function)
		{
			m_elements.emplace_back(std::move(function));
		}

		page& page::and_(element &&function)
		{
			m_elements.emplace_back(std::move(function));
			return *this;
		}

		void page::start(const clock::time_point &time)
		{
			for (auto &element : m_elements)
			{
				element.start(time);
			}
		}

		void page::stop()
		{
			for (auto &element : m_elements)
			{
				element.stop();
			}
		}

		bool page::update(const clock::time_point &time)
		{
			bool is_all_finish = true;

			for (auto &element : m_elements)
			{
				if (element.update(time))
				{
					is_all_finish = false;
				}
			}

			return !is_all_finish;
		}

		queue::queue(page &&page_)
		{
			then(std::move(page_));
		}

		queue& queue::then(page &&page)
		{
			pages.emplace_back(std::move(page));
			return *this;
		}

		void queue::append(const queue &rhs)
		{
			pages.insert(pages.end(), rhs.pages.begin(), rhs.pages.end());
		}

		void queue::stop()
		{
			if (animating)
			{
				pages.front().stop();
			}
		}

		bool queue::update(const clock::time_point &time)
		{
			if (!animating)
			{
				pages.front().start(time);
				animating = true;
			}
			else
			{
				if (!pages.front().update(time))
				{
					pages.erase(pages.begin());
					animating = false;

					if (pages.empty())
						return false;
				}
			}

			return true;
		}

		void animable::play(queue&& animation, insert_mode mode)
		{
			std::lock_guard<std::mutex> lock(m_mtx);

			switch (mode)
			{
			case insert_mode::parallel:
				m_queues.emplace_back(std::move(animation));
				break;

			case insert_mode::append:
				if (m_queues.empty())
					m_queues.emplace_back(std::move(animation));
				else
					m_queues.back().append(animation);
				break;

			case insert_mode::replace:
				for (auto &queue : m_queues)
					queue.stop();

				m_queues.clear();
				m_queues.emplace_back(std::move(animation));
				break;
			}
		}

		std::size_t animable::count() const
		{
			return m_queues.size();
		}

		bool animable::empty() const
		{
			return m_queues.empty();
		}

		void animable::update(const clock::time_point &time)
		{
			std::lock_guard<std::mutex> lock(m_mtx);

			for (std::size_t i = 0; i < m_queues.size();)
			{
				if (!m_queues[i].update(time))
				{
					m_queues.erase(m_queues.begin() + i);
				}
				else
				{
					++i;
				}
			}
		}
	}
}
