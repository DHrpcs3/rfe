#pragma once
#include <memory>
#include "texture.h"
#include "color.h"

namespace rfe
{
	namespace graphics
	{
		class material
		{
			class texture m_texture;
			class color m_color;

		public:
			void set_texture(const class texture& texture_)
			{
				m_texture = texture_;
			}
			void set_color(const class color &color_)
			{
				m_color = color_;
			}

			void clear_texture()
			{
				m_texture.clear();
			}
			void clear_color()
			{
				m_color.clear();
			}

			void clear()
			{
				clear_texture();
				clear_color();
			}

			bool empty() const
			{
				return m_texture.empty() && m_color.empty();
			}

			bool operator ==(std::nullptr_t) const
			{
				return empty();
			}

			bool operator !=(std::nullptr_t) const
			{
				return !empty();
			}

			explicit operator bool() const
			{
				return !empty();
			}

			const class texture& texture() const
			{
				return m_texture;
			}

			const class color& color() const
			{
				return m_color;
			}
		};
	}
}
