#pragma once

#include <unordered_set>
#include "types.h"

namespace rfe
{
	inline namespace core
	{
		template<typename IdType, IdType BadId = ~IdType{}>
		class id_manager_t
		{
		public:
			using id_type = IdType;

		private:
			id_type m_next_id = 0;
			std::unordered_set<id_type, self_hasher> m_free_ids;
			std::mutex m_mtx;

		public:
			static constexpr id_type bad_id = BadId;

			void clear()
			{
				std::lock_guard<std::mutex> lock(m_mtx);
				m_free_ids.clear();
				m_next_id = 0;
			}

			void free_id(id_type id)
			{
				std::lock_guard<std::mutex> lock(m_mtx);

				if (m_next_id == id + 1)
				{
					m_next_id = id;
				}
				else
				{
					m_free_ids.insert(id);
				}
			}

			id_type new_id()
			{
				std::lock_guard<std::mutex> lock(m_mtx);

				if (!m_free_ids.empty())
				{
					auto iter = m_free_ids.begin();
					u32 result = *iter;
					m_free_ids.erase(iter);
					return result;
				}

				id_type result = m_next_id++;

				if (result == bad_id)
				{
					throw std::runtime_error("cannot alloc new id");
				}

				return result;
			}
		};
	}
}
