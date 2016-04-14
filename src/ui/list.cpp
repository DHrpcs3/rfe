#include <rfe/ui/list.h>

namespace rfe
{
	namespace ui
	{
		list::list() : scrollable{ this }
		{
			name = "list widget";

			onselection_change += [=](std::shared_ptr<list_entry> widget)
			{
				m_selection = widget;
				return event_result::handled;
			};

			oninit += [=]
			{
				append_child(background).fit(ui::fit_type::never).fill();
			};

			background->onclick += [=](ignore)
			{
				for (auto& entry : m_entries)
				{
					entry->unselect();
				}
			};

			onmotion += [=](point2i start_point, point2i current_point)
			{
				vertical_scroll = vertical_scroll() + (current_point.y() - start_point.y());
			};
		}

		ui::sizer_flags& list::add_entry(std::shared_ptr<list_entry> widget)
		{
			m_entries.emplace_back(widget);
			auto &binder = m_entries.back()->event_binder;

			binder(widget->onactivate) += [=]
			{
				onactivate(widget);
			};

			binder(widget->selected.onchanged) += [=](ignore, bool selected)
			{
				if (selected)
				{
					for (auto& entry : m_entries)
					{
						if (entry != widget)
						{
							entry->unselect();
						}
					}

					onselection_change(widget);
				}
				else
				{
					onselection_change(nullptr);
				}
			};

			return static_cast<ui::widget&>(*this) += widget;
		}

		bool list::remove_entry(std::shared_ptr<list_entry> widget)
		{
			for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
			{
				if (*it == widget)
				{
					widget->onremove();
					m_entries.erase(it);
					return true;
				}
			}

			return false;
		}

		ui::sizer_flags& list::operator +=(std::shared_ptr<list_entry> widget)
		{
			return add_entry(widget);
		}

		bool list::operator -=(std::shared_ptr<list_entry> widget)
		{
			return remove_entry(widget);
		}

		std::shared_ptr<list_entry> list::selection() const
		{
			return m_selection.lock();
		}
	}
}
