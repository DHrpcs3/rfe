#include <rfe/ui.h>
#include <rfe/ui/opengl.h>
#include <memory>

int main()
{
	std::shared_ptr<rfe::ui::window> main_window = rfe::ui::make_shared<rfe::ui::window>();

	main_window->size = { 400, 300 };
	main_window->title = "opengl draw context sample";
	main_window->make_dc<rfe::ui::opengl::draw_context>();
	main_window->clear_color = { 0.5f, 0.0f, 0.5f, 1.0f };

	rfe::ui::application::run(main_window);
}
