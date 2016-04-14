#pragma once
#include "event.h"
#include "types.h"

namespace rfe
{
	inline namespace core
	{
		namespace events
		{
			namespace keyboard
			{
				extern event<int> onkey_up;
				extern event<int> onkey_down;
			}

			namespace mouse
			{
				extern event<int, point2i> onkey_up;
				extern event<int, point2i> onkey_down;
				extern event<point2i> motion;
			}
		}
	}
}
