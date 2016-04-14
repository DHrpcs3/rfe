#pragma once
#include <exception>
#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>

#include <rfe/core/types.h>
#include <rfe/core/fmt.h>

#include "opengl.h"

#include "buffer.h"

namespace rfe
{
	namespace graphics
	{
		namespace opengl
		{
#ifdef _DEBUG
			struct __glcheck_impl_t
			{
				const char* file;
				const char* function;
				long line;

				constexpr __glcheck_impl_t(const char* file, const char* function, long line)
					: file(file)
					, function(function)
					, line(line)
				{
				}

				~__glcheck_impl_t() noexcept(false)
				{
					if (GLenum err = glGetError())
					{
						std::string error;
						switch (err)
						{
						case GL_INVALID_OPERATION:      error = "invalid operation";      break;
						case GL_INVALID_ENUM:           error = "invalid enum";           break;
						case GL_INVALID_VALUE:          error = "invalid value";          break;
						case GL_OUT_OF_MEMORY:          error = "out of memory";          break;
						case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "invalid framebuffer operation";  break;
						default: error = "unknown error"; break;
						}

						throw std::runtime_error(::rfe::fmt::format("OpenGL error: %s. file '%s' function '%s' line %ld", error.c_str(), file, function, line));
					}
				}
			};
#define __glcheck ::rfe::graphics::opengl::__glcheck_impl_t{ __FILE__, __FUNCTION__, __LINE__ },
#else
#define __glcheck
#endif

			class exception : public std::exception
			{
			protected:
				std::string m_what;

			public:
				const char* what() const noexcept override
				{
					return m_what.c_str();
				}
			};

			template<typename Type, uint BindId, uint GetStateId>
			class save_binding_state_base
			{
				GLint m_last_binding;

			public:
				save_binding_state_base(const Type& new_state) : save_binding_state_base()
				{
					new_state.bind();
				}

				save_binding_state_base()
				{
					glGetIntegerv(GetStateId, &m_last_binding);
				}

				~save_binding_state_base()
				{
					glBindBuffer(BindId, m_last_binding);
				}
			};

			enum class filter
			{
				nearest = GL_NEAREST,
				linear = GL_LINEAR
			};

			enum class min_filter
			{
				nearest = GL_NEAREST,
				linear = GL_LINEAR,
				nearest_mipmap_nearest = GL_NEAREST_MIPMAP_NEAREST,
				nearest_mipmap_linear = GL_NEAREST_MIPMAP_LINEAR,
				linear_mipmap_nearest = GL_LINEAR_MIPMAP_NEAREST,
				linear_mipmap_linear = GL_LINEAR_MIPMAP_LINEAR
			};

			enum class buffers
			{
				color = GL_COLOR_BUFFER_BIT,
				depth = GL_DEPTH_BUFFER_BIT,
				stencil = GL_STENCIL_BUFFER_BIT,

				color_depth = color | depth,
				color_depth_stencil = color | depth | stencil,
				color_stencil = color | stencil,

				depth_stencil = depth | stencil
			};
	
			class pixel_pack_settings
			{
				bool m_swap_bytes = false;
				bool m_lsb_first = false;
				int m_row_length = 0;
				int m_image_height = 0;
				int m_skip_rows = 0;
				int m_skip_pixels = 0;
				int m_skip_images = 0;
				int m_aligment = 4;

			public:
				void apply() const
				{
					glPixelStorei(GL_PACK_SWAP_BYTES, m_swap_bytes ? GL_TRUE : GL_FALSE);
					glPixelStorei(GL_PACK_LSB_FIRST, m_lsb_first ? GL_TRUE : GL_FALSE);
					glPixelStorei(GL_PACK_ROW_LENGTH, m_row_length);
					glPixelStorei(GL_PACK_IMAGE_HEIGHT, m_image_height);
					glPixelStorei(GL_PACK_SKIP_ROWS, m_skip_rows);
					glPixelStorei(GL_PACK_SKIP_PIXELS, m_skip_pixels);
					glPixelStorei(GL_PACK_SKIP_IMAGES, m_skip_images);
					glPixelStorei(GL_PACK_ALIGNMENT, m_aligment);
				}

				pixel_pack_settings& swap_bytes(bool value = true)
				{
					m_swap_bytes = value;
					return *this;
				}
				pixel_pack_settings& lsb_first(bool value = true)
				{
					m_lsb_first = value;
					return *this;
				}
				pixel_pack_settings& row_length(int value)
				{
					m_row_length = value;
					return *this;
				}
				pixel_pack_settings& image_height(int value)
				{
					m_image_height = value;
					return *this;
				}
				pixel_pack_settings& skip_rows(int value)
				{
					m_skip_rows = value;
					return *this;
				}
				pixel_pack_settings& skip_pixels(int value)
				{
					m_skip_pixels = value;
					return *this;
				}
				pixel_pack_settings& skip_images(int value)
				{
					m_skip_images = value;
					return *this;
				}
				pixel_pack_settings& aligment(int value)
				{
					m_aligment = value;
					return *this;
				}
			};

			class pixel_unpack_settings
			{
				bool m_swap_bytes = false;
				bool m_lsb_first = false;
				int m_row_length = 0;
				int m_image_height = 0;
				int m_skip_rows = 0;
				int m_skip_pixels = 0;
				int m_skip_images = 0;
				int m_aligment = 4;

			public:
				void apply() const
				{
					glPixelStorei(GL_UNPACK_SWAP_BYTES, m_swap_bytes ? GL_TRUE : GL_FALSE);
					glPixelStorei(GL_UNPACK_LSB_FIRST, m_lsb_first ? GL_TRUE : GL_FALSE);
					glPixelStorei(GL_UNPACK_ROW_LENGTH, m_row_length);
					glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, m_image_height);
					glPixelStorei(GL_UNPACK_SKIP_ROWS, m_skip_rows);
					glPixelStorei(GL_UNPACK_SKIP_PIXELS, m_skip_pixels);
					glPixelStorei(GL_UNPACK_SKIP_IMAGES, m_skip_images);
					glPixelStorei(GL_UNPACK_ALIGNMENT, m_aligment);
				}

				pixel_unpack_settings& swap_bytes(bool value = true)
				{
					m_swap_bytes = value;
					return *this;
				}
				pixel_unpack_settings& lsb_first(bool value = true)
				{
					m_lsb_first = value;
					return *this;
				}
				pixel_unpack_settings& row_length(int value)
				{
					m_row_length = value;
					return *this;
				}
				pixel_unpack_settings& image_height(int value)
				{
					m_image_height = value;
					return *this;
				}
				pixel_unpack_settings& skip_rows(int value)
				{
					m_skip_rows = value;
					return *this;
				}
				pixel_unpack_settings& skip_pixels(int value)
				{
					m_skip_pixels = value;
					return *this;
				}
				pixel_unpack_settings& skip_images(int value)
				{
					m_skip_images = value;
					return *this;
				}
				pixel_unpack_settings& aligment(int value)
				{
					m_aligment = value;
					return *this;
				}
			};

			

