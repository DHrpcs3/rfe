#include <rfe/ui/list_entry.h>

namespace rfe
{
	namespace ui
	{
		list_entry::list_entry()
		{
			name = "list entry";
			orientation = ui::orientation::horizontal;

			onremove += [=]
			{
				if (selected())
				{
					unselect();
				}

				return event_result::handled;
			};

			ontry_click += [=](ignore)
			{
				select();
				return event_result::handled;
			};
		}

		void list_entry::select()
		{
			selected = true;
		}

		void list_entry::unselect()
		{
			selected = false;
		}

		void list_entry::activate()
		{
			onactivate();
		}
	}
}
