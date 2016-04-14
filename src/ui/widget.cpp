#include <rfe/ui/widget.h>
#include <rfe/core/thread_queue.h>
#include <sstream>

namespace rfe
{
	namespace ui
	{
		std::shared_ptr<id_manager_t<u32>> id_manager()
		{
			static auto instance = std::make_shared<id_manager_t<u32>>();

			return instance;
		}

		std::shared_ptr<const widget> widget::shared_ptr() const
		{
			return shared_from_this();
		}

		std::shared_ptr<widget> widget::shared_ptr()
		{
			return shared_from_this();
		}

		void widget::invalidate()
		{
			m_invalidate = true;

			if (m_parent)
			{
				m_parent->invalidate();
			}
		}

		matrix4f widget::calc_matrix_for(coord2f coords, size2f global_size)
		{
			rfe::point2f scale = coords.size / global_size;
			rfe::point2f translate = coords.position * 2 / global_size - (rfe::point2f{ 1.f, 1.f } - scale);

			return mtx::scale_offset({ scale.x(), scale.y(), 1.f }, { translate.x(), -translate.y(), 0 });
		}

		void widget::calc_matrix()
		{
			if (!visible_test())
			{
				matrix = { 0.f };
				clip = {};

				return;
			}

			auto widget_size = size();

			auto parent_ = parent();
			size2i parent_size;
			
			if (parent_)
			{
				parent_size = parent_->size();
			}
			else
			{
				parent_size = widget_size;
			}

			std::shared_ptr<const widget> top_widget;
			point2i absolute_positon = local_to_absolute_point({}, &top_widget);
			auto window_size = top_widget->size();

			if (window_size.width() <= 0 || window_size.height() <= 0)
			{
				matrix = { 0.f };
				clip = {};
				return;
			}

			point2f clip_point_top, clip_point_bottom;

			if (auto parent_ = parent())
			{
				clip_point_top = (point2f)(absolute_positon - position());
				clip_point_bottom = clip_point_top + (size2f)parent_size;
			}
			else
			{
				clip_point_top = {};
				clip_point_bottom = (size2f)widget_size;
			}

			clip_point_top = (clip_point_top * 2.f) / window_size - 1.f;
			clip_point_bottom = (clip_point_bottom * 2.f) / window_size - 1.f;

			matrix = calc_matrix_for({ (point2f)absolute_positon, (size2f)widget_size }, (size2f)window_size);

			clip = { clip_point_top.x(), clip_point_top.y(), clip_point_bottom.x(), clip_point_bottom.y() };
		}

		widget::widget()
		{
			//bind default listeners

			onclose += [this] { return doclose(); };
			ongot_focus += [this] { return dogot_focus(); };
			onlose_focus += [this] { return dolose_focus(); };

			auto call_positional_event = [this](point2i point, std::function<event_result(widget&, point2i)> bind)
			{
				if (!visible())
				{
					return event_result::skip;
				}

				std::lock_guard<shared_read_mutex_read> lock(m_childs_mtx.read);

				for (auto it = m_childs.rbegin(); it != m_childs.rend(); ++it)
				{
					auto &child = *it;

					if (child->visible())
					{
						point2i child_position = child->position();

						if (point >= child_position && point < child_position + child->size())
						{
							if (bind(*child, point - child_position) == event_result::handled)
							{
								return event_result::handled;
							}
						}
					}
				}

				return event_result::skip;
			};

			event_binder(events::mouse::motion) += [=](point2i point)
			{
				if (m_touched)
				{
					auto local_point = absolute_to_local_point(point);

					if (!m_motion_started)
					{
						point2i distance = (local_point - m_touch_point).abs();

						if (distance.x() > 5 || distance.y() > 5)
						{
							m_motion_started = true;
						}
						else
						{
							return;
						}
					}

					onmotion(m_touch_point, local_point);
				}
			};

			onmotion_end += [=](point2i point)
			{
				m_motion_started = false;
				m_touched = false;

				return call_positional_event(point, [](widget& widget_, point2i point_)
				{
					return widget_.onmotion_end(synchronized, point_);
				});
			};

			ontry_click += [=](point2i point)
			{
				m_touched = false;

				return onclick(synchronized, point);
			};

			onclick += [=](point2i point)
			{
				return call_positional_event(point, [](widget& widget_, point2i point_)
				{
					return widget_.ontry_click(synchronized, point_);
				});
			};

			ontouch += [=](point2i point)
			{
				m_touch_point = point;
				m_touched = true;

				return call_positional_event(point, [](widget& widget_, point2i point_)
				{
					return widget_.ontouch(synchronized, point_);
				});
			};

			size.onchanged += [this](ignore, ignore)
			{
				refresh();
			};

			position.onchanged += [this](ignore, ignore)
			{
				refresh();
			};

			shown.onchanged += [this](ignore, ignore)
			{
				refresh();
			};

			rotation.onchanged += [this](ignore, ignore)
			{
				refresh();
			};

			oninit += [this]
			{
				show();
				return event_result::handled;
			};
		}