			class vao
			{
				template<buffer::target BindId, uint GetStateId>
				class entry
				{
					vao& m_parent;

				public:
					using save_binding_state = save_binding_state_base<entry, (GLuint)BindId, GetStateId>;

					entry(vao* parent) noexcept : m_parent(*parent)
					{
					}

					entry& operator = (const buffer& buf) noexcept
					{
						m_parent.bind();
						buf.bind(BindId);

						return *this;
					}
				};

				GLuint m_id = GL_NONE;

			public:
				entry<buffer::target::pixel_pack, GL_PIXEL_PACK_BUFFER_BINDING> pixel_pack_buffer{ this };
				entry<buffer::target::pixel_unpack, GL_PIXEL_UNPACK_BUFFER_BINDING> pixel_unpack_buffer{ this };
				entry<buffer::target::array, GL_ARRAY_BUFFER_BINDING> array_buffer{ this };
				entry<buffer::target::element_array, GL_ELEMENT_ARRAY_BUFFER_BINDING> element_array_buffer{ this };

				vao() = default;
				vao(vao&) = delete;

				vao(vao&& vao_) noexcept
				{
					swap(vao_);
				}
				vao(GLuint id) noexcept
				{
					set_id(id);
				}

				~vao() noexcept
				{
					if (created())
						remove();
				}

				void swap(vao& vao_) noexcept
				{
					auto my_old_id = id();
					set_id(vao_.id());
					vao_.set_id(my_old_id);
				}

				vao& operator = (const vao& rhs) = delete;
				vao& operator = (vao&& rhs) noexcept
				{
					swap(rhs);
					return *this;
				}

				void bind() const noexcept
				{
					glBindVertexArray(m_id);
				}

				void create() noexcept
				{
					glGenVertexArrays(1, &m_id);
				}

				void remove() noexcept
				{
					glDeleteVertexArrays(1, &m_id);
					m_id = GL_NONE;
				}

				uint id() const noexcept
				{
					return m_id;
				}

				void set_id(uint id) noexcept
				{
					m_id = id;
				}

				bool created() const noexcept
				{
					return m_id != GL_NONE;
				}

				explicit operator bool() const noexcept
				{
					return created();
				}

				void enable_for_attributes(std::initializer_list<GLuint> indexes) const noexcept
				{
					for (auto &index : indexes)
					{
						__glcheck glEnableVertexAttribArray(index);
					}
				}

				void disable_for_attributes(std::initializer_list<GLuint> indexes) const noexcept
				{
					for (auto &index : indexes)
					{
						glDisableVertexAttribArray(index);
					}
				}

				void enable_for_attribute(GLuint index) const noexcept
				{
					enable_for_attributes({ index });
				}

				void disable_for_attribute(GLuint index) const noexcept
				{
					disable_for_attributes({ index });
				}

				buffer_pointer operator + (u32 offset) const noexcept
				{
					return{ (vao*)this, offset };
				}

				buffer_pointer operator >> (u32 stride) const noexcept
				{
					return{ (vao*)this, {}, stride };
				}

				operator buffer_pointer() const noexcept
				{
					return{ (vao*)this };
				}
			};

			class texture_view;
			class texture
			{
				GLuint m_id = 0;
				GLuint m_level = 0;
				class pixel_pack_settings m_pixel_pack_settings;
				class pixel_unpack_settings m_pixel_unpack_settings;

			public:
				enum class type
				{
					ubyte = GL_UNSIGNED_BYTE,
					ushort = GL_UNSIGNED_SHORT,
					uint = GL_UNSIGNED_INT,

					ubyte_3_3_2 = GL_UNSIGNED_BYTE_3_3_2,
					ubyte_2_3_3_rev = GL_UNSIGNED_BYTE_2_3_3_REV,

					ushort_5_6_5 = GL_UNSIGNED_SHORT_5_6_5,
					ushort_5_6_5_rev = GL_UNSIGNED_SHORT_5_6_5_REV,
					ushort_4_4_4_4 = GL_UNSIGNED_SHORT_4_4_4_4,
					ushort_4_4_4_4_rev = GL_UNSIGNED_SHORT_4_4_4_4_REV,
					ushort_5_5_5_1 = GL_UNSIGNED_SHORT_5_5_5_1,
					ushort_1_5_5_5_rev = GL_UNSIGNED_SHORT_1_5_5_5_REV,

					uint_8_8_8_8 = GL_UNSIGNED_INT_8_8_8_8,
					uint_8_8_8_8_rev = GL_UNSIGNED_INT_8_8_8_8_REV,
					uint_10_10_10_2 = GL_UNSIGNED_INT_10_10_10_2,
					uint_2_10_10_10_rev = GL_UNSIGNED_INT_2_10_10_10_REV,
					uint_24_8 = GL_UNSIGNED_INT_24_8,

					sbyte = GL_BYTE,
					sshort = GL_SHORT,
					sint = GL_INT,
					f16 = GL_HALF_FLOAT,
					f32 = GL_FLOAT,
					f64 = GL_DOUBLE,
				};

				enum class channel
				{
					zero = GL_ZERO,
					one = GL_ONE,
					r = GL_RED,
					g = GL_GREEN,
					b = GL_BLUE,
					a = GL_ALPHA,
				};

				enum class format
				{
					red = GL_RED,
					r = GL_R,
					rg = GL_RG,
					rgb = GL_RGB,
					rgba = GL_RGBA,

					bgr = GL_BGR,
					bgra = GL_BGRA,

					stencil = GL_STENCIL_INDEX,
					depth = GL_DEPTH_COMPONENT,
					depth_stencil = GL_DEPTH_STENCIL
				};

				enum class internal_format
				{
					red = GL_RED,
					r = GL_R,
					rg = GL_RG,
					rgb = GL_RGB,

					r3_g3_b2 = GL_R3_G3_B2,
					rgb4 = GL_RGB4,
					rgb5 = GL_RGB5,
					rgb8 = GL_RGB8,
					rgb8_snorm = GL_RGB8_SNORM,
					rgb10 = GL_RGB10,
					rgb12 = GL_RGB12,
					rgb16 = GL_RGB16,
					rgb16_snorm = GL_RGB16_SNORM,

					rgba = GL_RGBA,

					bgr = GL_BGR,
					bgra = GL_BGRA,

					stencil = GL_STENCIL_INDEX,
					stencil8 = GL_STENCIL_INDEX8,
					depth = GL_DEPTH_COMPONENT,
					depth16 = GL_DEPTH_COMPONENT16,
					depth_stencil = GL_DEPTH_STENCIL,
					depth24_stencil8 = GL_DEPTH24_STENCIL8,

