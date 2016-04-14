#include "widget.h"
#include <rfe/core/event.h>

namespace rfe
{
	namespace ui
	{
		class list_entry : public widget
		{
		public:
			data_event<bool> selected;
			event<> onremove;
			event<> onactivate;

			list_entry();

			void select();
			void unselect();
			void activate();
		};
	}
}
