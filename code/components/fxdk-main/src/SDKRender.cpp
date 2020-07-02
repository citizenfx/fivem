#include <StdInc.h>

#include <HostSharedData.h>
#include <ReverseGameData.h>

#include <d3d11.h>
#include <dcomp.h>

#include <wrl.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <LauncherIPC.h>
#include <BgfxPrepare.h>

namespace WRL = Microsoft::WRL;

#include <bx/uint32_t.h>

bgfx::VertexLayout DebugUvVertex::ms_decl;

namespace fxdk
{
namespace
{
	std::mutex& m_renderMutex = *(new std::mutex());

	int g_width = 1280, g_height = 720;

	bgfx::TextureHandle m_textures[4];
	bgfx::TextureHandle m_backTexture{ bgfx::kInvalidHandle };
	bgfx::TextureHandle m_frontTexture2{ bgfx::kInvalidHandle };
	bgfx::FrameBufferHandle m_frontBuffer{ bgfx::kInvalidHandle };
	bgfx::ProgramHandle m_program{ bgfx::kInvalidHandle };

	bgfx::VertexBufferHandle m_vbuf{ bgfx::kInvalidHandle };
	bgfx::IndexBufferHandle m_ibuf{ bgfx::kInvalidHandle };

	bgfx::UniformHandle s_texColor{ bgfx::kInvalidHandle };
	bgfx::UniformHandle u_params{ bgfx::kInvalidHandle };
	bgfx::UniformHandle u_modelViewProj{ bgfx::kInvalidHandle };
}

struct GameRenderData
{
	HANDLE handle;
	int width;
	int height;
	bool requested;

	GameRenderData()
		: requested(false)
	{
	}
};

static void CreateTextures(int width, int height)
{
	std::vector<uint32_t> mem(width * height);

	for (size_t i = 0; i < mem.size(); i++)
	{
		mem[i] = 0xFF0000FF;
	}

	for (int i = 0; i < std::size(m_textures); i++)
	{
		if (bgfx::isValid(m_textures[i]))
		{
			bgfx::destroy(m_textures[i]);
		}

		m_textures[i] = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8);

		bgfx::updateTexture2D(m_textures[i], 0, 0, 0, 0, width, height, bgfx::copy(mem.data(), mem.size() * 4));
	}

	if (bgfx::isValid(m_backTexture))
	{
		bgfx::destroy(m_backTexture);
	}

	m_backTexture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_BLIT_DST);

	if (bgfx::isValid(m_frontBuffer))
	{
		bgfx::destroy(m_frontBuffer);
	}

	m_frontBuffer = bgfx::createFrameBuffer(width, height, bgfx::TextureFormat::BGRA8);

	m_frontTexture2 = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_BLIT_DST);
	bgfx::updateTexture2D(m_frontTexture2, 0, 0, 0, 0, width, height, bgfx::copy(mem.data(), mem.size() * 4));
	bgfx::frame();

	auto d3dDevice = (ID3D11Device*)bgfx::getInternalData()->context;

	D3D11_TEXTURE2D_DESC texDesc = { 0 };
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

	WRL::ComPtr<ID3D11Texture2D> d3dTex;
	HRESULT hr = d3dDevice->CreateTexture2D(&texDesc, nullptr, &d3dTex);
	if (FAILED(hr))
	{
		return;
	}

	bgfx::overrideInternal(m_frontTexture2, (uintptr_t)d3dTex.Get());
	bgfx::updateTexture2D(m_frontTexture2, 0, 0, 0, 0, width, height, bgfx::copy(mem.data(), mem.size() * 4));

	static HostSharedData<GameRenderData> handleData("CfxGameRenderHandle");
	handleData->width = width;
	handleData->height = height;

	WRL::ComPtr<IDXGIResource> dxgiResource;
	HANDLE sharedHandle;
	hr = d3dTex.As(&dxgiResource);
	if (FAILED(hr))
	{
		// error handling code
		return;
	}

	hr = dxgiResource->GetSharedHandle(&sharedHandle);
	if FAILED (hr)
	{
		// error handling code
	}

	handleData->handle = sharedHandle;
}

