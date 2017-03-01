/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <DrawCommands.h>
#include <grcTexture.h>
#include <RGBA.h>

#include <wrl.h>
#include <wincodec.h>

#include <boost/bimap.hpp>

#include <mmsystem.h>

#include <mutex>

#include <GfxUtil.h>

#include <Error.h>

using namespace Microsoft::WRL;

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "windowscodecs.lib")

// API for compatibility with the ViSH 'texture' API.

// FIXME: copied from nui:core - might need to be shared?
static rage::grcTexture* InitializeTextureFromFile(const std::wstring& filename)
{
	ComPtr<IWICBitmapDecoder> decoder;
	ComPtr<IWICImagingFactory> imagingFactory;

	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void**)imagingFactory.GetAddressOf());

	if (FAILED(hr))
	{
		return nullptr;
	}

	hr = imagingFactory->CreateDecoderFromFilename(filename.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());

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
				
				// swap the pixels before writing
				std::vector<uint8_t> inDataConverted;
				inDataConverted.resize(width * height * 4);

				ConvertImageDataRGBA_BGRA(0, 0, width, height, width * 4, pixelData, width * 4, &inDataConverted[0]);

				delete[] pixelData;

				reference.pixelData = (uint8_t*)&inDataConverted[0];

				return rage::grcTextureFactory::getInstance()->createImage(&reference, nullptr);
			}
		}
	}

	return nullptr;
}

struct VishTexture
{
	struct Instance
	{
		int level;
		uint32_t expire;
		float size[2];
		float center[2];
		float pos[2];
		float rotation;
		CRGBA color;
		float aspectValue;
	};

	rage::grcTexture* texture;
	std::map<int, Instance> instances;

	VishTexture()
		: texture(nullptr)
	{

	}

	VishTexture(rage::grcTexture* texture)
		: texture(texture)
	{

	}
};

static std::map<int, VishTexture> g_vishTextures;
static int g_vishTextureId;

// to bind textures to existing IDs
static boost::bimap<int, std::string> g_vishTextureIds;

static std::mutex g_textureLock;

DLL_EXPORT int createTexture(const char* fileName)
{
	// lock
	std::unique_lock<std::mutex> lock(g_textureLock);

	// look up if a texture with this 'name' already exists, if so, return it
 	std::string fileNameStr = fileName;
 
 	auto it = g_vishTextureIds.right.find(fileNameStr);
 
 	if (it != g_vishTextureIds.right.end())
 	{
 		return it->second;
 	}

	// convert the filename from UTF-8...
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	std::wstring passedFileName = converter.from_bytes(fileNameStr);
	std::wstring retFileName = passedFileName;

	// then, try finding the requested file in various locations
	bool found = false;

	// absolute path?
	if (passedFileName[1] == L':' || passedFileName[0] == '\\')
	{
		found = true;
	}

	// plugins directory path?
	if (!found)
	{
		retFileName = MakeRelativeCitPath(va(L"plugins\\%s", passedFileName.c_str()));

		found = (GetFileAttributes(retFileName.c_str()) != INVALID_FILE_ATTRIBUTES);
	}

	// root Citizen directory?
	if (!found)
	{
		retFileName = MakeRelativeCitPath(passedFileName);

		found = (GetFileAttributes(retFileName.c_str()) != INVALID_FILE_ATTRIBUTES);
	}

	// root game directory?
	if (!found)
	{
		retFileName = MakeRelativeGamePath(passedFileName);

		found = (GetFileAttributes(retFileName.c_str()) != INVALID_FILE_ATTRIBUTES);
	}

	// nowhere at all?
	if (!found)
	{
		return -1;
	}

	// create the texture
	auto texture = InitializeTextureFromFile(retFileName);

	if (!texture)
	{
		GlobalError("Failed to initialize ViSH compatibility texture %s.", converter.to_bytes(retFileName).c_str());

		return -1;
	}

	int thisId = g_vishTextureId++;
	g_vishTextures[thisId] = VishTexture{ texture };
	g_vishTextureIds.insert({ thisId, fileName });

	return thisId;
}

