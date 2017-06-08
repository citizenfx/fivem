#include <StdInc.h>
#include <CefOverlay.h>

#include <fiDevice.h>
#include <DrawCommands.h>
#include <grcTexture.h>
#include <InputHook.h>

#include "ResumeComponent.h"

using nui::GITexture;
using nui::GITextureFormat;
using nui::ResultingRectangle;

class GtaNuiInterface : public nui::GameInterface
{
private:
	uint32_t m_oldBlendState;

	uint32_t m_oldRasterizerState;

	uint32_t m_oldDepthStencilState;

	uint32_t m_oldSamplerState;

	uint32_t m_pointSamplerState;

public:
	virtual void GetGameResolution(int* width, int* height) override;

	virtual GITexture* CreateTexture(int width, int height, GITextureFormat format, void* pixelData) override;

	virtual GITexture* CreateTextureBacking(int width, int height, GITextureFormat format) override;

	virtual GITexture* CreateTextureFromShareHandle(HANDLE shareHandle) override;

	virtual void SetTexture(GITexture* texture, bool pm) override;

	virtual void DrawRectangles(int numRectangles, const ResultingRectangle* rectangles) override;

	virtual void UnsetTexture() override;

	virtual void SetGameMouseFocus(bool val) override
	{
		InputHook::SetGameMouseFocus(val);
	}

	virtual HWND GetHWND() override
	{
		return FindWindowW(L"grcWindow", NULL);
	}

	virtual void BlitTexture(GITexture* dst, GITexture* src) override
	{
		::GetD3D11DeviceContext()->CopyResource((ID3D11Resource*)dst->GetNativeTexture(), (ID3D11Resource*)src->GetNativeTexture());
	}

	virtual ID3D11Device* GetD3D11Device() override
	{
		return ::GetD3D11Device();
	}

	virtual ID3D11DeviceContext* GetD3D11DeviceContext() override
	{
		return ::GetD3D11DeviceContext();
	}

	virtual GITexture* CreateTextureFromD3D11Texture(ID3D11Texture2D* texture) override
	{
		// unused
		return NULL;
	}
};

class GtaNuiTexture : public nui::GITexture
{
private:
	rage::grcTexture* m_texture;

public:
	explicit GtaNuiTexture(rage::grcTexture* texture)
		: m_texture(texture)
	{

	}

	virtual ~GtaNuiTexture()
	{
		// TODO: delete overridden SRV/...

		delete m_texture;
	}

	inline rage::grcTexture* GetTexture() { return m_texture; }

	virtual void* GetNativeTexture() override
	{
		return m_texture->texture;
	}
	virtual void* GetHostTexture() override
	{
		return m_texture;
	}

	virtual bool Map(int numSubLevels, int subLevel, nui::GILockedTexture* lockedTexture, nui::GILockFlags flags) override
	{
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

		return false;
	}

	virtual void Unmap(nui::GILockedTexture* lockedTexture) override
	{
		rage::grcLockedTexture rlt;
		rlt.format = lockedTexture->format;
		rlt.height = lockedTexture->height;
		rlt.level = lockedTexture->level;
		rlt.numSubLevels = lockedTexture->numSubLevels;
		rlt.pBits = lockedTexture->pBits;
		rlt.pitch = lockedTexture->pitch;
		rlt.width = lockedTexture->width;

		m_texture->Unmap(&rlt);
	}
};

void GtaNuiInterface::GetGameResolution(int* width, int* height)
{
	int w, h;
	::GetGameResolution(w, h);

	*width = w;
	*height = h;
}

GITexture* GtaNuiInterface::CreateTexture(int width, int height, GITextureFormat format, void* pixelData)
{
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
}

GITexture* GtaNuiInterface::CreateTextureBacking(int width, int height, GITextureFormat format)
{
	rage::sysMemAllocator::UpdateAllocatorValue();

	assert(format == GITextureFormat::ARGB);

	rage::grcManualTextureDef textureDef;
	memset(&textureDef, 0, sizeof(textureDef));
	textureDef.isStaging = 0;
	textureDef.arraySize = 1;

	return new GtaNuiTexture(rage::grcTextureFactory::getInstance()->createManualTexture(width, height, 2 /* maps to BGRA DXGI format */, nullptr, true, &textureDef));
}

GITexture* GtaNuiInterface::CreateTextureFromShareHandle(HANDLE shareHandle)
{
	rage::sysMemAllocator::UpdateAllocatorValue();

	ID3D11Device* device = ::GetD3D11Device();

	ID3D11Resource* resource = nullptr;
	if (SUCCEEDED(device->OpenSharedResource(shareHandle, __uuidof(IDXGIResource), (void**)&resource)))
	{
		ID3D11Texture2D* texture;
		assert(SUCCEEDED(resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture)));

		ID3D11Texture2D* oldTexture = nullptr;

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
				texRef->texture->Release();
			}

			texRef->texture = texture;
			texture->AddRef();

			if (texRef->srv)
			{
				texRef->srv->Release();
			}

			deviceStuff->rawDevice->CreateShaderResourceView(texture, nullptr, &texRef->srv);
		}

		return new GtaNuiTexture(texRef);
	}

	return new GtaNuiTexture(nullptr);
}

void GtaNuiInterface::SetTexture(GITexture* texture, bool pm)
{
	rage::sysMemAllocator::UpdateAllocatorValue();

	m_oldSamplerState = GetImDiffuseSamplerState();

	SetTextureGtaIm(static_cast<GtaNuiTexture*>(texture)->GetTexture());

	m_oldRasterizerState = GetRasterizerState();
	SetRasterizerState(GetStockStateIdentifier(RasterizerStateNoCulling));

	m_oldBlendState = GetBlendState();
	SetBlendState(GetStockStateIdentifier(pm ? BlendStatePremultiplied : BlendStateDefault));

	m_oldDepthStencilState = GetDepthStencilState();
	SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));

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

	SetRasterizerState(m_oldRasterizerState);
	SetBlendState(m_oldBlendState);
	SetDepthStencilState(m_oldDepthStencilState);
	SetImDiffuseSamplerState(m_oldSamplerState);
}

static GtaNuiInterface nuiGi;

static InitFunction initFunction([]()
{
	OnGrcCreateDevice.Connect([]()
	{
		nuiGi.OnInitRenderer();
	});

	OnPostFrontendRender.Connect([]()
	{
		nuiGi.OnRender();
	}, -1000);

	rage::fiDevice::OnInitialMount.Connect([]()
	{
		nuiGi.OnInitVfs();
	}, 100);

	OnResumeGame.Connect([]()
	{
		nui::Initialize(&nuiGi);
	});

	InputHook::QueryInputTarget.Connect([](std::vector<InputTarget*>& targets)
	{
		return nuiGi.QueryInputTarget(targets);
	});

	InputHook::DeprecatedOnWndProc.Connect([](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& pass, LRESULT& lresult)
	{
		nuiGi.OnWndProc(hWnd, msg, wParam, lParam, pass, lresult);
	});

	InputHook::QueryMayLockCursor.Connect([](int& a)
	{
		nuiGi.QueryMayLockCursor(a);
	});
});
