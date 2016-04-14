
#ifdef _WIN32
#define NOMINMAX
//#undef WINAPI_FAMILY
//#define WINAPI_FAMILY WINAPI_FAMILY_DESKTOP_APP
#include <Windows.h>
//#include <GL/GL.h>
//#include "GL/wglext.h"
#pragma comment (lib, "opengl32.lib")
#else
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

struct handle_t
{
	Display* display;
	Window window;
};
#endif

#include <rfe/ui/opengl/draw_context.h>
#include <rfe/core/fmt.h>
#include <rfe/graphics/texture.h>
#include <rfe/graphics/model.h>
#include <rfe/graphics/opengl/opengl.h>
#include <rfe/graphics/opengl/helpers.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>

namespace rfe
{
	namespace graphics
	{
		namespace opengl
		{
			namespace glsl
			{
				program_view color_program{ 0 };
				program_view texture_program{ 0 };
				program_view texture_sector_program{ 0 };
				program_view font_program{ 0 };
			}
		}
	}

	namespace ui
	{
		namespace opengl
		{
			draw_context::draw_context(window* parent)
				: m_parent(parent)
			{
			}

			draw_context::~draw_context()
			{
				close();
			}

			void draw_context::use() const
			{
#ifdef _WIN32
				wglMakeCurrent((HDC)m_dc, (HGLRC)m_gl_context);
#else
				handle_t *handle = (handle_t*)std::static_pointer_cast<ui::window>(parent())->handle();
				glXMakeCurrent(handle->display, handle->window, (GLXContext)m_gl_context);
#endif
				if (auto parent_ = parent())
				{
					auto size = parent_->size();
					glViewport(0, 0, size.width(), size.height());
				}
			}

			void draw_context::create(const settings& cfg)
			{
				//std::cerr << "gl_window_draw_context::create() " << std::endl;
				close();
#ifdef _WIN32
				HWND hwnd = (HWND)m_parent->handle();

				m_dc = GetDC(hwnd);

				DWORD flags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;

				if (cfg.double_buffer())
					flags |= PFD_DOUBLEBUFFER;

				PIXELFORMATDESCRIPTOR pfd =
				{
					sizeof(PIXELFORMATDESCRIPTOR),
					1,
					flags,
					PFD_TYPE_RGBA,
					32, //Colordepth of the framebuffer.
					0, 0, 0, 0, 0, 0,
					0,
					0,
					0,
					0, 0, 0, 0,
					(BYTE)cfg.depth_size(), //Number of bits for the depthbuffer
					(BYTE)cfg.stencil_size(), //Number of bits for the stencilbuffer
					0,
					PFD_MAIN_PLANE,
					0,
					0, 0, 0
				};

				if (!SetPixelFormat((HDC)m_dc, ChoosePixelFormat((HDC)m_dc, &pfd), &pfd))
				{
					MessageBox(hwnd, "Cannot set the PixelFormat.", "ERROR", MB_ICONEXCLAMATION | MB_OK);
					return;
				}

				//SelectObject((HDC)m_dc, GetStockObject(SYSTEM_FONT));

				m_gl_context = wglCreateContext((HDC)m_dc);
#else
				handle_t *handle = (handle_t*)parent->handle();

				GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, cfg.depth_size(), GLX_STENCIL_SIZE, cfg.stencil_size(), (cfg.double_buffer() ? GLX_DOUBLEBUFFER : None), None };
				GLXContext glc = glXCreateContext(handle->display, glXChooseVisual(handle->display, 0, att), NULL, GL_TRUE);
				m_gl_context = (void*)glc;
#endif
				use();
				graphics::opengl::init();

				gl::glsl::color_program = std::move(gl::glsl::programs::color());
				gl::glsl::texture_program = std::move(gl::glsl::programs::texture());
				gl::glsl::texture_sector_program = std::move(gl::glsl::programs::texture_sector());
				gl::glsl::font_program = std::move(gl::glsl::programs::font());

				gl::texture font_texture(gl::texture::target::texture2D);
				gl::vao font_vao;
				gl::buffer font_vbo;

				font_texture.create();
				font_texture.config().filter(gl::min_filter::linear, gl::filter::linear);
				font_vao.create();
				font_vbo.create(sizeof(point4f) * 4);

				m_font_texture_id = font_texture.id();
				m_font_vao_id = font_vao.id();
				m_font_buffer_id = font_vbo.id();

				font_texture.set_id(0);
				font_vao.set_id(0);
				font_vbo.set_id(0);

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}

			void draw_context::present()
			{
#ifdef _WIN32
				SwapBuffers((HDC)m_dc);
#else
				handle_t *handle = (handle_t*)parent()->handle();
				glXSwapBuffers(handle->display, handle->window);
#endif
				graphics::draw_context::present();
			}

