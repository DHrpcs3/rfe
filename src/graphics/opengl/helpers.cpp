#include <rfe/graphics/opengl/helpers.h>

namespace rfe
{
	namespace graphics
	{
		namespace opengl
		{
			const fbo screen{};

			void fbo::create()
			{
				glGenFramebuffers(1, &m_id);
			}

			void fbo::bind() const
			{
				glBindFramebuffer(GL_FRAMEBUFFER, m_id);
			}

			void fbo::blit(const fbo& dst, area2i src_area, area2i dst_area, buffers buffers_, filter filter_) const
			{
				bind_as(target::read_frame_buffer);
				dst.bind_as(target::draw_frame_buffer);
				glBlitFramebuffer(
					src_area.p1.x(), src_area.p1.y(), src_area.p2.x(), src_area.p2.y(),
					dst_area.p1.x(), dst_area.p1.y(), dst_area.p2.x(), dst_area.p2.y(),
					(GLbitfield)buffers_, (GLenum)filter_);
			}

			void fbo::bind_as(target target_) const
			{
				glBindFramebuffer((int)target_, id());
			}

			void fbo::remove()
			{
				glDeleteFramebuffers(1, &m_id);
				m_id = 0;
			}

			bool fbo::created() const
			{
				return m_id != 0;
			}

			void texture::settings::apply(const texture &texture) const
			{
				save_binding_state save(texture);

				texture.pixel_unpack_settings().apply();

				if (compressed_format(m_internal_format))
				{
					int compressed_image_size = m_compressed_image_size;
					if (!compressed_image_size)
					{
						switch (m_internal_format)
						{
						case texture::internal_format::compressed_rgb_s3tc_dxt1:
							compressed_image_size = ((m_width + 2) / 3) * ((m_height + 2) / 3) * 6;
							break;

						case texture::internal_format::compressed_rgba_s3tc_dxt1:
							compressed_image_size = ((m_width + 3) / 4) * ((m_height + 3) / 4) * 8;
							break;

						case texture::internal_format::compressed_rgba_s3tc_dxt3:
						case texture::internal_format::compressed_rgba_s3tc_dxt5:
							compressed_image_size = ((m_width + 3) / 4) * ((m_height + 3) / 4) * 16;
							break;
						}
					}

					__glcheck glCompressedTexImage2D((GLenum)texture.get_target(), m_level, (GLint)m_internal_format, m_width, m_height, 0, compressed_image_size, m_pixels);
				}
				else
				{
					__glcheck glTexImage2D((GLenum)texture.get_target(), m_level, (GLint)m_internal_format, m_width, m_height, 0, (GLint)m_format, (GLint)m_type, m_pixels);
				}

				__glcheck glTexParameteri((GLenum)texture.get_target(), GL_TEXTURE_MAX_LEVEL, m_max_level);

				if (m_pixels)
				{
					__glcheck glTexParameteri((GLenum)texture.get_target(), GL_GENERATE_MIPMAP, m_generate_mipmap ? GL_TRUE : GL_FALSE);
				}

				__glcheck glTexParameteri((GLenum)texture.get_target(), GL_TEXTURE_WRAP_S, (GLint)m_wrap_s);
				__glcheck glTexParameteri((GLenum)texture.get_target(), GL_TEXTURE_WRAP_T, (GLint)m_wrap_t);
				__glcheck glTexParameteri((GLenum)texture.get_target(), GL_TEXTURE_WRAP_R, (GLint)m_wrap_r);

				__glcheck glTexParameteri((GLenum)texture.get_target(), GL_TEXTURE_COMPARE_MODE, (GLint)m_compare_mode);
				__glcheck glTexParameteri((GLenum)texture.get_target(), GL_TEXTURE_COMPARE_FUNC, (GLint)m_compare_func);

				__glcheck glTexParameterf((GLenum)texture.get_target(), GL_TEXTURE_MIN_LOD, m_max_lod);
				__glcheck glTexParameterf((GLenum)texture.get_target(), GL_TEXTURE_MAX_LOD, m_min_lod);
				__glcheck glTexParameterf((GLenum)texture.get_target(), GL_TEXTURE_LOD_BIAS, m_lod);

				__glcheck glTexParameterfv((GLenum)texture.get_target(), GL_TEXTURE_BORDER_COLOR, m_border_color.rgba());

				__glcheck glTexParameteri((GLenum)texture.get_target(), GL_TEXTURE_MIN_FILTER, (GLint)m_min_filter);
				__glcheck glTexParameteri((GLenum)texture.get_target(), GL_TEXTURE_MAG_FILTER, (GLint)m_mag_filter);

				__glcheck glTexParameteri((GLenum)texture.get_target(), GL_TEXTURE_SWIZZLE_R, (GLint)m_swizzle_r);
				__glcheck glTexParameteri((GLenum)texture.get_target(), GL_TEXTURE_SWIZZLE_G, (GLint)m_swizzle_g);
				__glcheck glTexParameteri((GLenum)texture.get_target(), GL_TEXTURE_SWIZZLE_B, (GLint)m_swizzle_b);
				__glcheck glTexParameteri((GLenum)texture.get_target(), GL_TEXTURE_SWIZZLE_A, (GLint)m_swizzle_a);

				__glcheck glTexParameterf((GLenum)texture.get_target(), GL_TEXTURE_MAX_ANISOTROPY_EXT, m_aniso);
			}

