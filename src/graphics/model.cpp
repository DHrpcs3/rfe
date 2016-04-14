#include <rfe/graphics/model.h>
#include <exception>
#include <stdexcept>

namespace rfe
{
	namespace graphics
	{
		model& model::draw_mode(graphics::draw_mode mode)
		{
			m_draw_mode = mode;
			return *this;
		}

		graphics::draw_mode model::draw_mode() const
		{
			return m_draw_mode;
		}

		point::point(const point2f& a)
		{
			model::draw_mode(graphics::draw_mode::points);
			model::add_point(a);
		}

		line::line(const point2f& a, const point2f& b)
		{
			model::draw_mode(graphics::draw_mode::lines);
			model::add_point(a);
			model::add_point(b);
		}

		triangle::triangle(const point2f& a, const point2f& b, const point2f& c)
		{
			model::draw_mode(graphics::draw_mode::triangles);
			model::add_point(a);
			model::add_point(b);
			model::add_point(c);
		}

		quad::quad(const area2f& a)
		{
			model::draw_mode(graphics::draw_mode::quads);
			model::add_point({ a.p1.x(), a.p1.y() });
			model::add_point({ a.p2.x(), a.p1.y() });
			model::add_point({ a.p2.x(), a.p2.y() });
			model::add_point({ a.p1.x(), a.p2.y() });
		}
	}
}
