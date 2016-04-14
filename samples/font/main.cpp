#include <rfe/ui.h>
#include <rfe/ui/label.h>
#include <rfe/ui/opengl.h>
#include <memory>

int main()
{
	rfe::font::init();

	std::shared_ptr<rfe::ui::window> main_window = rfe::ui::make_shared<rfe::ui::window>();

	main_window->size = { 400, 300 };
	main_window->title = "font sample";
	main_window->make_dc<rfe::ui::opengl::draw_context>();

	rfe::font::face free_sans = rfe::font::new_face("./resources/FreeSans.ttf");

	rfe::ui::label::default_font = { free_sans, 16 };

	std::shared_ptr<rfe::ui::label> label1 = rfe::ui::make_shared<rfe::ui::label>();
	label1->font = { free_sans, 11 };
	label1->text = "Hello, world!";
	label1->color = { 1.0f, 0.0f, 0.0f };

	std::shared_ptr<rfe::ui::label> label2 = rfe::ui::make_shared<rfe::ui::label>();
	//using default font
	label2->text = "Hello, world!";
	label2->color = { 1.0f, 1.0f, 0.0f };

	std::shared_ptr<rfe::ui::label> label3 = rfe::ui::make_shared<rfe::ui::label>();
	label3->font = { free_sans, 46 };
	label3->text = "Hello, world!";
	label3->color = { 1.0f, 0.0f, 0.0f };

	*main_window += label1;
	*main_window += label2;
	*main_window += label3;

	rfe::ui::application::run(main_window);
}
