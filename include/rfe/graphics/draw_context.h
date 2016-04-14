#pragma once
#include <rfe/core/types.h>
#include <rfe/core/event.h>
#include <rfe/core/thread_queue.h>
#include <chrono>
#include "model.h"
#include <array>

namespace rfe
{
	namespace font
	{
		using namespace core;

		enum load_bits
		{
			render = 1 << 2
		};

		void init();

		struct face
		{
			void *ft_face;

			face& set_pixel_sizes(int pixel_height, int pixel_width = 0);
			void load_char(char c, load_bits load_bits_ = load_bits::render) const;
			bool load_char(std::nothrow_t, char c, load_bits load_bits_ = load_bits::render) const noexcept;
		};

		struct info
		{
			face face;
			int size;

			bool operator == (const info &rhs) const
			{
				return binary_equals()(*this, rhs);
			}
		};

		face new_face(const std::string &file_path, int face_index = 0);
	}

	namespace graphics
	{
		class drawable_base
		{
		public:
			virtual ~drawable_base() = default;
		};

		class drawable : public drawable_base
		{
		public:
			matrix4f matrix{ 1.0f };
			float sector_l;
			uint texture_id = 0;

			~drawable() override = default;

			virtual void draw(vector4f clip, const matrix4f& matrix_ = { 1.0f }) = 0;
		};

		using char_coords_t = std::array<point4f, 6>;
		using text_coords_t = std::vector<char_coords_t>;

		class drawable_text : public drawable_base
		{
		public:
			virtual text_coords_t prepare(const std::string &text) = 0;

			virtual void draw(const text_coords_t &coords, const color4f &color, vector4f clip, const core::matrix<float, 4>& matrix_) = 0;
			void draw(const std::string &text, const color4f &color, vector4f clip, const core::matrix<float, 4>& matrix_)
			{
				draw(prepare(text), color, clip, matrix_);
			}
		};

		class draw_context
		{
		protected:
			using clock = std::chrono::high_resolution_clock;

			std::recursive_mutex m_mtx;

			u64 m_frames = 0;
			clock::time_point m_fps_flush_time = clock::now();
			bool m_invalidated = true;
			std::vector<std::weak_ptr<graphics::drawable_base>> m_drawables;

			u32 m_font_texture_id = 0;
			u32 m_font_vao_id = 0;
			u32 m_font_buffer_id = 0;

		public:
			thread_queue thread;

			void lock()
			{
				m_mtx.lock();
			}

			void unlock()
			{
				m_mtx.unlock();
			}

			struct settings
			{
			private:
				bool m_double_buffer = true;
				int m_stencil_size = 8;
				int m_depth_size = 24;

			public:
				settings& double_buffer(bool value)
				{
					m_double_buffer = value;
					return *this;
				}
				settings& stencil_size(int value)
				{
					m_stencil_size = value;
					return *this;
				}
				settings& depth_size(int value)
				{
					m_depth_size = value;
					return *this;
				}

				bool double_buffer() const
				{
					return m_double_buffer;
				}
				int stencil_size() const
				{
					return m_stencil_size;
				}
				int depth_size() const
				{
					return m_depth_size;
				}
			};

			virtual ~draw_context() = default;

			virtual void create(const settings &) = 0;
			virtual void use() const = 0;
			virtual void present();
			virtual void clear() = 0;
			virtual void close() = 0;

		protected:
			virtual std::weak_ptr<drawable> prepare(std::shared_ptr<void> parent, const model &m) = 0;
			virtual std::weak_ptr<drawable_text> prepare(std::shared_ptr<void> parent, const font::face &m) = 0;

			void detach(std::weak_ptr<drawable_base> drawable_);

		public:
			void prepare(std::shared_ptr<void> parent, std::weak_ptr<drawable> &drawable, const model &m);
			void prepare(std::shared_ptr<void> parent, std::weak_ptr<drawable_text> &drawable, const font::face &m);

			data_event<double> fps;
			void invalidate();
		};

	}
}
