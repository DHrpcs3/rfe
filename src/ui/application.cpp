#include <rfe/ui/application.h>

namespace rfe
{
	namespace ui
	{
		void application::run(window& main_window)
		{
			main_window.show();

			while (main_window.alive())
			{
				main_window.update();
				main_window.draw();
			}
		}

		void application::run(std::shared_ptr<window> main_window)
		{
			run(*main_window);
		}
	}
}
