#include <StdInc.h>
#include "ConsoleHost.h"

#include "ServerConsoleHost.h"

#ifdef _WIN32
#include <wrl.h>

// SvCon stuff
fwEvent<> ConHost::OnDrawGui;
fwEvent<bool*> ConHost::OnShouldDrawGui;

void DrawConsole();
void DrawDevGui();

extern bool g_consoleFlag;
void SendPrintMessage(const std::string& channel, const std::string& message);

bool ConHost::IsConsoleOpen()
{
	return g_consoleFlag;
}

void ConHost::Print(const std::string& channel, const std::string& message)
{
	SendPrintMessage(channel, message);
}

// Dear ImGui: standalone example application for DirectX 11
#pragma comment(lib, "d3d11.lib")

#include "imgui.h"

#include "menlo.h"
#include "../src/backends/imgui_impl_win32.h"
#include "../src/backends/imgui_impl_dx11.h"

ImFont* consoleFontSmall;

#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>

// Data
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
namespace ServerConsoleHost
{
class ConHostSvImpl
{
public:
	ConHostSvImpl();

	virtual ~ConHostSvImpl();

	void Run(std::function<bool()>&& fn);

	inline void* GetHwnd()
	{
		return hwnd;
	}

private:
	WNDCLASSEX wc;
	HWND hwnd;

	bool initialized = false;
	bool isWarp = false;
};

ConHostSv::ConHostSv()
{
	m_impl = std::make_unique<ConHostSvImpl>();
}

ConHostSv::~ConHostSv()
{

}

void ConHostSv::Run(std::function<bool()>&& fn)
{
	m_impl->Run(std::move(fn));
}

void* ConHostSv::GetPlatformWindowHandle()
{
	return m_impl->GetHwnd();
}

ConHostSvImpl::ConHostSvImpl()
{
	// Create application window
	//ImGui_ImplWin32_EnableDpiAwareness();
	wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("FXS"), NULL };
	::RegisterClassEx(&wc);
	hwnd = ::CreateWindow(wc.lpszClassName, _T("VMP.ir Server (VMPServer)"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return;
	}

	{
		Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
		if (SUCCEEDED(g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice))))
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
			if (SUCCEEDED(dxgiDevice->GetAdapter(&dxgiAdapter)))
			{
				Microsoft::WRL::ComPtr<IDXGIAdapter1> adap1;	

				if (SUCCEEDED(dxgiAdapter.As(&adap1)))
				{
					DXGI_ADAPTER_DESC1 adapDesc1;
					if (SUCCEEDED(adap1->GetDesc1(&adapDesc1)))
					{
						if (adapDesc1.VendorId == 0x1414)
						{
							isWarp = true;
						}
					}
				}
			}
		}
	}

	g_consoleFlag = true;

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
	}

	auto fontData = menlo;
	auto fontSize = std::size(menlo);

	ImFontConfig font_cfg;
	font_cfg.FontDataOwnedByAtlas = false;

	io.Fonts->AddFontFromMemoryTTF(fontData, fontSize, 18.0f, &font_cfg);

	consoleFontSmall = io.Fonts->AddFontFromMemoryTTF(fontData, fontSize, 18.0f, &font_cfg);

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	initialized = true;
}

void ConHostSvImpl::Run(std::function<bool()>&& fn)
{
	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	HANDLE hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
	std::chrono::microseconds frameStart{ 0 };

	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			continue;
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		bool cont = fn();
		DrawDevGui();
		DrawConsole();
		ConHost::OnDrawGui();

		// Rendering
		ImGui::Render();
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);

		float clear_color[4] = { 0.f, 0.f, 0.f, 1.f };
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		if ((isWarp || IsIconic(hwnd)) && hTimer)
		{
			g_pSwapChain->Present(0, 0);

			std::chrono::microseconds frameLimit{ 75000 };
			auto timeLeft = frameLimit - (std::chrono::high_resolution_clock::now().time_since_epoch() - frameStart);

			LARGE_INTEGER liDueTime;
			liDueTime.QuadPart = -(LONGLONG)(std::chrono::duration_cast<std::chrono::microseconds>(timeLeft).count() * 10);

			if (SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0))
			{
				WaitForSingleObject(hTimer, INFINITE);
			}

			frameStart = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
		}
		else
		{
			g_pSwapChain->Present(1, 0);
		}

		if (!cont)
		{
			break;
		}
	}

	CloseHandle(hTimer);
}

ConHostSvImpl::~ConHostSvImpl()
{
	// Cleanup
	if (initialized)
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);

	// run one message loop to ensure WM_DESTROY is handled
	MSG msg = { 0 };

	while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0,
	};
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain)
	{
		g_pSwapChain->Release();
		g_pSwapChain = NULL;
	}
	if (g_pd3dDeviceContext)
	{
		g_pd3dDeviceContext->Release();
		g_pd3dDeviceContext = NULL;
	}
	if (g_pd3dDevice)
	{
		g_pd3dDevice->Release();
		g_pd3dDevice = NULL;
	}
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView)
	{
		g_mainRenderTargetView->Release();
		g_mainRenderTargetView = NULL;
	}
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
		case WM_SIZE:
			if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
			{
				CleanupRenderTarget();
				g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
				CreateRenderTarget();
			}
			return 0;
		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				return 0;
			break;
		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
#endif
