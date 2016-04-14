#pragma once
#include <rfe/ui/window.h>
#include <rfe/graphics/draw_context.h>
#include <rfe/graphics/opengl/helpers.h>
#include <fstream>
#include <string>

namespace rfe
{
	static std::string file_to_string(const std::string& path)
	{
		std::ifstream ifs(path);
		std::string result, line;

		while (std::getline(ifs, line))
		{
			result += line + "\n";
		}

		return result;
	}

	static std::vector<u8> file_to_binary(const std::string& path)
	{
		std::ifstream ifs(path, std::ios::binary | std::ios::ate);
		std::vector<u8> result((std::size_t)ifs.tellg());

		ifs.seekg(ifs.beg);
		ifs.read((char*)result.data(), result.size());
		return result;
	}

	namespace graphics
	{
		namespace opengl
		{
			namespace glsl
			{
				extern program_view color_program;
				extern program_view texture_program;
				extern program_view texture_sector_program;
				extern program_view font_program;

				namespace programs
				{
					class color : public program
					{
					public:
						struct entry_type { color4f color;  point4f position; };

						color()
						{
							__glcheck create()
								.attach(shader{ shader::type::fragment, file_to_string("shaders/gl/color.fp.glsl") }.compile())
								.attach(shader{ shader::type::vertex, file_to_string("shaders/gl/color.vp.glsl") }.compile())
								.bind_fragment_data_location("ocolor", 0)
								.make();
						}
					};

					class texture : public program
					{
					public:
						struct entry_type { point2f coord;  point4f position; };

						texture()
						{
							__glcheck create()
								.attach(shader{ shader::type::fragment, file_to_string("shaders/gl/texture.fp.glsl") }.compile())
								.attach(shader{ shader::type::vertex, file_to_string("shaders/gl/texture.vp.glsl") }.compile())
								.bind_fragment_data_location("ocolor", 0)
								.make();
						}
					};

					class texture_sector : public program
					{
					public:
						struct entry_type { point2f coord;  point4f position; };

						texture_sector()
						{
							__glcheck create()
								.attach(shader{ shader::type::fragment, file_to_string("shaders/gl/texture_sector.fp.glsl") }.compile())
								.attach(shader{ shader::type::vertex, file_to_string("shaders/gl/texture_sector.vp.glsl") }.compile())
								.bind_fragment_data_location("ocolor", 0)
								.make();
						}
					};

					class font : public program
					{
					public:
						struct entry_type { point2f coord;  point4f position; };

						font()
						{
							__glcheck create()
								.attach(shader{ shader::type::fragment, file_to_string("shaders/gl/font.fp.glsl") }.compile())
								.attach(shader{ shader::type::vertex, file_to_string("shaders/gl/font.vp.glsl") }.compile())
								.bind_fragment_data_location("ocolor", 0)
								.make();
						}
					};
				}
			}
		}
	}

	namespace ui
	{
		namespace opengl
		{
			class draw_context;
			namespace gl = graphics::opengl;

			class drawable : public graphics::drawable
			{
				gl::vao vao;
				gl::buffer gpu_buffer;
				gl::glsl::program *program;
				gl::draw_mode draw_mode;
				int draw_count;
				bool allow_sector_draw;

			public:
				drawable(const graphics::model &m);
				void draw(vector4f clip, const matrix4f& matrix_) override;
			};

			class drawable_text : public graphics::drawable_text
			{
				draw_context *dc;
				gl::vao vao;
				gl::buffer vbo;
				gl::texture texture;

				struct char_info_t
				{
					area2f tex_coord;
					coord2i coord;
					point2i advance;
				};

				std::vector<char_info_t> char_info;

			public:
				drawable_text(draw_context *parent, const font::face& face);
				graphics::text_coords_t prepare(const std::string &text) override;
				void draw(const graphics::text_coords_t &coords, const color4f &color, vector4f clip, const matrix4f& matrix_ = { 1 }) override;
			};

			class draw_context final : public graphics::draw_context, public std::enable_shared_from_this<draw_context>
			{
				window* m_parent;

				void *m_dc = nullptr;
				void *m_gl_context = nullptr;

			public:
				draw_context(window* parent);
				draw_context(const draw_context&) = delete;

				virtual ~draw_context();
				void create(const settings & cfg) override;
				void use() const override;
				void present() override;
				void close() override;
				void clear() override;

				window* parent() const
				{
					return m_parent;
				}

				std::weak_ptr<graphics::drawable> prepare(std::shared_ptr<void> parent, const graphics::model &m) override;
				std::weak_ptr<graphics::drawable_text> prepare(std::shared_ptr<void> parent, const font::face &m) override;
			};
		}
	}
}
