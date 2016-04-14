#pragma once
#include "widget.h"
#include <unordered_map>

namespace rfe
{
	namespace ui
	{
		struct font_cache
		{
			std::unordered_map<font::info, std::weak_ptr<graphics::drawable_text>, fnv_1a_hasher> data;
		};

		class label : public widget
		{
		public:
			static font_cache cache;
			static font::info default_font;

			data_event<std::string> text;
			data_event<font::info> font;
			data_event<color4f> color;

			label(const font::info &font = default_font);

		private:
			bool m_font_invalidated = true;
			bool m_text_invalidated = true;
			bool m_matrix_invalidated = true;

			std::weak_ptr<graphics::drawable_text> m_text_drawable;
			graphics::text_coords_t m_text_coords;
			matrix4f m_matrix;

			void set_parent(std::shared_ptr<widget> parent) override;

			void dodraw() override;
		};
	}
}