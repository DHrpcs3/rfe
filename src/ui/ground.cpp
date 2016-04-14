#include <rfe/ui/ground.h>

namespace rfe
{
	namespace ui
	{
		ground::ground()
		{
			name = "ground";

			color.onchanged += [=](ignore, const color4f &new_color)
			{
				make_color_drawable(new_color);
			};

			texture.onchanged += [=](ignore, const graphics::texture& texture)
			{
				make_texture_drawable(texture);
			};

			m_draw_context.onchanged += [=](ignore, const std::shared_ptr<graphics::draw_context>& draw_context)
			{
				if (draw_context)
				{
					make_color_drawable(color());
					make_texture_drawable(texture());
				}
			};
		}

		void ground::make_color_drawable(const color4f &value)
		{
			auto color_ = color();

			if (color_.a() <= 0.01f)
			{
				color_drawable.reset();
				return;
			}

			auto dc = m_draw_context();
			if (!dc)
			{
				return;
			}

			dc->thread.invoke([=]
			{
				auto material = std::make_shared<graphics::material>();
				material->set_color(color_);

				dc->prepare(shared_ptr(), color_drawable, model().material(material));
			}, std::launch::async);
		}

		void ground::make_texture_drawable(const graphics::texture& tex)
		{
			if (tex.empty())
			{
				texture_drawable.reset();
				return;
			}

			auto dc = m_draw_context();

			if (!dc)
			{
				return;
			}

			dc->thread.invoke([=]
			{
				this->size = tex.image().size();

				auto material = std::make_shared<graphics::material>();
				material->set_texture(tex);

				dc->prepare(shared_ptr(), texture_drawable, model().material(material));
			}, std::launch::async);
		}

		void ground::dodraw()
		{
			auto size_ = size();

			if (size_.width() > 0 && size_.height() > 0)
			{
				if (auto drawable = color_drawable.lock())
				{
					drawable->draw(clip, matrix);
				}

				if (auto drawable = texture_drawable.lock())
				{
					drawable->draw(clip, matrix);
				}
			}
		}
	}
}