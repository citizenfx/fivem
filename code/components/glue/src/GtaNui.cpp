#include <StdInc.h>
#include <CefOverlay.h>

#include <fiDevice.h>
#include <DrawCommands.h>
#include <grcTexture.h>
#include <InputHook.h>

#include <HostSharedData.h>
#include <CfxState.h>
#include <CrossBuildRuntime.h>

#include <tbb/concurrent_queue.h>

#include "ResumeComponent.h"

#include <wrl.h>

#if __has_include(<GameAudioState.h>)
#include <GameAudioState.h>
#endif

namespace WRL = Microsoft::WRL;

using nui::GITexture;
using nui::GITextureFormat;
using nui::ResultingRectangle;

class GtaNuiInterface : public nui::GameInterface
{
private:
	uint32_t m_oldBlendState;

	uint32_t m_oldRasterizerState;

	uint32_t m_oldDepthStencilState;

	bool m_targetMouseFocus = true;

	bool m_lastTargetMouseFocus = true;

public:
	virtual void GetGameResolution(int* width, int* height) override;

	virtual fwRefContainer<GITexture> CreateTexture(int width, int height, GITextureFormat format, void* pixelData) override;

	virtual fwRefContainer<GITexture> CreateTextureBacking(int width, int height, GITextureFormat format) override;

	virtual fwRefContainer<GITexture> CreateTextureFromShareHandle(HANDLE shareHandle) override
	{
		assert(!"don't do that on vulkan games");

		return nullptr;
	}

	virtual fwRefContainer<GITexture> CreateTextureFromShareHandle(HANDLE shareHandle, int width, int height) override;

	virtual void SetTexture(fwRefContainer<GITexture> texture, bool pm) override;

	virtual void DrawRectangles(int numRectangles, const ResultingRectangle* rectangles) override;

	virtual void UnsetTexture() override;

	virtual void SetGameMouseFocus(bool val) override
	{
		m_targetMouseFocus = val;
	}

	void UpdateMouseFocus()
	{
		if (m_targetMouseFocus != m_lastTargetMouseFocus)
		{
			InputHook::SetGameMouseFocus(m_targetMouseFocus);
			m_lastTargetMouseFocus = m_targetMouseFocus;
		}
	}

	virtual HWND GetHWND() override
	{
		return CoreGetGameWindow();
	}

	virtual void BlitTexture(fwRefContainer<GITexture> dst, fwRefContainer<GITexture> src) override
	{
#ifdef GTA_FIVE
		::GetD3D11DeviceContext()->CopyResource((ID3D11Resource*)dst->GetNativeTexture(), (ID3D11Resource*)src->GetNativeTexture());
#endif
	}

	virtual ID3D11Device* GetD3D11Device() override
	{
#ifdef GTA_FIVE
		return ::GetD3D11Device();
#endif

		return NULL;
	}

	virtual ID3D11DeviceContext* GetD3D11DeviceContext() override
	{
#ifdef GTA_FIVE
		return ::GetD3D11DeviceContext();
#endif

		return NULL;
	}

	virtual fwRefContainer<GITexture> CreateTextureFromD3D11Texture(ID3D11Texture2D* texture) override
	{
		// unused
		return NULL;
	}

	virtual bool RequestMediaAccess(const std::string& frameOrigin, const std::string& url, int permissions, const std::function<void(bool, int)>& onComplete) override;

#ifdef INPUT_HOOK_HOST_CURSOR_SUPPORT
	virtual bool CanDrawHostCursor() override;

	virtual void SetHostCursorEnabled(bool enabled) override;

	virtual void SetHostCursor(HCURSOR cursor) override;
#endif
};

static tbb::concurrent_queue<std::function<void()>> g_onRenderQueue;
static tbb::concurrent_queue<std::function<void()>> g_earlyOnRenderQueue;
static std::mutex g_frontendDeletionMutex;

class GtaNuiTexture : public nui::GITexture
{
private:
	rage::grcTexture* m_texture;

	std::shared_ptr<GtaNuiTexture*> m_canary;

	bool m_overriddenTexture;

	bool m_overriddenSRV;

public:
	explicit GtaNuiTexture(rage::grcTexture* texture)
		: m_texture(texture), m_overriddenTexture(false), m_overriddenSRV(false)
	{

	}

