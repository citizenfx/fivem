#include <StdInc.h>
#include <Hooking.h>

#include <CoreConsole.h>
#include <sysAllocator.h>

#include <DirectXTex/DirectXTex.h>

#include <d3d9.h> // for D3DFMT
#include <dxgi.h> // for DXGI_FORMAT

#include "Streaming.h"

static hook::cdecl_stub<DXGI_FORMAT(int, uint8_t)> _mapD3DFMT([]
{
	return hook::get_call(hook::get_pattern("8A 53 5F 8B 4B 58 E8 ? ? ? ? B9", 6));
});

#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE

#define RAGE_FORMATS_IN_GAME
#include "../../rage-formats-x/include/rmcDrawable.h"

namespace rage
{
using grcTexturePC = rage::five::grcTexturePC;
}

static rage::grcTexturePC* (*g_texturePCCtor_Resource_orig)(rage::grcTexturePC* texture, void* resource);
static ptrdiff_t (*_resolvePtr_Orig)(void* resource, uintptr_t ptr);

static thread_local bool ignoreNextPtr = false;

extern std::string GetCurrentStreamingName();
extern uint32_t GetCurrentStreamingIndex();
extern bool IsHandleCache(uint32_t handle, std::string* outFileName);

static ptrdiff_t grcTextureDX11_resolvePtr(void* resource, uintptr_t ptr)
{
	if (ignoreNextPtr)
	{
		ignoreNextPtr = false;
		return 0;
	}

	return _resolvePtr_Orig(resource, ptr);
}

static void* grcTexturePC_CtorWrap(rage::grcTexturePC* texture, void* resource)
{
	texture = g_texturePCCtor_Resource_orig(texture, resource);

	if (_strnicmp(texture->GetName(), "script_rt", 9) == 0)
	{
		// format will still be a D3DFMT at this call time
		auto pixelFormat = texture->GetPixelFormat();
		auto dxgiFmt = _mapD3DFMT(pixelFormat, *((uint8_t*)texture + 95));

		// as to the stride check:
		//     it seems R* at times makes this mistake as well, and their resource compiler compensates
		//     for this by allocating a larger buffer, but not fixing the format.
		if (DirectX::IsCompressed(dxgiFmt) && texture->GetStride() < (texture->GetWidth() * 4))
		{
			// wrong, these should never be compressed
			uintptr_t& dataPtr = *(uintptr_t*)((char*)texture + 112);
			dataPtr += _resolvePtr_Orig(resource, dataPtr);
			ignoreNextPtr = true;

			DirectX::Image image;
			image.format = dxgiFmt;
			image.width = texture->GetWidth();
			image.height = texture->GetHeight();
			image.rowPitch = texture->GetStride();
			image.slicePitch = image.rowPitch * image.height;
			image.pixels = (uint8_t*)dataPtr;

			DirectX::ScratchImage tgtImage;

			// first uncompress
			HRESULT hr = DirectX::Decompress(image, DXGI_FORMAT_R8G8B8A8_UNORM, tgtImage);

			if (FAILED(hr))
			{
				// ... failed?!
				__debugbreak();
			}

			// get the future allocator
			auto globalAllocator = rage::GetAllocator();
			auto otherAllocator = globalAllocator->GetAllocator(1);
			auto streamingAllocator = globalAllocator->GetAllocator(3);

			void* data = (void*)dataPtr;

			// then free the original buffer (if applicable)
			// there's a & 0x10 check but this only matters *after* we try to create resources
			if (!streamingAllocator->GetSize(data))
			{
				globalAllocator->TryFree(data);
			}

			// allocate the new buffer from the 'other' allocator (as the streaming allocator
			// will be treated as if it's part of the main buffer)
			auto newData = otherAllocator->Allocate(tgtImage.GetPixelsSize(), 16, 0);
			memcpy(newData, tgtImage.GetPixels(), tgtImage.GetPixelsSize());

			// set the new data pointer
			dataPtr = (uintptr_t)newData;

			// finally, set a more sensible format
			*(uint32_t*)((char*)texture + 88) = D3DFMT_A8B8G8R8;

			// log what we did
			std::string resourceName;

			if (auto index = GetCurrentStreamingIndex(); index != 0)
			{
				auto handle = streaming::Manager::GetInstance()->Entries[index].handle;
				std::string cacheName;
				if (IsHandleCache(handle, &cacheName))
				{
					auto slashIndex = cacheName.find('/') + 1;
					auto secondSlashIndex = cacheName.find('/', slashIndex);

					resourceName = cacheName.substr(slashIndex, secondSlashIndex - slashIndex);
				}
			}

			auto channel = (!resourceName.empty()) ? fmt::sprintf("script:%s:stream", resourceName) : "streaming";
			console::PrintWarning(channel, "Texture %s (in txd %s) was set to a compressed texture format, but 'script_rt' textures should always be uncompressed.\nThis file was likely processed by a bad tool. To improve load performance and reduce the risk of it crashing, fix or update the tool used.\n", texture->GetName(), GetCurrentStreamingName());
		}
	}

	return texture;
}

static HookFunction hookFunction([]
{
	auto location = hook::get_pattern<char>("8A 53 5F 8B 4B 58 E8 ? ? ? ? B9", -0x34);
	hook::set_call(&g_texturePCCtor_Resource_orig, location + 16);
	hook::call(location + 16, grcTexturePC_CtorWrap);

	hook::set_call(&_resolvePtr_Orig, location + 0x2B);
	hook::call(location + 0x2B, grcTextureDX11_resolvePtr);
});
