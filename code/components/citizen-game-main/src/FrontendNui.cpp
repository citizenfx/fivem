#include <StdInc.h>
#include <CefOverlay.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <GameWindow.h>

#include <BgfxPrepare.h>

#include <Manager.h>
#include <RelativeDevice.h>

#include <wrl.h>

#include <tbb/concurrent_queue.h>

tbb::concurrent_queue<std::function<void()>> g_onRenderQueue;
tbb::concurrent_queue<std::function<void()>> g_earlyOnRenderQueue;

namespace WRL = Microsoft::WRL;

using nui::GITexture;
using nui::GITextureFormat;
using nui::ResultingRectangle;

static std::mutex g_frontendDeletionMutex;

class FrontendNuiTexture : public nui::GITexture
{
private:
	bgfx::TextureHandle m_texture;

	std::shared_ptr<FrontendNuiTexture*> m_canary;

public:
	FrontendNuiTexture()
		: m_texture()
	{
		m_texture.idx = bgfx::kInvalidHandle;
	}

	FrontendNuiTexture(bgfx::TextureHandle tex)
		: m_texture(tex)
	{

	}

	FrontendNuiTexture(std::function<bgfx::TextureHandle(FrontendNuiTexture*)> fn)
		: m_texture()
	{
		m_texture.idx = bgfx::kInvalidHandle;
		m_canary = std::make_shared<FrontendNuiTexture*>(this);

		// make a weak reference to the class pointer, so if it gets `delete`d, we can just ignore this creation attempt
		std::weak_ptr<FrontendNuiTexture*> weakCanary = m_canary;

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

	virtual ~FrontendNuiTexture() override
	{
		std::unique_lock<std::mutex> lock(g_frontendDeletionMutex);

		auto tex = m_texture;

		if (bgfx::isValid(tex))
		{
			auto nativeRef = this->native;

			g_onRenderQueue.push([tex, nativeRef]()
			{
				// only destroy this next frame
				g_earlyOnRenderQueue.push([tex, nativeRef]()
				{
					if (bgfx::isValid(tex))
					{
						bgfx::destroy(tex);
					}

					// to make sure the lambda doesn't eat it (might not be needed?)
					nativeRef.Get();
				});
			});
		}
	}

	// Inherited via GITexture
	inline bgfx::TextureHandle GetTexture()
	{
		return m_texture;
	}

	virtual void* GetNativeTexture() override
	{
		return NULL;
	}

	virtual void* GetHostTexture() override
	{
		return &m_texture;
	}

	virtual bool Map(int numSubLevels, int subLevel, nui::GILockedTexture* lockedTexture, nui::GILockFlags flags) override
	{
		return false;
	}

	virtual void Unmap(nui::GILockedTexture* lockedTexture) override
	{
	}

public:
	WRL::ComPtr<ID3D11Resource> native;
};

class FrontendNuiInterface : public nui::GameInterface
{
private:
	bgfx::ProgramHandle m_program;

	bgfx::UniformHandle s_texColor;
	bgfx::UniformHandle u_params;
	bgfx::UniformHandle u_modelViewProj;

	citizen::GameWindow* m_window;

	HANDLE m_lastShareHandle;

public:
	void Initialize(citizen::GameWindow* window);

	virtual void GetGameResolution(int* width, int* height) override
	{
		int w, h;
		m_window->GetMetrics(w, h);

		*width = w;
		*height = h;
	}

	virtual GITexture* CreateTexture(int width, int height, GITextureFormat format, void* pixelData) override
	{
		auto pixelMem = std::make_shared<std::vector<uint8_t>>(width * height * 4);
		memcpy(pixelMem->data(), pixelData, pixelMem->size());

		return new FrontendNuiTexture([width, height, pixelMem](FrontendNuiTexture*)
		{
			auto tex = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8);
			bgfx::updateTexture2D(tex, 0, 0, 0, 0, width, height, bgfx::copy(pixelMem->data(), pixelMem->size()));

			return tex;
		});
	}