					compressed_rgb_s3tc_dxt1 = GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
					compressed_rgba_s3tc_dxt1 = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
					compressed_rgba_s3tc_dxt3 = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
					compressed_rgba_s3tc_dxt5 = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
				};

				enum class wrap
				{
					repeat = GL_REPEAT,
					mirrored_repeat = GL_MIRRORED_REPEAT,
					clamp_to_edge = GL_CLAMP_TO_EDGE,
					clamp_to_border = GL_CLAMP_TO_BORDER,
					mirror_clamp = GL_MIRROR_CLAMP_EXT,
					mirror_clamp_to_edge = GL_MIRROR_CLAMP_TO_EDGE,
					mirror_clamp_to_border = GL_MIRROR_CLAMP_TO_BORDER_EXT
				};

				enum class compare_mode
				{
					none = GL_NONE,
					ref_to_texture = GL_COMPARE_REF_TO_TEXTURE
				};

				enum class compare_func
				{
					never = GL_NEVER,
					less = GL_LESS,
					equal = GL_EQUAL,
					lequal = GL_LEQUAL,
					greater = GL_GREATER,
					notequal = GL_NOTEQUAL,
					gequal = GL_GEQUAL,
					always = GL_ALWAYS
				};

				enum class target
				{
					texture1D = GL_TEXTURE_1D,
					texture2D = GL_TEXTURE_2D,
					texture3D = GL_TEXTURE_3D
				};

				enum class channel_type
				{
					none = GL_NONE,
					signed_normalized = GL_SIGNED_NORMALIZED,
					unsigned_normalized = GL_UNSIGNED_NORMALIZED,
					float_ = GL_FLOAT,
					int_ = GL_INT,
					uint_ = GL_UNSIGNED_INT
				};

				enum class channel_name
				{
					red = GL_TEXTURE_RED_TYPE,
					green = GL_TEXTURE_GREEN_TYPE,
					blue = GL_TEXTURE_BLUE_TYPE,
					alpha = GL_TEXTURE_ALPHA_TYPE,
					depth = GL_TEXTURE_DEPTH_TYPE
				};

				class save_binding_state
				{
					GLint m_last_binding;
					GLenum m_target;

				public:
					save_binding_state(const texture& new_binding) noexcept
					{
						GLenum pname;
						switch (new_binding.get_target())
						{
						case target::texture1D: pname = GL_TEXTURE_1D_BINDING_EXT; break;
						case target::texture2D: pname = GL_TEXTURE_2D_BINDING_EXT; break;
						case target::texture3D: pname = GL_TEXTURE_3D_BINDING_EXT; break;
						default:
							abort();
						}

						glGetIntegerv(pname, &m_last_binding);
				
						new_binding.bind();
						m_target = (GLenum)new_binding.get_target();
					}

					~save_binding_state() noexcept
					{
						glBindTexture(m_target, m_last_binding);
					}
				};

				class settings;

			private:
				target m_target = target::texture2D;

			public:
				target get_target() const noexcept
				{
					return m_target;
				}

				void set_target(target target) noexcept
				{
					m_target = target;
				}

				static bool compressed_format(texture::internal_format format_) noexcept
				{
					switch (format_)
					{
					case texture::internal_format::compressed_rgb_s3tc_dxt1:
					case texture::internal_format::compressed_rgba_s3tc_dxt1:
					case texture::internal_format::compressed_rgba_s3tc_dxt3:
					case texture::internal_format::compressed_rgba_s3tc_dxt5:
						return true;
					}

					return false;
				}

				uint id() const noexcept
				{
					return m_id;
				}

				uint level() const noexcept
				{
					return m_level;
				}

				void recreate() noexcept
				{
					if (created())
						remove();

					create();
				}

				void recreate(target target_) noexcept
				{
					if (created())
						remove();

					create(target_);
				}

				void create() noexcept
				{
					glGenTextures(1, &m_id);
				}

				void create(target target_) noexcept
				{
					set_target(target_);
					create();
				}

				bool created() const noexcept
				{
					return m_id != 0;
				}

				void remove() noexcept
				{
					glDeleteTextures(1, &m_id);
					m_id = 0;
				}

				void set_id(GLuint id) noexcept
				{
					m_id = id;
				}

				void set_level(int level) noexcept
				{
					m_level = level;
				}

				texture_view with_level(int level);

				explicit operator bool() const noexcept
				{
					return created();
				}

				void bind() const noexcept
				{
					glBindTexture((GLenum)get_target(), id());
				}

				settings config();

				void config(const settings& settings_);

				class pixel_pack_settings& pixel_pack_settings()
				{
					return m_pixel_pack_settings;
				}

				const class pixel_pack_settings& pixel_pack_settings() const
				{
					return m_pixel_pack_settings;
				}

				class pixel_unpack_settings& pixel_unpack_settings()
				{
					return m_pixel_unpack_settings;
				}

				const class pixel_unpack_settings& pixel_unpack_settings() const
				{
					return m_pixel_unpack_settings;
				}

				int width() const
				{
					save_binding_state save(*this);
					GLint result;
					glGetTexLevelParameteriv((GLenum)get_target(), level(), GL_TEXTURE_WIDTH, &result);
					return (int)result;
				}

				int height() const
				{
					save_binding_state save(*this);
					GLint result;
					glGetTexLevelParameteriv((GLenum)get_target(), level(), GL_TEXTURE_HEIGHT, &result);
					return (int)result;
				}

				int depth() const
				{
					save_binding_state save(*this);
					GLint result;
					glGetTexLevelParameteriv((GLenum)get_target(), level(), GL_TEXTURE_DEPTH, &result);
					return (int)result;
				}

				size2i size() const
				{
					return{ width(), height() };
				}

				size3i size3d() const
				{
					return{ width(), height(), depth() };
				}

				texture::internal_format get_internal_format() const
				{
					save_binding_state save(*this);
					GLint result;
					glGetTexLevelParameteriv((GLenum)get_target(), level(), GL_TEXTURE_INTERNAL_FORMAT, &result);
					return (texture::internal_format)result;
				}

				texture::channel_type get_channel_type(texture::channel_name channel) const
				{
					save_binding_state save(*this);
					GLint result;
					glGetTexLevelParameteriv((GLenum)get_target(), level(), (GLenum)channel, &result);
					return (texture::channel_type)result;
				}

				int get_channel_count() const
				{
					int result = 0;

					if (get_channel_type(channel_name::red) != channel_type::none)
						result++;
					if (get_channel_type(channel_name::green) != channel_type::none)
						result++;
					if (get_channel_type(channel_name::blue) != channel_type::none)
						result++;
					if (get_channel_type(channel_name::alpha) != channel_type::none)
						result++;
					if (get_channel_type(channel_name::depth) != channel_type::none)
						result++;

					return result;
				}

