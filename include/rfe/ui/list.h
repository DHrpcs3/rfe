#pragma once
#include "scrollable.h"
#include "widget.h"
#include "list_entry.h"
#include "ground.h"

namespace rfe
{
	namespace ui
	{
		class list : public widget, public scrollable
		{
			std::weak_ptr<list_entry> m_selection;

			std::list<std::shared_ptr<list_entry>> m_entries;
			bool m_clicked = false;

		public:
			std::shared_ptr<ground> background = make_shared<ground>();

			event<std::shared_ptr<list_entry>> onactivate;
			event<std::shared_ptr<list_entry>> onselection_change;

			list();

			ui::sizer_flags& add_entry(std::shared_ptr<list_entry> widget);

			bool remove_entry(std::shared_ptr<list_entry> widget);

			ui::sizer_flags& operator +=(std::shared_ptr<list_entry> widget);
			bool operator -=(std::shared_ptr<list_entry> widget);
			std::shared_ptr<list_entry> selection() const;
		};
	}
}
