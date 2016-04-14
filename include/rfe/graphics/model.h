#pragma once
#include "core.h"
#include "rfe/core/types.h"
#include "material.h"
#include "texture.h"
#include <vector>
#include "draw_mode.h"
#include "shader.h"

namespace rfe
{
	namespace graphics
	{
		class model
		{
		public:
			static const std::size_t dimension = 3;
			static const std::size_t vertex_count = 4;

		protected:
			std::shared_ptr<class material> m_material;
			shader m_shader;

			graphics::draw_mode m_draw_mode = graphics::draw_mode::polygone;
			std::vector<core::point2f> m_points;

		public:
			//point3f rotation{ 0.0, 0.0, 0.0 };
			point2f offset{ 0.0, 0.0 };
			point2f scale{ 1.0f, 1.0f };

			const class material* material() const
			{
				return m_material.get();
			}

			model& material(std::shared_ptr<class material> material_)
			{
				m_material = material_;
				return *this;
			}

			void add_point(core::point2f point)
			{
				m_points.push_back(point);
			}

			std::size_t points_count() const
			{
				return m_points.size();
			}

			point2f point(std::size_t index) const
			{
				return m_points[index];
			}

			model& draw_mode(const graphics::draw_mode mode);
			graphics::draw_mode draw_mode() const;
		};

		struct point : model
		{
			point(const point2f& a);
		};

		struct line : model
		{
			line(const point2f& a, const point2f& b);
		};

		struct triangle : model
		{
			triangle(const point2f& a, const point2f& b, const point2f& c);
		};

		struct quad : model
		{
			quad(const area2f& a);
		};
	}
}
