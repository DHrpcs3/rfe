#pragma once
#include <deque>
#include <thread>
#include <future>
#include <unordered_set>

namespace rfe
{
	inline namespace core
	{
		struct make_thread_t {} static constexpr make_thread;
		struct no_thread_t {} static constexpr no_thread;
		struct ignore_result_t {} static constexpr ignore_result{};

		struct thread_queue_context_base
		{
			virtual void push(std::function<void()> function) = 0;
			virtual bool empty() const = 0;
			virtual void init(std::shared_ptr<thread_queue_context_base>) {}

			virtual void process_queue()
			{
				wait();
			}

			virtual bool process_queue_if_current()
			{
				return false;
			}

			void wait()
			{
				if (!process_queue_if_current())
				{
					while (!empty())
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}
				}
			}
		};

		struct direct_thread_queue_context : thread_queue_context_base
		{
			void push(std::function<void()> function) override
			{
				function();
			}

			bool empty() const override
			{
				return true;
			}
		};

		class thread_queue
		{
			std::shared_ptr<thread_queue_context_base> m_context;

		public:
			thread_queue(std::shared_ptr<thread_queue_context_base> context) : m_context{ context }
			{
				m_context->init(context);
			}

			thread_queue() : thread_queue(main_thread())
			{
			}

			static thread_queue& main_thread();

		public:
			template<typename Type>
			auto invoke(Type function) -> std::enable_if_t<!std::is_same<std::result_of_t<Type()>, void>::value, std::result_of_t<Type()>>
			{
				if (m_context->process_queue_if_current())
				{
					return function();
				}

				return async_invoke(function).get();
			}

			template<typename Type>
			auto invoke(Type function, std::launch launch = std::launch::deferred) -> std::enable_if_t<std::is_same<std::result_of_t<Type()>, void>::value, void>
			{
				if (launch == std::launch::deferred)
				{
					if (m_context->process_queue_if_current())
					{
						function();
						return;
					}
					
					async_invoke(function).wait();
				}
				else
				{
					async_invoke(ignore_result, function);
				}
			}

			void wait()
			{
				m_context->wait();
			}

			bool empty()
			{
				return m_context->empty();
			}

		private:
			template<typename T>
			struct set_promise_value
			{
				static void func(std::promise<T> &promise, std::function<T()> func)
				{
					promise.set_value(func());
				}
			};

			template<>
			struct set_promise_value<void>
			{
				static void func(std::promise<void> &promise, std::function<void()> func)
				{
					func();
					promise.set_value();
				}
			};

		public:
			template<typename FuntionType, typename ResultType = std::result_of_t<FuntionType()>>
			std::shared_future<ResultType> async_invoke(FuntionType function)
			{
				using result_t = ResultType;

				std::shared_ptr<std::promise<result_t>> promise = std::make_shared<std::promise<result_t>>();
				std::shared_future<result_t> result = promise->get_future();

				m_context->push([=]
				{
					set_promise_value<result_t>::func(*promise, function);
				});

				return result;
			}

			template<typename FuntionType>
			void async_invoke(ignore_result_t, FuntionType function)
			{
				m_context->push([=]
				{
					function();
				});
			}

			void process_queue()
			{
				m_context->process_queue();
			}
		};

		struct thread_queue_context : thread_queue_context_base
		{
			std::deque<std::function<void()>> queue;
			std::mutex mtx;
			std::thread thread;
			std::thread::id thread_id;
			std::shared_ptr<thread_queue_context_base> this_;
			std::atomic<bool> initialized = false;

			thread_queue_context(no_thread_t)
			{
				thread_id = std::this_thread::get_id();
			}

			thread_queue_context(std::function<void()> loop_func)
			{
				thread = std::thread([=] { loop(loop_func); });
				thread_id = thread.get_id();
			}

			thread_queue_context(std::thread thread_)
			{
				thread = std::move(thread_);
				thread_id = thread.get_id();
			}

			thread_queue_context(make_thread_t) : thread_queue_context(nullptr)
			{
			}

			void init(std::shared_ptr<thread_queue_context_base> context) override
			{
				this_ = context;
				initialized = true;
			}

			void push(std::function<void()> function) override
			{
				std::lock_guard<std::mutex> lock(mtx);
				queue.push_back(function);
			}

			bool process_queue_if_current() override
			{
				if (thread_id == std::this_thread::get_id())
				{
					process_queue();
					return true;
				}

				return false;
			}

			bool empty() const override
			{
				return queue.empty();
			}

			void process_queue() override
			{
				std::function<void()> call_event;

				while (!queue.empty())
				{
					{
						std::lock_guard<std::mutex> lock(mtx);
						call_event = std::move(queue.front());
						queue.pop_front();
					}

					call_event();
				}
			}

			void loop(std::function<void()> function)
			{
				while (!initialized)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}

				while (!this_.unique())
				{
					if (function)
					{
						function();
					}

					process_queue();

					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}

				thread.detach();
			}
		};

		inline thread_queue make_thread_queue(no_thread_t)
		{
			return std::make_shared<thread_queue_context>(no_thread);
		}

		inline thread_queue make_thread_queue(std::function<void()> loop_func)
		{
			return std::make_shared<thread_queue_context>(loop_func);
		}

		inline thread_queue make_thread_queue(std::thread thread)
		{
			return std::make_shared<thread_queue_context>(std::move(thread));
		}

		inline thread_queue make_thread_queue(make_thread_t)
		{
			return std::make_shared<thread_queue_context>(make_thread);
		}

		inline thread_queue make_direct_thread_queue()
		{
			return std::make_shared<direct_thread_queue_context>();
		}

		class multi_thread_queue_context : public thread_queue_context_base
		{
			std::list<thread_queue> m_free_threads;
			std::mutex m_mtx;
			std::atomic<std::size_t> m_threads_in_progress{ 0 };

			thread_queue get_free_thread()
			{
				std::lock_guard<std::mutex> lock{ m_mtx };

				if (m_free_threads.empty())
				{
					return make_thread_queue(make_thread);
				}

				auto result = std::move(m_free_threads.front());
				m_free_threads.pop_front();
				return result;
			}

			void put_free_thread(thread_queue queue)
			{
				const std::size_t limit = 1;

				std::lock_guard<std::mutex> lock{ m_mtx };

				if (m_free_threads.size() < limit)
				{
					m_free_threads.push_back(queue);
				}
			}

		public:
			void push(std::function<void()> function) override
			{
				++m_threads_in_progress;

				auto thread = get_free_thread();

				thread.invoke([=]
				{
					function();
					put_free_thread(thread);
					--m_threads_in_progress;
				}, std::launch::async);
			}

			bool empty() const override
			{
				return m_threads_in_progress == 0;
			}
		};

		inline thread_queue& thread_queue::main_thread()
		{
			static thread_queue result{ std::make_shared<thread_queue_context>(no_thread) };

			return result;
		}

		inline thread_queue make_multi_thread_queue()
		{
			return std::make_shared<multi_thread_queue_context>();
		}
	}
}
