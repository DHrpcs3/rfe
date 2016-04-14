#pragma once

namespace rfe
{
	namespace graphics
	{
		class shader
		{
			u32 m_id;

		public:
			bool operator ==(const shader &rhs) const
			{
				return m_id == rhs.m_id;
			}
		};
	}
}
