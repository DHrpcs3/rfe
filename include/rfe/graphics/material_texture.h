#pragma once
#include "rfe/core/types.h"

using nullptr_t = decltype(nullptr);

namespace rfe
{
	namespace graphics
	{
		using namespace core;

		struct texture_t
		{
			u32 id;
			area2f clip;

			texture_t(u32 id = {}, area2f clip = { {0.f, 0.f},{1.f, 1.f} })
				: id(id)
				, clip(clip)
			{
			}

			bool operator == (const texture_t& rhs) const;
			bool operator != (const texture_t& rhs) const;
			bool operator == (nullptr_t) const;
			bool operator != (nullptr_t) const;

			explicit operator bool() const;
		};
	}
}