	explicit GtaNuiTexture(std::function<rage::grcTexture*(GtaNuiTexture*)> fn)
		: m_texture(nullptr), m_overriddenTexture(false), m_overriddenSRV(false)
	{
		m_canary = std::make_shared<GtaNuiTexture*>(this);

		// make a weak reference to the class pointer, so if it gets `delete`d, we can just ignore this creation attempt
		std::weak_ptr<GtaNuiTexture*> weakCanary = m_canary;

		g_onRenderQueue.push([weakCanary, fn]()
		{
			std::unique_lock<std::mutex> lock(g_frontendDeletionMutex);
			auto ref = weakCanary.lock();

			if (ref)
			{
				(*ref)->m_texture = fn(*ref);
			}
			else
			{

			}
		});
	}

	virtual ~GtaNuiTexture()
	{
		auto texture = m_texture;
		m_texture = nullptr;

#ifdef GTA_FIVE
		// if overridden stuff i.e. not managed by grcResourceCache, manually release
		if (m_overriddenTexture)
		{
			auto baseTexture = texture->texture;

			if (baseTexture)
			{
				g_onRenderQueue.push([baseTexture]()
				{
					baseTexture->Release();
				});

				texture->texture = NULL;
			}
		}

		if (m_overriddenSRV)
		{
			auto baseSRV = texture->srv;

			if (baseSRV)
			{
				g_onRenderQueue.push([baseSRV]()
				{
					baseSRV->Release();
				});

				texture->srv = NULL;
			}
		}
#endif

		g_onRenderQueue.push([texture]()
		{
			delete texture;
		});
	}

	inline void MarkOverriddenTexture()
	{
		m_overriddenTexture = true;
	}

	inline void MarkOverriddenSRV()
	{
		m_overriddenSRV = true;
	}

	inline rage::grcTexture* GetTexture() { return m_texture; }

	virtual void* GetNativeTexture() override
	{
#ifdef GTA_FIVE
		if (!m_texture)
		{
			return nullptr;
		}

		return m_texture->texture;
#else
		return m_texture;
#endif
	}
	virtual void* GetHostTexture() override
	{
		return m_texture;
	}

	virtual bool Map(int numSubLevels, int subLevel, nui::GILockedTexture* lockedTexture, nui::GILockFlags flags) override
	{
#ifdef GTA_FIVE
		rage::grcLockedTexture rlt;

		if (m_texture->Map(numSubLevels, subLevel, &rlt, (rage::grcLockFlags)flags))
		{
			lockedTexture->format = rlt.format;
			lockedTexture->height = rlt.height;
			lockedTexture->level = rlt.level;
			lockedTexture->numSubLevels = rlt.numSubLevels;
			lockedTexture->pBits = rlt.pBits;
			lockedTexture->pitch = rlt.pitch;
			lockedTexture->width = rlt.width;

			return true;
		}
#elif defined(GTA_NY)
		if (!m_texture || !m_texture->m_pITexture)
		{
			return false;
		}

		D3DLOCKED_RECT lockedRect;
		if (SUCCEEDED(m_texture->m_pITexture->LockRect(0, &lockedRect, NULL, (flags == nui::GILockFlags::WriteDiscard) ? D3DLOCK_DISCARD : 0)))
		{
			lockedTexture->pBits = lockedRect.pBits;
			lockedTexture->pitch = lockedRect.Pitch;

			return true;
		}
#endif

		return false;
	}

	virtual void Unmap(nui::GILockedTexture* lockedTexture) override
	{
#ifdef GTA_FIVE
		rage::grcLockedTexture rlt;
		rlt.format = lockedTexture->format;
		rlt.height = lockedTexture->height;
		rlt.level = lockedTexture->level;
		rlt.numSubLevels = lockedTexture->numSubLevels;
		rlt.pBits = lockedTexture->pBits;
		rlt.pitch = lockedTexture->pitch;
		rlt.width = lockedTexture->width;

		m_texture->Unmap(&rlt);
#elif defined(GTA_NY)
		m_texture->m_pITexture->UnlockRect(0);
#endif
	}
};

