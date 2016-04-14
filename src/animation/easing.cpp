#include <rfe/animation/easing.h>
#include <rfe/core/types.h>

namespace rfe
{
	namespace animation
	{
		namespace easing
		{
			double linear(double x)
			{
				return x;
			}

			double swing(double x)
			{
				return -cos(x * pi<double>()) / 2.f + 0.5f;
			}

			double fall(double x)
			{
				return sin(x * pi<double>() / 2.f);
			}

			double quad(double x)
			{
				return (x < 0.0001) ? 0.0 : (1.0 / (x * x));
			}

			double ease_out_bounce(double x)
			{
				static const double s = 7.5625;
				static const double p = 2.75;

				if (x < (1.0 / p))
					return s * pow(x, 2.0);

				if (x < (2.0 / p))
					return s * pow(x - 1.5 / p, 2.0) + 0.75;

				if (x < (2.5 / p))
					return s * pow(x - 2.25 / p, 2.0) + 0.9375;

				return s * pow(x - 2.625f / p, 2.0) + 0.984375;
			}
		}

		const easing_function default_easing_function = easing::swing;
	}
}
