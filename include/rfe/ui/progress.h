#pragma once
#include "widget.h"
#include "ground.h"
#include <atomic>

namespace rfe
{
	namespace ui
	{
		class progress : public widget
		{
			std::atomic<float> m_complete{ 0.0 };

		public:
			std::shared_ptr<class ground> background = ui::make_shared<class ground>();
			std::shared_ptr<class ground> foreground = ui::make_shared<class ground>();

			event<> onloaded;

			progress();
			void set_complete(float value, bool loaded = false);

		private:
			double m_complete_start;
			animation::queue complete_animation(float value, bool loaded);
		};
	}
}
