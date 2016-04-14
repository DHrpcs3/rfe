#pragma once
#include "core.h"
#include <vector>

namespace rfe
{
	namespace graphics
	{
		class color
		{
			std::vector<color4f> m_colors;

		public:
			color() = default;
			color(const std::vector<color4f> &colors) : m_colors(colors)
			{
			}

			color(const color4f &color)
			{
				add(color);
			}

			void add(const color4f& color)
			{
				m_colors.push_back(color);
			}

			void set(const color4f& color)
			{
				clear();
				add(color);
			}

			void clear()
			{
				m_colors.clear();
			}

			bool empty() const
			{
				return m_colors.empty();
			}

			const color4f& operator[](std::size_t index) const
			{
				return m_colors[index];
			}

			std::size_t size() const
			{
				return m_colors.size();
			}

			bool operator ==(::std::nullptr_t) const
			{
				return empty();
			}

			bool operator !=(::std::nullptr_t) const
			{
				return !empty();
			}

			explicit operator bool() const
			{
				return !empty();
			}
		};
	}
}
