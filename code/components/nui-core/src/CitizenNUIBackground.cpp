/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "CefOverlay.h"

extern nui::GameInterface* g_nuiGi;

#include <CfxRGBA.h>

#include <VFSManager.h>
#include <VFSWin32.h>

#include <DirectXColors.h>
#include <wrl.h>
#include <wincodec.h>

#include "memdbgon.h"

using namespace DirectX;
using namespace Microsoft::WRL;

#pragma comment(lib, "windowscodecs.lib")

struct DWMP_GET_COLORIZATION_PARAMETERS_DATA
{
	uint32_t ColorizationColor;
	uint32_t ColorizationAfterglow;
	uint32_t ColorizationColorBalance;
	uint32_t ColorizationAfterglowBalance;
	uint32_t ColorizationBlurBalance;
	uint32_t ColorizationGlassReflectionIntensity;
	uint32_t ColorizationOpaqueBlend;
};

static ComPtr<IWICImagingFactory> g_imagingFactory;

class CitizenNUIBackground
{
private:
	fwRefContainer<nui::GITexture> m_backdropTexture;

	typedef void (WINAPI* DwmpGetColorizationParameters_t)(DWMP_GET_COLORIZATION_PARAMETERS_DATA*);

	DwmpGetColorizationParameters_t m_fDwmpGetColorizationParameters;

private:
	fwRefContainer<nui::GITexture> InitializeTextureFromFile(const std::string& filename);

	void EnsureTextures();

	void DrawBackground(fwRefContainer<nui::GITexture> texture, CRGBA colorValue, bool cover = false);

	CRGBA GetCurrentDWMColor(bool usePulse);

	CRGBA GetCurrentColor(bool usePulse);

	void TryInitializeDWM();

public:
	void Initialize();
};

#include <mmsystem.h>

CRGBA CitizenNUIBackground::GetCurrentColor(bool usePulse)
{
	// TODO: make this into a user-toggleable option
	if (m_fDwmpGetColorizationParameters)
	{
		return GetCurrentDWMColor(usePulse);
	}

	static CRGBA colorSet[] = {
		CRGBA(0xCB, 0xCB, 0xCB),
		CRGBA(0xD8, 0xBF, 0x1A),
		CRGBA(0x6D, 0xB2, 0x17),
		CRGBA(0xE1, 0x7E, 0x9A),
		CRGBA(0x17, 0x88, 0x16),
		CRGBA(0x9A, 0x61, 0xC8),
		CRGBA(0x02, 0xCD, 0xC7),
		CRGBA(0x07, 0x76, 0xC0),
		CRGBA(0xB4, 0x44, 0xC0),
		CRGBA(0xE5, 0xA7, 0x08),
		CRGBA(0x87, 0x5B, 0x1E),
		CRGBA(0xE3, 0x41, 0x2A)
	};

	SYSTEMTIME time;
	GetSystemTime(&time);

	int dayValue = (int)((time.wDay / 31.0f) * _countof(colorSet));
	int nextValue = (dayValue + 1) % _countof(colorSet);

	CRGBA dayRGBA = colorSet[dayValue];
	CRGBA nextRGBA = colorSet[nextValue];

	XMVECTOR dayColor = XMVectorSet(dayRGBA.red / 255.0f, dayRGBA.green / 255.0f, dayRGBA.blue / 255.0f, 1.0f);
	XMVECTOR nextColor = XMVectorSet(nextRGBA.red / 255.0f, nextRGBA.green / 255.0f, nextRGBA.blue / 255.0f, 1.0f);

	float baseFraction = (time.wHour / 24.0f) + (time.wMinute / 24.0f / 60.0f) + (time.wSecond / 24.0f / 60.0f / 60.0f);

	if (usePulse)
	{
		baseFraction += (1.0f / 24.0f) * sin(timeGetTime() / 1750.0f);
	}

	dayColor = XMColorRGBToHSV(dayColor);
	nextColor = XMColorRGBToHSV(nextColor);

	XMVECTOR baseColor = XMVectorLerp(dayColor, nextColor, baseFraction);

	baseColor = XMColorHSVToRGB(baseColor);

	return CRGBA::FromFloat(XMVectorGetX(baseColor), XMVectorGetY(baseColor), XMVectorGetZ(baseColor), 1.0f);
}