void GtaNuiInterface::GetGameResolution(int* width, int* height)
{
	int w, h;
	::GetGameResolution(w, h);

	if (w == 0 || h == 0)
	{
		w = 1919;
		h = 1080;
	}

	*width = w;
	*height = h;
}

fwRefContainer<GITexture> GtaNuiInterface::CreateTexture(int width, int height, GITextureFormat format, void* pixelData)
{
#ifdef GTA_FIVE
	rage::sysMemAllocator::UpdateAllocatorValue();

	rage::grcTextureReference reference;
	memset(&reference, 0, sizeof(reference));
	reference.width = width;
	reference.height = height;
	reference.depth = 1;
	reference.stride = width * 4;
	reference.format = (format == GITextureFormat::ARGB) ? 11 : -1; // dxt5?
	reference.pixelData = (uint8_t*)pixelData;

	rage::grcTexture* texture = rage::grcTextureFactory::getInstance()->createImage(&reference, nullptr);

	return new GtaNuiTexture(texture);
#else
	auto pixelMem = std::make_shared<std::vector<uint8_t>>(width * height * 4);
	memcpy(pixelMem->data(), pixelData, pixelMem->size());

	return new GtaNuiTexture([width, height, format, pixelMem](GtaNuiTexture*)
	{
		rage::grcTextureReference reference;
		memset(&reference, 0, sizeof(reference));
		reference.width = width;
		reference.height = height;
		reference.depth = 1;
		reference.stride = width * 4;
#if GTA_NY
		reference.format = 1;
#else
		reference.format = (format == GITextureFormat::ARGB) ? 11 : -1; // dxt5?
#endif
		reference.pixelData = (uint8_t*)pixelMem->data();

		return rage::grcTextureFactory::getInstance()->createImage(&reference, nullptr);
	});
#endif
}

fwRefContainer<GITexture> GtaNuiInterface::CreateTextureBacking(int width, int height, GITextureFormat format)
{
#ifndef GTA_NY
	rage::sysMemAllocator::UpdateAllocatorValue();
#endif

	assert(format == GITextureFormat::ARGB);

#if defined(GTA_FIVE)
	rage::grcManualTextureDef textureDef;
	memset(&textureDef, 0, sizeof(textureDef));
	textureDef.isStaging = 0;
	textureDef.arraySize = 1;

	return new GtaNuiTexture(rage::grcTextureFactory::getInstance()->createManualTexture(width, height, 2 /* maps to BGRA DXGI format */, nullptr, true, &textureDef));
#else
	return new GtaNuiTexture([width, height](GtaNuiTexture*)
	{
		rage::grcManualTextureDef textureDef;
		memset(&textureDef, 0, sizeof(textureDef));
		textureDef.isStaging = 0;
		textureDef.arraySize = 1;

#ifdef GTA_NY
		return rage::grcTextureFactory::getInstance()->createManualTexture(width, height, FORMAT_A8R8G8B8, true, &textureDef);
#else
		return rage::grcTextureFactory::getInstance()->createManualTexture(width, height, 2 /* maps to BGRA DXGI format */, nullptr, true, &textureDef);
#endif
	});
#endif
}

#include <d3d12.h>

#pragma comment(lib, "vulkan-1.lib")

fwRefContainer<GITexture> GtaNuiInterface::CreateTextureFromShareHandle(HANDLE shareHandle, int width, int height)
{
#ifndef GTA_NY
	rage::sysMemAllocator::UpdateAllocatorValue();
#endif

#ifdef GTA_FIVE
	ID3D11Device* device = ::GetD3D11Device();

	WRL::ComPtr<ID3D11Texture2D> texture;
	if (SUCCEEDED(device->OpenSharedResource(shareHandle, IID_PPV_ARGS(&texture))))
	{
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);

		struct
		{
			void* vtbl;
			ID3D11Device* rawDevice;
		}*deviceStuff = (decltype(deviceStuff))device;

		rage::grcManualTextureDef textureDef;
		memset(&textureDef, 0, sizeof(textureDef));
		textureDef.isStaging = 0;
		textureDef.arraySize = 1;

		auto texRef = rage::grcTextureFactory::getInstance()->createManualTexture(desc.Width, desc.Height, 2 /* maps to BGRA DXGI format */, nullptr, true, &textureDef);

		if (texRef)
		{
			if (texRef->texture)
			{
#ifdef GTA_FIVE
				rage::grcResourceCache::GetInstance()->QueueDelete(texRef->texture);
				
				g_onRenderQueue.push([]()
				{
					rage::grcResourceCache::GetInstance()->FlushQueue();
				});
#else
				texRef->texture->Release();
#endif
				texRef->texture = NULL;
			}

			texture.CopyTo(&texRef->texture);

			if (texRef->srv)
			{
				texRef->srv->Release();
			}

			deviceStuff->rawDevice->CreateShaderResourceView(texture.Get(), nullptr, &texRef->srv);
		}

		auto texture = new GtaNuiTexture(texRef);
		texture->MarkOverriddenSRV();
		texture->MarkOverriddenTexture();

		return texture;
	}
