#pragma once
#include <functional>

namespace rfe
{
	namespace animation
	{
		namespace easing
		{
			double linear(double x);
			double swing(double x);
			double fall(double x);
			double quad(double x);
			double ease_out_bounce(double x);
		}

		using easing_function = std::function<double(double x)>;
		extern const easing_function default_easing_function;
	}
}
