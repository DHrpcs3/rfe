#pragma once
#include <rfe/graphics/image.h>
#include <istream>

namespace rfe
{
	namespace loaders
	{
		namespace png
		{
			std::shared_ptr<graphics::image> load(std::istream& stream);
			std::shared_ptr<graphics::image> load(const std::string &path);
		}
	}
}