		void widget::init()
		{
			oninit();
		}

		widget::~widget()
		{
			close();

			parent_event_binder.unbind_all();
			event_binder.unbind_all();

			m_id_manager->free_id(m_id);
			m_id = id_manager_t<decltype(m_id)>::bad_id;
		}

		void widget::update_animation()
		{
			animation::animable::update();
		}

		void widget::update_childs()
		{
			std::lock_guard<shared_read_mutex_read> lock(m_childs_mtx.read);
			for (auto &child : m_childs)
			{
				child->update();
			}
		}

		void widget::update_sizers()
		{
		}

		void widget::update()
		{
			if (m_const_sizer_invalidated)
			{
				m_const_sizer_invalidated = false;
				recalc_sizers(recalc_sizers_type::set_constants);
			}

			update_childs();
			update_animation();

			if (m_relative_sizer_invalidated)
			{
				m_relative_sizer_invalidated = false;
				recalc_sizers(recalc_sizers_type::set_relatives);
			}

			if (m_parent == nullptr)
			{
				//update_sizers();

				thread_queue::main_thread().process_queue();

				while (!events_thread().empty())
				{
					thread_queue::main_thread().process_queue();
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}

			queue.process_queue();

			calc_matrix();
			onrefresh();
		}

		void widget::set_dc(std::shared_ptr<graphics::draw_context> dc)
		{
			m_draw_context = dc;

			std::lock_guard<shared_read_mutex_read> lock(m_childs_mtx.read);
			for (auto &child : m_childs)
			{
				child->set_dc(dc);
			}
		}

		void widget::set_top_widget(std::shared_ptr<widget> widget_)
		{
			m_top_widget = widget_;
			auto top_widget_ = top_widget();

			std::lock_guard<shared_read_mutex_read> lock(m_childs_mtx.read);
			for (auto &child : m_childs)
			{
				child->set_top_widget(top_widget_);
			}
		}

		void widget::set_parent(std::shared_ptr<widget> parent)
		{
			parent_event_binder.unbind_all();

			m_parent = parent;
			std::shared_ptr<graphics::draw_context> parent_dc = parent ? parent->dc() : nullptr;
			set_dc(parent_dc);

			std::string new_full_name = name();

			for (auto parent_ = parent; parent_; parent_ = parent_->parent())
			{
				new_full_name = parent_->name() + " > " + new_full_name;
			}

			full_name = new_full_name;

			set_top_widget(parent ? parent->top_widget() : nullptr);

			if (parent)
			{
				parent_event_binder(parent->size.onchanged) += [=](ignore, ignore)
				{
					refresh();
				};

				parent_event_binder(parent->position.onchanged) += [=](ignore, ignore)
				{
					refresh();
				};
			}
		}

		std::shared_ptr<widget> widget::top_widget()
		{
			return m_top_widget ? m_top_widget : shared_ptr();
		}

		sizer_flags& widget::append_child(std::shared_ptr<widget> child)
		{
			child->set_parent(shared_ptr());

			{
				std::lock_guard<shared_read_mutex_write> lock(m_childs_mtx.write);
				m_childs.insert(m_childs.end(), child);
			}

			return child->sizer_flags();
		}

		void widget::remove_child(std::shared_ptr<widget> child)
		{
			child->set_parent(nullptr);

			std::lock_guard<shared_read_mutex_write> lock(m_childs_mtx.write);

			for (auto it = m_childs.begin(); it != m_childs.end();)
			{
				if (*it == child)
				{
					it = m_childs.erase(it);
				}
				else
					++it;
			}
		}

		event_result widget::doclose()
		{
			m_alive = false;

			if (m_parent)
			{
				m_parent->remove_child(shared_ptr());
				set_parent(nullptr);
			}

			std::lock_guard<shared_read_mutex_write> lock(m_childs_mtx.write);

			for (auto &child : m_childs)
			{
				child->set_parent(nullptr);
				child->close();
			}

			m_childs.clear();

			return event_result::handled;
		}

		event_result widget::dogot_focus()
		{
			m_focused = true;
			return event_result::handled;
		}

		event_result widget::dolose_focus()
		{
			m_focused = false;
			return event_result::handled;
		}

		void widget::dodraw()
		{
			auto size_ = size();

			if (size_.width() < 1 || size_.height() < 1)
			{
				return;
			}

			auto draw_context = m_draw_context();

			if (draw_context)
			{
				//m_draw_context->clear();
				ondraw(&*draw_context);
			}
		}

		void widget::show(bool show)
		{
			if (!alive())
			{
				return;
			}

			if (show)
			{
				shown = true;
			}
			else
			{
				hide();
			}
		}

		void widget::hide()
		{
			if (alive())
			{
				shown = false;
			}
		}

		void widget::close()
		{
			if (alive())
				onclose();
		}

		void widget::rotate(float angle)
		{
			rotation += angle;
		}

		void widget::focus()
		{
		}

		void widget::move(point2i position_)
		{
			if (alive() && position_ != position.get())
				position = position_;
		}

		void widget::resize(size2i size_)
		{
			if (alive() && size_ != size())
				size = size_;
		}

		void widget::width(int value)
		{
			size = { value, size().height() };
		}

		void widget::height(int value)
		{
			size = { size().width(), value };
		}

		int widget::width() const
		{
			return size().width();
		}

		int widget::height() const
		{
			return size().height();
		}

		void widget::x(int value)
		{
			position = { value, position().y() };
		}

		void widget::y(int value)
		{
			position = { position().x(), value };
		}

		int widget::x() const
		{
			return position.get().x();
		}

		int widget::y() const
		{
			return position.get().y();
		}

		bool widget::visible() const
		{
			return m_visible && alive() && shown();
		}

		bool widget::hidden() const
		{
			return !shown();
		}

		bool widget::visible_test()
		{
			auto test = [this]
			{
				if (!shown())
				{
					return false;
				}

				if (auto parent_ = parent())
				{
					point2i top_pos = position();
					point2i down_pos = top_pos + size();

					if (down_pos.x() <= 0 || down_pos.y() <= 0)
					{
						return false;
					}

					size2i parent_size = parent_->size();

					if (parent_size.width() == 0 || parent_size.height() == 0)
					{
						return false;
					}

					if (top_pos.x() >= parent_size.width() || top_pos.y() >= parent_size.height())
					{
						return false;
					}
				}

				return true;
			};

			bool test_result = test();

			m_visible = test_result;

			return test_result;
		}

		coord2i widget::coord()
		{
			return{ position.get(), size.get() };
		}

		point2i widget::absolute_to_local_point(point2i point, std::shared_ptr<const widget>* top_widget) const
		{
			point2i local_positon = point;

			if (top_widget)
			{
				*top_widget = shared_ptr();
			}

			if (auto parent_ = parent())
			{
				local_positon -= position();
				while (parent_->parent())
				{
					local_positon -= parent_->position();
					parent_ = parent_->parent();
				}

				if (top_widget)
				{
					*top_widget = parent_;
				}
			}

			return local_positon;
		}

		point2i widget::local_to_absolute_point(point2i point, std::shared_ptr<const widget>* top_widget) const
		{
			point2i absolute_positon = point;

			if (top_widget)
			{
				*top_widget = shared_ptr();
			}

			if (auto parent_ = parent())
			{
				absolute_positon += position();
				while (parent_->parent())
				{
					absolute_positon += parent_->position();
					parent_ = parent_->parent();
				}

				if (top_widget)
				{
					*top_widget = parent_;
				}
			}

			return absolute_positon;
		}

		void widget::refresh(bool is_const)
		{
			if (is_const)
			{
				m_const_sizer_invalidated = true;
			}
			else
			{
				m_relative_sizer_invalidated = true;
			}
			//recalc_sizers(recalc_sizers_type::set_relatives);
		}

		void widget::draw(bool clear_and_flip)
		{
			if (!visible())
			{
				return;
			}

			auto dc = m_draw_context();

			if (!dc)
			{
				return;
			}

			dc->thread.invoke([=]
			{
				std::unique_lock<widget> lock{ *this };

				if (clear_and_flip && m_parent == nullptr)
				{
					dc->use();
					dc->clear();
				}

				dodraw();

				{
					std::lock_guard<shared_read_mutex_read> lock(m_childs_mtx.read);
					for (auto &child : m_childs)
					{
						child->draw();
					}
				}

				if (clear_and_flip && m_parent == nullptr)
				{
					dc->present();

					if (0)
					{
						using clock = std::chrono::high_resolution_clock;

						static auto s_timer = clock::now();
						static const double limit = 40.;

						auto sleep = std::chrono::milliseconds(s64(1000.0 / limit))
							- std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - s_timer);

						std::this_thread::sleep_for(sleep);

						s_timer = clock::now();
					}
				}
			});
		}

