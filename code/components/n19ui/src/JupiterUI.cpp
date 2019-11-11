#include <StdInc.h>

#include <DrawCommands.h>

#pragma comment(lib, "delayimp.lib")

#ifdef GTA_FIVE
#pragma comment(lib, "runtimeobject.lib")

#include <winrt/windows.ui.xaml.media.h>

using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Composition;
using namespace winrt::Windows::UI::Xaml::Hosting;
using namespace winrt::Windows::Foundation::Numerics;

HWND g_hwnd;
HWND g_childHwnd;
IDXGISwapChain1* g_swapChain;

static LONG_PTR g_oldWndProc;

static LRESULT APIENTRY HookedWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_SETFOCUS)
	{
		SetFocus(g_hwnd);
		return 0;
	}

	return CallWindowProc((WNDPROC)g_oldWndProc, hWnd, uMsg, wParam, lParam);
}

static InitFunction initFunction([]()
{
	DWORDLONG viMask = 0;
	OSVERSIONINFOEXW osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	osvi.dwBuildNumber = 18362; // 19H1+

	VER_SET_CONDITION(viMask, VER_BUILDNUMBER, VER_GREATER_EQUAL);

	if (!VerifyVersionInfoW(&osvi, VER_BUILDNUMBER, viMask))
	{
		return;
	}

	if (GetFileAttributes(MakeRelativeCitPath(L"citizen/use_n19ui.txt").c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return;
	}

	OnTryCreateSwapChain.Connect([](IDXGIFactory2* factory, ID3D11Device* device, HWND hWnd, DXGI_SWAP_CHAIN_DESC1* pDesc, DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFsDesc, IDXGISwapChain1** ppSwapChain)
	{
		static auto g_xamlManager = WindowsXamlManager::InitializeForCurrentThread();

		//pDesc->SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		pDesc->SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		pDesc->Scaling = DXGI_SCALING_STRETCH;

		HRESULT hr = factory->CreateSwapChainForComposition(device, pDesc, nullptr, ppSwapChain);
		winrt::check_hresult(hr);

		g_hwnd = hWnd;
		g_swapChain = *ppSwapChain;
	});

	OnPostFrontendRender.Connect([]()
	{
		static bool inited;

		if (!inited)
		{
			static DesktopWindowXamlSource desktopSource;
			auto interop = desktopSource.as<IDesktopWindowXamlSourceNative>();
			winrt::check_hresult(interop->AttachToWindow(g_hwnd));

			interop->get_WindowHandle(&g_childHwnd);

			RECT rect;
			GetClientRect(g_hwnd, &rect);
			SetWindowPos(g_childHwnd, 0, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);

			// you're supposed to use WM_NCHITTEST on the child window, but some optimization in Windows means this never gets called for windows that
			// are unlikely to have a non-client area.
			SetWindowLong(g_childHwnd, GWL_EXSTYLE, GetWindowLong(g_childHwnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT | WS_EX_LAYERED);
			g_oldWndProc = SetWindowLongPtr(g_childHwnd, GWLP_WNDPROC, (LONG_PTR)HookedWndProc);

			static winrt::Windows::UI::Xaml::Controls::SwapChainPanel panel;

			auto native = panel.as<ISwapChainPanelNative>();
			native->SetSwapChain(g_swapChain);

			static auto myBrush = winrt::Windows::UI::Xaml::Media::AcrylicBrush();
			myBrush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::Backdrop);
			myBrush.TintColor(winrt::Windows::UI::ColorHelper::FromArgb(255, 255, 0, 255));
			myBrush.FallbackColor(winrt::Windows::UI::ColorHelper::FromArgb(255, 202, 24, 37));
			myBrush.TintOpacity(0.6);

			static winrt::Windows::UI::Xaml::Controls::Grid xamlContainer;
			xamlContainer.Background(myBrush);
			xamlContainer.Width(150);
			xamlContainer.Height(150);
			xamlContainer.HorizontalAlignment(winrt::Windows::UI::Xaml::HorizontalAlignment::Right);
			xamlContainer.VerticalAlignment(winrt::Windows::UI::Xaml::VerticalAlignment::Bottom);

			static winrt::Windows::UI::Xaml::Controls::TextBlock tb;
			tb.Text(ToWide(u8"♞"));
			tb.VerticalAlignment(winrt::Windows::UI::Xaml::VerticalAlignment::Center);
			tb.HorizontalAlignment(winrt::Windows::UI::Xaml::HorizontalAlignment::Center);
			tb.Foreground(winrt::Windows::UI::Xaml::Media::SolidColorBrush(winrt::Windows::UI::Colors::White()));
			tb.FontSize(64);

			xamlContainer.Children().Append(tb);
			xamlContainer.UpdateLayout();

			panel.Children().Append(xamlContainer);
			panel.UpdateLayout();
			desktopSource.Content(panel);

			inited = true;
		}
	});
});
#endif
