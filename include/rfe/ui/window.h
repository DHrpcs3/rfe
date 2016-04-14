#pragma once
#include "widget.h"
#include <string>
#include <rfe/core/thread_queue.h>
#include <future>

namespace rfe
{
	namespace ui
	{
		class window : public widget
		{
			void *m_handle = nullptr;

		public:
			thread_queue thread;

			window(bool make_thread = true);
			virtual ~window() = default;

			data_event<std::string, combined_data<std::string>> title{};
			data_event<color4f> clear_color;
			data_event<f64> clear_depth;
			data_event<u8> clear_stencil;

		public:
			template<typename Type, typename = std::enable_if_t<std::is_base_of<graphics::draw_context, Type>::value>>
			void make_dc(graphics::draw_context::settings cfg = {}, bool create_thread = true)
			{
				std::shared_ptr<graphics::draw_context> dc = std::make_shared<Type>(this);

				if (create_thread)
				{
					dc->thread = make_thread_queue(make_thread);
				}

				dc->thread.invoke([=] { dc->create(cfg); });
				set_dc(dc);
			}

		protected:
			void create();
			event_result doclose() override;

			void remove_dc();

		public:
			void focus() override;
			void* handle() const;
			using widget::operator+=;
		};
	}
}