CRGBA CitizenNUIBackground::GetCurrentDWMColor(bool usePulse)
{
	DWMP_GET_COLORIZATION_PARAMETERS_DATA parameters;
	m_fDwmpGetColorizationParameters(&parameters);

	CRGBA colorizationColor = CRGBA::FromARGB(parameters.ColorizationColor);
	float gray = 230.0f;
	float alpha = colorizationColor.alpha / 255.0f;
	float intensity = 0.5f;//parameters.ColorizationColorBalance / 100.0f;

	float maxVal = 255.0f / fwMax(colorizationColor.red, fwMax(colorizationColor.green, colorizationColor.blue));
	
	// calculate base color
	float r = colorizationColor.red * maxVal;
	float g = colorizationColor.green * maxVal;
	float b = colorizationColor.blue * maxVal;

	// calculate core color
	float r2 = maxVal + (r - maxVal) - ((r - maxVal) * alpha);
	float g2 = maxVal + (g - maxVal) - ((g - maxVal) * alpha);
	float b2 = maxVal + (b - maxVal) - ((b - maxVal) * alpha);

	// convert core color to a valid HSV value with saturation at 0.70
	XMVECTOR color = XMVectorSet(r2, g2, b2, 1.0f) / 255.0f;
	color = XMVectorSetY(XMColorRGBToHSV(color), 0.70f);
	color = XMColorHSVToRGB(color) * 255.0f;

	r2 = XMVectorGetX(color);
	g2 = XMVectorGetY(color);
	b2 = XMVectorGetZ(color);

	// add intensity to a set of two colors
	float rCur = r2 + (gray - r2) - ((gray - r2) * intensity);
	float gCur = g2 + (gray - g2) - ((gray - g2) * intensity);
	float bCur = b2 + (gray - b2) - ((gray - b2) * intensity);

	// decide on the next color to interpolate with
	if (usePulse)
	{
		float baseFraction = fabs(sin(timeGetTime() / 1750.0f));

		intensity *= 0.9f;

		float rNext = r2 + (gray - r2) - ((gray - r2) * intensity);
		float gNext = g2 + (gray - g2) - ((gray - g2) * intensity);
		float bNext = b2 + (gray - b2) - ((gray - b2) * intensity);

		rCur = (rCur * baseFraction) + (rNext * (1.0f - baseFraction));
		gCur = (gCur * baseFraction) + (gNext * (1.0f - baseFraction));
		bCur = (bCur * baseFraction) + (bNext * (1.0f - baseFraction));
	}

	return CRGBA::FromFloat(rCur / 255.0f, gCur / 255.0f, bCur / 255.0f, 1.0f);
}

void CitizenNUIBackground::TryInitializeDWM()
{
	HMODULE hDwmAPI = LoadLibrary(L"dwmapi.dll");

	if (false)
	{
		m_fDwmpGetColorizationParameters = (DwmpGetColorizationParameters_t)GetProcAddress(hDwmAPI, MAKEINTRESOURCEA(127));

		// check if the stack is at least somewhat valid upon returning from this function (at least, if __try allows for this!)
		if (m_fDwmpGetColorizationParameters)
		{
			__try
			{
				DWMP_GET_COLORIZATION_PARAMETERS_DATA data;
				m_fDwmpGetColorizationParameters(&data);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				m_fDwmpGetColorizationParameters = nullptr;
			}
		}
	}
}

void CitizenNUIBackground::Initialize()
{
	// try to initialize DWMAPI
	TryInitializeDWM();

	// COM stuff
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void**)g_imagingFactory.GetAddressOf());

	if (FAILED(hr))
	{
		return;
	}

	nui::OnDrawBackground.Connect([=] (bool isMainUI)
	{
		EnsureTextures();

		if (isMainUI)
		{
			DrawBackground(m_backdropTexture, CRGBA(255, 255, 255, 255), true);
		}
	});
}

