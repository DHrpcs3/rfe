#ifdef _WIN32
#ifndef WINAPI_FAMILY
#define WINAPI_FAMILY WINAPI_FAMILY_DESKTOP_APP
#endif

#include <Windows.h>
#include <Windowsx.h>
#include <iostream>
#include <rfe/core/fmt.h>
#include <rfe/core/events.h>
#include <rfe/ui/window.h>

namespace rfe
{
	namespace ui
	{
		LRESULT APIENTRY CustomWindowProc(
			HWND hwnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam)
		{
			if (LONG_PTR user_data = GetWindowLongPtr(hwnd, GWLP_USERDATA))
			{
				window *wnd = reinterpret_cast<window*>(user_data);

				event_result result = event_result::skip;

				switch (uMsg)
				{
				case WM_MOVE:
					result = wnd->position.change(ignore_custom_invoker, { LOWORD(lParam), HIWORD(lParam) });
					break;

				case WM_SIZING:
				{
					LPRECT lprc = (LPRECT)lParam;
					result = wnd->size.change(ignore_custom_invoker, { lprc->right - lprc->left, lprc->bottom - lprc->top });
				}
				break;

				case WM_SIZE:
					result = wnd->size.change(ignore_custom_invoker, { LOWORD(lParam), HIWORD(lParam) });
					break;

				case WM_CLOSE:
					result = wnd->onclose(synchronized);
					break;

				case WM_LBUTTONUP:
					result = events::mouse::onkey_up(synchronized, 0, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });

					if (result == event_result::skip)
					{
						if (wnd->motion_started())
						{
							result = wnd->onmotion_end(synchronized, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
						}
						else
						{
							result = wnd->ontry_click(synchronized, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
						}
					}

					break;

				case WM_LBUTTONDOWN:
					result = events::mouse::onkey_down(synchronized, 0, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });

					if (result == event_result::skip)
					{
						result = wnd->ontouch(synchronized, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
					}
					break;

				case WM_MBUTTONUP:
					result = events::mouse::onkey_up(synchronized, 1, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
					break;

				case WM_MBUTTONDOWN:
					result = events::mouse::onkey_down(synchronized, 1, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
					break;

				case WM_RBUTTONUP:
					result = events::mouse::onkey_up(synchronized, 2, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
					break;

				case WM_RBUTTONDOWN:
					result = events::mouse::onkey_down(synchronized, 2, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
					break;

				case WM_MOUSEMOVE:
					result = events::mouse::motion(synchronized, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
					break;

				case WM_KEYDOWN:
					result = events::keyboard::onkey_down(synchronized, (int)wParam);
					break;

				case WM_KEYUP:
					result = events::keyboard::onkey_up(synchronized, (int)wParam);
					break;
				}

				if (result == event_result::handled)
				{
					return 0;
				}
			}

			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}

		window::window(bool make_thread)
		{
			if (make_thread)
			{
				thread = make_thread_queue([this]
				{
					if (m_handle)
					{
						MSG msg;
						while (PeekMessageW(&msg, (HWND)m_handle, 0, 0, PM_REMOVE))
						{
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}
					}
				});

				events_thread() = thread;
			}

			clear_color = { 0.4f, 0.4f, 0.4f, 1.0f };
			clear_depth = 1.f;
			clear_stencil = 0xff;

			title.get_function([&]() -> std::string
			{
				if (!m_handle)
					return{};

				char result[512];
				GetWindowTextA((HWND)m_handle, result, 512);
				return result;
			});

			title.invoke_event_function([&](std::string value)
			{
				if (!m_handle)
					return;

				SetWindowTextA((HWND)m_handle, value.c_str());
			});

			position.get_function([&]() ->point2i
			{
				if (!m_handle)
					return{};

				RECT rc;
				GetWindowRect((HWND)handle(), &rc);
				return{ rc.left, rc.top };
			});

			position.invoke_event_function([&](point2i value)
			{
				if (!m_handle)
					return;

				//RECT wr = { value.x, value.y, 0, 0 };
				//AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

				SetWindowPos((HWND)m_handle, nullptr, value.x(), value.y(), 0, 0, SWP_NOSIZE);
			});

			size.get_function([&]() -> size2i
			{
				if (!m_handle)
					return{};

				RECT rc;
				GetClientRect((HWND)handle(), &rc);

				//AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
				return size2i{ rc.right - rc.left, rc.bottom - rc.top };
			});

			size.invoke_event_function([&](size2i value)
			{
				if (!m_handle)
					return;

				RECT wr = { 0, 0, value.width(), value.height() };
				AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

				SetWindowPos((HWND)m_handle, nullptr, 0, 0, wr.right - wr.left, wr.bottom - wr.top, SWP_NOMOVE);
			});

			shown.get_function([&]() -> bool
			{
				if (!m_handle)
					return{};

				return IsWindowVisible((HWND)m_handle) != FALSE;
			});

			shown.invoke_event_function([&](bool value)
			{
				if (!m_handle)
					return;

				ShowWindow((HWND)m_handle, value ? SW_NORMAL : SW_HIDE);
				UpdateWindow((HWND)m_handle);
			});

			thread.invoke([=] { create(); });
		}

		void window::create()
		{
			std::cout << "window::create\n";
			WNDCLASSEX wc;
			HINSTANCE hInstance = GetModuleHandle(nullptr);
			const auto lpszClassName = fmt::format("UiWindowClass%u", id());

			// Register the Window class
			wc.cbSize = sizeof(WNDCLASSEX);
			wc.style = 0;
			wc.lpfnWndProc = CustomWindowProc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = hInstance;
			wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
			wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wc.hbrBackground = (HBRUSH)COLOR_WINDOWFRAME;
			wc.lpszMenuName = nullptr;
			wc.lpszClassName = lpszClassName.c_str();
			wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

			if (!RegisterClassEx(&wc))
			{
				MessageBox(NULL, "Window Registration Failed!", "ERROR", MB_ICONEXCLAMATION | MB_OK);
				return;
			}

			// Set initial size
			RECT rc;
			rc.left = 0;
			rc.top = 0;
			rc.right = 800;
			rc.bottom = 600;

			AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);

			// Create the Window
			m_handle = CreateWindowEx(
				WS_EX_APPWINDOW,
				lpszClassName.c_str(),
				"",
				WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
				CW_USEDEFAULT, CW_USEDEFAULT,
				rc.right - rc.left, rc.bottom - rc.top,
				nullptr,
				nullptr,
				hInstance,
				nullptr);

			if (!m_handle)
			{
				MessageBox(nullptr, "Window Creation Failed!", "ERROR", MB_ICONEXCLAMATION | MB_OK);
				return;
			}

			SetWindowLongPtr((HWND)m_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		}

		void window::remove_dc()
		{
			m_draw_context = nullptr;
		}

		void *window::handle() const
		{
			return m_handle;
		}

		event_result window::doclose()
		{
			remove_dc();
			DestroyWindow((HWND)m_handle);
			return widget::doclose();
		}

		void window::focus()
		{
			if (m_handle)
			{
				SetFocus((HWND)m_handle);
				widget::focus();
			}
		}
	}
}
#endif
