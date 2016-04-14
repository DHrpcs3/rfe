#pragma once
#include "widget.h"
#include "ground.h"
#include "label.h"

namespace rfe
{
	namespace ui
	{
		class button : public widget
		{
		public:
			std::shared_ptr<ground> background = make_shared<ground>();
			std::shared_ptr<class label> label = make_shared<class label>();

			button();
		};
	}
}