			void draw_context::close()
			{
				if (!m_gl_context)
				{
					return;
				}

				thread.invoke([=]
				{
					use();

					m_drawables.clear();

					if (gl::glsl::color_program)
						gl::glsl::color_program.remove();
					if (gl::glsl::texture_program)
						gl::glsl::texture_program.remove();
					if (gl::glsl::texture_sector_program)
						gl::glsl::texture_sector_program.remove();
					if (gl::glsl::font_program)
						gl::glsl::font_program.remove();

					if (m_font_texture_id)
						gl::texture_view(gl::texture::target::texture2D, m_font_texture_id).remove();

					if (m_font_vao_id)
						gl::vao(m_font_vao_id).remove();

					if (m_font_buffer_id)
						gl::buffer_view(m_font_buffer_id).remove();

#ifdef _WIN32
					wglMakeCurrent(nullptr, nullptr);

					if (m_gl_context)
					{
						wglDeleteContext((HGLRC)m_gl_context);
					}

					if (m_parent)
					{
						if (HWND handle = (HWND)m_parent->handle())
						{
							ReleaseDC(handle, (HDC)m_dc);
						}
					}
#else
					if (m_parent)
					{
						if (handle_t *handle = (handle_t*)m_parent->handle())
						{
							glXMakeCurrent(handle->display, handle->window, nullptr);
							glXDestroyContext(handle->display, (GLXContext)m_gl_context);
						}
					}
#endif
				});

				m_gl_context = nullptr;
			}

			void draw_context::clear()
			{
				__glcheck graphics::opengl::screen.clear(graphics::opengl::buffers::color_depth_stencil,
					m_parent->clear_color.get(), m_parent->clear_depth.get(), m_parent->clear_stencil.get());
			}

			std::weak_ptr<graphics::drawable> draw_context::prepare(std::shared_ptr<void> parent, const graphics::model &model)
			{
				std::shared_ptr<drawable> result = std::shared_ptr<drawable>(parent, new drawable(model));
				m_drawables.push_back(result);
				return result;
			}

			std::weak_ptr<graphics::drawable_text> draw_context::prepare(std::shared_ptr<void> parent, const font::face &face)
			{
				std::shared_ptr<drawable_text> result = std::shared_ptr<drawable_text>(parent, new drawable_text(this, face));
				m_drawables.push_back(result);
				return result;
			}

			drawable::drawable(const graphics::model &m)
			{
				__glcheck vao.create();
				__glcheck vao.bind();

				const graphics::material *material = m.material();
				if (!material || material->empty())
				{
					return;
				}

				if (auto texture = material->texture())
				{
					std::vector<gl::glsl::programs::texture::entry_type> buffer(m.points_count());

					std::cout << "WIP: draw texture" << std::endl;

					for (std::size_t i = 0; i < m.points_count(); ++i)
					{
						buffer[i] = { texture.coord(i % texture.coords_count()), (point4f)m.point(i) };
					}

					gpu_buffer.create(buffer.size() * sizeof(buffer[0]), buffer.data());

					//TODO: destroy objects
					const auto &image = texture.image();

					if (!texture_id)
					{
						__glcheck glGenTextures(1, &texture_id);
					}

					__glcheck glBindTexture(GL_TEXTURE_2D, texture_id);
					switch (image.type())
					{
					case graphics::pixels_type::rgb8:
						__glcheck glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.size().width(), image.size().height(), 0, GL_RGB, GL_UNSIGNED_BYTE, texture.image().get());
						break;

					case graphics::pixels_type::rgba8:
						__glcheck glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.size().width(), image.size().height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.image().get());
						break;

					default:
						throw;
					}

					__glcheck glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					__glcheck glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

					program = &gl::glsl::texture_program;
				}
				else if (const auto& color = material->color())
				{
					std::vector<gl::glsl::programs::color::entry_type> buffer(m.points_count());

					for (std::size_t i = 0; i < m.points_count(); ++i)
					{
						buffer[i] = { color[i % color.size()], point4f{ m.point(i) } };
					}

					__glcheck gpu_buffer.create(buffer.size() * sizeof(buffer[0]), buffer.data());

					program = &gl::glsl::color_program;
				}

				draw_mode = (gl::draw_mode)m.draw_mode();

