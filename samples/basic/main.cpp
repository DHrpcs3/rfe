#include <rfe/ui.h>

int main()
{
	auto window = rfe::ui::make_shared<rfe::ui::window>();

	window->title = "Hello, world!";

	rfe::ui::application::run(window);
}
