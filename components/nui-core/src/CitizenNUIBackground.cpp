/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "CefOverlay.h"

#include <DrawCommands.h>
#include <RGBA.h>

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

class CitizenNUIBackground
{
private:
	rage::grcTexture* m_backdropTexture;
	rage::grcTexture* m_overlayTexture;

	ComPtr<IWICImagingFactory> m_imagingFactory;

	typedef void (WINAPI* DwmpGetColorizationParameters_t)(DWMP_GET_COLORIZATION_PARAMETERS_DATA*);

	DwmpGetColorizationParameters_t m_fDwmpGetColorizationParameters;

private:
	rage::grcTexture* InitializeTextureFromFile(std::wstring filename);

	void EnsureTextures();

	void DrawBackground(rage::grcTexture* texture, CRGBA colorValue);

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

	float maxVal = 255.0f / max(colorizationColor.red, max(colorizationColor.green, colorizationColor.blue));
	
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

	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void**)m_imagingFactory.GetAddressOf());

	if (FAILED(hr))
	{
		return;
	}

	nui::OnDrawBackground.Connect([=] (bool isMainUI)
	{
		EnsureTextures();

		if (isMainUI)
		{
			CRGBA color = GetCurrentColor(true);

			static uint32_t lastColorUpdate;

			if ((GetTickCount() - lastColorUpdate) > 30000)
			{
				CRGBA baseColor = GetCurrentColor(false);

				nui::ExecuteRootScript(va("citFrames[\"mpMenu\"].contentWindow.postMessage({ type: 'setColor', color: [%d, %d, %d] }, '*');", baseColor.red, baseColor.green, baseColor.blue));

				lastColorUpdate = GetTickCount();
			}

			DrawBackground(m_backdropTexture, CRGBA(255, 255, 255, 255));

			DrawBackground(m_overlayTexture, color);
		}
	});
}

void CitizenNUIBackground::DrawBackground(rage::grcTexture* texture, CRGBA colorValue)
{
#ifdef _HAVE_GRCORE_NEWSTATES
	auto oldBlendState = GetBlendState();
	SetBlendState(GetStockStateIdentifier(BlendStateDefault));
#endif

	SetTextureGtaIm(texture);

	int resX, resY;
	GetGameResolution(resX, resY);

	uint32_t color = colorValue.AsARGB();

	DrawImSprite(0.0f, 0.0f, resX, resY, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, &color, 0);

#ifdef _HAVE_GRCORE_NEWSTATES
	SetBlendState(oldBlendState);
#endif
}

rage::grcTexture* g_cursorTexture;

void CitizenNUIBackground::EnsureTextures()
{
	if (!m_backdropTexture)
	{
		m_backdropTexture = InitializeTextureFromFile(MakeRelativeCitPath(L"citizen\\resources\\background_main.jpg"));
	}

	if (!m_overlayTexture)
	{
		m_overlayTexture = InitializeTextureFromFile(MakeRelativeCitPath(L"citizen\\resources\\base_color.png"));
	}

	if (!g_cursorTexture)
	{
		g_cursorTexture = InitializeTextureFromFile(MakeRelativeCitPath(L"citizen\\resources\\citizen_cursor.png"));
	}
}

rage::grcTexture* CitizenNUIBackground::InitializeTextureFromFile(std::wstring filename)
{
	ComPtr<IWICBitmapDecoder> decoder;

	HRESULT hr = m_imagingFactory->CreateDecoderFromFilename(filename.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());

	if (SUCCEEDED(hr))
	{
		ComPtr<IWICBitmapFrameDecode> frame;
		
		hr = decoder->GetFrame(0, frame.GetAddressOf());

		if (SUCCEEDED(hr))
		{
			ComPtr<IWICBitmapSource> source;
			ComPtr<IWICBitmapSource> convertedSource;

			UINT width = 0, height = 0;

			frame->GetSize(&width, &height);

			// try to convert to a pixel format we like
			frame.As(&source);

			hr = WICConvertBitmapSource(GUID_WICPixelFormat32bppRGBA, source.Get(), convertedSource.GetAddressOf());

			if (SUCCEEDED(hr))
			{
				source = convertedSource;
			}

			// create a pixel data buffer
			uint32_t* pixelData = new uint32_t[width * height];

			hr = source->CopyPixels(nullptr, width * 4, width * height * 4, reinterpret_cast<BYTE*>(pixelData));

			if (SUCCEEDED(hr))
			{
				rage::grcTextureReference reference;
				memset(&reference, 0, sizeof(reference));
				reference.width = width;
				reference.height = height;
				reference.depth = 1;
				reference.stride = width * 4;
#ifdef GTA_NY
				reference.format = 1; // RGBA
#elif defined(GTA_FIVE)
				reference.format = 11; // should correspond to DXGI_FORMAT_B8G8R8A8_UNORM
#endif
				reference.pixelData = (uint8_t*)pixelData;

				return rage::grcTextureFactory::getInstance()->createImage(&reference, nullptr);
			}
		}
	}

	return nullptr;
}

static CitizenNUIBackground g_nuiBackground;

static HookFunction initFunction([] ()
{
	g_nuiBackground.Initialize();
});