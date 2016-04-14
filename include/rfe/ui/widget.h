#pragma once
#include "sizer.h"

#include <rfe/core/types.h>
#include <rfe/core/event.h>
#include <rfe/core/events.h>
#include <rfe/core/event_binder_t.h>
#include <rfe/core/id_manager.h>
#include <rfe/graphics/draw_context.h>
#include <rfe/core/events_queue.h>
#include <rfe/animation/animation.h>

#include <functional>
#include <unordered_set>
#include <list>
#include <mutex>

namespace rfe
{
	namespace ui
	{
		enum class recalc_sizers_type
		{
			set_constants,
			set_relatives
		};

		class shared_read_mutex_read
		{
			std::mutex& m_mutex;
			std::atomic<std::uint64_t> m_readers;

		public:
			shared_read_mutex_read(std::mutex& mutex) : m_mutex(mutex)
			{
			}

			void lock()
			{
				if (!m_readers++)
				{
					m_mutex.lock();
				}
			}

			void unlock()
			{
				if (!--m_readers)
				{
					m_mutex.unlock();
				}
			}
		};

		class shared_read_mutex_write
		{
			std::mutex& m_mutex;

		public:
			shared_read_mutex_write(std::mutex& mutex) : m_mutex(mutex)
			{
			}

			void lock()
			{
				m_mutex.lock();
			}

			bool try_lock()
			{
				return m_mutex.try_lock();
			}

			void unlock()
			{
				m_mutex.unlock();
			}
		};

		class shared_read_mutex
		{
			std::mutex m_mutex;

		public:
			shared_read_mutex_read read{ m_mutex };
			shared_read_mutex_write write{ m_mutex };
		};

		template<typename Type, typename... ArgsT>
		std::shared_ptr<Type> make_shared(ArgsT... args)
		{
			std::shared_ptr<Type> result = std::make_shared<Type>(args...);
			result->init();

			return result;
		}

		std::shared_ptr<id_manager_t<u32>> id_manager();

		enum class orientation
		{
			horizontal,
			vertical,
		};