DLL_EXPORT void drawTexture(int id, int index, int level, int time,
							float sizeX, float sizeY, float centerX, float centerY,
							float posX, float posY, float rotation, float screenHeightScaleFactor,
							float r, float g, float b, float a)
{
	// lock
	std::unique_lock<std::mutex> lock(g_textureLock);

	// check if existent
	auto it = g_vishTextures.find(id);

	if (it == g_vishTextures.end())
	{
		return;
	}

	// store ref
	auto& texture = it->second;

	VishTexture::Instance instance;
	instance.level = level;
	instance.expire = timeGetTime() + time;
	instance.size[0] = sizeX;
	instance.size[1] = sizeY;
	instance.center[0] = centerX;
	instance.center[1] = centerY;
	instance.pos[0] = posX;
	instance.pos[1] = posY;
	instance.rotation = rotation;
	instance.aspectValue = screenHeightScaleFactor;
	instance.color = CRGBA::FromFloat(r, g, b, a);

	texture.instances[index] = instance;
}

static InitFunction initFunction([] ()
{
	OnPostFrontendRender.Connect([] ()
	{
		uintptr_t a1;
		uintptr_t a2;

		EnqueueGenericDrawCommand([] (uintptr_t, uintptr_t)
		{
			uint32_t renderTime = timeGetTime();
			std::vector<std::pair<rage::grcTexture*, VishTexture::Instance>> drawList;

			// get a draw list during the lock
			{
				std::unique_lock<std::mutex> lock(g_textureLock);

				for (auto& texture : g_vishTextures)
				{
					for (auto& instance : texture.second.instances)
					{
						if (renderTime <= instance.second.expire)
						{
							drawList.push_back({ texture.second.texture, instance.second });
						}
					}
				}
			}

			// sort the draw list by 'global level'
			std::sort(drawList.begin(), drawList.end(), [] (const auto& left, const auto& right)
			{
				return left.second.level < right.second.level;
			});

			// render based on the draw list
			auto oldBlendState = GetBlendState();
			SetBlendState(GetStockStateIdentifier(BlendStateDefault));

			auto oldRasterizerState = GetRasterizerState();
			SetRasterizerState(GetStockStateIdentifier(StateType::RasterizerStateNoCulling));

			auto oldDepthStencilState = GetDepthStencilState();
			SetDepthStencilState(GetStockStateIdentifier(StateType::DepthStencilStateNoDepth));

			// get screen resolution
			int width, height;
			GetGameResolution(width, height);

			for (auto& entry : drawList)
			{
				SetTextureGtaIm(entry.first);

				PushDrawBlitImShader();

				uint32_t color = entry.second.color.AsARGB();

				// this swaps ABGR (as CRGBA is ABGR in little-endian) to ARGB by rotating left
				color = (color & 0xFF00FF00) | _rotl(color & 0x00FF00FF, 16);

				// calculate verts
				float xG = entry.second.pos[0];
				float yG = entry.second.pos[1];

				float s = sin(entry.second.rotation * (3.14159265358979323846 / 180.0));
				float c = cos(entry.second.rotation * (3.14159265358979323846 / 180.0));

				auto rotatePoint = [&] (float x, float y) -> POINTF
				{
					x -= entry.second.center[0];
					y -= entry.second.center[1];

					float x2 = (x * c) + (y * s);
					float y2 = (-x * s) + (y * c);

					return POINTF{ x2, y2 };
				};

				auto scaleAndRotatePoint = [&] (float x, float y)
				{
					auto p = rotatePoint(x, y);

					return POINTF{ ((p.x * entry.second.size[0]) + xG) * width, ((p.y * entry.second.size[1] * entry.second.aspectValue) + yG) * height };
				};

				POINTF p1 = scaleAndRotatePoint(0.0f, 0.0f);
				POINTF p2 = scaleAndRotatePoint(1.0f, 0.0f);
				POINTF p3 = scaleAndRotatePoint(0.0f, 1.0f);
				POINTF p4 = scaleAndRotatePoint(1.0f, 1.0f);

				BeginImVertices(4, 4);

				AddImVertex(p1.x, p1.y, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 0.0f);
				AddImVertex(p2.x, p2.y, 0.0f, 0.0f, 0.0f, -1.0f, color, 1.0f, 0.0f);
				AddImVertex(p3.x, p3.y, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 1.0f);
				AddImVertex(p4.x, p4.y, 0.0f, 0.0f, 0.0f, -1.0f, color, 1.0f, 1.0f);

				DrawImVertices();

				PopDrawBlitImShader();
			}

			SetDepthStencilState(oldDepthStencilState);
			SetRasterizerState(oldRasterizerState);

			SetBlendState(oldBlendState);
		}, &a1, &a2);
	});
});