				bool compressed() const
				{
					save_binding_state save(*this);
					GLint result;
					glGetTexLevelParameteriv((GLenum)get_target(), level(), GL_TEXTURE_COMPRESSED, &result);
					return result != 0;
				}

				int compressed_size() const
				{
					save_binding_state save(*this);
					GLint result;
					glGetTexLevelParameteriv((GLenum)get_target(), level(), GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &result);
					return (int)result;
				}

				texture() = default;
				texture(texture&) = delete;

				texture(texture&& texture_)
				{
					swap(texture_);
				}
				texture(target target_, GLuint id = 0)
				{
					m_target = target_;
					set_id(id);
				}

				~texture()
				{
					if (created())
						remove();
				}

				void swap(texture& texture_)
				{
					auto my_old_id = id();
					auto my_old_target = get_target();
					set_id(texture_.id());
					set_target(texture_.get_target());
					texture_.set_id(my_old_id);
					texture_.set_target(my_old_target);
				}

				texture& operator = (const texture& rhs) = delete;
				texture& operator = (texture&& rhs)
				{
					swap(rhs);
					return *this;
				}

				void copy_from(const void* src, texture::internal_format format, texture::type type, class pixel_unpack_settings pixel_settings)
				{
					save_binding_state save(*this);
					pixel_settings.apply();
					glTexSubImage2D((GLenum)get_target(), level(), 0, 0, width(), height(), (GLenum)format, (GLenum)type, src);
				}

				void copy_from(const buffer& buf, texture::internal_format format, texture::type type, class pixel_unpack_settings pixel_settings)
				{
					buffer::save_binding_state save_buffer(buffer::target::pixel_unpack, buf);
					copy_from(nullptr, format, type, pixel_settings);
				}

				void copy_from(void* dst, texture::internal_format format, texture::type type)
				{
					copy_from(dst, format, type, pixel_unpack_settings());
				}

				void copy_from(const buffer& buf, texture::internal_format format, texture::type type)
				{
					copy_from(buf, format, type, pixel_unpack_settings());
				}

				void copy_to(void* dst, texture::internal_format format, texture::type type, class pixel_pack_settings pixel_settings) const
				{
					save_binding_state save(*this);
					pixel_settings.apply();
					glGetTexImage((GLenum)get_target(), level(), (GLenum)format, (GLenum)type, dst);
				}

				void copy_to(void* dst, texture::type type, class pixel_pack_settings pixel_settings) const
				{
					copy_to(dst, get_internal_format(), type, pixel_settings);
				}

				void copy_to(const buffer& buf, texture::internal_format format, texture::type type, class pixel_pack_settings pixel_settings) const
				{
					buffer::save_binding_state save_buffer(buffer::target::pixel_pack, buf);
					copy_to(nullptr, format, type, pixel_settings);
				}

				void copy_to(const buffer& buf, texture::type type, class pixel_pack_settings pixel_settings) const
				{
					buffer::save_binding_state save_buffer(buffer::target::pixel_pack, buf);
					copy_to(nullptr, get_internal_format(), type, pixel_settings);
				}

				void copy_to(void* dst, texture::internal_format format, texture::type type) const
				{
					copy_to(dst, format, type, pixel_pack_settings());
				}

				void copy_to(void* dst, texture::type type) const
				{
					copy_to(dst, get_internal_format(), type, pixel_pack_settings()) ;
				}

				void copy_to(const buffer& buf, texture::internal_format format, texture::type type) const
				{
					copy_to(buf, format, type, pixel_pack_settings());
				}

				void copy_to(const buffer& buf, texture::type type) const
				{
					copy_to(buf, get_internal_format(), type, pixel_pack_settings());
				}

				void image_data(const size2i &size, internal_format internal_format_, format format_, type type_, const void *pixels)
				{
					save_binding_state save(*this);
					glTexImage2D((GLenum)get_target(), 0, (GLint)internal_format_, size.width(), size.height(), 0, (GLenum)format_, (GLenum)type_, pixels);
				}

				void image_data(const size2i &size, format format_, type type_, const void *pixels)
				{
					image_data(size, get_internal_format(), format_, type_, pixels);
				}

				void image_data(format format_, type type_, const void *pixels)
				{
					image_data(size(), get_internal_format(), format_, type_, pixels);
				}
			};

			class rbo
			{
				GLuint m_id = 0;

			public:
				rbo() = default;

				rbo(GLuint id)
				{
					set_id(id);
				}

				~rbo()
				{
					if (created())
						remove();
				}

				class save_binding_state
				{
					GLint m_old_value;

				public:
					save_binding_state(const rbo& new_state)
					{
						glGetIntegerv(GL_RENDERBUFFER_BINDING, &m_old_value);
						new_state.bind();
					}

					~save_binding_state()
					{
						glBindRenderbuffer(GL_RENDERBUFFER, m_old_value);
					}
				};

				void recreate()
				{
					if (created())
						remove();

					create();
				}

				void recreate(texture::format format, u32 width, u32 height)
				{
					if (created())
						remove();

					create(format, width, height);
				}

				void create()
				{
					glGenRenderbuffers(1, &m_id);
				}

				void create(texture::format format, u32 width, u32 height)
				{
					create();
					storage(format, width, height);
				}

				void bind() const
				{
					glBindRenderbuffer(GL_RENDERBUFFER, m_id);
				}

				void storage(texture::format format, u32 width, u32 height)
				{
					save_binding_state save(*this);
					glRenderbufferStorage(GL_RENDERBUFFER, (GLenum)format, width, height);
				}

				void remove()
				{
					glDeleteRenderbuffers(1, &m_id);
					m_id = 0;
				}

				uint id() const
				{
					return m_id;
				}

				void set_id(uint id)
				{
					m_id = id;
				}

				bool created() const
				{
					return m_id != 0;
				}

				explicit operator bool() const
				{
					return created();
				}
			};

			class texture::settings
			{
				texture *m_parent;

				texture::channel m_swizzle_r = texture::channel::r;
				texture::channel m_swizzle_g = texture::channel::g;
				texture::channel m_swizzle_b = texture::channel::b;
				texture::channel m_swizzle_a = texture::channel::a;

				texture::format m_format = texture::format::rgba;
				texture::internal_format m_internal_format = texture::internal_format::rgba;
				texture::type m_type = texture::type::ubyte;

				opengl::min_filter m_min_filter = opengl::min_filter::nearest;
				opengl::filter m_mag_filter = opengl::filter::nearest;

				uint m_width = 0;
				uint m_height = 0;
				int m_level = 0;

				int m_compressed_image_size = 0;

