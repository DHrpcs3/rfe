#include <rfe/graphics/draw_context.h>
#include <chrono>

#include <ft2build.h>
#include FT_FREETYPE_H


using namespace std::chrono_literals;

namespace rfe
{
	namespace font
	{
		FT_Library ft_lib;

		void init()
		{
			if (FT_Init_FreeType(&ft_lib))
			{
				throw;
			}
		}

		face& face::set_pixel_sizes(int pixel_height, int pixel_width)
		{
			FT_Set_Pixel_Sizes((FT_Face)ft_face, pixel_width, pixel_height);

			return *this;
		}

		void face::load_char(char c, load_bits load_bits_) const
		{
			if (FT_Load_Char((FT_Face)ft_face, c, (int)load_bits_))
			{
				throw;
			}
		}

		bool face::load_char(std::nothrow_t, char c, load_bits load_bits_) const noexcept
		{
			if (FT_Load_Char((FT_Face)ft_face, c, (int)load_bits_))
			{
				return false;
			}

			return true;
		}

		face new_face(const std::string &file_path, int face_index)
		{
			FT_Face result;
			if (FT_New_Face(ft_lib, file_path.c_str(), face_index, &result))
			{
				throw;
			}

			return{ result };
		}
	}

	namespace graphics
	{
		void draw_context::detach(std::weak_ptr<drawable_base> detach_drawable)
		{
			if (auto detach = detach_drawable.lock())
			{
				for (auto it = m_drawables.begin(); it != m_drawables.end(); ++it)
				{
					if (auto locked = it->lock())
					{
						if (locked == detach)
						{
							m_drawables.erase(it);
							return;
						}
					}
					else
					{
						it = m_drawables.erase(it);
					}
				}
			}
		}

		void draw_context::prepare(std::shared_ptr<void> parent, std::weak_ptr<drawable> &drawable, const model &m)
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtx);

			if (!drawable.expired())
			{
				detach(drawable);
			}

			drawable = prepare(parent, m);
		}

		void draw_context::prepare(std::shared_ptr<void> parent, std::weak_ptr<drawable_text> &drawable, const font::face &m)
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtx);

			if (!drawable.expired())
			{
				detach(drawable);
			}

			drawable = prepare(parent, m);
		}

		void draw_context::present()
		{
			++m_frames;

			auto diff = clock::now() - m_fps_flush_time;
			if (diff >= 1s)
			{
				fps = m_frames / (double(diff.count()) / clock::period::den);
				m_frames = 0;
				m_fps_flush_time = clock::now();
			}
		}

		void draw_context::invalidate()
		{
			m_invalidated = true;
		}
	}
}