		void widget::recalc_sizers(recalc_sizers_type recalc_type)
		{
			size2i internal_size{};
			int widget_axe = (int)orientation();
			int position[2] = { 0, get_size(widget_axe) };

			std::lock_guard<shared_read_mutex_read> lock(m_childs_mtx.read);
			for (auto &child : m_childs)
			{
				for (byte axe = 0; axe < 2; ++axe)
				{
					switch (sizer_elem_type type = child->m_sizer_flags.m_pos[axe].type)
					{
					case sizer_elem_type::ignore:
						break;

					case sizer_elem_type::counter:
					case sizer_elem_type::expand:
						if (!child->m_sizer_flags.m_pull_back)
						{
							if (axe == widget_axe)
							{
								child->set_position(axe, position[0] + child->get_front_border(axe));
							}
							else
							{
								child->set_position(axe, child->get_front_border(axe));
							}
						}
						else if (type == sizer_elem_type::expand)
						{
							child->set_position(axe, child->get_front_border(axe));
						}
						break;

					case sizer_elem_type::absolute:
						if (recalc_type == recalc_sizers_type::set_constants)
						{
							child->set_position(axe, child->m_sizer_flags.m_pos[axe].value);
						}
						break;

					case sizer_elem_type::relative:
						child->set_position(axe, child->m_sizer_flags.m_pos[axe].rel_to());
						break;
					}

					switch (child->m_sizer_flags.m_size[axe].type)
					{
					case sizer_elem_type::automatical:
					case sizer_elem_type::ignore:
					case sizer_elem_type::counter:
						break;

					case sizer_elem_type::expand:
						if (recalc_type == recalc_sizers_type::set_relatives)
						{
							if (!child->m_sizer_flags.m_pull_back)
							{
								child->set_size(axe, get_size(axe) - child->get_position(axe) - child->get_back_border(axe));
							}
							else
							{
								child->set_size(axe, (widget_axe == axe ? position[1] : get_size(axe)) - child->get_back_border(axe) - child->get_position(axe));
							}
						}
						break;

					case sizer_elem_type::absolute:
						if (recalc_type == recalc_sizers_type::set_constants)
						{
							child->set_size(axe, child->m_sizer_flags.m_size[axe].value);
						}
						break;

					case sizer_elem_type::relative:
						if (recalc_type == recalc_sizers_type::set_relatives)
						{
							child->set_size(axe, child->m_sizer_flags.m_size[axe].rel_to());
						}
						break;
					}

					if (recalc_type == recalc_sizers_type::set_relatives)
					{
						if (child->m_sizer_flags.m_pull_back &&
							(child->m_sizer_flags.m_pos[axe].type == sizer_elem_type::counter || child->m_sizer_flags.m_pos[axe].type == sizer_elem_type::expand))
						{
							child->set_position(axe, (widget_axe == axe ? position[1] : get_size(axe)) - child->get_front_border(axe) - child->get_size(axe));
						}
					}

					switch (child->m_sizer_flags.m_fit)
					{
					case fit_type::never:
						break;

					case fit_type::if_shown:
					case fit_type::always:
						if (child->m_sizer_flags.m_fit == fit_type::always || child->shown())
						{
							int down = child->get_position(axe) + child->get_size(axe) + child->get_back_border(axe);

							if (axe == widget_axe)
							{
								if (!child->m_sizer_flags.m_pull_back)
								{
									position[0] = std::max<int>(position[0], down);
								}
								else
								{
									position[1] = std::min<int>(position[1], child->get_position(axe));
								}
							}

							switch (axe)
							{
							case 0: internal_size.width(std::max<int>(internal_size.width(), down)); break;
							case 1: internal_size.height(std::max<int>(internal_size.height(), down)); break;
							}
						}
						break;
					}
				}
			}

			if (recalc_type == recalc_sizers_type::set_relatives)
			{
				bool auto_width = m_sizer_flags.m_size[0].type == sizer_elem_type::automatical;
				bool auto_height = m_sizer_flags.m_size[1].type == sizer_elem_type::automatical;

				if (auto_width || auto_height)
				{
					auto widget_size = size();
					int width_ = auto_width ? internal_size.width() : widget_size.width();
					int height_ = auto_height ? internal_size.height() : widget_size.height();

					size = { width_, height_ };
				}
			}
		}
	}
}

