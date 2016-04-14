#pragma once
#include "widget.h"

namespace rfe
{
	namespace ui
	{
		class ground : public widget
		{
		public:
			data_event<color4f> color{ color4f{ 0.f, 0.f, 0.f, 0.f } };
			data_event<graphics::texture> texture;

			ground();

		private:
			std::weak_ptr<graphics::drawable> color_drawable;
			std::weak_ptr<graphics::drawable> texture_drawable;

			void make_color_drawable(const color4f &value);
			void make_texture_drawable(const graphics::texture& tex);

			void dodraw() override;
		};
	}
}