				const void* m_pixels = nullptr;
				float m_aniso = 1.f;
				texture::compare_mode m_compare_mode = texture::compare_mode::none;
				texture::compare_func m_compare_func = texture::compare_func::greater;

				texture::wrap m_wrap_s = texture::wrap::repeat;
				texture::wrap m_wrap_t = texture::wrap::repeat;
				texture::wrap m_wrap_r = texture::wrap::repeat;

				float m_max_lod = 1000.f;
				float m_min_lod = -1000.f;
				float m_lod = 0.f;
				int m_max_level = 1000;
				bool m_generate_mipmap = false;

				color4f m_border_color;

			public:
				settings(texture *parent = nullptr) : m_parent(parent)
				{
				}

				~settings()
				{
					apply();
				}

				void apply(const texture &texture) const;
				void apply();

				settings& swizzle(
					texture::channel r = texture::channel::r,
					texture::channel g = texture::channel::g,
					texture::channel b = texture::channel::b,
					texture::channel a = texture::channel::a);

				settings& format(texture::format format);
				settings& type(texture::type type);
				settings& internal_format(texture::internal_format format);
				settings& filter(min_filter min_filter, filter mag_filter);
				settings& width(uint width);
				settings& height(uint height);
				settings& size(size2i size);
				settings& level(int value);
				settings& compressed_image_size(int size);
				settings& pixels(const void* pixels);
				settings& aniso(float value);
				settings& compare_mode(texture::compare_mode value);
				settings& compare_func(texture::compare_func value);
				settings& compare(texture::compare_func func, texture::compare_mode mode);

				settings& wrap_s(texture::wrap value);
				settings& wrap_t(texture::wrap value);
				settings& wrap_r(texture::wrap value);
				settings& wrap(texture::wrap s, texture::wrap t, texture::wrap r);

				settings& max_lod(float value);
				settings& min_lod(float value);
				settings& lod(float value);
				settings& max_level(int value);
				settings& generate_mipmap(bool value);
				settings& mipmap(int level, int max_level, float lod, float min_lod, float max_lod, bool generate);

				settings& border_color(color4f value);
			};

			enum class draw_mode
			{
				points = GL_POINTS,
				lines = GL_LINES,
				line_loop = GL_LINE_LOOP,
				line_strip = GL_LINE_STRIP,
				triangles = GL_TRIANGLES,
				triangle_strip = GL_TRIANGLE_STRIP,
				triangle_fan = GL_TRIANGLE_FAN,
				quads = GL_QUADS,
				quad_strip = GL_QUAD_STRIP,
				polygone = GL_POLYGON
			};

			enum class indices_type
			{
				ubyte = GL_UNSIGNED_BYTE,
				ushort = GL_UNSIGNED_SHORT,
				uint = GL_UNSIGNED_INT
			};

			class fbo
			{
				GLuint m_id = GL_NONE;

			public:
				fbo() = default;

				fbo(GLuint id)
				{
					set_id(id);
				}

				~fbo()
				{
					if (created())
						remove();
				}

				class save_binding_state
				{
					GLint m_last_binding;

				public:
					save_binding_state(const fbo& new_binding)
					{
						glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_last_binding);
						new_binding.bind();
					}

					~save_binding_state()
					{
						glBindFramebuffer(GL_FRAMEBUFFER, m_last_binding);
					}
				};

				class attachment
				{
				public:
					enum class type
					{
						color = GL_COLOR_ATTACHMENT0,
						depth = GL_DEPTH_ATTACHMENT,
						stencil = GL_STENCIL_ATTACHMENT,
						depth_stencil = GL_DEPTH_STENCIL_ATTACHMENT
					};

				protected:
					GLuint m_id;
					fbo &m_parent;

				public:
					attachment(fbo& parent, type type)
						: m_id((int)type)
						, m_parent(parent)
					{
					}

					void set_id(uint id)
					{
						m_id = id;
					}

					uint id() const
					{
						return m_id;
					}

					void operator = (const rbo& rhs)
					{
						save_binding_state save(m_parent);
						glFramebufferRenderbuffer(GL_FRAMEBUFFER, m_id, GL_RENDERBUFFER, rhs.id());
					}

					void operator = (const texture& rhs)
					{
						save_binding_state save(m_parent);

						switch (rhs.get_target())
						{
						case texture::target::texture1D: glFramebufferTexture1D(GL_FRAMEBUFFER, m_id, GL_TEXTURE_1D, rhs.id(), rhs.level()); break;
						case texture::target::texture2D: glFramebufferTexture2D(GL_FRAMEBUFFER, m_id, GL_TEXTURE_2D, rhs.id(), rhs.level()); break;
						case texture::target::texture3D: glFramebufferTexture3D(GL_FRAMEBUFFER, m_id, GL_TEXTURE_3D, rhs.id(), rhs.level(), 0); break;
						}
					}
				};

				class indexed_attachment : public attachment
				{
				public:
					indexed_attachment(fbo& parent, type type) : attachment(parent, type)
					{
					}

					attachment operator[](int index) const
					{
						return{ m_parent, type(id() + index) };
					}

					std::vector<attachment> range(int from, int count) const
					{
						std::vector<attachment> result;

						for (int i = from; i < from + count; ++i)
							result.push_back((*this)[i]);

						return result;
					}

					using attachment::operator =;
				};

				indexed_attachment color{ *this, attachment::type::color };
				attachment depth{ *this, attachment::type::depth };
				attachment stencil{ *this, attachment::type::stencil };
				attachment depth_stencil{ *this, attachment::type::depth_stencil };

				enum class target
				{
					read_frame_buffer = GL_READ_FRAMEBUFFER,
					draw_frame_buffer = GL_DRAW_FRAMEBUFFER
				};

				void create();
				void bind() const;
				void blit(const fbo& dst, area2i src_area, area2i dst_area, buffers buffers_ = buffers::color, filter filter_ = filter::nearest) const;
				void bind_as(target target_) const;
				void remove();
				bool created() const;
				void check() const
				{
					GLenum status = glCheckFramebufferStatus(id());

					if (status != GL_FRAMEBUFFER_COMPLETE)
					{
						throw std::logic_error(fmt::format("0x%04x", status));
					}
				}

				void recreate()
				{
					if (created())
						remove();

					create();
				}

				void draw_buffer(const attachment& buffer) const
				{
					save_binding_state save(*this);
					GLenum buf = buffer.id();
					glDrawBuffers(1, &buf);
				}

				void draw_buffers(const std::initializer_list<attachment>& indexes) const
				{
					save_binding_state save(*this);
					std::vector<GLenum> ids;

					for (auto &index : indexes)
						ids.push_back(index.id());

					glDrawBuffers((GLsizei)ids.size(), ids.data());
				}

