#include <rfe/core/events.h>

namespace rfe
{
	inline namespace core
	{
		namespace events
		{
			namespace keyboard
			{
				event<int> onkey_up;
				event<int> onkey_down;
			}

			namespace mouse
			{
				event<int, point2i> onkey_up;
				event<int, point2i> onkey_down;
				event<point2i> motion;
			}
		}
	}
}
