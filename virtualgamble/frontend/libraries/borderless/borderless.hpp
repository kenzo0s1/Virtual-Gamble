#pragma once

#include <memory>
#include <string>

#include <Windows.h>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800

static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

inline bool CreateDeviceD3D(HWND hWnd)
{
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return false;

	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));

	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
		return false;

	return true;
}

inline void CleanupDeviceD3D()
{
	if (g_pd3dDevice)
	{
		g_pd3dDevice->Release(); g_pd3dDevice = NULL;
	}

	if (g_pD3D)
	{
		g_pD3D->Release(); g_pD3D = NULL;
	}
}

inline void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);

	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

struct hwnd_deleter {
	using pointer = HWND;
	auto operator()(HWND handle) const -> void {
		::DestroyWindow(handle);
	}
};

using unique_handle = std::unique_ptr<HWND, hwnd_deleter>;

class BorderlessWindow {
public:
	BorderlessWindow();
	auto set_borderless(bool enabled) -> void;

private:
	static auto CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept -> LRESULT;
	auto hit_test(POINT cursor) const->LRESULT;

	bool borderless = false; // is the window currently borderless
	bool borderless_resize = true; // should the window allow resizing by dragging the borders while borderless
	bool borderless_drag = true; // should the window allow moving my dragging the client area

	unique_handle handle;
};
