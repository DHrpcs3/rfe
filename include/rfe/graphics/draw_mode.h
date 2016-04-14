#pragma once

namespace rfe
{
	namespace graphics
	{
		enum class draw_mode
		{
			none = -1,
			points,
			lines,
			line_loop,
			line_strip,
			triangles,
			triangle_strip,
			triangle_fan,
			quads,
			quad_strip,
			polygone
		};
	}
}
