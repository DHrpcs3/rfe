#include <rfe/ui/label.h>

namespace rfe
{
	namespace ui
	{
		font_cache label::cache;
		font::info label::default_font;

		label::label(const font::info &font_)
		{
			name = "label";

			font = font_;
			size = { 0, font_.size };
			//set_borders(0, 0, 0, 0);

			text.onchanged += [=](ignore, ignore)
			{
				m_text_invalidated = true;
				return event_result::skip;
			};

			font.onchanged += [=](ignore, ignore)
			{
				m_font_invalidated = true;
				return event_result::skip;
			};

			size.onchanged += [=](ignore, ignore)
			{
				m_matrix_invalidated = true;
				return event_result::skip;
			};

			position.onchanged += [=](ignore, ignore)
			{
				m_matrix_invalidated = true;
				return event_result::skip;
			};
		}

		void label::set_parent(std::shared_ptr<widget> parent)
		{
			widget::set_parent(parent);

			if (m_parent)
			{
				parent_event_binder(m_parent->onrefresh) += [=]
				{
					m_matrix_invalidated = true;
					return event_result::skip;
				};
			}
		}

		void label::dodraw()
		{
			std::string text_ = text();

			if (text_.empty())
			{
				return;
			}

			if (m_text_invalidated || m_font_invalidated || m_text_drawable.expired())
			{
				auto font_ = font();

				if (m_text_drawable.expired())
				{
					m_text_drawable = cache.data[font_];

					if (m_text_drawable.expired())
					{
						dc()->prepare(shared_ptr(), m_text_drawable, font_.face.set_pixel_sizes(font_.size));
					}
				}

				size = size2i{ (int)text().length() * font_.size / 2, font_.size };
				m_font_invalidated = false;
			}

			if (m_matrix_invalidated)
			{
				//auto window_size = top_widget()->size();

				if (auto parent_ = parent())
				{
					point2i absolute_positon = position();
					while (parent_->parent())
					{
						absolute_positon += std::static_pointer_cast<widget>(parent_)->position();
						parent_ = parent_->parent();
					}

					auto window_size = std::static_pointer_cast<widget>(parent_)->size();
					size2d scale = window_size;
					point2d offset{};

					const double aq = 1.0;
					const double rq = (double)scale.width() / scale.height();
					const double q = aq / rq;

					if (q > 1.0)
					{
						scale.height(window_size.height() / q);
						offset.y((window_size.height() - scale.height()) / 2.0);
					}
					else if (q < 1.0)
					{
						scale.width(window_size.width() * q);
						offset.x((window_size.width() - scale.width()) / 2.0);
					}

					scale /= window_size;
					offset /= window_size;

					point2d translate = (point2d(absolute_positon) + point2d{ 0., (double)font().size }) * 2. / window_size - point2d{ 1., 1. };
					m_matrix = mtx::scale_offset((vector3f)scale, { (float)translate.x(), (float)-translate.y(), 0 });
					m_matrix_invalidated = false;
				}
			}

			if (auto drawable = m_text_drawable.lock())
			{
				if (m_text_invalidated)
				{
					m_text_coords = drawable->prepare(text_);
					m_text_invalidated = false;
				}

				vector4f clip;

				if (auto parent_ = std::static_pointer_cast<widget>(parent()))
				{
					clip = parent_->size();
				}
				else
				{
					clip = size();
				}

				drawable->draw(m_text_coords, color(), clip, m_matrix);
			}
		}
	}
}