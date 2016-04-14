#pragma once
#include <rfe/core/event.h>

namespace rfe
{
	namespace ui
	{
		class widget;

		class scrollable
		{
		public:
			data_event<int> vertical_scroll;
			data_event<int> horizontal_scroll;

			scrollable(widget *parent);
		};
	}
}