#if 0
size2i widget::recalc_sizes()
{
	size2i internal_size{};
	int widget_axe = (int)orientation();
	int position[2] = { 0, get_size(widget_axe) };

	for (auto &child : clone(m_childs, m_childs_mtx))
	{
		for (byte axe = 0; axe < 2; ++axe)
		{
			switch (child->m_sizer_flags.m_pos[axe].type)
			{
			case sizer_elem_type::ignore:
				break;

			case sizer_elem_type::counter:
				if (!child->m_sizer_flags.m_pull_back)
				{
					if (axe == widget_axe)
					{
						child->set_position(axe, position[0] + child->get_front_border(axe));
					}
					else
					{
						child->set_position(axe, child->get_front_border(axe));
					}
				}
				else
				{
					if (child->m_sizer_flags.m_expand[axe])
					{
						child->set_position(axe, child->get_front_border(axe));
					}
				}
				break;

			case sizer_elem_type::absolute:
				child->set_position(axe, child->m_sizer_flags.m_pos[axe].value);
				break;

			case sizer_elem_type::relative:
				child->set_position(axe, child->m_sizer_flags.m_pos[axe].rel_to());
				break;
			}

			switch (child->m_sizer_flags.m_size[axe].type)
			{
			case sizer_elem_type::ignore:
				break;

			case sizer_elem_type::counter:
				if (child->m_sizer_flags.m_expand[axe])
				{
					if (!child->m_sizer_flags.m_pull_back)
					{
						child->set_size(axe, get_size(axe) - child->get_position(axe) - child->get_back_border(axe));
					}
					else
					{
						child->set_size(axe, (widget_axe == axe ? position[1] : get_size(axe)) - child->get_back_border(axe) - child->get_position(axe));
					}
				}
				break;

			case sizer_elem_type::absolute:
				child->set_size(axe, child->m_sizer_flags.m_size[axe].value);
				break;

			case sizer_elem_type::relative:
				child->set_size(axe, child->m_sizer_flags.m_size[axe].rel_to());
				break;
			}

			if (child->m_sizer_flags.m_pull_back && child->m_sizer_flags.m_pos[axe].type == sizer_elem_type::counter)
			{
				child->set_position(axe, (widget_axe == axe ? position[1] : get_size(axe)) - child->get_front_border(axe) - child->get_size(axe));
			}

			switch (child->m_sizer_flags.m_fit)
			{
			case fit_type::never:
				break;

			case fit_type::if_shown:
			case fit_type::always:
				if (child->m_sizer_flags.m_fit == fit_type::always || child->shown())
				{
					int down = child->get_position(axe) + child->get_size(axe) + child->get_back_border(axe);

					if (axe == widget_axe)
					{
						if (!child->m_sizer_flags.m_pull_back)
						{
							position[0] = std::max<int>(position[0], down);
						}
						else
						{
							position[1] = std::min<int>(position[1], child->get_position(axe));
						}
					}

					switch (axe)
					{
					case 0: internal_size.width(std::max<int>(internal_size.width(), down)); break;
					case 1: internal_size.height(std::max<int>(internal_size.height(), down)); break;
					}
				}
				break;
			}
		}
	}

	return internal_size;
}
#endif
