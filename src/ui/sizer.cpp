#include <rfe/ui/sizer.h>
#include <rfe/ui/widget.h>

namespace rfe
{
	namespace ui
	{
		sizer_flags::sizer_flags(class widget* parent)
			: m_parent(parent)
		{
		}

		sizer_flags& sizer_flags::floating()
		{
			m_pos[0].type = sizer_elem_type::ignore;
			m_pos[1].type = sizer_elem_type::ignore;

			return *this;
		}

		sizer_flags& sizer_flags::width(std::function<int()> rel_to)
		{
			width_binder.unbind_all();
			m_size[0].type = sizer_elem_type::relative;
			m_size[0].rel_to = rel_to;
			return *this;
		}

		sizer_flags& sizer_flags::width(int value)
		{
			height_binder.unbind_all();
			m_size[0].type = sizer_elem_type::absolute;
			m_size[0].value = value;
			invalidate_widget(true);
			return *this;
		}

		sizer_flags& sizer_flags::height(std::function<int()> rel_to)
		{
			height_binder.unbind_all();
			m_size[1].type = sizer_elem_type::relative;
			m_size[1].rel_to = rel_to;
			return *this;
		}

		sizer_flags& sizer_flags::height(int value)
		{
			height_binder.unbind_all();
			m_size[1].type = sizer_elem_type::absolute;
			m_size[1].value = value;
			invalidate_widget(true);
			return *this;
		}

		sizer_flags& sizer_flags::x(std::function<int()> rel_to)
		{
			x_binder.unbind_all();
			m_pos[0].type = sizer_elem_type::relative;
			m_pos[0].rel_to = rel_to;
			return *this;
		}

		sizer_flags& sizer_flags::x(int value)
		{
			x_binder.unbind_all();
			m_pos[0].type = sizer_elem_type::absolute;
			m_pos[0].value = value;
			invalidate_widget(true);
			return *this;
		}

		sizer_flags& sizer_flags::y(std::function<int()> rel_to)
		{
			y_binder.unbind_all();
			m_pos[1].type = sizer_elem_type::relative;
			m_pos[1].rel_to = rel_to;
			return *this;
		}

		sizer_flags& sizer_flags::y(int value)
		{
			y_binder.unbind_all();
			m_pos[1].type = sizer_elem_type::absolute;
			m_pos[1].value = value;
			invalidate_widget(true);
			return *this;
		}

		sizer_flags& sizer_flags::size(int width_, int height_)
		{
			return width(width_).height(height_);
		}
		sizer_flags& sizer_flags::size(const size2i &size_)
		{
			return size(size_.width(), size_.height());
		}
		sizer_flags& sizer_flags::size(std::function<int()> width_, std::function<int()> height_)
		{
			return width(width_).height(height_);
		}
		sizer_flags& sizer_flags::size(std::function<size2i()> size_)
		{
			return width([=]() { return size_().width(); }).height([=]() { return size_().height(); });
		}

		sizer_flags& sizer_flags::auto_width(bool value)
		{
			width_binder(m_parent->parent()->size.onchanged) += [=](ignore, ignore) { invalidate_widget(); };

			m_size[0].type = value ? sizer_elem_type::automatical : sizer_elem_type::counter;

			return *this;
		}
		sizer_flags& sizer_flags::auto_height(bool value)
		{
			height_binder(m_parent->parent()->size.onchanged) += [=](ignore, ignore) { invalidate_widget(); };

			m_size[1].type = value ? sizer_elem_type::automatical : sizer_elem_type::counter;

			return *this;
		}
		sizer_flags& sizer_flags::auto_size(bool value)
		{
			return auto_width(value).auto_height(value);
		}

		sizer_flags& sizer_flags::position(int x_, int y_)
		{
			return x(x_).y(y_);
		}
		sizer_flags& sizer_flags::position(std::function<int()> x_, std::function<int()> y_)
		{
			return x(x_).y(y_);
		}
		sizer_flags& sizer_flags::position(std::function<point2i()> position_)
		{
			return x([=]() { return position_().x(); }).y([=]() { return position_().y(); });
		}
		sizer_flags& sizer_flags::position(const point2i &position_)
		{
			return position(position_.x(), position_.y());
		}

		sizer_flags& sizer_flags::center_horizontal()
		{
			return x(m_parent->parent()->size, [=]
			{
				return (m_parent->parent()->width() - m_parent->width()) / 2;
			});
		}

		sizer_flags& sizer_flags::center_vertical()
		{
			return y(m_parent->parent()->size, [=]
			{
				return (m_parent->parent()->height() - m_parent->height()) / 2;
			});
		}

		sizer_flags& sizer_flags::center()
		{
			return center_horizontal().center_vertical();
		}

		sizer_flags& sizer_flags::fill_horizontal()
		{
			return x(0).width(m_parent->parent()->size, [=]
			{
				return m_parent->parent()->width();
			});
		}

		sizer_flags& sizer_flags::fill_vertical()
		{
			return y(0).height(m_parent->parent()->size, [=]
			{
				return m_parent->parent()->height();
			});
		}

		sizer_flags& sizer_flags::fill()
		{
			return fill_horizontal().fill_vertical();
		}

		sizer_flags& sizer_flags::expand_vertical(bool value)
		{
			height_binder.unbind_all();
			y_binder.unbind_all();

			height_binder(m_parent->parent()->size.onchanged) += [=](ignore, ignore) { invalidate_widget(); };
			y_binder(m_parent->parent()->position.onchanged) += [=](ignore, ignore) { invalidate_widget(); };

			m_size[1].type = value ? sizer_elem_type::expand : sizer_elem_type::counter;
			m_pos[1].type = value ? sizer_elem_type::expand : sizer_elem_type::counter;
			return *this;
		}

		sizer_flags& sizer_flags::expand_horizontal(bool value)
		{
			width_binder.unbind_all();
			x_binder.unbind_all();

			width_binder(m_parent->parent()->size.onchanged) += [=](ignore, ignore) { invalidate_widget(); };
			x_binder(m_parent->parent()->position.onchanged) += [=](ignore, ignore) { invalidate_widget(); };

			m_size[0].type = value ? sizer_elem_type::expand : sizer_elem_type::counter;
			m_pos[0].type = value ? sizer_elem_type::expand : sizer_elem_type::counter;
			return *this;
		}

		sizer_flags& sizer_flags::expand(bool value)
		{
			return expand_horizontal(value).expand_vertical(value);
		}

		sizer_flags& sizer_flags::fit(fit_type type)
		{
			m_fit = type;
			return *this;
		}

		sizer_flags& sizer_flags::pull_back(bool value)
		{
			m_pull_back = value;
			return *this;
		}

		void sizer_flags::invalidate_widget(bool is_const)
		{
			if (auto parent_ = m_parent->parent())
			{
				parent_->refresh(is_const);
			}
		}
	}
}

