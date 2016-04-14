#pragma once
#include "opengl.h"
#include <exception>

namespace rfe
{
	namespace graphics
	{
		namespace opengl
		{
			class vao;

			class buffer_pointer
			{
			public:
				enum class type
				{
					s8 = GL_BYTE,
					u8 = GL_UNSIGNED_BYTE,
					s16 = GL_SHORT,
					u16 = GL_UNSIGNED_SHORT,
					s32 = GL_INT,
					u32 = GL_UNSIGNED_INT,
					f16 = GL_HALF_FLOAT,
					f32 = GL_FLOAT,
					f64 = GL_DOUBLE,
					fixed = GL_FIXED,
					s32_2_10_10_10_rev = GL_INT_2_10_10_10_REV,
					u32_2_10_10_10_rev = GL_UNSIGNED_INT_2_10_10_10_REV,
					u32_10f_11f_11f_rev = GL_UNSIGNED_INT_10F_11F_11F_REV
				};

			private:
				vao* m_vao;
				u32 m_offset;
				u32 m_stride;
				u32 m_size = 4;
				type m_type = type::f32;
				bool m_normalize = false;

			public:
				buffer_pointer(vao* vao, u32 offset = 0, u32 stride = 0)
					: m_vao(vao)
					, m_offset(offset)
					, m_stride(stride)
				{
				}

				const class opengl::vao& get_vao() const
				{
					return *m_vao;
				}

				class opengl::vao& get_vao()
				{
					return *m_vao;
				}

				buffer_pointer& offset(u32 value)
				{
					m_offset = value;
					return *this;
				}

				u32 offset() const
				{
					return m_offset;
				}

				buffer_pointer& stride(u32 value)
				{
					m_stride = value;
					return *this;
				}

				u32 stride() const
				{
					return m_stride;
				}

				buffer_pointer& size(u32 value)
				{
					m_size = value;
					return *this;
				}

				u32 size() const
				{
					return m_size;
				}

				buffer_pointer& set_type(type value)
				{
					m_type = value;
					return *this;
				}

				type get_type() const
				{
					return m_type;
				}

				buffer_pointer& normalize(bool value)
				{
					m_normalize = value;
					return *this;
				}

				bool normalize() const
				{
					return m_normalize;
				}

				buffer_pointer& operator >> (u32 value)
				{
					return stride(value);
				}

				buffer_pointer& config(type type_ = type::f32, u32 size_ = 4, bool normalize_ = false)
				{
					return set_type(type_).size(size_).normalize(normalize_);
				}
			};

			class buffer
			{
			public:
				enum class target
				{
					pixel_pack = GL_PIXEL_PACK_BUFFER,
					pixel_unpack = GL_PIXEL_UNPACK_BUFFER,
					array = GL_ARRAY_BUFFER,
					element_array = GL_ELEMENT_ARRAY_BUFFER,
					copy_read_buffer = GL_COPY_READ_BUFFER,
					copy_write_buffer = GL_COPY_WRITE_BUFFER,
				};
				enum class access
				{
					read = GL_READ_ONLY,
					write = GL_WRITE_ONLY,
					read_write = GL_READ_WRITE
				};

			private:
				GLuint m_id = GL_NONE;
				target m_target = target::array;

			public:
				buffer() = default;

				buffer(GLuint id)
				{
					set_id(id);
				}

				~buffer()
				{
					if (created())
						remove();
				}

				class save_binding_state
				{
					GLint m_last_binding;
					GLenum m_target;

				public:
					save_binding_state(target target_, const buffer& new_state) : save_binding_state(target_)
					{
						new_state.bind(target_);
					}

					save_binding_state(target target_)
					{
						GLenum pname;
						switch (target_)
						{
						case target::pixel_pack: pname = GL_PIXEL_PACK_BUFFER_BINDING; break;
						case target::pixel_unpack: pname = GL_PIXEL_UNPACK_BUFFER_BINDING; break;
						case target::array: pname = GL_ARRAY_BUFFER_BINDING; break;
						case target::element_array: pname = GL_ELEMENT_ARRAY_BUFFER_BINDING; break;
						default:
							throw std::runtime_error("unsupported buffer target");
						}

						glGetIntegerv(pname, &m_last_binding);
						m_target = (GLenum)target_;
					}


					~save_binding_state()
					{
						glBindBuffer(m_target, m_last_binding);
					}
				};

				void recreate()
				{
					if (created())
					{
						remove();
					}

					create();
				}

				void recreate(GLsizeiptr size, const void* data = nullptr)
				{
					if (created())
					{
						remove();
					}

					create(size, data);
				}

				void create()
				{
					glGenBuffers(1, &m_id);
				}

				void create(GLsizeiptr size, const void* data_ = nullptr)
				{
					create();
					data(size, data_);
				}

				void data(GLsizeiptr size, const void* data_ = nullptr)
				{
					target target_ = current_target();
					save_binding_state save(target_, *this);
					glBufferData((GLenum)target_, size, data_, GL_STREAM_COPY);
				}

				void sub_data(GLintptr offset, GLsizeiptr size, const void* data_ = nullptr)
				{
					target target_ = current_target();
					save_binding_state save(target_, *this);
					glBufferSubData((GLenum)target_, offset, size, data_);
				}

				void bind(target target_) const
				{
					glBindBuffer((GLenum)target_, m_id);
				}

				target current_target() const
				{
					return m_target;
				}

				void set_target(target target_)
				{
					m_target = target_;
				}

				void remove()
				{
					glDeleteBuffers(1, &m_id);
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

				void map(std::function<void(GLubyte*)> impl, access access_)
				{
					target target_ = current_target();
					save_binding_state save(target_, *this);

					if (GLubyte* ptr = (GLubyte*)glMapBuffer((GLenum)target_, (GLenum)access_))
					{
						impl(ptr);
						glUnmapBuffer((GLenum)target_);
					}
				}

				class mapper
				{
					buffer *m_parent;
					GLubyte *m_data;

				public:
					mapper(buffer& parent, access access_)
					{
						m_parent = &parent;
						m_data = parent.map(access_);
					}

					~mapper()
					{
						m_parent->unmap();
					}

					GLubyte* get() const
					{
						return m_data;
					}
				};

				template<typename Type = GLubyte>
				Type* map(access access_)
				{
					bind(current_target());

					return (Type*)glMapBuffer((GLenum)current_target(), (GLenum)access_);
				}

				template<typename Type = GLubyte>
				Type* map_range(access access_)
				{
					bind(current_target());

					return (Type*)glMapBuffer((GLenum)current_target(), (GLenum)access_);
				}

				void unmap()
				{
					glUnmapBuffer((GLenum)current_target());
				}
			};
		}
	}
}