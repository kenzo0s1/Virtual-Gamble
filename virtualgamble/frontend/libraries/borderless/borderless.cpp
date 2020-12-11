#include "borderless.hpp"

#include <stdexcept>
#include <system_error>

#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include "../interface/interface.hpp"

#include "../../../backend/configurations/configuration.h"
#include "../../../globaloperands.h"
#include "rawdata.h"

#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")
//#include <d3dx9.h>
//#pragma comment(lib, "C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x86\\d3dx9.lib")
#define DIRECTINPUT_VERSION 0x0800


namespace {
	enum class Style : DWORD {
		windowed = WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		aero_borderless = WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
		basic_borderless = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX
	};

	auto maximized(HWND hwnd) -> bool {
		WINDOWPLACEMENT placement;
		if (!::GetWindowPlacement(hwnd, &placement)) {
			return false;
		}

		return placement.showCmd == SW_MAXIMIZE;
	}

	auto adjust_maximized_client_rect(HWND window, RECT& rect) -> void {
		if (!maximized(window)) {
			return;
		}

		auto monitor = ::MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
		if (!monitor) {
			return;
		}

		MONITORINFO monitor_info{};
		monitor_info.cbSize = sizeof(monitor_info);
		if (!::GetMonitorInfoW(monitor, &monitor_info)) {
			return;
		}

		// when maximized, make the client area fill just the monitor (without task bar) rect,
		// not the whole window rect which extends beyond the monitor.
		rect = monitor_info.rcWork;
	}

	auto last_error(const std::string& message) -> std::system_error {
		return std::system_error(
			std::error_code(::GetLastError(), std::system_category()),
			message
		);
	}

	auto window_class(WNDPROC wndproc) -> const wchar_t* {
		static const wchar_t* window_class_name = [&] {
			WNDCLASSEXW wcx{};
			wcx.cbSize = sizeof(wcx);
			wcx.style = CS_HREDRAW | CS_VREDRAW;
			wcx.hInstance = nullptr;
			wcx.lpfnWndProc = wndproc;
			wcx.lpszClassName = L"BorderlessWindowClass";
			wcx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
			wcx.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
			const ATOM result = ::RegisterClassExW(&wcx);
			if (!result) {
				throw last_error("failed to register window class");
			}
			return wcx.lpszClassName;
		}();
		return window_class_name;
	}

	auto composition_enabled() -> bool {
		BOOL composition_enabled = FALSE;
		bool success = ::DwmIsCompositionEnabled(&composition_enabled) == S_OK;
		return composition_enabled && success;
	}

	auto select_borderless_style() -> Style {
		return composition_enabled() ? Style::aero_borderless : Style::basic_borderless;
	}

	auto create_window(WNDPROC wndproc, void* userdata) -> unique_handle
	{
		auto handle = CreateWindowExW(0, window_class(wndproc), L"Artem Gambling Service", static_cast<DWORD>(Style::basic_borderless), 100, 100, 990, 580, nullptr, nullptr, nullptr, userdata);

		if (!handle)
			throw last_error("failed to create window");

		return unique_handle{ handle };
	}
}