	virtual GITexture* CreateTextureBacking(int width, int height, GITextureFormat format) override
	{
		return new FrontendNuiTexture([width, height](FrontendNuiTexture*)
		{
			auto t = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8);

			return t;
		});
	}

	virtual GITexture* CreateTextureFromShareHandle(HANDLE shareHandle) override
	{
		m_lastShareHandle = shareHandle;

		auto texture = new FrontendNuiTexture([this, shareHandle](FrontendNuiTexture* self)
		{
			if (shareHandle != m_lastShareHandle)
			{
				return bgfx::TextureHandle{ bgfx::kInvalidHandle };
			}

			WRL::ComPtr<ID3D11Texture2D> sharedTexture;

			HRESULT hr = GetD3D11Device()->OpenSharedResource(shareHandle, __uuidof(ID3D11Texture2D), (void**)sharedTexture.GetAddressOf());

			if (FAILED(hr))
			{
				return bgfx::TextureHandle{bgfx::kInvalidHandle};
			}

			self->native = sharedTexture;

			D3D11_TEXTURE2D_DESC d;
			sharedTexture->GetDesc(&d);

			std::vector<uint32_t> mem(d.Width * d.Height);

			for (size_t i = 0; i < mem.size(); i++)
			{
				mem[i] = 0xFF0000FF;
			}

			auto tex = bgfx::createTexture2D(d.Width, d.Height, false, 1, bgfx::TextureFormat::BGRA8);
			bgfx::updateTexture2D(tex, 0, 0, 0, 0, d.Width, d.Height, bgfx::copy(mem.data(), mem.size() * 4));

			g_earlyOnRenderQueue.push([tex, sharedTexture]()
			{
				bgfx::overrideInternal(tex, (uintptr_t)sharedTexture.Get());
			});

			return tex;
		});

		return texture;
	}

	virtual void SetTexture(GITexture* texture, bool pm = false) override
	{
		bgfx::setState(BGFX_STATE_WRITE_RGB | ((!pm) ? BGFX_STATE_BLEND_ALPHA : BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA)) | BGFX_STATE_PT_TRISTRIP);

		if (bgfx::isValid(((FrontendNuiTexture*)texture)->GetTexture()))
		{
			bgfx::setTexture(0, s_texColor, ((FrontendNuiTexture*)texture)->GetTexture());
		}

		float mtx[4][4] = { 0 };
		mtx[0][0] = 1.0f;
		mtx[1][1] = 1.0f;
		mtx[2][2] = 1.0f;
		mtx[3][3] = 1.0f;

		bgfx::setTransform(&mtx);
	}

	virtual void DrawRectangles(int numRectangles, const ResultingRectangle* rectangles) override
	{
		bgfx::TransientVertexBuffer tvb;
		bgfx::TransientIndexBuffer tib;

		bgfx::allocTransientBuffers(&tvb, DebugUvVertex::ms_decl, numRectangles * 4, &tib, numRectangles * 4);

		uint32_t v = 0;
		uint32_t i = 0;

		DebugUvVertex* verts = (DebugUvVertex*)tvb.data;
		uint16_t* inds = (uint16_t*)tib.data;

		int w, h;
		m_window->GetMetrics(w, h);

		for (int i = 0; i < numRectangles; i++)
		{
			const auto& rectBit = rectangles[i];
			const auto& rect = rectBit.rectangle;

			// TODO: do GPU transforms
			auto u1 = 0.0f;
			auto v1 = 0.0f;
			auto u2 = 1.0f;
			auto v2 = 1.0f;

			auto color = rectBit.color.AsARGB();

			auto scaleX = [w](float x)
			{
				return ((x / w) * 2.0f) - 1.0f;
			};

			auto scaleY = [h](float y)
			{
				return ((y / h) * -2.0f) + 1.0f;
			};

			verts[v + 0] = { scaleX(rect.fX1), scaleY(rect.fY1), 0.0f, u1, v1, color };
			verts[v + 1] = { scaleX(rect.fX2), scaleY(rect.fY1), 0.0f, u2, v1, color };
			verts[v + 2] = { scaleX(rect.fX1), scaleY(rect.fY2), 0.0f, u1, v2, color };
			verts[v + 3] = { scaleX(rect.fX2), scaleY(rect.fY2), 0.0f, u2, v2, color };

			inds[i + 0] = v + 0;
			inds[i + 1] = v + 1;
			inds[i + 2] = v + 2;
			inds[i + 3] = v + 3;

			i += 4;
			v += 4;
		}

		bgfx::setVertexBuffer(0, &tvb);
		bgfx::setIndexBuffer(&tib);

		bgfx::submit(0, m_program);
	}

	virtual void UnsetTexture() override
	{
	}

	virtual void SetGameMouseFocus(bool val) override
	{
	}

	virtual HWND GetHWND() override
	{
		return (HWND)m_window->GetNativeHandle();
	}

	virtual void BlitTexture(GITexture* dst, GITexture* src) override
	{
		bgfx::blit(0, ((FrontendNuiTexture*)dst)->GetTexture(), 0, 0, ((FrontendNuiTexture*)src)->GetTexture());
	}

	virtual ID3D11Device* GetD3D11Device() override
	{
		return ((ID3D11Device*)bgfx::getInternalData()->context);
	}

	virtual ID3D11DeviceContext* GetD3D11DeviceContext() override
	{
		static ID3D11DeviceContext* cxt;

		if (!cxt)
		{
			GetD3D11Device()->GetImmediateContext(&cxt);
		}

		return cxt;
	}

	virtual GITexture* CreateTextureFromD3D11Texture(ID3D11Texture2D* texture) override
	{
		return nullptr;
	}
};

void FrontendNuiInterface::Initialize(citizen::GameWindow* window)
{
	m_window = window;

	m_program = bgfx::createProgram(bgfx::createShader(bgfx::makeRef(vs_debugdraw_fill_texture_dx11, sizeof(vs_debugdraw_fill_texture_dx11))),
		bgfx::createShader(bgfx::makeRef(fs_debugdraw_fill_texture_dx11, sizeof(fs_debugdraw_fill_texture_dx11))));

	u_params = bgfx::createUniform("u_params", bgfx::UniformType::Vec4, 4);
	u_modelViewProj = bgfx::createUniform("u_modelViewProj", bgfx::UniformType::Vec4, 4);
	s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
}

static FrontendNuiInterface nuiGi;

nui::GameInterface* InitializeNui(citizen::GameWindow* window)
{
	nuiGi.Initialize(window);
	nui::Initialize(&nuiGi);

	Instance<vfs::Manager>::Set(new vfs::ManagerServer());

	vfs::Mount(new vfs::RelativeDevice(ToNarrow(MakeRelativeCitPath(L"citizen/"))), "citizen:/");

	nuiGi.OnInitVfs();
	nuiGi.OnInitRenderer();

	nuiGi.OnRender.Connect([]()
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
	});

	return &nuiGi;
}