void CitizenNUIBackground::DrawBackground(fwRefContainer<nui::GITexture> texture, CRGBA colorValue, bool cover /* = false */)
{
	int resX, resY;
	g_nuiGi->GetGameResolution(&resX, &resY);

	nui::ResultingRectangle rr;
	rr.rectangle = CRect(0.0f, 0.0f, resX, resY);
	rr.color = colorValue;

	if (cover)
	{
		float oWidth = 2538.0f;
		float oHeight = 1355.0f;

		float originalRatios[2] = {
			resX / oWidth,
			resY / oHeight
		};

		float coverRatio = std::max(originalRatios[0], originalRatios[1]);

		float newImageWidth = oWidth * coverRatio;
		float newImageHeight = oHeight * coverRatio;

		float nx = (resX - newImageWidth) / 2;
		float ny = (resY - newImageHeight) / 2;

		rr.rectangle = CRect{
			nx,
			ny,
			nx + newImageWidth,
			ny + newImageHeight
		};
	}

	g_nuiGi->SetTexture(texture);
	g_nuiGi->DrawRectangles(1, &rr);
	g_nuiGi->UnsetTexture();
}

fwRefContainer<nui::GITexture> g_cursorTexture;

void CitizenNUIBackground::EnsureTextures()
{
	if (!m_backdropTexture.GetRef())
	{
		m_backdropTexture = InitializeTextureFromFile("citizen:/ui/app/static/media/32000f5a0b55eccb6d79bg2.jpg");

		if (!m_backdropTexture.GetRef())
		{
			m_backdropTexture = InitializeTextureFromFile("citizen:/resources/background_main.jpg");
		}
	}

	if (!g_cursorTexture.GetRef())
	{
		g_cursorTexture = InitializeTextureFromFile("citizen:/resources/citizen_cursor.png");
	}
}

static std::vector<uint32_t> LoadFileBitmap(const std::string& filename, int* outWidth, int* outHeight, float dpiScale = 1.0f)
{
	ComPtr<IWICBitmapDecoder> decoder;
	std::vector<uint8_t> v;

	{
		auto s = vfs::OpenRead(filename);

		if (!s.GetRef())
		{
			return {};
		}

		v = s->ReadToEnd();
	}

	auto stream = vfs::CreateComStream(vfs::OpenRead(fmt::sprintf("memory:$%016llx,%d,0:%s", (uintptr_t)v.data(), v.size(), filename)));
	HRESULT hr = g_imagingFactory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());

	if (SUCCEEDED(hr))
	{
		ComPtr<IWICBitmapFrameDecode> frame;

		hr = decoder->GetFrame(0, frame.GetAddressOf());

		if (SUCCEEDED(hr))
		{
			UINT width = 0, height = 0;
			ComPtr<IWICBitmapSource> source;

			frame->GetSize(&width, &height);
			frame.As(&source);

			if (dpiScale > 1.0f)
			{
				ComPtr<IWICBitmapScaler> scaler;
				if (SUCCEEDED(g_imagingFactory->CreateBitmapScaler(&scaler)))
				{
					auto targetWidth = static_cast<UINT>(width * dpiScale);
					auto targetHeight = static_cast<UINT>(height * dpiScale);

					if (SUCCEEDED(scaler->Initialize(
						frame.Get(),
						targetWidth,
						targetHeight,
						WICBitmapInterpolationModeFant)))
					{
						width = targetWidth;
						height = targetHeight;

						scaler.As(&source);
					}
				}
			}

			ComPtr<IWICBitmapSource> convertedSource;

			if (outWidth)
			{
				*outWidth = width;
			}

			if (outHeight)
			{
				*outHeight = height;
			}

			// try to convert to a pixel format we like
			hr = WICConvertBitmapSource(GUID_WICPixelFormat32bppBGRA, source.Get(), convertedSource.GetAddressOf());

			if (SUCCEEDED(hr))
			{
				source = convertedSource;
			}

			// create a pixel data buffer
			std::vector<uint32_t> pixelData(width * height);

			hr = source->CopyPixels(nullptr, width * 4, width * height * 4, reinterpret_cast<BYTE*>(pixelData.data()));

			if (SUCCEEDED(hr))
			{
				return pixelData;
			}
		}
	}

	return {};
}