#elif GTA_NY
	// ?
#else
	if (GetCurrentGraphicsAPI() == GraphicsAPI::D3D12)
	{
		ID3D12Device* device = (ID3D12Device*)GetGraphicsDriverHandle();

		ID3D12Resource* resource = nullptr;
		if (SUCCEEDED(device->OpenSharedHandle(shareHandle, __uuidof(ID3D12Resource), (void**)&resource)))
		{
			return new GtaNuiTexture([resource, shareHandle](GtaNuiTexture* texture)
			{
				ID3D12Resource* oldTexture = nullptr;

				auto desc = resource->GetDesc();

				auto width = desc.Width;
				auto height = desc.Height;

				rage::grcManualTextureDef textureDef;
				memset(&textureDef, 0, sizeof(textureDef));
				textureDef.isStaging = 0;
				textureDef.arraySize = 1;

				auto texRef = (rage::sga::TextureD3D12*)rage::grcTextureFactory::getInstance()->createManualTexture(width, height, 2, nullptr, true, &textureDef);

				if (texRef)
				{
					rage::sga::Driver_Destroy_Texture(texRef);

					texRef->resource = resource;

					rage::sga::TextureViewDesc srvDesc;
					srvDesc.mipLevels = 1;
					srvDesc.arrayStart = 0;
					srvDesc.dimension = 4;
					srvDesc.arraySize = 1;

					rage::sga::Driver_Create_ShaderResourceView(texRef, srvDesc);

					CloseHandle(shareHandle);
				}

				return (rage::grcTexture*)texRef;
			});
		}
	}
	else if (GetCurrentGraphicsAPI() == GraphicsAPI::Vulkan)
	{
		// meanwhile in Vulkan, this is infinitely annoying
		return new GtaNuiTexture([shareHandle, width, height](GtaNuiTexture* texture)
		{
			std::vector<uint8_t> pixelData(size_t(width) * size_t(height) * 4);

			rage::grcTextureReference reference;
			memset(&reference, 0, sizeof(reference));
			reference.width = width;
			reference.height = height;
			reference.depth = 1;
			reference.stride = width * 4;
			reference.format = 11;
			reference.pixelData = (uint8_t*)pixelData.data();

			auto texRef = (rage::sga::TextureVK*)rage::grcTextureFactory::getInstance()->createImage(&reference, nullptr);

			if (texRef)
			{
				rage::sga::Driver_Destroy_Texture(texRef);

				// Vulkan API magic time (copy/pasted from samples on GH)
				VkDevice device = (VkDevice)GetGraphicsDriverHandle();
				
				VkExtent3D Extent = { width, height, 1 };

				VkExternalMemoryImageCreateInfo ExternalMemoryImageCreateInfo = { VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO };
				ExternalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
				VkImageCreateInfo ImageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
				ImageCreateInfo.pNext = &ExternalMemoryImageCreateInfo;
				ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
				ImageCreateInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
				ImageCreateInfo.extent = Extent;
				ImageCreateInfo.mipLevels = 1;
				ImageCreateInfo.arrayLayers = 1;
				ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
				ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				ImageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
				ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				VkImage Image;
				assert(vkCreateImage(device, &ImageCreateInfo, nullptr, &Image) == VK_SUCCESS);

				VkMemoryRequirements MemoryRequirements;
				vkGetImageMemoryRequirements(device, Image, &MemoryRequirements);

				VkMemoryDedicatedAllocateInfo MemoryDedicatedAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO };
				MemoryDedicatedAllocateInfo.image = Image;
				VkImportMemoryWin32HandleInfoKHR ImportMemoryWin32HandleInfo = { VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR };
				ImportMemoryWin32HandleInfo.pNext = &MemoryDedicatedAllocateInfo;
				ImportMemoryWin32HandleInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
				ImportMemoryWin32HandleInfo.handle = shareHandle;
				VkMemoryAllocateInfo MemoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
				MemoryAllocateInfo.pNext = &ImportMemoryWin32HandleInfo;
				MemoryAllocateInfo.allocationSize = MemoryRequirements.size;

				unsigned long typeIndex;
				_BitScanForward(&typeIndex, MemoryRequirements.memoryTypeBits);
				MemoryAllocateInfo.memoryTypeIndex = typeIndex;

				static auto _vkBindImageMemory2 = (PFN_vkBindImageMemory2)vkGetDeviceProcAddr(device, "vkBindImageMemory2");

				VkDeviceMemory ImageMemory;
				assert(vkAllocateMemory(device, &MemoryAllocateInfo, nullptr, &ImageMemory) == VK_SUCCESS);
				VkBindImageMemoryInfo BindImageMemoryInfo = { VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO };
				BindImageMemoryInfo.image = Image;
				BindImageMemoryInfo.memory = ImageMemory;
				assert(_vkBindImageMemory2(device, 1, &BindImageMemoryInfo) == VK_SUCCESS);

				auto newImage = new rage::sga::TextureVK::ImageData;
				//memcpy(newImage, texRef->image, sizeof(*newImage));
				memset(newImage, 0, sizeof(*newImage));
				// these come from a fast allocator(?)
				//delete texRef->image;
				texRef->image = newImage;

				texRef->image->image = Image;
				texRef->image->memory = ImageMemory;

				rage::sga::TextureViewDesc srvDesc;
				srvDesc.mipLevels = 1;
				srvDesc.arrayStart = 0;
				srvDesc.dimension = 4;
				srvDesc.arraySize = 1;

				rage::sga::Driver_Create_ShaderResourceView(texRef, srvDesc);

				CloseHandle(shareHandle);
			}

			return (rage::grcTexture*)texRef;
		});
	}
