#pragma once
#include "widget.h"
#include <atomic>

namespace rfe
{
	namespace ui
	{
		class progress_circle : public widget
		{
			std::atomic<float> complete{ 0.0 };

		public:
			data_event<std::shared_ptr<graphics::material>> material;
			event<> onloaded;

			progress_circle();

			void set_complete(float value, bool loaded = false);

		private:
			double m_complete_start;
			animation::queue complete_animation(float value, bool loaded);

			std::weak_ptr<graphics::drawable> drawable;

			void update_drawable();
			void dodraw() override;
		};
	}
}
