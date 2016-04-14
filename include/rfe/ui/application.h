#pragma once
#include <memory>
#include "window.h"

namespace rfe
{
	namespace ui
	{
		class application
		{
		public:
			static void run(window& main_window);
			static void run(std::shared_ptr<window> main_window);
		};
	}
}