#endif

	return new GtaNuiTexture(nullptr);
}

static thread_local fwRefContainer<nui::GITexture> g_currentTexture;

void GtaNuiInterface::SetTexture(fwRefContainer<GITexture> texture, bool pm)
{
#ifndef GTA_NY
	rage::sysMemAllocator::UpdateAllocatorValue();
#endif

	g_currentTexture = texture;

	SetTextureGtaIm(static_cast<GtaNuiTexture*>(texture.GetRef())->GetTexture());

#if _HAVE_GRCORE_NEWSTATES
	m_oldRasterizerState = GetRasterizerState();
	SetRasterizerState(GetStockStateIdentifier(RasterizerStateNoCulling));

	m_oldBlendState = GetBlendState();
	SetBlendState(GetStockStateIdentifier(pm ? BlendStatePremultiplied : BlendStateDefault));

	m_oldDepthStencilState = GetDepthStencilState();
	SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));
#else
	SetRenderState(0, grcCullModeNone);
	SetRenderState(2, pm ? 13 : 0); // alpha blending m8
#endif

	PushDrawBlitImShader();
}

void GtaNuiInterface::DrawRectangles(int numRectangles, const nui::ResultingRectangle* rectangles)
{
	for (int i = 0; i < numRectangles; i++)
	{
		auto rectangle = &rectangles[i];

		rage::grcBegin(4, 4);

		auto& rect = rectangle->rectangle;
		uint32_t color = *(uint32_t*)&rectangle->color;

		// this swaps ABGR (as CRGBA is ABGR in little-endian) to ARGB by rotating left
		if (!rage::grcTexture::IsRenderSystemColorSwapped())
		{
			color = (color & 0xFF00FF00) | _rotl(color & 0x00FF00FF, 16);
		}

		auto u1 = 0.0f;
		auto v1 = 0.0f;
		auto u2 = 1.0f;
		auto v2 = 1.0f;

		rage::grcVertex(rect.fX1, rect.fY1, 0.0f, 0.0f, 0.0f, -1.0f, color, u1, v1);
		rage::grcVertex(rect.fX2, rect.fY1, 0.0f, 0.0f, 0.0f, -1.0f, color, u2, v1);
		rage::grcVertex(rect.fX1, rect.fY2, 0.0f, 0.0f, 0.0f, -1.0f, color, u1, v2);
		rage::grcVertex(rect.fX2, rect.fY2, 0.0f, 0.0f, 0.0f, -1.0f, color, u2, v2);

		rage::grcEnd();
	}
}

