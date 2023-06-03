#include "dll_log.hpp"
#include "ini_file.hpp"
#include "hook_manager.hpp"
#include "addon_manager.hpp"
#include "version.h"

#include "runtime.hpp"

#include <windows.h>
#include <VersionHelpers.h>

#include "d3d11/d3d11_device.hpp"
#include "d3d11/d3d11_device_context.hpp"
#include "dxgi/dxgi_swapchain.hpp"

#pragma comment(lib, "dwmapi.lib")

#include "imgui.h"

char ImGuiTextBuffer::EmptyString[1];

// to take in 'addon.obj'
extern "C" void* ReShadeGetImGuiFunctionTable(uint32_t version);

extern "C" __declspec(dllexport) const char* ReShadeVersion = VERSION_STRING_PRODUCT;

HMODULE g_module_handle = nullptr;
std::filesystem::path g_reshade_dll_path;
std::filesystem::path g_reshade_base_path;
std::filesystem::path g_target_executable_path;

std::filesystem::path get_module_path(HMODULE module)
{
	WCHAR buf[4096];
	return GetModuleFileNameW(module, buf, ARRAYSIZE(buf)) ? buf : std::filesystem::path();
}

bool is_uwp_app()
{
	return false;
}

bool is_windows7()
{
	return !IsWindows8OrGreater();
}

namespace libreshade
{
void init(const std::wstring& path)
{
	ReShadeGetImGuiFunctionTable(18600);

	GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCWSTR>(&init), &g_module_handle);

	g_reshade_dll_path = path + L"plugins/reshade.dll";
	g_target_executable_path = path + L"plugins/reshade.dll";
	g_reshade_base_path = path + L"plugins/";

	ini_file& config = reshade::global_config();
}

DXGISwapChain* g_proxy;

void setup_d3d11(ID3D11Device*& device, ID3D11DeviceContext*& context, IDXGISwapChain*& swapChain)
{
	// Query for the DXGI device and immediate device context since we need to reference them in the hooked device
	com_ptr<IDXGIDevice1> dxgi_device;
	HRESULT hr = device->QueryInterface(&dxgi_device);
	assert(SUCCEEDED(hr));

	D3D11Device* device_proxy = nullptr;

	device = device_proxy = new D3D11Device(dxgi_device.get(), device);
	device_proxy->_immediate_context = new D3D11DeviceContext(device_proxy, context);

	device->GetImmediateContext(&context);

	DXGISwapChain* swapchain_proxy = new DXGISwapChain(device_proxy, swapChain);
	swapChain = swapchain_proxy;

	g_proxy = swapchain_proxy;
}

void toggle_gui(bool gui)
{
	if (g_proxy)
	{
		g_proxy->_impl->_show_overlay = gui;
	}
}

void draw_gui()
{
	if (g_proxy->_impl->is_initialized())
	{
		g_proxy->_impl->draw_gui();
	}
}
}

