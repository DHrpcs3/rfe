#pragma once
#include "material.h"
#include "rfe/core/types.h"

namespace rfe
{
	struct colors4f_impl
	{
		using type = float;

		static constexpr float min()
		{
			return 0.0f;
		}

		static constexpr float max()
		{
			return 1.0f;
		}
	};

	struct colors
	{
		static const int dimension = 3;
		using impl = colors4f_impl;
		using type = typename colors4f_impl::type;

		using color_t = color<dimension, type>;

		static constexpr type from(double value)
		{
			return type(value * (impl::max() - impl::min()) + impl::min());
		}

		static color_t from(double r, double g, double b)
		{
			return{ from(r), from(g), from(b) };
		}

		static constexpr color_t black{ from(0.0, 0.0, 0.0) };
	};

	namespace graphics
	{
		class material_color : public material
		{
			color4f m_color;

		public:
			material_color() = default;

			material_color(const color4f &color) : m_color(color)
			{
				colors::black;
			}
		};
	}
}
