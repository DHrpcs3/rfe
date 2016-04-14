#pragma once
#include "core.h"
#include <memory>

namespace rfe
{
	namespace graphics
	{
		enum class pixels_type
		{
			rgba8,
			rgb8,
			//TODO
		};

		class image
		{
			std::unique_ptr<char[]> m_pixels;
			size2i m_size;
			pixels_type m_type;

		public:
			image() = default;
			image(std::unique_ptr<char[]> pixels, size2i size, pixels_type type)
				: m_pixels(std::move(pixels))
				, m_size(size)
				, m_type(type)
			{
			}

			image(const image&) = delete;
			image(image&&) = default;
			image& operator=(const image&) = delete;
			image& operator=(image&&) = default;

			void clear()
			{
				m_pixels.reset();
				m_size = {};
			}

			size2i size() const
			{
				return m_size;
			}

			pixels_type type() const
			{
				return m_type;
			}

			bool empty() const
			{
				return m_pixels == nullptr;
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

			void* get() const
			{
				return m_pixels.get();
			}

			//TODO:
			//image copy() const {}
		};
	}
}