BorderlessWindow::BorderlessWindow() : handle{ create_window(&BorderlessWindow::WndProc, this) }
{
	set_borderless(borderless);

	if (!CreateDeviceD3D(handle.get()))
	{
		CleanupDeviceD3D();
		::UnregisterClass(L"BorderlessWindowClass", nullptr);
		return;
	}

	::ShowWindow(handle.get(), SW_SHOW);
	::UpdateWindow(handle.get());

	ImGui::CreateContext();

	ImGui::GetStyle().FrameBorderSize = 0;
	ImGui::GetStyle().WindowBorderSize = 0;
	ImGui::GetStyle().PopupBorderSize = 0;
	ImGui::GetStyle().WindowPadding = { 0,0 };
	ImGui::GetStyle().WindowRounding = 0;

	ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.21f, 0.21f, 0.21f, 1.f);
	ImGui::GetStyle().Colors[ImGuiCol_PopupBg] = ImVec4(22 / 255.f, 22 / 255.f, 22 / 255.f, 1.f);

	ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(0.89f, 0.89f, 0.89f, 1.f);

	ImGui::GetStyle().Colors[ImGuiCol_Header] = ImVec4(0.21f, 0.21f, 0.21f, 1.f);
	ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = ImVec4(0.28f, 0.28f, 0.28f, 1.f);
	ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.f);

	ImFontConfig font_config;
	font_config.OversampleH = 1; //or 2 is the same
	font_config.OversampleV = 1;
	font_config.PixelSnapH = 1;

	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x2000, 0x206F, // General Punctuation
		0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
		0x31F0, 0x31FF, // Katakana Phonetic Extensions
		0xFF00, 0xFFEF, // Half-width characters
		0x4e00, 0x9FAF, // CJK Ideograms
		0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
		0x2DE0, 0x2DFF, // Cyrillic Extended-A
		0xA640, 0xA69F, // Cyrillic Extended-B
		0,
	};

	ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 13.0f, &font_config, ranges);
	tahomabig = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 32.0f, &font_config, ranges);
	firacode = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Font_compressed_data, Font_compressed_size, 14, &font_config, ranges);

	ImGui_ImplWin32_EnableDpiAwareness();
	ImGui_ImplWin32_Init(handle.get());
	ImGui_ImplDX9_Init(g_pd3dDevice);

	initialized_interface = true;

	MSG msg;

	while (::GetMessageW(&msg, nullptr, 0, 0) == TRUE)
	{
		::TranslateMessage(&msg);
		::DispatchMessageW(&msg);

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();

		QuarcInterface quarcui;

		ImGui::EndFrame();

		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(55), (int)(55), (int)(55), (int)(255));
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);

		if (g_pd3dDevice->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			g_pd3dDevice->EndScene();
		}

		HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

		if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			ResetDevice();
	}

	qSaver.SaveSettings();

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void BorderlessWindow::set_borderless(bool enabled)
{
	Style new_style = (enabled) ? select_borderless_style() : Style::windowed;
	Style old_style = static_cast<Style>(::GetWindowLongPtrW(handle.get(), GWL_STYLE));

	if (new_style != old_style)
	{
		borderless = enabled;

		::SetWindowLongPtrW(handle.get(), GWL_STYLE, static_cast<LONG>(new_style));

		::SetWindowPos(handle.get(), nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
		::ShowWindow(handle.get(), SW_SHOW);
	}
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

auto CALLBACK BorderlessWindow::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept -> LRESULT {

	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
		return true;

	if (msg == WM_NCCREATE)
	{
		auto userdata = reinterpret_cast<CREATESTRUCTW*>(lparam)->lpCreateParams;
		::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(userdata));
	}

	if (auto window_ptr = reinterpret_cast<BorderlessWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA)))
	{
		auto& window = *window_ptr;

		switch (msg) {
		case WM_SIZE:
			if (g_pd3dDevice != NULL && wparam != SIZE_MINIMIZED)
			{
				g_d3dpp.BackBufferWidth = LOWORD(lparam);
				g_d3dpp.BackBufferHeight = HIWORD(lparam);
				ResetDevice();
			}
			return 0;
		case WM_GETMINMAXINFO:
			LPMINMAXINFO(lparam)->ptMinTrackSize.x = 990;
			LPMINMAXINFO(lparam)->ptMinTrackSize.y = 580;
			break;
		case WM_NCCALCSIZE: {
			if (wparam == TRUE && window.borderless) {
				auto& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);
				adjust_maximized_client_rect(hwnd, params.rgrc[0]);
				return 0;
			}
			break;
		}
		case WM_NCHITTEST: {
			// When we have no border or title bar, we need to perform our
			// own hit testing to allow resizing and moving.
			if (window.borderless) {
				return window.hit_test(POINT{
					GET_X_LPARAM(lparam),
					GET_Y_LPARAM(lparam)
					});
			}
			break;
		}
		case WM_NCACTIVATE: {
			if (!composition_enabled()) {
				// Prevents window frame reappearing on window activation
				// in "basic" theme, where no aero shadow is present.
				return 1;
			}
			break;
		}

		case WM_CLOSE: {
			::DestroyWindow(hwnd);
			return 0;
		}

		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: {
			switch (wparam) {
			case VK_F8: { window.borderless_drag = !window.borderless_drag;        return 0; }
			case VK_F9: { window.borderless_resize = !window.borderless_resize;    return 0; }
			case VK_F11: { window.set_borderless(!window.borderless);               return 0; }
			}
			break;
		}
		}
	}

	return ::DefWindowProcW(hwnd, msg, wparam, lparam);
}

auto BorderlessWindow::hit_test(POINT cursor) const -> LRESULT {
	// identify borders and corners to allow resizing the window.
	// Note: On Windows 10, windows behave differently and
	// allow resizing outside the visible window frame.
	// This implementation does not replicate that behavior.
	const POINT border{
		::GetSystemMetrics(SM_CXFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER),
		::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER)
	};
	RECT window;
	if (!::GetWindowRect(handle.get(), &window)) {
		return HTNOWHERE;
	}

	const auto drag = borderless_drag ? HTCAPTION : HTCLIENT;

	enum region_mask {
		client = 0b0000,
		left = 0b0001,
		right = 0b0010,
		top = 0b0100,
		bottom = 0b1000,
	};

	const auto result =
		left * (cursor.x < (window.left + border.x)) |
		right * (cursor.x >= (window.right - border.x)) |
		top * (cursor.y < (window.top + border.y)) |
		bottom * (cursor.y >= (window.bottom - border.y));

	switch (result) {
	case left: return borderless_resize ? HTLEFT : drag;
	case right: return borderless_resize ? HTRIGHT : drag;
	case top: return borderless_resize ? HTTOP : drag;
	case bottom: return borderless_resize ? HTBOTTOM : drag;
	case top | left: return borderless_resize ? HTTOPLEFT : drag;
	case top | right: return borderless_resize ? HTTOPRIGHT : drag;
	case bottom | left: return borderless_resize ? HTBOTTOMLEFT : drag;
	case bottom | right: return borderless_resize ? HTBOTTOMRIGHT : drag;
	case client: return drag;
	default: return HTNOWHERE;
	}
}
