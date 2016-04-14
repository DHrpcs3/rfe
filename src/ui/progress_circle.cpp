#include <rfe/ui/progress_circle.h>
#include <chrono>
using namespace std::chrono_literals;

namespace rfe
{
	namespace ui
	{
		progress_circle::progress_circle()
		{
			name = "progress circle";
		}

		animation::queue progress_circle::complete_animation(float value, bool loaded)
		{
			animation::element element{ [=](double coeff)
			{
				complete = float(m_complete_start + (value - m_complete_start) * coeff);
				update_drawable();
				return true;
			}, 1s };

			element.onstart([&]()
			{
				m_complete_start = complete;
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

		void progress_circle::set_complete(float value, bool loaded)
		{
			play(complete_animation(value, loaded), animation::insert_mode::replace);
		}

		void progress_circle::update_drawable()
		{
			if (!m_draw_context())
			{
				return;
			}

			if (complete == 0.0)
			{
				drawable.reset();
				return;
			}

			if (drawable.expired())
			{
				m_draw_context()->prepare(shared_ptr(), drawable, model().material(material()));
			}

			if (auto drawable_ = drawable.lock())
				drawable_->sector_l = (2 * rfe::pi()) * complete;
		}

		void progress_circle::dodraw()
		{
			if (auto drawable_ = drawable.lock())
			{
				drawable_->draw(clip, matrix);
			}
		}
	}
}