void GtaNuiInterface::UnsetTexture()
{
	PopDrawBlitImShader();

#ifdef _HAVE_GRCORE_NEWSTATES
	SetRasterizerState(m_oldRasterizerState);
	SetBlendState(m_oldBlendState);
	SetDepthStencilState(m_oldDepthStencilState);
#endif

	g_currentTexture = {};
}

extern bool HandleMediaRequest(const std::string& frameOrigin, const std::string& url, int permissions, const std::function<void(bool, int)>& onComplete);

bool GtaNuiInterface::RequestMediaAccess(const std::string& frameOrigin, const std::string& url, int permissions, const std::function<void(bool, int)>& onComplete)
{
	return HandleMediaRequest(frameOrigin, url, permissions, onComplete);
}

#ifdef INPUT_HOOK_HOST_CURSOR_SUPPORT
bool GtaNuiInterface::CanDrawHostCursor()
{
	return true;
}

void GtaNuiInterface::SetHostCursor(HCURSOR cursor)
{
	SetClassLongPtr(CoreGetGameWindow(), GCLP_HCURSOR,
		static_cast<LONG>(reinterpret_cast<LONG_PTR>(cursor)));

	SetCursor(cursor);
}

void GtaNuiInterface::SetHostCursorEnabled(bool enabled)
{
	static bool wasEnabled;

	if (!enabled && wasEnabled)
	{
		SetClassLongPtr(CoreGetGameWindow(), GCLP_HCURSOR,
			static_cast<LONG>(reinterpret_cast<LONG_PTR>(IDC_ARROW)));
	}

	wasEnabled = enabled;

	InputHook::SetHostCursorEnabled(enabled);
}
#endif

static GtaNuiInterface nuiGi;

static void DoRender()
{
	std::function<void()> fn;

	while (g_earlyOnRenderQueue.try_pop(fn))
	{
		fn();
	}

	while (g_onRenderQueue.try_pop(fn))
	{
		fn();
	}

	nuiGi.OnRender();
}

static InitFunction initFunction([]()
{
#if __has_include(<GameAudioState.h>)
	nuiGi.QueryShouldMute.Connect([](bool& shouldMute)
	{
		shouldMute = shouldMute || ShouldMuteGameAudio();
	});
#endif

	OnGrcCreateDevice.Connect([]()
	{
		nuiGi.OnInitRenderer();
	});

	OnPostFrontendRender.Connect([]()
	{
		if (IsOnRenderThread())
		{
			DoRender();
		}
		else
		{
			uintptr_t a = 0, b = 0;
			EnqueueGenericDrawCommand([](uintptr_t, uintptr_t)
			{
				DoRender();
			}, &a, &b);
		}
		
#if !IS_RDR3 && !GTA_NY
		if (nui::HasMainUI() || nui::HasFrame("loadingScreen"))
		{
			GfxForceVsync(true);
		}
		else
		{
			GfxForceVsync(false);
		}
#endif

		nuiGi.UpdateMouseFocus();
	}, -1000);

	rage::fiDevice::OnInitialMount.Connect([]()
	{
		nuiGi.OnInitVfs();
	}, 100);

	static HostSharedData<CfxState> initState("CfxInitState");

	if (initState->IsGameProcess())
	{
		nui::Initialize(&nuiGi);
	}

	InputHook::QueryInputTarget.Connect([](std::vector<InputTarget*>& targets)
	{
		return nuiGi.QueryInputTarget(targets);
	}, 20);

	InputHook::DeprecatedOnWndProc.Connect([](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& pass, LRESULT& lresult)
	{
		nuiGi.OnWndProc(hWnd, msg, wParam, lParam, pass, lresult);
	}, 20);

	InputHook::QueryMayLockCursor.Connect([](int& a)
	{
		nuiGi.QueryMayLockCursor(a);
	});
});