fwRefContainer<nui::GITexture> CitizenNUIBackground::InitializeTextureFromFile(const std::string& filename)
{
	int width = 0;
	int height = 0;
	auto pixelData = LoadFileBitmap(filename, &width, &height);

	if (!pixelData.empty())
	{
		return g_nuiGi->CreateTexture(width, height, nui::GITextureFormat::ARGB, pixelData.data());
	}

	return nullptr;
}

// adapted from 'dear imgui' impl_win32
#include <shellscalingapi.h>

static float GetDpiScaleForMonitor(void* monitor)
{
	UINT xdpi = 96, ydpi = 96;
	static HINSTANCE shcore = LoadLibraryW(L"shcore.dll");
	if (shcore)
	{
		static auto _GetDpiForMonitor = (decltype(&GetDpiForMonitor))GetProcAddress(shcore, "GetDpiForMonitor");
		if (_GetDpiForMonitor)
		{
			_GetDpiForMonitor((HMONITOR)monitor, MDT_EFFECTIVE_DPI, &xdpi, &ydpi);
			return xdpi / 96.0f;
		}
	}

	return 1.0f;
}

static float GetDpiScaleForHwnd(void* hwnd)
{
	HMONITOR monitor = ::MonitorFromWindow((HWND)hwnd, MONITOR_DEFAULTTONEAREST);
	return GetDpiScaleForMonitor(monitor);
}

HCURSOR InitDefaultCursor()
{
	auto hWnd = g_nuiGi->GetHWND();
	auto dpiScale = GetDpiScaleForHwnd(hWnd);

	int width = 0;
	int height = 0;
	auto pixelData = LoadFileBitmap("citizen:/resources/citizen_cursor.png", &width, &height, dpiScale);

	if (!pixelData.empty())
	{
		BITMAPINFO bitmapInfo = { 0 };
		bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmapInfo.bmiHeader.biWidth = width;
		bitmapInfo.bmiHeader.biHeight = -height;
		bitmapInfo.bmiHeader.biPlanes = 1;
		bitmapInfo.bmiHeader.biBitCount = 32;
		bitmapInfo.bmiHeader.biCompression = BI_RGB;
		bitmapInfo.bmiHeader.biSizeImage = 0;
		bitmapInfo.bmiHeader.biXPelsPerMeter = 1;
		bitmapInfo.bmiHeader.biYPelsPerMeter = 1;
		bitmapInfo.bmiHeader.biClrUsed = 0;
		bitmapInfo.bmiHeader.biClrImportant = 0;

		HDC workingDC;
		HBITMAP bitmap;

		{
			auto dc = GetDC(hWnd);
			workingDC = CreateCompatibleDC(dc);
			bitmap = CreateDIBSection(dc, &bitmapInfo, DIB_RGB_COLORS, 0, 0, 0);

			ReleaseDC(hWnd, dc);
		}

		SetDIBits(0, bitmap, 0, height, pixelData.data(), &bitmapInfo, DIB_RGB_COLORS);

		// mask
		HBITMAP oldBitmap = HBITMAP(SelectObject(workingDC, bitmap));
		SetBkMode(workingDC, TRANSPARENT);
		SelectObject(workingDC, oldBitmap);

		HBITMAP mask = CreateBitmap(width, height, 1, 1, NULL);
		ICONINFO ii = { 0 };
		ii.fIcon = FALSE;
		ii.xHotspot = 0;
		ii.yHotspot = 0;
		ii.hbmMask = mask;
		ii.hbmColor = bitmap;

		HICON icon = CreateIconIndirect(&ii);

		DeleteDC(workingDC);
		DeleteObject(mask);
		DeleteObject(bitmap);

		return icon;
	}

	return NULL;
}

static CitizenNUIBackground* g_nuiBackground = new CitizenNUIBackground();

static HookFunction initFunction([] ()
{
	g_nuiBackground->Initialize();
});