				void draw_arrays(draw_mode mode, GLsizei count, GLint first = 0) const
				{
					save_binding_state save(*this);
					__glcheck glDrawArrays((GLenum)mode, first, count);
				}

				void draw_arrays(const buffer& buffer, draw_mode mode, GLsizei count, GLint first = 0) const
				{
					buffer.bind(buffer::target::array);
					draw_arrays(mode, count, first);
				}

				void draw_arrays(const vao& buffer, draw_mode mode, GLsizei count, GLint first = 0) const
				{
					buffer.bind();
					draw_arrays(mode, count, first);
				}

				void draw_elements(draw_mode mode, GLsizei count, indices_type type, const GLvoid *indices) const
				{
					save_binding_state save(*this);
					glDrawElements((GLenum)mode, count, (GLenum)type, indices);
				}

				void draw_elements(const buffer& buffer, draw_mode mode, GLsizei count, indices_type type, const GLvoid *indices) const
				{
					buffer.bind(buffer::target::array);
					glDrawElements((GLenum)mode, count, (GLenum)type, indices);
				}

				void draw_elements(draw_mode mode, GLsizei count, indices_type type, const buffer& indices, size_t indices_buffer_offset = 0) const
				{
					indices.bind(buffer::target::element_array);
					glDrawElements((GLenum)mode, count, (GLenum)type, (GLvoid*)indices_buffer_offset);
				}

				void draw_elements(const buffer& buffer_, draw_mode mode, GLsizei count, indices_type type, const buffer& indices, size_t indices_buffer_offset = 0) const
				{
					buffer_.bind(buffer::target::array);
					draw_elements(mode, count, type, indices, indices_buffer_offset);
				}

				void draw_elements(draw_mode mode, GLsizei count, const GLubyte *indices) const
				{
					draw_elements(mode, count, indices_type::ubyte, indices);
				}

				void draw_elements(const buffer& buffer, draw_mode mode, GLsizei count, const GLubyte *indices) const
				{
					draw_elements(buffer, mode, count, indices_type::ubyte, indices);
				}

				void draw_elements(draw_mode mode, GLsizei count, const GLushort *indices) const
				{
					draw_elements(mode, count, indices_type::ushort, indices);
				}

				void draw_elements(const buffer& buffer, draw_mode mode, GLsizei count, const GLushort *indices) const
				{
					draw_elements(buffer, mode, count, indices_type::ushort, indices);
				}

				void draw_elements(draw_mode mode, GLsizei count, const GLuint *indices) const
				{
					draw_elements(mode, count, indices_type::uint, indices);
				}

				void draw_elements(const buffer& buffer, draw_mode mode, GLsizei count, const GLuint *indices) const
				{
					draw_elements(buffer, mode, count, indices_type::uint, indices);
				}

				void clear(buffers buffers_) const
				{
					save_binding_state save(*this);
					glClear((GLbitfield)buffers_);
				}

				void clear(buffers buffers_, color4f color_value, double depth_value, u8 stencil_value) const
				{
					save_binding_state save(*this);
					glClearColor(color_value.r(), color_value.g(), color_value.b(), color_value.a());
					glClearDepth(depth_value);
					glClearStencil(stencil_value);
					clear(buffers_);
				}

				void copy_from(const void* pixels, size2i size, opengl::texture::format format_, opengl::texture::type type_, class pixel_unpack_settings pixel_settings = pixel_unpack_settings()) const
				{
					save_binding_state save(*this);
					pixel_settings.apply();
					glDrawPixels(size.width(), size.height(), (GLenum)format_, (GLenum)type_, pixels);
				}

				void copy_from(const buffer& buf, size2i size, opengl::texture::format format_, opengl::texture::type type_, class pixel_unpack_settings pixel_settings = pixel_unpack_settings()) const
				{
					save_binding_state save(*this);
					buffer::save_binding_state save_buffer(buffer::target::pixel_unpack, buf);
					pixel_settings.apply();
					glDrawPixels(size.width(), size.height(), (GLenum)format_, (GLenum)type_, nullptr);
				}

				void copy_to(void* pixels, coord2i coord, opengl::texture::format format_, opengl::texture::type type_, class pixel_pack_settings pixel_settings = pixel_pack_settings()) const
				{
					save_binding_state save(*this);
					pixel_settings.apply();
					glReadPixels(coord.position.x(), coord.position.y(), coord.size.width(), coord.size.height(), (GLenum)format_, (GLenum)type_, pixels);
				}

				void copy_to(const buffer& buf, coord2i coord, opengl::texture::format format_, opengl::texture::type type_, class pixel_pack_settings pixel_settings = pixel_pack_settings()) const
				{
					save_binding_state save(*this);
					buffer::save_binding_state save_buffer(buffer::target::pixel_pack, buf);
					pixel_settings.apply();
					glReadPixels(coord.position.x(), coord.position.y(), coord.size.width(), coord.size.height(), (GLenum)format_, (GLenum)type_, nullptr);
				}

				static fbo get_binded_draw_buffer()
				{
					GLint value;
					glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &value);

					return{ (GLuint)value };
				}

				static fbo get_binded_read_buffer()
				{
					GLint value;
					glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &value);