		class widget
			: public animation::animable
			, public std::enable_shared_from_this<widget>
		{
		protected:
			std::shared_ptr<id_manager_t<u32>> m_id_manager = id_manager();
			bool m_invalidate = true;
			std::shared_ptr<widget> m_parent;
			data_event<std::shared_ptr<graphics::draw_context>> m_draw_context;
			std::list<std::shared_ptr<widget>> m_childs;
			std::shared_ptr<widget> m_top_widget = nullptr;
			bool m_alive = true;
			bool m_focused = false;
			bool m_visible = false;
			event_binder_t parent_event_binder;
			u32 m_id = m_id_manager->new_id();

			shared_read_mutex m_childs_mtx;
			std::mutex m_flip_mtx;

			bool m_touched = false;
			point2i m_touch_point;
			bool m_motion_started = false;
			bool m_const_sizer_invalidated = true;
			bool m_relative_sizer_invalidated = true;

		public:
			std::shared_ptr<const widget> shared_ptr() const;
			std::shared_ptr<widget> shared_ptr();

			void invalidate();

			event_binder_t event_binder;

			event<point2i> ontouch;
			event<point2i> ontry_click;
			event<point2i> onclick;

			data_event<bool, combined_data<bool>> shown;
			event<> onclose;
			event<graphics::draw_context*> ondraw;
			event<> ongot_focus;
			event<> onlose_focus;
			data_event<point2i, combined_data<point2i>> position{ { -1, -1 } };
			data_event<size2i, combined_data<size2i>> size;
			data_event<float, combined_data<float>> rotation;
			data_event<std::string> name;
			data_event<std::string> full_name;
			event<> onrefresh;
			event<> oninit;
			data_event<ui::orientation> orientation{ orientation::vertical };

			event<point2i, point2i> onmotion;
			event<point2i> onmotion_end;

			thread_queue queue = make_thread_queue(no_thread);

		public:
			bool motion_started() const
			{
				return m_motion_started;
			}

			matrix4f matrix{ 1.0f };
			vector4f clip{};

			static matrix4f calc_matrix_for(coord2f coords, size2f global_size);
			void calc_matrix();

			graphics::model model() const
			{
				return graphics::quad{ { {-1, -1},{ 1, 1 } } };
			}

		private:
			void set_top_widget(std::shared_ptr<widget> widget_);

		public:
			widget();

			void init();
			virtual ~widget();

			void update_animation();
			void update_sizers();
			void update_childs();
			void update();

			void set_dc(std::shared_ptr<graphics::draw_context> dc);
			virtual void set_parent(std::shared_ptr<widget> parent);

			std::shared_ptr<graphics::draw_context> dc() const
			{
				return m_draw_context;
			}

			std::shared_ptr<widget> parent() const
			{
				return m_parent;
			}

			std::shared_ptr<widget> top_widget();

			const std::list<std::shared_ptr<widget>>& childs() const
			{
				return m_childs;
			}

			void lock()
			{
				m_flip_mtx.lock();
			}

			bool try_lock()
			{
				return m_flip_mtx.try_lock();
			}

			void unlock()
			{
				m_flip_mtx.unlock();
			}

			u32 id() const
			{
				return m_id;
			}

			sizer_flags& append_child(std::shared_ptr<widget> child);
			void remove_child(std::shared_ptr<widget> child);

			sizer_flags& operator += (std::shared_ptr<widget> child)
			{
				return append_child(child);
			}

			void operator -= (std::shared_ptr<widget> child)
			{
				remove_child(child);
			}

			class sizer_flags& sizer_flags()
			{
				return m_sizer_flags;
			}

		protected:
			virtual event_result doclick(point2i)
			{
				return event_result::skip;
			}

			virtual event_result doclose();
			virtual event_result dogot_focus();
			virtual event_result dolose_focus();

			virtual void dodraw();

		public:
			void show(bool show = true);
			void hide();
			void close();
			void rotate(float angle);

			bool alive() const
			{
				return m_alive;
			}

			bool focused() const
			{
				return m_focused;
			}

			virtual void focus();

			void move(point2i position_);
			void resize(size2i size_);

			void width(int value);
			void height(int value);
			int width() const;
			int height() const;
			void x(int value);
			void y(int value);
			int x() const;
			int y() const;
			bool visible() const;
			bool hidden() const;
			bool visible_test();
			coord2i coord();

			point2i absolute_to_local_point(point2i point, std::shared_ptr<const widget>* top_widget = nullptr) const;
			point2i local_to_absolute_point(point2i point, std::shared_ptr<const widget>* top_widget = nullptr) const;

			void refresh(bool is_const = false);
			void draw(bool clear_and_flip = true);

		private:
			class sizer_flags m_sizer_flags { this };

			int get_front_border(int axe) const
			{
				switch (axe)
				{
				case 0: return border_left;
				case 1: return border_top;
				}

				throw;
			}

			int get_back_border(int axe) const
			{
				switch (axe)
				{
				case 0: return border_right;
				case 1: return border_bottom;
				}

				throw;
			}

			void set_borders(int left, int right, int top, int bottom)
			{
				border_left = left;
				border_right = right;
				border_top = top;
				border_bottom = bottom;
			}

			int get_size(int axe) const
			{
				switch (axe)
				{
				case 0: return width();
				case 1: return height();
				}

				throw;
			}

			void set_size(int axe, int value)
			{
				switch (axe)
				{
				case 0: width(value); return;
				case 1: height(value); return;
				}

				throw;
			}

			int get_position(int axe) const
			{
				switch (axe)
				{
				case 0: return x();
				case 1: return y();
				}

				throw;
			}

			void set_position(int axe, int value)
			{
				switch (axe)
				{
				case 0: x(value); return;
				case 1: y(value); return;
				}

				throw;
			}

			int border_left = 5;
			int border_right = 5;
			int border_top = 5;
			int border_bottom = 5;

			void recalc_sizers(recalc_sizers_type type);
		};
	}
}
