#pragma once

#include <rfe/core/types.h>
#include <rfe/core/event.h>
#include <rfe/core/event_binder_t.h>
#include <functional>
#include <memory>
#include <list>
#include <algorithm>

namespace rfe
{
	namespace ui
	{
		using namespace core;

		enum class sizer_elem_type : byte
		{
			counter,
			ignore,
			relative,
			absolute,
			expand,
			automatical
		};

		enum class fit_type
		{
			if_shown,
			never,
			always
		};

		class sizer_flags
		{
			template<typename AbsoluteType>
			struct elem
			{
				sizer_elem_type type = sizer_elem_type::counter;

				//union
				//{
				std::function<AbsoluteType()> rel_to;
				AbsoluteType value;
				//};
			};

		public:
			static constexpr int dimension = 2;

		private:
			elem<int> m_pos[dimension];
			elem<int> m_size[dimension];
			fit_type m_fit = fit_type::if_shown;
			class widget* m_parent;
			bool m_pull_back = 0;

		public:
			event_binder_t x_binder;
			event_binder_t y_binder;
			event_binder_t width_binder;
			event_binder_t height_binder;

			sizer_flags(class widget* parent);

			sizer_flags& floating();

			sizer_flags& width(std::function<int()> rel_to);
			sizer_flags& width(int value);
			sizer_flags& height(std::function<int()> rel_to);
			sizer_flags& height(int value);

			sizer_flags& x(std::function<int()> rel_to);
			sizer_flags& x(int value);
			sizer_flags& y(std::function<int()> rel_to);
			sizer_flags& y(int value);

			sizer_flags& size(int width_, int height_);
			sizer_flags& size(const size2i &size_);
			sizer_flags& size(std::function<int()> width_, std::function<int()> height_);
			sizer_flags& size(std::function<size2i()> size_);

			sizer_flags& auto_width(bool value = true);
			sizer_flags& auto_height(bool value = true);
			sizer_flags& auto_size(bool value = true);

			sizer_flags& position(int x_, int y_);
			sizer_flags& position(std::function<int()> x_, std::function<int()> y_);
			sizer_flags& position(std::function<point2i()> position_);
			sizer_flags& position(const point2i &position_);

			sizer_flags& center_horizontal();
			sizer_flags& center_vertical();
			sizer_flags& center();

			sizer_flags& fill_horizontal();
			sizer_flags& fill_vertical();
			sizer_flags& fill();

			sizer_flags& expand_vertical(bool value = true);
			sizer_flags& expand_horizontal(bool value = true);
			sizer_flags& expand(bool value = true);

			sizer_flags& fit(fit_type type);
			sizer_flags& pull_back(bool value = true);

			template<typename DataType, typename DataStorage>
			sizer_flags& width(data_event<DataType, DataStorage> &event, std::function<int()> rel_to)
			{
				width(rel_to);
				width_binder(event.onchanged) += [=](ignore, ignore) { invalidate_widget(); };
				return *this;
			}

			template<typename DataType, typename DataStorage>
			sizer_flags& height(data_event<DataType, DataStorage> &event, std::function<int()> rel_to)
			{
				height(rel_to);
				height_binder(event.onchanged) += [=](ignore, ignore) { invalidate_widget(); };
				return *this;
			}

			template<typename DataType, typename DataStorage>
			sizer_flags& size(data_event<DataType, DataStorage> &event, std::function<int()> width_, std::function<int()> height_)
			{
				return width(event, width_).height(event, height_);
			}

			template<typename DataType, typename DataStorage>
			sizer_flags& x(data_event<DataType, DataStorage> &event, std::function<int()> rel_to)
			{
				x(rel_to);
				x_binder(event.onchanged) += [=](ignore, ignore) { invalidate_widget(); };
				return *this;
			}

			template<typename DataType, typename DataStorage>
			sizer_flags& y(data_event<DataType, DataStorage> &event, std::function<int()> rel_to)
			{
				y(rel_to);
				y_binder(event.onchanged) += [=](ignore, ignore) { invalidate_widget(); };
				return *this;
			}

			template<typename DataType, typename DataStorage>
			sizer_flags& position(data_event<DataType, DataStorage> &event, std::function<int()> x_, std::function<int()> y_)
			{
				return x(event, x_).y(event, y_);
			}

		private:
			void invalidate_widget(bool is_const = false);

			friend class widget;
		};
	}
}
