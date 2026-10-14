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

#include <deque>
#include <utility>
#include <vector>

#if __has_include(<GameAudioState.h>)
#include <GameAudioState.h>
#endif

#ifdef IS_RDR3
#include <VulkanHelper.h>
#endif

#include <Error.h>
#include <variant>

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

	bool m_flushMouse = true;

#ifdef GTA_FIVE
	Microsoft::WRL::ComPtr<ID3D11Device1> m_d3d11Device1 = nullptr;
#endif
public:
	virtual void GetGameResolution(int* width, int* height) override;

	virtual fwRefContainer<GITexture> CreateTexture(int width, int height, GITextureFormat format, void* pixelData) override;

	virtual fwRefContainer<GITexture> CreateTextureBacking(int width, int height, GITextureFormat format) override;

	virtual fwRefContainer<GITexture> CreateTextureFromShareHandle(HANDLE shareHandle, nui::GameInterface::UpdateTextureCB cb = nullptr) override
	{
		assert(!"don't do that on vulkan games");

		return nullptr;
	}

	virtual fwRefContainer<GITexture> CreateTextureFromShareHandle(HANDLE shareHandle, int width, int height, nui::GameInterface::UpdateTextureCB cb = nullptr) override;

	virtual void UpdateTexture(HANDLE shareHandle, fwRefContainer<GITexture> texture, int width, int height, std::pair<uint64_t, uint64_t> key, nui::GameInterface::UpdateTextureCB cb = nullptr) override;

	virtual void SetTexture(fwRefContainer<GITexture> texture, bool pm) override;

	virtual void DrawRectangles(int numRectangles, const ResultingRectangle* rectangles) override;

	virtual void UnsetTexture() override;

	virtual void SetGameMouseFocus(bool val, bool flushMouse = true) override
	{
		m_targetMouseFocus = val;
		m_flushMouse = flushMouse;
	}

	void UpdateMouseFocus()
	{
		if (m_targetMouseFocus != m_lastTargetMouseFocus)
		{
			InputHook::SetGameMouseFocus(m_targetMouseFocus, m_flushMouse);
			m_lastTargetMouseFocus = m_targetMouseFocus;
			m_flushMouse = true;
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

#ifdef GTA_FIVE
	virtual Microsoft::WRL::ComPtr<ID3D11Device1> GetD3D11Device1()
	{
		if (!m_d3d11Device1)
		{
			auto device = ::GetD3D11Device();
			if (device)
			{
				HRESULT hr = device->QueryInterface(IID_PPV_ARGS(&m_d3d11Device1));
				if (FAILED(hr))
				{
					FatalErrorNoReport("Unable to retrieve D3D11.1 Device\n Query Failed with 0x%x, This systems GPU drivers may not support D3D11.1", hr);
				}
			}
		}
		return m_d3d11Device1;
	}
#endif
};

static tbb::concurrent_queue<std::function<void()>> g_onRenderQueue;
static tbb::concurrent_queue<std::function<void()>> g_earlyOnRenderQueue;

namespace nui
{
	struct NUITextureUpdate
	{
	public:
		fwRefContainer<nui::GITexture> texture;
		nui::GameInterface::UpdateTextureCB cb;
		HANDLE handle;
		std::pair<uint64_t, uint64_t> key;
		uint32_t width = 0;
		uint32_t height = 0;

		NUITextureUpdate(fwRefContainer<nui::GITexture> texture, HANDLE handle, nui::GameInterface::UpdateTextureCB cb, std::pair<uint64_t, uint64_t> key, uint32_t width, uint32_t height)
			: texture(texture), cb(std::move(cb)), handle(handle), key(std::move(key)), width(width), height(height)
		{
		}

		NUITextureUpdate() = delete;
		virtual ~NUITextureUpdate() = default;
	};

	///
	///
	///
	class TextureUpdates
	{
	public:
		std::unique_ptr<NUITextureUpdate> Put(std::unique_ptr<NUITextureUpdate>&& update)
		{
			if (!update)
			{
				return nullptr;
			}

			std::lock_guard<std::mutex> _(m_mutex);
			auto key = update->texture.GetRef();
			auto it = m_slots.find(key);
			if (it == m_slots.end())
			{
				m_slots.emplace(key, std::move(update));
				return nullptr;
			}
			std::unique_ptr<NUITextureUpdate> old = std::move(it->second);
			it->second = std::move(update);
			return old;
		}

		std::vector<std::unique_ptr<NUITextureUpdate>> Drain()
		{
			decltype(m_slots) data;
			{
				std::scoped_lock lock(m_mutex);
				data.swap(m_slots);
			}
			std::vector<std::unique_ptr<NUITextureUpdate>> out;
			out.reserve(data.size());
			for (auto& kv : data)
			{
				out.push_back(std::move(kv.second));
			}
			return out;
		}
	private:
		std::mutex m_mutex;
		std::unordered_map<nui::GITexture*, std::unique_ptr<NUITextureUpdate>> m_slots;
	};
	static TextureUpdates g_textureUpdate;

	///
	/// 
	///
	class TextureCache
	{
	public:
		using Key = std::pair<uint64_t, uint64_t>;

		struct CachedEntry
		{
#ifdef IS_RDR3
			// RedM supports vulkan and dx12.
			// TODO: improve
			void* m_texture; // VK image
			void* m_srv; // VK imageView
			VkDeviceMemory m_deviceMemory; // VK
#else
			WRL::ComPtr<ID3D11Texture2D> m_texture;
			WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
#endif
			uint32_t m_width;
			uint32_t m_height;

			nui::GITexture* m_nuiTex = nullptr;
			uint64_t m_lastUsed; 
		};

		CachedEntry* GetEntry(Key& key)
		{
			auto it = m_entries.find(key);
			if (it != m_entries.end())
			{
				it->second.m_lastUsed = ++m_tick;
				return &it->second;
			}

			return nullptr;
		}

		void AddEntry(Key& key, nui::GITexture* texture, CachedEntry entry)
		{
			entry.m_nuiTex = texture;
			entry.m_lastUsed = ++m_tick;

			auto [it, inserted] = m_entries.insert_or_assign(key, entry);
			if (inserted)
			{
				m_textures[texture].push_back(key);
			}

			Evict(texture);
		}

		void ReleaseTexture(nui::GITexture* texture)
		{
			size_t released = 0;
			for (auto entIt = m_entries.begin(); entIt != m_entries.end();)
			{
				if (entIt->second.m_nuiTex == texture)
				{
					ReleaseCache(entIt->second);
					entIt = m_entries.erase(entIt);
					++released;
				}
				else
				{
					++entIt;
				}
			}

			m_textures.erase(texture);		
		}

		void ReleaseCache(CachedEntry& entry)
		{
#ifdef GTA_FIVE
			g_earlyOnRenderQueue.emplace([tex = std::move(entry.m_texture), srv = std::move(entry.m_srv)]() mutable
			{        
				srv.Reset();
				tex.Reset();
			});
#elif IS_RDR3
			if (GetCurrentGraphicsAPI() == GraphicsAPI::Vulkan)
			{
				g_earlyOnRenderQueue.emplace([entry]()
				{
					auto device = (VkDevice)GetGraphicsDriverHandle();

					if (entry.m_texture)
					{
						vkDestroyImage(device, (VkImage)entry.m_texture, nullptr);
					}

					if (entry.m_deviceMemory)
					{
						vkFreeMemory(device, entry.m_deviceMemory, nullptr);
					}

					if (entry.m_srv)
					{
						vkDestroyImageView(device, (VkImageView)entry.m_srv, nullptr);
					}
				});
			}
			else
			{
				auto resource = static_cast<ID3D12Resource*>(entry.m_texture);
				g_earlyOnRenderQueue.emplace([resource]()
				{
					if (resource)
					{
						resource->Release();
					}
				});
			}
#endif
		}

		/// 
		/// 
		///
		void Clear()
		{
			for (auto& [key, entry] : m_entries)
			{
				ReleaseCache(entry);
			}
			m_entries.clear();
			m_textures.clear();
		}
	private:
		// How many frames should be cached per GITexture
		static constexpr size_t kMaxFramesPerSource = 6;

		void Evict(nui::GITexture* texture)
		{
			auto& keys = m_textures[texture];
			
			while (keys.size() > kMaxFramesPerSource)
			{
				auto lruIt = keys.begin();
				uint64_t oldest = UINT64_MAX;
				for (auto it = keys.begin(); it != keys.end(); ++it)
				{
					auto e = m_entries.find(*it);
					if (e != m_entries.end() && e->second.m_lastUsed < oldest)
					{
						oldest = e->second.m_lastUsed;
						lruIt = it;
					}
				}

				auto e = m_entries.find(*lruIt);
				if (e != m_entries.end())
				{
					ReleaseCache(e->second);
					m_entries.erase(e);
				}
				keys.erase(lruIt);
			}
		}

		struct HashPair
		{
			std::size_t operator()(const Key& key) const
			{
				return static_cast<std::size_t>(key.first ^ key.second);
			}
		};

		uint64_t m_tick = 0;
		std::unordered_map<Key, CachedEntry, HashPair> m_entries;
		std::unordered_map<nui::GITexture*, std::vector<Key>> m_textures;
	};

	static TextureCache g_texCache;
}
class GtaNuiTextureBase : public nui::GITexture
{
public:
	virtual rage::grcTexture* GetTexture() = 0;
};

class GtaNuiTexture final : public GtaNuiTextureBase
{
private:
	rage::grcTexture* m_texture;

	bool m_overriddenTexture;

	bool m_overriddenSRV;

	nui::GameInterface::UpdateTextureCB m_releaseCB;
public:
	explicit GtaNuiTexture(rage::grcTexture* texture)
		: m_texture(texture), m_overriddenTexture(false), m_overriddenSRV(false)
	{

	}

	explicit GtaNuiTexture(std::function<rage::grcTexture*(GtaNuiTexture*)> fn)
		: m_texture(nullptr), m_overriddenTexture(false), m_overriddenSRV(false)
	{
		g_onRenderQueue.push([self = fwRefContainer(this), fn]()
		{
			if (self.GetRefCount() > 1) // Don't bother creating the texture if we are the only reference left
			{
				std::unique_lock _(self->TextureLock);

				self->m_texture = fn(self.GetRef());

				self->OnMaterialize(self->m_texture);
				self->OnMaterialize.Reset();
			}
		});
	}

	virtual ~GtaNuiTexture()
	{
		auto texture = m_texture;
		m_texture = nullptr;
		if (m_releaseCB)
		{
			m_releaseCB();
			m_releaseCB = nullptr;
		}
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

		g_onRenderQueue.push([texture, self = static_cast<nui::GITexture*>(this)]()
		{
			nui::g_texCache.ReleaseTexture(self);
			delete texture;
		});
	}

	nui::GameInterface::UpdateTextureCB& GetTextureRelease()
	{
		return m_releaseCB;
	}

	inline void MarkOverriddenTexture()
	{
		m_overriddenTexture = true;
	}

	inline void MarkOverriddenSRV()
	{
		m_overriddenSRV = true;
	}

	virtual rage::grcTexture* GetTexture() override { return m_texture; }

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

	virtual void* GetNativeSRV() override
	{
#ifdef GTA_FIVE
		if (!m_texture)
		{
			return nullptr;
		}

		return m_texture->srv;
#else
		return m_texture;
#endif
	}

	virtual void* GetHostTexture() override
	{
		return m_texture;
	}

	virtual void WithHostTexture(std::function<void(void*)>&& callback) override
	{
		std::unique_lock _(TextureLock);

		if (auto texture = GetHostTexture())
		{
			callback(texture);
		}
		else
		{
			OnMaterialize.Connect([callback = std::move(callback)](void* texture)
			{
				callback(texture);
			});
		}
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

private:
	fwEvent<void*> OnMaterialize;
	std::mutex TextureLock;
};

#ifdef IS_RDR3
class GtaNuiDynamicTexture final : public GtaNuiTextureBase
{
private:
	rage::sga::ext::DynamicTexture2* m_texture;

public:
	explicit GtaNuiDynamicTexture(std::function<rage::sga::ext::DynamicTexture2*(GtaNuiDynamicTexture*)> fn)
		: m_texture(nullptr)
	{
		g_onRenderQueue.push([self = fwRefContainer(this), fn]()
		{
			if (self.GetRefCount() > 1) // Don't bother creating the texture if we are the only reference left
			{
				self->m_texture = fn(self.GetRef());
			}
		});
	}

	virtual ~GtaNuiDynamicTexture()
	{
		auto texture = m_texture;
		m_texture = nullptr;

		g_onRenderQueue.push([texture]()
		{
			delete texture;
		});
	}

	inline rage::grcTexture* GetTexture()
	{
		if (m_texture)
		{
			return static_cast<rage::grcTexture*>(m_texture->GetTexture());
		}

		return nullptr;
	}

	virtual void* GetNativeTexture() override
	{
		return GetTexture();
	}

	virtual void* GetNativeSRV() override
	{
		return nullptr;
	}

	virtual void* GetHostTexture() override
	{
		return m_texture;
	}

	virtual bool Map(int numSubLevels, int subLevel, nui::GILockedTexture* lockedTexture, nui::GILockFlags flags) override
	{
		if (m_texture)
		{
			m_texture->MakeReady(rage::sga::GraphicsContext::GetCurrent());

			rage::sga::MapData mapData;
			if (m_texture->Map(nullptr, mapData))
			{
				lockedTexture->pBits = mapData.GetBuffer();
				lockedTexture->pitch = mapData.GetStride();

				lastMapData = mapData;

				return true;
			}
		}

		return false;
	}

	virtual void Unmap(nui::GILockedTexture* lockedTexture) override
	{
		m_texture->Unmap(nullptr, lastMapData);
	}

private:
	rage::sga::MapData lastMapData;
};
#endif

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
	return new GtaNuiTexture([width, height](GtaNuiTexture*)
	{
		rage::grcManualTextureDef textureDef;
		memset(&textureDef, 0, sizeof(textureDef));
		textureDef.isStaging = 1;
		textureDef.usage = 1;
		textureDef.arraySize = 1;
		return rage::grcTextureFactory::getInstance()->createManualTexture(width, height, 2 /* maps to BGRA DXGI format */, nullptr, true, &textureDef);
	});
#elif defined(IS_RDR3)
	return new GtaNuiDynamicTexture([width, height](GtaNuiDynamicTexture*)
	{
		rage::sga::ImageParams ip;
		ip.width = width;
		ip.height = height;
		ip.depth = 1;
		ip.levels = 1;
		ip.dimension = 1;
		ip.bufferFormat = rage::sga::BufferFormat::B8G8R8A8_UNORM;

		auto texture = new rage::sga::ext::DynamicTexture2();
		texture->Init(3, nullptr, ip, 0, 2, nullptr, 8, 1, nullptr);
		return texture;
	});
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


fwRefContainer<GITexture> GtaNuiInterface::CreateTextureFromShareHandle(HANDLE shareHandle, int width, int height, nui::GameInterface::UpdateTextureCB cb)
{
#ifndef GTA_NY
	rage::sysMemAllocator::UpdateAllocatorValue();
#endif

#ifdef GTA_FIVE
	return new GtaNuiTexture([this, shareHandle, cb](GtaNuiTexture* texture) -> rage::grcTexture*
	{
		WRL::ComPtr<ID3D11Device1> device = GetD3D11Device1();

		WRL::ComPtr<ID3D11Texture2D> resource;
		auto hr = device->OpenSharedResource1(shareHandle, IID_PPV_ARGS(&resource));

		if (FAILED(hr) || !resource)
		{
			trace("Failed to open shared resource for NUI texture creation 0x%x\n", hr);
			cb();
			return nullptr;
		}

		D3D11_TEXTURE2D_DESC desc;
		resource->GetDesc(&desc);

		struct
		{
			void* vtbl;
			ID3D11Device* rawDevice;
		}* deviceStuff = (decltype(deviceStuff))GetD3D11Device();

		rage::grcManualTextureDef textureDef;
		memset(&textureDef, 0, sizeof(textureDef));
		textureDef.isStaging = 1;
		textureDef.usage = 1;
		textureDef.arraySize = 1;

		auto texRef = rage::grcTextureFactory::getInstance()->createManualTexture(desc.Width, desc.Height, 2 /* maps to BGRA DXGI format */, nullptr, true, &textureDef);

		if (texRef)
		{
			if (texRef->texture)
			{
#ifdef GTA_FIVE
				rage::grcResourceCache::GetInstance()->QueueDelete(texRef->texture);
				rage::grcResourceCache::GetInstance()->FlushQueue();
#else
				texRef->texture->Release();
#endif
				texRef->texture = NULL;
			}

			resource.CopyTo(&texRef->texture);

			if (texRef->srv)
			{
				texRef->srv->Release();
			}

			deviceStuff->rawDevice->CreateShaderResourceView(resource.Get(), nullptr, &texRef->srv);
		}

		texture->MarkOverriddenSRV();
		texture->MarkOverriddenTexture();
		
		auto previous = std::move(texture->GetTextureRelease());
		texture->GetTextureRelease() = std::move(cb);
		if (previous)
		{
			previous();
		}

		return texRef;
	});
#else
	if (GetCurrentGraphicsAPI() == GraphicsAPI::D3D12)
	{
		return new GtaNuiTexture([shareHandle, cb](GtaNuiTexture* texture) -> rage::grcTexture*
		{
			ID3D12Resource* oldTexture = nullptr;
			ID3D12Device* device = (ID3D12Device*)GetGraphicsDriverHandle();
			ID3D12Resource* resource = nullptr;
			if (FAILED(device->OpenSharedHandle(shareHandle, __uuidof(ID3D12Resource), (void**)&resource)))
			{
				cb();
				return nullptr;
			}
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
				texRef->resource->AddRef();

				rage::sga::TextureViewDesc srvDesc;
				srvDesc.mipLevels = 1;
				srvDesc.arrayStart = 0;
				srvDesc.dimension = 4;
				srvDesc.arraySize = 1;

				rage::sga::Driver_Create_ShaderResourceView(texRef, srvDesc);
			}

			auto previous = std::move(texture->GetTextureRelease());
			texture->GetTextureRelease() = std::move(cb);
			if (previous)
			{
				previous();
			}

			return (rage::grcTexture*)texRef;
		});
	}
	else if (GetCurrentGraphicsAPI() == GraphicsAPI::Vulkan)
	{
		return new GtaNuiTexture([shareHandle, width, height, cb](GtaNuiTexture* texture)
		{
			VkDevice device = (VkDevice)GetGraphicsDriverHandle();
			VkImage Image = {};
			VkDeviceMemory ImageMemory = {};
			vk::CreateImageFromShareHandle(device, shareHandle, width, height, Image, ImageMemory, true);

			rage::grcManualTextureDef textureDef;
			memset(&textureDef, 0, sizeof(textureDef));
			textureDef.isStaging = 0;
			textureDef.arraySize = 1;

			auto texRef = (rage::sga::TextureVK*)rage::grcTextureFactory::getInstance()->createManualTexture(width, height, 2, nullptr, true, &textureDef);
			if (texRef)
			{
				rage::sga::Driver_Destroy_Texture(texRef);

				auto newImage = new rage::sga::TextureVK::ImageData;
				memset(newImage, 0, sizeof(*newImage));
				// these come from a fast allocator(?)
				texRef->image = newImage;

				texRef->image->image = Image;
				texRef->image->memory = ImageMemory;

				texRef->width = width;
				texRef->height = height;

				rage::sga::TextureViewDesc srvDesc;
				srvDesc.mipLevels = 1;
				srvDesc.arrayStart = 0;
				srvDesc.dimension = 4;
				srvDesc.arraySize = 1;

				rage::sga::Driver_Create_ShaderResourceView(texRef, srvDesc);
			}

			auto previous = std::move(texture->GetTextureRelease());
			texture->GetTextureRelease() = std::move(cb);
			if (previous)
			{
				previous();
			}
			return (rage::grcTexture*)texRef;
		});
	}
#endif

	if (cb)
	{
		cb();
	}

	return new GtaNuiTexture(nullptr);
}

void GtaNuiInterface::UpdateTexture(HANDLE shareHandle, fwRefContainer<GITexture> texture, int width, int height, std::pair<uint64_t, uint64_t> key, nui::GameInterface::UpdateTextureCB cb)
{
#ifdef GTA_FIVE
	auto device1 = GetD3D11Device1();
	if (!device1)
	{	
		cb();
		return;
	}
#endif

	auto texRef = (rage::grcTexture*)texture->GetHostTexture();
	if (!texRef)
	{
		cb();
		return;
	}

	auto replaced = nui::g_textureUpdate.Put(std::make_unique<nui::NUITextureUpdate>(texture, shareHandle, cb, key, width, height));
	if (replaced && replaced->cb)
	{
		replaced->cb();
	}
}

static thread_local fwRefContainer<nui::GITexture> g_currentTexture;

void GtaNuiInterface::SetTexture(fwRefContainer<GITexture> texture, bool pm)
{
#ifndef GTA_NY
	rage::sysMemAllocator::UpdateAllocatorValue();
#endif

	g_currentTexture = texture;

	SetTextureGtaIm(static_cast<GtaNuiTextureBase*>(texture.GetRef())->GetTexture());

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
	InputHook::SetHostCursorEnabled(enabled);
}
#endif

static GtaNuiInterface nuiGi;

#ifdef IS_RDR3
// This should be improved in the future to use DX12/VK Fences
constexpr uint64_t kReleaseDelay = 2;
static uint64_t g_releaseId = 0;
static std::deque<std::pair<uint64_t, std::function<void()>>> g_pendingReleases;

static void QueueDelayedRelease(std::function<void()> fn)
{
	g_pendingReleases.emplace_back(g_releaseId + kReleaseDelay, std::move(fn));
}

static void ProcessReleases()
{
	++g_releaseId;
	while (!g_pendingReleases.empty() && g_pendingReleases.front().first <= g_releaseId)
	{
		g_pendingReleases.front().second();
		g_pendingReleases.pop_front();
	}
}
#else
// in five we can use fences easily instead of a delay.
static std::vector<nui::GameInterface::UpdateTextureCB> g_pendingReleases;
static std::deque<std::pair<WRL::ComPtr<ID3D11Query>, std::vector<nui::GameInterface::UpdateTextureCB>>> g_fencedFrames;

static void ProcessReleases()
{
	auto ctx = ::GetD3D11DeviceContext();

	while (!g_fencedFrames.empty())
	{
		auto& [fence, cbs] = g_fencedFrames.front();
		if (fence)
		{
			BOOL done = false;
			if (ctx->GetData(fence.Get(), &done, sizeof(done), 0x1) != S_OK || !done)
			{
				break;
			}
		}

		for (auto& cb : cbs)
		{
			cb();
		}

		g_fencedFrames.pop_front();
	}

	if (!g_pendingReleases.empty())
	{
		D3D11_QUERY_DESC desc{ D3D11_QUERY_EVENT, 0 };
		WRL::ComPtr<ID3D11Query> fence;
		if (SUCCEEDED(nuiGi.GetD3D11Device1()->CreateQuery(&desc, &fence)))
		{
			ctx->End(fence.Get());
		}

		g_fencedFrames.emplace_back(fence, std::move(g_pendingReleases));
		g_pendingReleases.clear();
	}
}
#endif

#ifdef GTA_FIVE
static void UpdateTexture(std::unique_ptr<nui::NUITextureUpdate>& texture)
{
	auto texRef = (rage::grcTexture*)texture->texture->GetHostTexture();
	if (!texRef)
	{
		texture->cb();
		return;
	}

	auto renderFrame = [texRef](std::unique_ptr<nui::NUITextureUpdate>& texture, ID3D11Texture2D* tex, ID3D11ShaderResourceView* srv)
	{
		bool sameFrame = texRef->texture == tex && texRef->srv == srv;
		if (!sameFrame)
		{
			tex->AddRef();
			srv->AddRef();

			auto oldTex = texRef->texture;
			auto oldSrv = texRef->srv;

			texRef->texture = tex;
			texRef->srv = srv;

			g_earlyOnRenderQueue.emplace([oldTex, oldSrv]()
			{
				if (oldSrv)
				{
					oldSrv->Release();
				}

				if (oldTex)
				{
					oldTex->Release();
				}
			});
		}

		auto nuiTex = static_cast<GtaNuiTexture*>(texture->texture.GetRef());

		auto previous = std::move(nuiTex->GetTextureRelease());
		nuiTex->GetTextureRelease() = std::move(texture->cb);

		if (!previous)
		{
			return;
		}

		if (sameFrame)
		{
			previous();
		}
		else
		{
			g_pendingReleases.push_back(std::move(previous));
		}
	};

	if (auto entry = nui::g_texCache.GetEntry(texture->key))
	{
		renderFrame(texture, entry->m_texture.Get(), entry->m_srv.Get());
		return;
	}

	WRL::ComPtr<ID3D11Texture2D> cefTexture;
	auto hr = nuiGi.GetD3D11Device1()->OpenSharedResource1(texture->handle, IID_PPV_ARGS(&cefTexture));
	if (FAILED(hr) || !cefTexture)
	{
		trace("Failed to open shared resource for NUI Update 0x%x\n", hr);
		texture->cb();
		return;
	}

	struct
	{
		void* vtbl;
		ID3D11Device* rawDevice;
	}* gameDevice = (decltype(gameDevice))::GetD3D11Device();

	WRL::ComPtr<ID3D11ShaderResourceView> cefSrv;
	hr = gameDevice->rawDevice->CreateShaderResourceView(cefTexture.Get(), nullptr, &cefSrv);
	if (FAILED(hr))
	{
		texture->cb();
		return;
	}

	nui::g_texCache.AddEntry(texture->key, texture->texture.GetRef(), nui::TextureCache::CachedEntry{ cefTexture, cefSrv });
	renderFrame(texture, cefTexture.Get(), cefSrv.Get());
}
#elif IS_RDR3
static void UpdateTextureVK(std::unique_ptr<nui::NUITextureUpdate>& texture)
{
	auto texRef = (rage::sga::TextureVK*)texture->texture->GetHostTexture();
	if (!texRef || !texRef->image || !texRef->data)
	{
		if (texture->cb)
		{
			texture->cb();
		}
		return;
	}

	bool sameFrame = false;
	if (auto entry = nui::g_texCache.GetEntry(texture->key))
	{
		sameFrame = texRef->image->image == (VkImage)entry->m_texture;
		if (!sameFrame)
		{
			texRef->image->image = (VkImage)entry->m_texture;
			texRef->image->memory = (VkDeviceMemory)entry->m_deviceMemory;
			texRef->data->imageView = (VkImageView)entry->m_srv;
			texRef->data->imageViewType = 2;
		}
	}
	else
	{
		VkDevice device = (VkDevice)GetGraphicsDriverHandle();
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		vk::CreateImageFromShareHandle(device, texture->handle, texture->width, texture->height, image, memory, true);

		texRef->image->image = image;
		texRef->image->memory = memory;

		rage::sga::TextureViewDesc srvDesc;
		srvDesc.mipLevels = 1;
		srvDesc.arrayStart = 0;
		srvDesc.dimension = 4;
		srvDesc.arraySize = 1;
		rage::sga::Driver_Create_ShaderResourceView(texRef, srvDesc);
		nui::g_texCache.AddEntry(texture->key, texture->texture.GetRef(), nui::TextureCache::CachedEntry{ image, texRef->data->imageView, memory, texture->width, texture->height });
	}

	auto nuiTex = static_cast<GtaNuiTexture*>(texture->texture.GetRef());
	auto previous = std::move(nuiTex->GetTextureRelease());
	nuiTex->GetTextureRelease() = std::move(texture->cb);
	if (previous)
	{
		if (sameFrame)
		{
			previous();
		}
		else
		{
			QueueDelayedRelease([previous = std::move(previous)]()
			{
				previous();
			});
		}
	}
}

static void UpdateTextureDX(std::unique_ptr<nui::NUITextureUpdate>& texture)
{
	auto texRef = (rage::sga::TextureD3D12*)texture->texture->GetHostTexture();
	if (!texRef)
	{
		texture->cb();
		return;
	}

	ID3D12Resource* resource = nullptr;
	size_t cachedSrv = 0;
	WRL::ComPtr<ID3D12Resource> opened;

	if (auto entry = nui::g_texCache.GetEntry(texture->key))
	{
		resource = static_cast<ID3D12Resource*>(entry->m_texture);
		cachedSrv = (size_t)entry->m_srv;
	}
	else
	{
		ID3D12Device* device = (ID3D12Device*)GetGraphicsDriverHandle();
		auto hr = device->OpenSharedHandle(texture->handle, IID_PPV_ARGS(&opened));
		if (FAILED(hr))
		{
			trace("Failed to open shared resource for NUI Update 0x%x (handle %p)\n", hr, (void*)texture->handle);
			texture->cb();
			return;
		}
		resource = opened.Get();
	}

	bool sameFrame = texRef->resource == resource;
	if (!sameFrame)
	{
		resource->AddRef();
		if (auto old = std::exchange(texRef->resource, resource))
		{
			QueueDelayedRelease([old]()
			{
				old->Release();
			});
		}

		if (opened)
		{
			rage::sga::TextureViewDesc srvDesc;
			srvDesc.mipLevels = 1;
			srvDesc.arrayStart = 0;
			srvDesc.dimension = 4;
			srvDesc.arraySize = 1;
			rage::sga::Driver_Create_ShaderResourceView(texRef, srvDesc);

			nui::g_texCache.AddEntry(texture->key, texture->texture.GetRef(), nui::TextureCache::CachedEntry{ (void*)opened.Detach(), (void*)texRef->data->handle.ptr, nullptr, 0, 0 });
		}
		else
		{
			texRef->data->handle.ptr = cachedSrv;
		}
	}

	auto nuiTex = static_cast<GtaNuiTexture*>(texture->texture.GetRef());
	auto previous = std::move(nuiTex->GetTextureRelease());
	nuiTex->GetTextureRelease() = std::move(texture->cb);

	if (previous)
	{
		if (sameFrame)
		{
			previous();
		}
		else
		{
			QueueDelayedRelease([previous = std::move(previous)]()
			{
				previous();
			});
		}
	}
}
#endif

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

	// NUI updates
	{
		auto updates = nui::g_textureUpdate.Drain();
		for (auto& update : updates)
		{
#ifndef IS_RDR3
			UpdateTexture(update);
#else
			if (GetCurrentGraphicsAPI() == GraphicsAPI::Vulkan)
			{
				UpdateTextureVK(update);
			}
			else
			{
				UpdateTextureDX(update);
			}
#endif
		}
	}

	nuiGi.OnRender();
	ProcessReleases();
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

	OnBackbufferCreation.Connect([]()
	{
		nui::g_texCache.Clear();
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
