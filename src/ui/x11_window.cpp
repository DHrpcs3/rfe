//TODO
#ifndef _WIN32
#include <X11/Xlib.h>
#include <iostream>
#include <rfe/core/fmt.h>
#include <rfe/core/events.h>
#include <rfe/ui/window.h>
#include <stdexcept>

namespace rfe
{
	namespace ui
	{
		struct handle_t
		{
			Display *display;
			Window window;
		};

		window::window()
		{
			clear_color = { 0.4f, 0.4f, 0.4f, 1.0f };
			clear_depth = 1.f;
			clear_stencil = 0xff;
			shown = false;

			title.get_function([&]() -> std::string
			{
				if (!m_handle)
					return{};

				handle_t *handle = (handle_t*)m_handle;
				char* result;
				XFetchName(handle->display, handle->window, &result);
				return result;
			});

			title.invoke_event_function([&](std::string value)
			{
				if (!m_handle)
					return;

				handle_t *handle = (handle_t*)m_handle;
				XStoreName(handle->display, handle->window, value.c_str());
			});

			position.get_function([&]() ->point2i
			{
				if (!m_handle)
					return{};

				handle_t *handle = (handle_t*)m_handle;

				XWindowAttributes attr = {};
				XGetWindowAttributes(handle->display, handle->window, &attr);

				return{ attr.x, attr.y };
			});

			position.invoke_event_function([&](point2i value)
			{
				if (!m_handle)
					return;

				handle_t *handle = (handle_t*)m_handle;
				XMoveWindow(handle->display, handle->window, value.x, value.y);
			});

			//			size.get_function([&]() -> size2i
			//			{
			//				if (!m_handle)
			//					return{};

			//				handle_t *handle = (handle_t*)m_handle;


			//				XWindowAttributes attr = {};

			//				XGetGeometry()
			//				XGetWindowAttributes(handle->display, handle->window, &attr);

			//				return size2i{ attr.width, attr.height };
			//			});

			size.invoke_event_function([&](size2i value)
			{
				if (!m_handle)
					return;

				handle_t *handle = (handle_t*)m_handle;
				XResizeWindow(handle->display, handle->window, value.width, value.height);
			});

			//shown.get_function([&]() -> bool
			//{
			//	if (!m_handle)
			//		return{};
			//
			//	handle_t *handle = (handle_t*)m_handle;
			//
			//	return IsWindowVisible((HWND)m_handle) != FALSE;
			//});

//			shown.invoke_event_function([&](bool value)
//			{
//				if (!m_handle)
//					return;

//				handle_t *handle = (handle_t*)m_handle;

//				if (value)
//					XMapWindow(handle->display, handle->window);
//				else
//					XUnmapWindow(handle->display, handle->window);

//				XFlush(handle->display);
//			});

			create();
		}

		static constexpr u32 g_window_events_masks =
			KeyPressMask | KeyReleaseMask |
			PropertyChangeMask | ResizeRedirectMask |
			ButtonPressMask | ButtonReleaseMask |
			Button1MotionMask | PointerMotionMask | PointerMotionHintMask |
			FocusChangeMask | VisibilityChangeMask | StructureNotifyMask | SubstructureRedirectMask;

		void window::create()
		{
			handle_t *handle = new handle_t();
			m_handle = handle;
			if (!(handle->display = XOpenDisplay(nullptr)))
			{
				throw std::runtime_error("display not found.");
			}

			Window root_window = RootWindow(handle->display, DefaultScreen(handle->display));

			if (!(handle->window = XCreateWindow(handle->display, root_window, 0, 0, 1024, 700,
				0, CopyFromParent, CopyFromParent,
				CopyFromParent, 0, nullptr)))
			{
				throw std::runtime_error("window creation failed.");
			}

			size = { 1024, 700 };
			position = {};

			XSelectInput(handle->display, handle->window, g_window_events_masks);
			XMapWindow(handle->display, handle->window);
			XFlush(handle->display);
			Atom WM_DELETE_WINDOW = XInternAtom(handle->display, "WM_DELETE_WINDOW", False);
			XSetWMProtocols(handle->display, handle->window, &WM_DELETE_WINDOW, 1);
		}

		void window::remove_dc()
		{
			std::lock_guard<std::mutex> lock(m_mtx);
			m_draw_context = nullptr;
		}

		int IfEventCheck(Display*, XEvent*, XPointer)
		{
			return True;
		}

		void window::update_events()
		{
			if (m_handle)
			{
				handle_t *handle = (handle_t*)m_handle;
				XEvent event;


				if (XCheckIfEvent(handle->display, &event, IfEventCheck, nullptr))//(XCheckMaskEvent(handle->display, g_window_events_masks, &event))
				{
					switch (event.type)
					{
					case KeyPress:
						events::keyboard::onkey_down(event.xkey.keycode);
						break;

					case KeyRelease:
						events::keyboard::onkey_up(event.xkey.keycode);
						break;

					case ButtonPress:
						events::mouse::onkey_down({ event.xbutton.button, {event.xbutton.x, event.xbutton.y} });
						break;

					case MotionNotify:
						events::mouse::motion({ event.xmotion.x, event.xmotion.y });
						break;

					case ButtonRelease:
						if (event.xbutton.button == 1 && event.xany.window == handle->window && event.xany.display == handle->display)
						{
							onclick({ event.xbutton.x, event.xbutton.y });
						}
						else
						{
							events::mouse::onkey_up({ event.xbutton.button, {event.xbutton.x, event.xbutton.y} });
						}
						break;

						//case DestroyNotify:
						//	if (event.xany.display == handle->display && event.xany.window == handle->window)
						//	{
						//		onclose();
						//	}
						//	break;

					case ConfigureNotify:
						//if (size() != size2i{event.xconfigurerequest.width, event.xconfigurerequest.height})
						size.change({ event.xconfigurerequest.width, event.xconfigurerequest.height }, false);
						//if (position() != position2i{event.xconfigurerequest.x, event.xconfigurerequest.y})
						position.change({ event.xconfigurerequest.x, event.xconfigurerequest.y }, false);
						break;

					case ClientMessage:
						onclose();
						break;
					}
				}
			}
		}

		void *window::hwnd() const
		{
			return m_handle;
		}

		event_result window::doclose()
		{
			remove_dc();
			event_result result = widget::doclose();

			if (handle_t *handle = (handle_t*)m_handle)
			{
				m_handle = nullptr;
				XDestroyWindow(handle->display, handle->window);
				XCloseDisplay(handle->display);
				delete handle;
			}

			return result;
		}

		void* window::handle() const
		{
			return m_handle;
		}

		void window::focus()
		{
			if (handle_t *handle = (handle_t*)m_handle)
			{
				XSetInputFocus(handle->display, handle->window, 0, 0);
				widget::focus();
			}
		}
	}
}
#endif

