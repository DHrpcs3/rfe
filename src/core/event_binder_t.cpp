#include <rfe/core/event_binder_t.h>

namespace rfe
{
	inline namespace core
	{
		event_binder_t::~event_binder_t()
		{
			unbind_all();
		}

		void event_binder_t::unbind_all()
		{
			for (auto &unbind_func : unbind_funcs)
			{
				unbind_func();
			}

			unbind_funcs.clear();
		}
	}
}