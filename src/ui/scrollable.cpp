#include <rfe/ui/scrollable.h>
#include <rfe/ui/widget.h>

namespace rfe
{
	namespace ui
	{
		scrollable::scrollable(widget *parent)
		{
			horizontal_scroll.onchanged += [=](int old_value, int new_value)
			{
				parent->sizer_flags().x(parent->x() + (new_value - old_value));
				parent->refresh();
				return event_result::handled;
			};

			vertical_scroll.onchanged += [=](int old_value, int new_value)
			{
				parent->sizer_flags().y(parent->y() + (new_value - old_value));
				parent->refresh();
				return event_result::handled;
			};
		}
	}
}