					return{ (GLuint)value };
				}

				static fbo get_binded_buffer()
				{
					GLint value;
					glGetIntegerv(GL_FRAMEBUFFER_BINDING, &value);

					return{ (GLuint)value };
				}

				GLuint id() const
				{
					return m_id;
				}

				void set_id(GLuint id)
				{
					m_id = id;
				}

				explicit operator bool() const
				{
					return created();
				}
			};

			extern const fbo screen;

			namespace glsl
			{
				class compilation_exception : public exception
				{
				public:
					explicit compilation_exception(const std::string& what_arg)
					{
						m_what = "compilation failed: '" + what_arg + "'";
					}
				};

				class link_exception : public exception
				{
				public:
					explicit link_exception(const std::string& what_arg)
					{
						m_what = "linkage failed: '" + what_arg + "'";
					}
				};

				class validation_exception : public exception
				{
				public:
					explicit validation_exception(const std::string& what_arg)
					{
						m_what = "compilation failed: '" + what_arg + "'";
					}
				};

				class not_found_exception : public exception
				{
				public:
					explicit not_found_exception(const std::string& what_arg)
					{
						m_what = what_arg + " not found.";
					}
				};

				class shader
				{
					GLuint m_id = GL_NONE;

				public:
					enum class type
					{
						fragment = GL_FRAGMENT_SHADER,
						vertex = GL_VERTEX_SHADER,
						geometry = GL_GEOMETRY_SHADER
					};

					shader() = default;

					shader(GLuint id)
					{
						set_id(id);
					}

					shader(type type_)
					{
						create(type_);
					}

					shader(type type_, const std::string& src)
					{
						create(type_);
						source(src);
					}

					~shader()
					{
						if (created())
							remove();
					}

					void recreate(type type_)
					{
						if (created())
							remove();

						create(type_);
					}

					void create(type type_)
					{
						m_id = glCreateShader((GLenum)type_);
					}

					void source(const std::string& src) const
					{
						const char* str = src.c_str();
						const GLint length = (GLint)src.length();

						glShaderSource(m_id, 1, &str, &length);
					}

					shader& compile()
					{
						glCompileShader(m_id);

						GLint status = GL_FALSE;
						glGetShaderiv(m_id, GL_COMPILE_STATUS, &status);

						if (status == GL_FALSE)
						{
							GLint length = 0;
							glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &length);

							std::string error_msg;
							if (length)
							{
								std::unique_ptr<GLchar[]> buf(new char[length + 1]);
								glGetShaderInfoLog(m_id, length, nullptr, buf.get());
								error_msg = buf.get();
							}

							throw compilation_exception(error_msg);
						}

						return *this;
					}

					void remove()
					{
						glDeleteShader(m_id);
						m_id = 0;
					}

					uint id() const
					{
						return m_id;
					}

					void set_id(uint id)
					{
						m_id = id;
					}

					bool created() const
					{
						return m_id != 0;
					}

					explicit operator bool() const
					{
						return created();
					}
				};

				class program
				{
					GLuint m_id = 0;

				public:
					class uniform_t
					{
						program& m_program;
						GLint m_location;

					public:
						uniform_t(program& program, GLint location)
							: m_program(program)
							, m_location(location)
						{
						}

						GLint location() const
						{
							return m_location;
						}

						void operator = (int rhs) const { m_program.use(); glUniform1i(location(), rhs); }
						void operator = (float rhs) const { m_program.use(); glUniform1f(location(), rhs); }
						//void operator = (double rhs) const { m_program.use(); glUniform1d(location(), rhs); }

						void operator = (const color1i& rhs) const { m_program.use(); glUniform1i(location(), rhs.r()); }
						void operator = (const color1f& rhs) const { m_program.use(); glUniform1f(location(), rhs.r()); }
						//void operator = (const color1d& rhs) const { m_program.use(); glUniform1d(location(), rhs.r()); }
						void operator = (const color2i& rhs) const { m_program.use(); glUniform2i(location(), rhs.r(), rhs.g()); }
						void operator = (const color2f& rhs) const { m_program.use(); glUniform2f(location(), rhs.r(), rhs.g()); }
						//void operator = (const color2d& rhs) const { m_program.use(); glUniform2d(location(), rhs.r(), rhs.g()); }
						void operator = (const color3i& rhs) const { m_program.use(); glUniform3i(location(), rhs.r(), rhs.g(), rhs.b()); }
						void operator = (const color3f& rhs) const { m_program.use(); glUniform3f(location(), rhs.r(), rhs.g(), rhs.b()); }
						//void operator = (const color3d& rhs) const { m_program.use(); glUniform3d(location(), rhs.r(), rhs.g(), rhs.b()); }
						void operator = (const color4i& rhs) const { m_program.use(); glUniform4i(location(), rhs.r(), rhs.g(), rhs.b(), rhs.a()); }
						void operator = (const color4f& rhs) const { m_program.use(); glUniform4f(location(), rhs.r(), rhs.g(), rhs.b(), rhs.a()); }
						//void operator = (const color4d& rhs) const { m_program.use(); glUniform4d(location(), rhs.r(), rhs.g(), rhs.b(), rhs.a()); }

						void operator = (const matrix2f& rhs) const { m_program.use(); glUniformMatrix2fv(location(), 1, GL_FALSE, &rhs[0][0]); }
						//void operator = (const glm::dmat2& rhs) const { m_program.use(); glUniformMatrix2dv(location(), 1, GL_FALSE, glm::value_ptr(rhs)); }
						void operator = (const matrix3f& rhs) const { m_program.use(); glUniformMatrix3fv(location(), 1, GL_FALSE, &rhs[0][0]); }
						//void operator = (const glm::dmat3& rhs) const { m_program.use(); glUniformMatrix3dv(location(), 1, GL_FALSE, glm::value_ptr(rhs)); }
						void operator = (const matrix4f& rhs) const { m_program.use(); glUniformMatrix4fv(location(), 1, GL_FALSE, &rhs[0][0]); }
						//void operator = (const glm::dmat4& rhs) const { m_program.use(); glUniformMatrix4dv(location(), 1, GL_FALSE, glm::value_ptr(rhs)); }
					};

					class attrib_t
					{
						GLuint m_program;
						GLint m_location;

					public:
						attrib_t(GLuint program, GLint location)
							: m_program(program)
							, m_location(location)
						{
						}

						GLint location() const
						{
							return m_location;
						}

						void operator = (float rhs) const { glDisableVertexAttribArray(location()); glVertexAttrib1f(location(), rhs); }
						void operator = (double rhs) const { glDisableVertexAttribArray(location()); glVertexAttrib1d(location(), rhs); }

						void operator = (const color1f& rhs) const { glDisableVertexAttribArray(location()); glVertexAttrib1f(location(), rhs.r()); }
						void operator = (const color1d& rhs) const { glDisableVertexAttribArray(location()); glVertexAttrib1d(location(), rhs.r()); }
						void operator = (const color2f& rhs) const { glDisableVertexAttribArray(location()); glVertexAttrib2f(location(), rhs.r(), rhs.g()); }
						void operator = (const color2d& rhs) const { glDisableVertexAttribArray(location()); glVertexAttrib2d(location(), rhs.r(), rhs.g()); }
						void operator = (const color3f& rhs) const { glDisableVertexAttribArray(location()); glVertexAttrib3f(location(), rhs.r(), rhs.g(), rhs.b()); }
						void operator = (const color3d& rhs) const { glDisableVertexAttribArray(location()); glVertexAttrib3d(location(), rhs.r(), rhs.g(), rhs.b()); }
						void operator = (const color4f& rhs) const { glDisableVertexAttribArray(location()); glVertexAttrib4f(location(), rhs.r(), rhs.g(), rhs.b(), rhs.a()); }
						void operator = (const color4d& rhs) const { glDisableVertexAttribArray(location()); glVertexAttrib4d(location(), rhs.r(), rhs.g(), rhs.b(), rhs.a()); }

						void operator =(const buffer_pointer& pointer) const
						{
							pointer.get_vao().bind();
							glEnableVertexAttribArray(location());
							__glcheck glVertexAttribPointer(location(), pointer.size(), (GLenum)pointer.get_type(), pointer.normalize(),
								pointer.stride(), (const void*)(size_t)pointer.offset());
						}
					};

					class uniforms_t
					{
						program& m_program;
						std::unordered_map<std::string, GLint> locations;
						std::unordered_map<std::string, GLint> textures;
						int active_texture = 0;

					public:
						uniforms_t(program* program) : m_program(*program)
						{
						}

						void clear()
						{
							locations.clear();
							textures.clear();
							active_texture = 0;
						}

						GLint location(const std::string &name)
						{
							auto finded = locations.find(name);

							if (finded != locations.end())
							{
								return finded->second;
							}

							int result = glGetUniformLocation(m_program.id(), name.c_str());

							if (result < 0)
								throw not_found_exception(name);

							locations[name] = result;

							return result;
						}

						int texture(const std::string &name, int active_texture, const opengl::texture& texture)
						{
							__glcheck glActiveTexture(GL_TEXTURE0 + active_texture);
							texture.bind();
							(*this)[name] = active_texture;

							return active_texture;
						}

						int texture(const std::string &name, const opengl::texture& tex)
						{
							int atex;
							auto finded = textures.find(name);

							if (finded != textures.end())
							{
								atex = finded->second;
							}
							else
							{
								atex = active_texture++;
								textures[name] = atex;
							}

							return texture(name, atex, tex);
						}

						uniform_t operator[](GLint location)
						{
							return{ m_program, location };
						}

						uniform_t operator[](const std::string &name)
						{
							return{ m_program, location(name) };
						}

						void swap(uniforms_t& uniforms)
						{
							locations.swap(uniforms.locations);
							std::swap(active_texture, uniforms.active_texture);
						}
					} uniforms{ this };

					class attribs_t
					{
						program& m_program;
						std::unordered_map<std::string, GLint> m_locations;

					public:
						attribs_t(program* program) : m_program(*program)
						{
						}

						GLint location(const std::string &name)
						{
							auto finded = m_locations.find(name);

							if (finded != m_locations.end())
							{
								return finded->second;
							}

							int result = glGetAttribLocation(m_program.id(), name.c_str());

							if (result < 0)
								throw not_found_exception(name);

							m_locations[name] = result;

							return result;
						}

						attrib_t operator[](GLint location)
						{
							return{ m_program.id(), location };
						}

						attrib_t operator[](const std::string &name)
						{
							return{ m_program.id(), location(name) };
						}

						void swap(attribs_t& attribs)
						{
							m_locations.swap(attribs.m_locations);
						}
					} attribs{ this };

					program& recreate()
					{
						if (created())
							remove();

						return create();
					}

					program& create()
					{
						m_id = glCreateProgram();
						return *this;
					}

					void remove()
					{
						glDeleteProgram(m_id);
						m_id = 0;
						uniforms.clear();
					}

					static program get_current_program()
					{
						GLint id;
						glGetIntegerv(GL_CURRENT_PROGRAM, &id);
						return{ (GLuint)id };
					}

					void use()
					{
						glUseProgram(m_id);
					}

					void link()
					{
						glLinkProgram(m_id);

						GLint status = GL_FALSE;
						glGetProgramiv(m_id, GL_LINK_STATUS, &status);

						if (status == GL_FALSE)
						{
							GLint length = 0;
							glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &length);

							std::string error_msg;
							if (length)
							{
								std::unique_ptr<GLchar[]> buf(new char[length + 1]);
								glGetProgramInfoLog(m_id, length, nullptr, buf.get());
								error_msg = buf.get();
							}

							throw link_exception(error_msg);
						}
					}

					void validate()
					{
						glValidateProgram(m_id);

						GLint status = GL_FALSE;
						glGetProgramiv(m_id, GL_VALIDATE_STATUS, &status);

						if (status == GL_FALSE)
						{
							GLint length = 0;
							glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &length);

							std::string error_msg;
							if (length)
							{
								std::unique_ptr<GLchar[]> buf(new char[length + 1]);
								glGetProgramInfoLog(m_id, length, nullptr, buf.get());
								error_msg = buf.get();
							}

							throw validation_exception(error_msg);
						}
					}

					void make()
					{
						link();
						validate();
					}

					uint id() const
					{
						return m_id;
					}

					void set_id(uint id)
					{
						uniforms.clear();
						m_id = id;
					}

					bool created() const
					{
						return m_id != 0;
					}

					explicit operator bool() const
					{
						return created();
					}

					program& attach(const shader& shader_)
					{
						glAttachShader(m_id, shader_.id());
						return *this;
					}

					program& bind_attribute_location(const std::string& name, int index)
					{
						glBindAttribLocation(m_id, index, name.c_str());
						return *this;
					}

					program& bind_fragment_data_location(const std::string& name, int color_number)
					{
						glBindFragDataLocation(m_id, color_number, name.c_str());
						return *this;
					}

					int attribute_location(const std::string& name)
					{
						return glGetAttribLocation(m_id, name.c_str());
					}

					int uniform_location(const std::string& name)
					{
						return glGetUniformLocation(m_id, name.c_str());
					}

					program& operator += (const shader& rhs)
					{
						return attach(rhs);
					}

					program& operator += (std::initializer_list<shader> shaders)
					{
						for (auto &shader : shaders)
							*this += shader;
						return *this;
					}

					program() = default;
					program(program&) = delete;
					program(program&& program_)
					{
						swap(program_);
					}

					program(GLuint id)
					{
						set_id(id);
					}

					~program()
					{
						if (created())
							remove();
					}

					void swap(program& program_)
					{
						auto my_old_id = id();
						set_id(program_.id());
						program_.set_id(my_old_id);
						uniforms.swap(program_.uniforms);
						attribs.swap(program_.attribs);
					}

					program& operator = (const program& rhs) = delete;
					program& operator = (program&& rhs)
					{
						swap(rhs);
						return *this;
					}
				};

				class shader_view : public shader
				{
				public:
					shader_view(GLuint id) : shader(id)
					{
					}

					~shader_view()
					{
						set_id(0);
					}
				};

				class program_view : public program
				{
				public:
					program_view(GLuint id) : program(id)
					{
					}

					~program_view()
					{
						set_id(0);
					}

					using program::operator=;
				};
			}

			class texture_view : public texture
			{
			public:
				texture_view(texture::target target_, GLuint id) : texture(target_, id)
				{
				}

				~texture_view()
				{
					set_id(0);
				}
			};

			class fbo_view : public fbo
			{
			public:
				fbo_view(GLuint id) : fbo(id)
				{
				}

				~fbo_view()
				{
					set_id(0);
				}
			};

			class rbo_view : public rbo
			{
			public:
				rbo_view(GLuint id) : rbo(id)
				{
				}

				~rbo_view()
				{
					set_id(0);
				}
			};

			class buffer_view : public buffer
			{
			public:
				buffer_view(GLuint id) : buffer(id)
				{
				}

				~buffer_view()
				{
					set_id(0);
				}
			};
		}
	}
}
