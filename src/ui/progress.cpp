#include <rfe/ui/progress.h>
#include <chrono>
using namespace std::chrono_literals;

namespace rfe
{
	namespace ui
	{
		progress::progress()
		{
			name = "progress";

			oninit += [=]
			{
				(*this += background).fill();
				(*this += foreground).fill_vertical().x(0).width([=] { return int(width() * m_complete); });
				return event_result::skip;
			};
		}

		animation::queue progress::complete_animation(float value, bool loaded)
		{
			animation::element element{ [=](double coeff)
			{
				m_complete = float(m_complete_start + (value - m_complete_start) * coeff);
				refresh();
				return true;
			}, 1s };

			element.onstart([&]()
			{
				m_complete_start = m_complete;
			});

			element.onstop([=]()
			{
				if (loaded)
				{
					onloaded();
				}
			});

			return animation::page{ element };
		}

		void progress::set_complete(float value, bool loaded)
		{
			play(complete_animation(value, loaded), animation::insert_mode::replace);
		}
	}
}