void InitRender()
{
	// set the platform info
	bgfx::PlatformData pd;
	pd.nwh = NULL;

	bgfx::setPlatformData(pd);

	bgfx::renderFrame();

	bgfx::Init init;
	init.type = bgfx::RendererType::Direct3D11;
	init.resolution.width = g_width;
	init.resolution.height = g_height;
	init.debug = false;
	bgfx::init(init);

	bgfx::reset(g_width, g_height, /*BGFX_RESET_VSYNC | */ BGFX_RESET_FLIP_AFTER_RENDER);

	bgfx::setDebug(0);

	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xff00ff00, 1.0f, 0);
	bgfx::setViewClear(1, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xff00ff00, 1.0f, 0);

	m_program = bgfx::createProgram(bgfx::createShader(bgfx::makeRef(vs_debugdraw_fill_texture_dx11, sizeof(vs_debugdraw_fill_texture_dx11))),
	bgfx::createShader(bgfx::makeRef(fs_debugdraw_fill_texture_dx11, sizeof(fs_debugdraw_fill_texture_dx11))));

	DebugUvVertex::init();

	u_params = bgfx::createUniform("u_params", bgfx::UniformType::Vec4, 4);
	u_modelViewProj = bgfx::createUniform("u_modelViewProj", bgfx::UniformType::Vec4, 4);
	s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

	m_vbuf = bgfx::createVertexBuffer(bgfx::makeRef(s_quadVertices, sizeof(s_quadVertices)), DebugUvVertex::ms_decl);
	m_ibuf = bgfx::createIndexBuffer(bgfx::makeRef(s_quadIndices, sizeof(s_quadIndices)));

	bgfx::frame();

	CreateTextures(init.resolution.width, init.resolution.height);

	static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");
	rgd->isLauncher = true;

	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;

	int surfaceLimit = 1;

	auto inputMutex = CreateMutex(&sa, FALSE, NULL);
	auto queueSema = CreateSemaphore(&sa, 0, surfaceLimit, NULL);
	auto queueSema2 = CreateSemaphore(&sa, surfaceLimit, surfaceLimit, NULL);

	rgd->inputMutex = inputMutex;
	rgd->consumeSema = queueSema;
	rgd->produceSema = queueSema2;
	rgd->surfaceLimit = surfaceLimit;
	rgd->produceIdx = 1;
}

extern ipc::Endpoint& GetIPC();
static bool widthChanged = false;

void ResizeRender(int w, int h)
{
	std::unique_lock<std::mutex> lock(m_renderMutex);

	if (g_width != w || g_height != h)
	{
		g_width = w;
		g_height = h;

		widthChanged = true;

		GetIPC().Call("resizeWindow", w, h);
	}
}

void Render()
{
	static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");
	static HostSharedData<GameRenderData> handleData("CfxGameRenderHandle");
	static bool inited;
	static WRL::ComPtr<IDXGIKeyedMutex> mutexes[4];

	rgd->width = g_width;
	rgd->height = g_height;

	if (!inited)
	{
		Sleep(5);
	}

	if (widthChanged)
	{
		bgfx::reset(g_width, g_height, /*BGFX_RESET_VSYNC | */ BGFX_RESET_FLIP_AFTER_RENDER);

		widthChanged = false;
	}

	std::unique_lock<std::mutex> lock(m_renderMutex);

	if (!rgd->mainWindowHandle)
	{
		rgd->mainWindowHandle = NULL;
	}

	if ((rgd->inited && !inited) || rgd->createHandles)
	{
		inited = true;

		// recreate in case we resized before
		CreateTextures(rgd->width, rgd->height);

		// overriding is only allowed a frame later
		bgfx::frame();

		for (int i = 0; i < rgd->surfaceLimit; i++)
		{
			WRL::ComPtr<ID3D11Texture2D> sharedTexture;

			HRESULT hr = ((ID3D11Device*)bgfx::getInternalData()->context)->OpenSharedResource(rgd->surfaces[i], __uuidof(ID3D11Texture2D), (void**)sharedTexture.GetAddressOf());

			sharedTexture.As(&mutexes[i]);

			if (SUCCEEDED(hr))
			{
				D3D11_TEXTURE2D_DESC d;
				sharedTexture->GetDesc(&d);
				bgfx::overrideInternal(m_textures[i], (uintptr_t)sharedTexture.Get());
			}
		}

		rgd->createHandles = false;
	}

	bool blitted = false;

	bgfx::setViewRect(0, 0, 0, g_width, g_height);

	if (inited)
	{
		lock.unlock();

		int texIdx = -1;

		do
		{
			texIdx = rgd->FetchSurface(500);

			if (!rgd->inited)
			{
				inited = false;
				break;
			}
		} while (texIdx == -1);

		lock.lock();

		if (texIdx >= 0)
		{
			bgfx::blit(0, m_frontTexture2, 0, 0, m_textures[texIdx]);
		}
	}

	bgfx::touch(0);

	bgfx::frame();

	if (inited)
	{
		rgd->ReleaseSurface();
	}
}
}
