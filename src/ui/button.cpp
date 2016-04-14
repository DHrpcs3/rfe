#include <rfe/ui/button.h>

namespace rfe
{
	namespace ui
	{
		button::button()
		{
			name = "button";
			background->color = {0.3f, 0.3f, 0.3f};

			oninit += [=]
			{
				(*this += background).fit(fit_type::never);
				(*this += label).center();

				return event_result::skip;
			};
		}
	}
}