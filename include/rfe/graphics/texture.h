#pragma once
#include "core.h"
#include "image.h"
#include <vector>

namespace rfe
{
	namespace graphics
	{
		class texture
		{
			std::shared_ptr<class image> m_image;
			std::vector<point2f> m_coords;

		public:
			texture() = default;

			texture(std::shared_ptr<class image> image_, const std::vector<point2f>& coords)
				: m_image(image_)
				, m_coords(coords)
			{
			}

			texture(std::shared_ptr<class image> image_, area2f area = coord2f{ { 0.0f, 0.0f },{ 1.0f, 1.0f } })
				: texture(image_, area_to_coords(area))
			{
			}

			void set_image(std::shared_ptr<class image> img)
			{
				m_image = img;
			}

			void set_coords(const std::vector<point2f>& coords)
			{
				m_coords = coords;
			}

			std::size_t coords_count() const
			{
				return m_coords.size();
			}

			const point2f& coord(std::size_t index) const
			{
				return m_coords[index];
			}

			const class image& image() const
			{
				return *m_image.get();
			}

			void set_coords(const area2f& area)
			{
				set_coords(area_to_coords(area));
			}

			void add_coord(const point2f& coord)
			{
				m_coords.push_back(coord);
			}

			void clear()
			{
				m_image.reset();
				m_coords.clear();
			}

			bool empty() const
			{
				return m_image == nullptr;
			}

			bool operator ==(std::nullptr_t) const
			{
				return empty();
			}

			bool operator !=(std::nullptr_t) const
			{
				return !empty();
			}

			bool operator ==(const texture& rhs) const
			{
				return m_image == rhs.m_image && m_coords == rhs.m_coords;
			}

			bool operator !=(const texture& rhs) const
			{
				return m_image != rhs.m_image || m_coords != rhs.m_coords;
			}

			explicit operator bool() const
			{
				return !empty();
			}

			static std::vector<point2f> area_to_coords(area2f area)
			{
				return
				{
					{ area.p1.x(), area.p1.y() },
					{ area.p2.x(), area.p1.y() },
					{ area.p2.x(), area.p2.y() },
					{ area.p1.x(), area.p2.y() }
				};
			}
		};
	}
}