			void texture::settings::apply()
			{
				if (m_parent)
				{
					apply(*m_parent);
					m_parent = nullptr;
				}
			}

			texture::settings& texture::settings::swizzle(texture::channel r, texture::channel g, texture::channel b, texture::channel a)
			{
				m_swizzle_r = r;
				m_swizzle_g = g;
				m_swizzle_b = b;
				m_swizzle_a = a;

				return *this;
			}

			texture::settings& texture::settings::format(texture::format format)
			{
				m_format = format;
				return *this;
			}

			texture::settings& texture::settings::type(texture::type type)
			{
				m_type = type;
				return *this;
			}

			texture::settings& texture::settings::internal_format(texture::internal_format format)
			{
				m_internal_format = format;
				return *this;
			}

			texture::settings& texture::settings::filter(min_filter min_filter, opengl::filter mag_filter)
			{
				m_min_filter = min_filter;
				m_mag_filter = mag_filter;

				return *this;
			}

			texture::settings& texture::settings::width(uint width)
			{
				m_width = width;
				return *this;
			}

			texture::settings& texture::settings::height(uint height)
			{
				m_height = height;
				return *this;
			}

			texture::settings& texture::settings::size(size2i size)
			{
				return width(size.width()).height(size.height());
			}

			texture::settings& texture::settings::level(int value)
			{
				m_level = value;
				return *this;
			}

			texture::settings& texture::settings::compressed_image_size(int size)
			{
				m_compressed_image_size = size;
				return *this;
			}

			texture::settings& texture::settings::pixels(const void* pixels)
			{
				m_pixels = pixels;
				return *this;
			}

			texture::settings& texture::settings::aniso(float value)
			{
				m_aniso = value;
				return *this;
			}

			texture::settings& texture::settings::compare_mode(texture::compare_mode value)
			{
				m_compare_mode = value;
				return *this;
			}
			texture::settings& texture::settings::compare_func(texture::compare_func value)
			{
				m_compare_func = value;
				return *this;
			}
			texture::settings& texture::settings::compare(texture::compare_func func, texture::compare_mode mode)
			{
				return compare_func(func).compare_mode(mode);
			}

			texture::settings& texture::settings::wrap_s(texture::wrap value)
			{
				m_wrap_s = value;
				return *this;
			}
			texture::settings& texture::settings::wrap_t(texture::wrap value)
			{
				m_wrap_t = value;
				return *this;
			}
			texture::settings& texture::settings::wrap_r(texture::wrap value)
			{
				m_wrap_r = value;
				return *this;
			}
			texture::settings& texture::settings::wrap(texture::wrap s, texture::wrap t, texture::wrap r)
			{
				return wrap_s(s).wrap_t(t).wrap_r(r);
			}

			texture::settings& texture::settings::max_lod(float value)
			{
				m_max_lod = value;
				return *this;
			}
			texture::settings& texture::settings::min_lod(float value)
			{
				m_min_lod = value;
				return *this;
			}
			texture::settings& texture::settings::lod(float value)
			{
				m_lod = value;
				return *this;
			}
			texture::settings& texture::settings::max_level(int value)
			{
				m_max_level = value;
				return *this;
			}
			texture::settings& texture::settings::generate_mipmap(bool value)
			{
				m_generate_mipmap = value;
				return *this;
			}
			texture::settings& texture::settings::mipmap(int level, int max_level, float lod, float min_lod, float max_lod, bool generate)
			{
				return this->level(level).max_level(max_level).lod(lod).min_lod(min_lod).max_lod(max_lod).generate_mipmap(generate);
			}

			texture::settings& texture::settings::border_color(color4f value)
			{
				m_border_color = value;
				return *this;
			}

			texture_view texture::with_level(int level)
			{
				return{ get_target(), id() };
			}

			texture::settings texture::config()
			{
				return{ this };
			}

			void texture::config(const settings& settings_)
			{
				settings_.apply(*this);
			}
		}
	}
}