				draw_count = (int)m.points_count();
			}

			void drawable::draw(vector4f clip, const matrix4f& matrix_)
			{
				__glcheck vao.bind();
				__glcheck vao.array_buffer = gpu_buffer;

				__glcheck program->use();
				auto MVP = matrix_ * matrix;
				__glcheck program->uniforms["MVP"] = MVP;
				__glcheck program->uniforms["clip"] = clip;

				if (texture_id == 0)
				{
					__glcheck program->attribs["position"] = (vao + offsetof(gl::glsl::programs::color::entry_type, position) >> sizeof(gl::glsl::programs::color::entry_type)).size(4);
					__glcheck program->attribs["icolor"] = (vao + offsetof(gl::glsl::programs::color::entry_type, color) >> sizeof(gl::glsl::programs::color::entry_type)).size(4);
				}
				else
				{
					if (allow_sector_draw)
					{
						program->uniforms["L"] = sector_l;
					}
					vao.bind();
					__glcheck program->uniforms.texture("tex", gl::texture_view(gl::texture::target::texture2D, texture_id));

					__glcheck program->attribs["position"] = (vao + offsetof(gl::glsl::programs::texture::entry_type, position) >> sizeof(gl::glsl::programs::texture::entry_type)).size(4);
					__glcheck program->attribs["icoord"] = (vao + offsetof(gl::glsl::programs::texture::entry_type, coord) >> sizeof(gl::glsl::programs::texture::entry_type)).size(2);
				}

				__glcheck gl::screen.draw_arrays(vao, draw_mode, draw_count);
			}

			drawable_text::drawable_text(draw_context *dc_, const font::face& face)
				: dc(dc_)
			{
				__glcheck vao.create();
				__glcheck vbo.create();
				__glcheck texture.create();

				FT_GlyphSlot g = ((FT_Face)face.ft_face)->glyph;

				uint max_width = 0;
				uint max_height = 0;

				for (int i = 0; i < 256; i++)
				{
					if (!face.load_char(std::nothrow, i))
						continue;

					max_width = std::max(max_width, g->bitmap.width);
					max_height = std::max(max_height, g->bitmap.rows);
				}

				__glcheck vao.create();
				__glcheck vbo.create();

				size2i texture_size(16 * max_width, 16 * max_height);

				__glcheck texture.create(gl::texture::target::texture2D);
				__glcheck texture.pixel_unpack_settings().aligment(1);
				__glcheck texture.config()
					.filter(gl::min_filter::linear, gl::filter::linear)
					.format(gl::texture::format::red)
					.internal_format(gl::texture::internal_format::red)
					.size({ texture_size.width(), texture_size.height() });

				char_info.resize(256);

				__glcheck texture.bind();

				for (int x = 0; x < 16; ++x)
				{
					for (int y = 0; y < 16; ++y)
					{
						int index = x * 16 + y;
						if (!face.load_char(std::nothrow, index))
							continue;

						point2i position(x * max_width, y * max_height);
						size2i size(g->bitmap.width, g->bitmap.rows);

						__glcheck glTexSubImage2D(GL_TEXTURE_2D, 0, position.x(), position.y(),
							size.width(), size.height(), GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

						char_info[index] =
						{
							area2f{ point2f(position) / texture_size, size2f(position + size) / texture_size },
							coord2i{ { g->bitmap_left, g->bitmap_top }, { (int)g->bitmap.width, (int)g->bitmap.rows } },
							point2i{ g->advance.x >> 6, g->advance.y >> 6 }
						};
					}
				}
			}

			graphics::text_coords_t drawable_text::prepare(const std::string &text)
			{
				auto window_size = dc->parent()->size();
				float sx = 2.f / window_size.width();
				float sy = 2.f / window_size.height();

				float x = 0.0;
				float y = 0.0;

				graphics::text_coords_t coords;
				coords.reserve(text.length() * 6);

				for (char c : text)
				{
					auto &info = char_info[c];

					float x2 = x + info.coord.position.x() * sx;
					float y2 = -y - info.coord.position.y() * sy;
					float w = info.coord.size.width() * sx;
					float h = info.coord.size.height() * sy;

					graphics::char_coords_t char_coords =
					{ {
						{ x2,     -y2    , info.tex_coord.p1.x(), info.tex_coord.p1.y() },
						{ x2 + w, -y2    , info.tex_coord.p2.x(), info.tex_coord.p1.y() },
						{ x2,     -y2 - h, info.tex_coord.p1.x(), info.tex_coord.p2.y() },

						{ x2 + w, -y2    , info.tex_coord.p2.x(), info.tex_coord.p1.y() },
						{ x2,     -y2 - h, info.tex_coord.p1.x(), info.tex_coord.p2.y() },
						{ x2 + w, -y2 - h, info.tex_coord.p2.x(), info.tex_coord.p2.y() },
					} };

					coords.push_back(char_coords);

					x += info.advance.x() * sx;
					y += info.advance.y() * sy;
				}

				return coords;
			}

			void drawable_text::draw(const graphics::text_coords_t &coords, const color4f &color, vector4f clip, const matrix4f& matrix_)
			{
				auto &program = gl::glsl::font_program;

				__glcheck vao.bind();
				__glcheck vbo.data(coords.size() * sizeof(graphics::char_coords_t), coords.data());
				__glcheck vao.array_buffer = vbo;

				__glcheck program.use();
				__glcheck program.uniforms["MVP"] = matrix_;
				__glcheck program.uniforms["color"] = color;
				//__glcheck program.uniforms["clip"] = clip;
				__glcheck program.uniforms.texture("tex", texture);

				__glcheck program.attribs["icoord"] = vao + 0;

				__glcheck glDrawArrays(GL_TRIANGLES, 0, GLsizei(coords.size() * (sizeof(graphics::char_coords_t) / sizeof(coords[0][0]))));
			}
		}
	}
}
