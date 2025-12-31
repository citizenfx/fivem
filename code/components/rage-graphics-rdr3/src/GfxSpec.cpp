#include <StdInc.h>
#include <DrawCommands.h>
#include <grcTexture.h>

#include <MinHook.h>
#include <CrossBuildRuntime.h>

#include <Hooking.h>
#include <HostSharedData.h>
#include <d3d12.h>
#include <d3d11.h>
#include <vulkan/vulkan.h>
#include <chrono>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "vulkan-1.lib")

static rage::grcTextureFactory* g_textureFactory;

namespace rage
{
grcTextureFactory* grcTextureFactory::getInstance()
{
	return g_textureFactory;
}

static hook::cdecl_stub<grcTexture * (grcTextureFactory*, const char*, grcTextureReference*, void*)> _create([]()
{
	return hook::get_pattern("48 8B F8 48 85 C0 0F 84 ? ? ? ? 41 8D 49 03", -0x28);
});

namespace sga
{
static hook::cdecl_stub < Texture * (const char* name, const ImageParams & params, int bufferType, uint32_t flags1, void* memInfo, uint32_t flags2, int cpuAccessType, void* clearValue, const void* conversionInfo, Texture* other)> _createFactory([]()
{
	return hook::get_call(hook::get_pattern("8B 45 50 89 44 24 28 48 8B 45 48 48 89 44 24 20 E8", 16));
});

Texture* Factory::CreateTexture(const char* name, const ImageParams& params, int bufferType, uint32_t flags1, void* memInfo, uint32_t flags2, int cpuAccessType, void* clearValue, const void* conversionInfo, Texture* other)
{
	return _createFactory(name, params, bufferType, flags1, memInfo, flags2, cpuAccessType, clearValue, conversionInfo, other);
}
}

grcTexture* grcTextureFactory::createImage(const char* name, grcTextureReference* reference, void* createParams)
{
	return _create(this, name, reference, createParams);
}

grcTexture* grcTextureFactory::createManualTexture(short width, short height, int format, void* unknown, bool, const grcManualTextureDef* templ)
{
	sga::ImageParams ip;
	ip.width = width;
	ip.height = height;
	ip.depth = 1;
	ip.levels = 1;
	ip.dimension = 1;
	ip.bufferFormat = sga::BufferFormat::B8G8R8A8_UNORM;

	return static_cast<grcTexture*>(sga::Factory::CreateTexture("grcTexture", ip, 0, 2, nullptr, 8, 0, nullptr, nullptr, nullptr));
}

grcTexture* grcTextureFactory::GetNoneTexture()
{
	static auto noneTexture = []
	{
		sga::ImageParams ip;
		ip.width = 1;
		ip.height = 1;
		ip.depth = 1;
		ip.levels = 1;
		ip.dimension = 1;
		ip.bufferFormat = sga::BufferFormat::B8G8R8A8_UNORM;

		uint32_t white = 0xFFFFFFFF;

		auto texture = new rage::sga::ext::DynamicTexture2();
		texture->Init(3, nullptr, ip, 0, 2, nullptr, 8, 1, nullptr);

		if (texture)
		{
			texture->MakeReady(rage::sga::GraphicsContext::GetCurrent());

			rage::sga::MapData mapData;
			if (texture->Map(nullptr, mapData))
			{
				memcpy(mapData.GetBuffer(), &white, 4);
				texture->Unmap(rage::sga::GraphicsContext::GetCurrent(), mapData);
			}
		}

		return texture;
	}();

	return reinterpret_cast<grcTexture*>(noneTexture->GetTexture());
}
}

static hook::cdecl_stub<void(void*)> drawMatrix_ctor([]()
{
	return hook::get_pattern("40 53 48 83 EC 30 66 C7 81 70 06 00 00 01 00 0F 57 C0", 0);
});

struct grcViewport
{
	char pad[1752];

	grcViewport()
	{
		drawMatrix_ctor(this);
	}
};

// grcViewport::SetCurrent
static hook::cdecl_stub<void(grcViewport*, bool)> activateMatrix([]()
{
	return hook::get_pattern("48 85 DB 74 ? 8A 83 71 06 00 00", -0x32);
});

static grcViewport* g_imMatrix;

static hook::cdecl_stub<void(void*, void*, bool)> setScreenSpaceMatrix([]()
{
	return hook::get_pattern("75 16 65 48 8B 0C 25 58 00 00 00 BB ? ? 00 00", -0x1C);
});

// rage::grmShader::BeginDraw
static hook::thiscall_stub<void(intptr_t, int, int, int32_t)> setSubShader([]()
{
	return hook::get_pattern("41 8A D8 4C 8B D1 41 80 F9 FF", -0xC);
});

// rage::sga::Effect::BeginTechnique
static hook::thiscall_stub<void(intptr_t, void* cxt, int)> setSubShaderUnk([]()
{
	return hook::get_pattern("57 41 56 41 57 48 83 EC 20 4C 8B 19", -0xF);
});

static uint32_t g_sgaGraphicsContextOffset;

static void* get_sgaGraphicsContext()
{
	return *(void**)(hook::get_tls() + g_sgaGraphicsContextOffset);
}

static intptr_t* gtaImShader;// = (intptr_t*)0x143D96A60;
static intptr_t* gtaImTechnique;// = (intptr_t*)0x143F1303C; // CS_BLIT

static HookFunction hf([]()
{
	gtaImShader = hook::get_address<intptr_t*>(hook::get_pattern("41 B9 10 00 00 00 48 8B 0D ? ? ? ? F3 0F 7F 44 24 20 E8", 9));
	gtaImTechnique = hook::get_address<intptr_t*>(hook::get_pattern("C7 40 C0 00 00 80 3F 45 0F 57 C0 4D 8B F9 E8", 22));
});

static hook::cdecl_stub<int(intptr_t, const char*)> _getFunc([]()
{
	return hook::get_pattern("E8 ? ? ? ? 48 8B 0B 8B D0 48 83 C4 20", -0x11);
});

void PushDrawBlitImShader()
{
	if (!g_imMatrix)
	{
		g_imMatrix = new grcViewport();
	}

	// activate matrix
	activateMatrix(g_imMatrix, true);

	// set screen space stuff
	/*float mtx[4];
	mtx[0] = 0.f;
	mtx[1] = 0.f;
	mtx[2] = 1.f;
	mtx[3] = 1.f;*/

	setScreenSpaceMatrix(nullptr, nullptr, true);

	// old gtadrawblit was split up in various clip-space methods which don't transform anymore,
	// but rage_unlit_draw is similar in that it does a basic transform + textured draw
	intptr_t blit = _getFunc(*gtaImShader, "rage_unlit_draw");

	// shader methods: set subshader?
	setSubShader(*gtaImShader, 0, 0, blit);//*gtaImTechnique);
	setSubShaderUnk(*gtaImShader, get_sgaGraphicsContext(), 0);
}

static hook::cdecl_stub<void()> popImShaderAndResetParams([]
{
	return hook::get_pattern("48 83 EC 38 65 48 8B 04 25 58", 0);
});

void PopDrawBlitImShader()
{
	popImShaderAndResetParams();
}

void EnqueueGenericDrawCommand(void(*cb)(uintptr_t, uintptr_t), uintptr_t* arg1, uintptr_t* arg2)
{
	if (!*gtaImShader)
	{
		return;
	}

	if (!IsOnRenderThread())
	{
		assert(!"render");
	}
	else
	{
		cb(*arg1, *arg2);
	}
}

static hook::cdecl_stub<void(rage::sga::Texture*)> setTextureGtaIm([]()
{
	return hook::pattern("40 53 48 83 EC 20 8B 15 ? ? ? ? 48 8B D9 4C 8B C1").count(2).get(1).get<void>();
});

static hook::cdecl_stub<void(const float*, const float*)> setImGenParams([]()
{
	return hook::get_pattern("41 B9 10 00 00 00 F3 0F 7F 44 24 20", -0x1E);
});

void SetTextureGtaIm(rage::sga::Texture* texture)
{
	alignas(16) const float params1[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	alignas(16) const float params2[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	setImGenParams(params1, params2);

	setTextureGtaIm(texture);
}

static int32_t g_renderThreadTlsIndex;

bool IsOnRenderThread()
{
	char* moduleTls = hook::get_tls();

	return (*reinterpret_cast<int32_t*>(moduleTls + g_renderThreadTlsIndex) & 2) != 0;
}

static hook::cdecl_stub<void()> drawImVertices([]()
{
	return hook::get_call(hook::get_pattern("F3 44 0F 11 44 24 20 E8 ? ? ? ? E8", 12));
});

static hook::cdecl_stub<void(int, int, int)> beginImVertices([]()
{
	return hook::get_pattern("4C 39 92 ? ? 00 00 74 2E", -0x49);
});

static hook::cdecl_stub<void(float, float, float, float, float, float, uint32_t, float, float)> addImVertex([]()
{
	return hook::get_pattern("48 83 EC 78 8B 05 ? ? ? ? 65 48", 0);
});

namespace rage
{
	void grcBegin(int type, int count)
	{
		beginImVertices(type, count, 0);
	}

	void grcVertex(float x, float y, float z, float nX, float nY, float nZ, uint32_t color, float u, float v)
	{
		// colors are the other way around in Payne again, so RGBA-swap we go
		//color = (color & 0xFF00FF00) | _rotl(color & 0x00FF00FF, 16);

		addImVertex(x, y, z, nX, nY, nZ, color, u, v);
	}

	void grcEnd()
	{
		drawImVertices();
	}
}

namespace rage
{
	int* g_WindowWidth;
	int* g_WindowHeight;
}

void GetGameResolution(int& x, int& y)
{
	if (!rage::g_WindowWidth || !rage::g_WindowHeight)
	{
		x = 0;
		y = 0;
		return;
	}

	x = *rage::g_WindowWidth;
	y = *rage::g_WindowHeight;
}

static hook::cdecl_stub<bool(float*, int)> _transformToScreenSpace([]()
{
	return hook::get_pattern("48 83 EC 30 48 63 DA 48 8B F9 E8", -6);
});

bool TransformToScreenSpace(float* verts2d, int len)
{
	return _transformToScreenSpace(verts2d, len);
}

static uint16_t* stockStates[StateTypeMax];

uint32_t GetStockStateIdentifier(StateType state)
{
	if (state >= 0 && state < StateTypeMax)
	{
		if (stockStates[state])
		{
			return *stockStates[state];
		}
	}

	return 0;
}

static uint32_t* diffSS;

uint32_t GetImDiffuseSamplerState()
{
	return 0x01D57A7E;
}

static hook::cdecl_stub<void(intptr_t, int, uint8_t)> _setSamplerState([]
{
	return hook::get_pattern("45 8A D0 84 D2 74", 0);
});

static hook::cdecl_stub<int(intptr_t, const char*, int, int)> _getSamplerIdx([]
{
	return hook::get_call(hook::get_pattern("89 47 48 48 8B D6 45 33 C9 41 B0 03", 15));
});

static hook::cdecl_stub<void(intptr_t, int)> _popSamplerState([]()
{
	return hook::get_pattern("48 8B 41 20 80 3C 02 FF 74 53 44 0F B6 0C 02", -0x21);
});

void SetImDiffuseSamplerState(uint32_t samplerStateIdentifier)
{
	int sampler = _getSamplerIdx(*gtaImShader, "DiffuseSampler", 2, 1);
	assert(sampler);

	if (samplerStateIdentifier == 0x01D57A7E)
	{
		return _popSamplerState(*gtaImShader, sampler);
	}

	_setSamplerState(*gtaImShader, sampler, samplerStateIdentifier);
}

static uint32_t g_rsOffset1;
static uint32_t g_rsOffset2;
static uint32_t g_rsOffset3;

static uint32_t g_swapchainBackbufferOffset;

uint32_t GetRasterizerState()
{
	return *(uint8_t*)((char*)get_sgaGraphicsContext() + g_rsOffset1);
}

void SetRasterizerState(uint32_t state)
{
	if (GetRasterizerState() != (uint8_t)state)
	{
		char* cxt = (char*)get_sgaGraphicsContext();

		uint32_t o = *(uint32_t*)(cxt + g_rsOffset2);

		if (*(uint8_t*)(cxt + g_rsOffset3) == (uint8_t)state)
		{
			*(uint32_t*)(cxt + g_rsOffset2) &= ~4;
		}
		else
		{
			*(uint32_t*)(cxt + g_rsOffset2) |= 4;
		}

		*(uint8_t*)((char*)get_sgaGraphicsContext() + g_rsOffset1) = state;
	}
}

static HookFunction hookFunctionRS([]()
{
	// 1207.80: 0x9CBC/40124
	g_rsOffset1 = *hook::get_pattern<uint32_t>("3A D1 74 22 8B 83 ? ? ? ? 88 8B", -4);

	// 1207.80: 0x9DC0/40384
	g_rsOffset2 = *hook::get_pattern<uint32_t>("3A D1 74 22 8B 83 ? ? ? ? 88 8B", 6);

	// 1207.80: 0x9D2C/40236
	g_rsOffset3 = *hook::get_pattern<uint32_t>("3A D1 74 22 8B 83 ? ? ? ? 88 8B", 18);

	// 1311.20: 0x778/1912, 1436.25: 0x7B8/1976
	g_swapchainBackbufferOffset = *hook::get_pattern<uint32_t>("41 B1 01 BA 01 00 00 00 48 8B 0C D8", -24);
});

uint32_t GetBlendState()
{
	// 1207.80: 40136
	return *(uint16_t*)((char*)get_sgaGraphicsContext() + g_rsOffset1 + 12);
}

void SetBlendState(uint32_t state)
{
	if (GetBlendState() != state)
	{
		char* cxt = (char*)get_sgaGraphicsContext();

		uint32_t o = *(uint32_t*)(cxt + g_rsOffset2);
		*(uint32_t*)(cxt + g_rsOffset2) |= 16;

		*(uint8_t*)((char*)get_sgaGraphicsContext() + g_rsOffset1 + 12) = state;
	}
}

uint32_t GetDepthStencilState()
{
	return *(uint16_t*)((char*)get_sgaGraphicsContext() + g_rsOffset1 + 2);
}

void SetDepthStencilState(uint32_t state)
{
	if (GetDepthStencilState() != state)
	{
		char* cxt = (char*)get_sgaGraphicsContext();

		// 1207.80: 40238
		if (*(uint8_t*)(cxt + g_rsOffset3 + 2) == state)
		{
			*(uint32_t*)(cxt + g_rsOffset2) &= ~8;
		}
		else
		{
			*(uint32_t*)(cxt + g_rsOffset2) |= 8;
		}

		// 1207.80: 40126
		*(uint8_t*)((char*)get_sgaGraphicsContext() + g_rsOffset1 + 2) = state;
	}
}

fwEvent<> OnPostFrontendRender;
fwEvent<> OnGrcCreateDevice;

uint16_t pointSampler;

static void InvokeRender()
{
	// only init fxdb loaded?
	if (!*gtaImShader)
	{
		return;
	}

	if (!pointSampler)
	{
		uint32_t state[9];
		state[0] = 0x1010100;
		state[1] = 0;
		state[2] = 0x101;
		state[3] = 0;
		state[4] = 0;
		state[5] = 0;
		state[6] = 0;
		state[7] = 0;
		state[8] = 0x41400000; // 12.0f

		static auto fn = hook::get_call(hook::get_pattern("66 C7 45 E1 01 00 40 88 7D E3", 14));
		pointSampler = ((uint8_t(*)(void* state))fn)(&state);
	}

	static std::once_flag of;

	std::call_once(of, []()
	{
		OnGrcCreateDevice();
	});

	OnPostFrontendRender();
}

// rage::sga::GraphicsContext::SetDepthStencil
static hook::cdecl_stub<void(void*, void*, uint8_t, uint8_t)> setDSs([]()
{
	return hook::get_call(hook::get_pattern("41 B0 01 48 8B D3 48 8B CF E8 ? ? ? ? 48 83", 9));
});

static hook::cdecl_stub<void(void*, int, void**, bool)> setRTs([]()
{
	return hook::get_pattern("41 56 48 83 EC 20 33 DB 41 8A E9", -0x13);
});

static hook::cdecl_stub<void(void*, int, int, int, int)> setSRs([]()
{
	return hook::get_pattern("89 44 24 0C 89 14 24 44 89 44", -0x8);
});

void SetScissorRect(int x, int y, int z, int w)
{
	setSRs(get_sgaGraphicsContext(), x, y, z - x, w - y);
}

static uint64_t** sgaDriver;

// D3D12 frame capture
static ID3D12Resource* g_d3d12StagingBuffer = nullptr;
static ID3D11Device* g_d3d11Device = nullptr;
static ID3D11DeviceContext* g_d3d11Context = nullptr;
static ID3D11Texture2D* g_d3d11StagingTexture = nullptr;
static ID3D11Texture2D* g_d3d11SharedTexture = nullptr;
static HANDLE g_d3d11SharedHandle = nullptr;
static ID3D12CommandQueue* g_captureCommandQueue = nullptr;
static ID3D12Fence* g_fence = nullptr;
static HANDLE g_fenceEvent = nullptr;
static UINT64 g_fenceValue = 0;
static ID3D12CommandQueue** g_commandQueuePtr = nullptr;
static int g_lastWidth = 0;
static int g_lastHeight = 0;

// Vulkan frame capture
static VkDevice g_vkDevice = VK_NULL_HANDLE;
static VkPhysicalDevice g_vkPhysicalDevice = VK_NULL_HANDLE;
static VkQueue g_vkGraphicsQueue = VK_NULL_HANDLE;
static uint32_t g_vkGraphicsQueueFamily = 0;
static VkCommandPool g_vkCommandPool = VK_NULL_HANDLE;
static VkImage g_vkIntermediateImage = VK_NULL_HANDLE;
static VkDeviceMemory g_vkIntermediateImageMemory = VK_NULL_HANDLE;
static int g_vkIntermediateImageWidth = 0;
static int g_vkIntermediateImageHeight = 0;
static VkBuffer g_vkStagingBuffer = VK_NULL_HANDLE;
static VkDeviceMemory g_vkStagingBufferMemory = VK_NULL_HANDLE;
static int g_vkStagingBufferSize = 0;
static VkFence g_vkCopyFence = VK_NULL_HANDLE;
static bool g_vkCopyInProgress = false;
static VkCommandBuffer g_vkReusableCommandBuffer = VK_NULL_HANDLE;
static std::vector<VkImage> g_cachedSwapchainImages;
static VkSwapchainKHR g_cachedSwapchain = VK_NULL_HANDLE;

struct GameRenderData
{
	HANDLE handle;
	int width;
	int height;
	bool requested;

	GameRenderData()
		: requested(false), handle(NULL), width(0), height(0)
	{
	}
};

static HostSharedData<GameRenderData> g_renderData("CfxGameRenderHandle");

static bool InitializeD3D11Device(int width, int height)
{
	if (g_d3d11Device)
	{
		return true;
	}

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	HRESULT hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&g_d3d11Device,
		nullptr,
		&g_d3d11Context
	);

	if (FAILED(hr))
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC stagingDesc = {};
	stagingDesc.Width = width;
	stagingDesc.Height = height;
	stagingDesc.MipLevels = 1;
	stagingDesc.ArraySize = 1;
	stagingDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	stagingDesc.SampleDesc.Count = 1;
	stagingDesc.Usage = D3D11_USAGE_DYNAMIC;
	stagingDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = g_d3d11Device->CreateTexture2D(&stagingDesc, nullptr, &g_d3d11StagingTexture);
	if (FAILED(hr))
	{
		g_d3d11Context->Release();
		g_d3d11Device->Release();
		g_d3d11Context = nullptr;
		g_d3d11Device = nullptr;
		return false;
	}

	D3D11_TEXTURE2D_DESC sharedDesc = {};
	sharedDesc.Width = width;
	sharedDesc.Height = height;
	sharedDesc.MipLevels = 1;
	sharedDesc.ArraySize = 1;
	sharedDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sharedDesc.SampleDesc.Count = 1;
	sharedDesc.Usage = D3D11_USAGE_DEFAULT;
	sharedDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	sharedDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

	hr = g_d3d11Device->CreateTexture2D(&sharedDesc, nullptr, &g_d3d11SharedTexture);
	if (FAILED(hr))
	{
		g_d3d11StagingTexture->Release();
		g_d3d11Context->Release();
		g_d3d11Device->Release();
		g_d3d11StagingTexture = nullptr;
		g_d3d11Context = nullptr;
		g_d3d11Device = nullptr;
		return false;
	}

	IDXGIResource* dxgiResource = nullptr;
	hr = g_d3d11SharedTexture->QueryInterface(__uuidof(IDXGIResource), (void**)&dxgiResource);
	if (SUCCEEDED(hr))
	{
		hr = dxgiResource->GetSharedHandle(&g_d3d11SharedHandle);
		dxgiResource->Release();
		
		if (SUCCEEDED(hr))
		{
			return true;
		}
	}

	g_d3d11SharedTexture->Release();
	g_d3d11StagingTexture->Release();
	g_d3d11Context->Release();
	g_d3d11Device->Release();
	g_d3d11SharedTexture = nullptr;
	g_d3d11StagingTexture = nullptr;
	g_d3d11Context = nullptr;
	g_d3d11Device = nullptr;
	return false;
}

static void CaptureFrame_D3D12(void* backbuffer, int width, int height)
{
	static int frameCount = 0;
	frameCount++;
	
	if (!IsOnRenderThread() || width <= 0 || height <= 0 || !backbuffer)
	{
		return;
	}

	// Capture only once per second
	static auto lastCaptureTime = std::chrono::steady_clock::now();
	auto currentTime = std::chrono::steady_clock::now();
	auto elapsedSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastCaptureTime).count() / 1000.0;
	
	if (elapsedSeconds < 1.0)
	{
		return;
	}
	
	lastCaptureTime = currentTime;

	if (!g_commandQueuePtr || !*g_commandQueuePtr)
	{
		return;
	}
	
	ID3D12CommandQueue* commandQueue = *g_commandQueuePtr;
	ID3D12Device* device = (ID3D12Device*)GetGraphicsDriverHandle();
	
	if (!device || !commandQueue)
	{
		return;
	}

	// Recreate D3D12 staging buffer and D3D11 textures on resolution change
	if (g_lastWidth != width || g_lastHeight != height)
	{
		if (g_d3d12StagingBuffer)
		{
			g_d3d12StagingBuffer->Release();
			g_d3d12StagingBuffer = nullptr;
		}
		
		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.MipLevels = 1;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		UINT64 bufferSize;
		device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &bufferSize);
		
		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = bufferSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_READBACK;
		
		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&g_d3d12StagingBuffer)
		);

		if (FAILED(hr) || !InitializeD3D11Device(width, height))
		{
			return;
		}

		g_lastWidth = width;
		g_lastHeight = height;
	}

	// Initialize command queue and fence
	if (!g_captureCommandQueue)
	{
		HRESULT hr = commandQueue->GetDevice(__uuidof(ID3D12Device), (void**)&device);
		if (SUCCEEDED(hr) && device)
		{
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			queueDesc.NodeMask = 0;
			
			hr = device->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), (void**)&g_captureCommandQueue);
			if (FAILED(hr))
			{
				device->Release();
				return;
			}
			
			hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&g_fence);
			if (FAILED(hr))
			{
				g_captureCommandQueue->Release();
				g_captureCommandQueue = nullptr;
				device->Release();
				return;
			}
			
			g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (!g_fenceEvent)
			{
				g_fence->Release();
				g_fence = nullptr;
				g_captureCommandQueue->Release();
				g_captureCommandQueue = nullptr;
			}
			
			device->Release();
		}
	}

	if (!g_captureCommandQueue || !g_fence || !g_fenceEvent)
	{
		return;
	}

	// Extract backbuffer resource
	uint8_t* innerStruct = *reinterpret_cast<uint8_t**>(reinterpret_cast<uint8_t*>(backbuffer) + 0x08);
	if (!innerStruct)
	{
		return;
	}

	uint8_t unk0x10 = *(innerStruct + 0x10);
	uint16_t unk0x0C = *reinterpret_cast<uint16_t*>(innerStruct + 0x0C);
	
	ID3D12Resource* backbufferResource = ((unk0x10 & 7) != 0 || (unk0x0C & 0xF000) != 0xC000)
		? *reinterpret_cast<ID3D12Resource**>(innerStruct + 0x20)
		: *reinterpret_cast<ID3D12Resource**>(innerStruct + 0x48);

	if (!backbufferResource)
	{
		return;
	}

	ID3D12CommandAllocator* frameAllocator = nullptr;
	ID3D12GraphicsCommandList* frameCommandList = nullptr;
	
	HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&frameAllocator);
	if (FAILED(hr) || !frameAllocator)
	{
		return;
	}
	
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frameAllocator, nullptr, __uuidof(ID3D12GraphicsCommandList), (void**)&frameCommandList);
	if (FAILED(hr) || !frameCommandList)
	{
		frameAllocator->Release();
		return;
	}

	// Copy backbuffer to staging buffer
	D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
	srcLocation.pResource = backbufferResource;
	srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	srcLocation.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
	dstLocation.pResource = g_d3d12StagingBuffer;
	dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	dstLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	dstLocation.PlacedFootprint.Footprint.Width = width;
	dstLocation.PlacedFootprint.Footprint.Height = height;
	dstLocation.PlacedFootprint.Footprint.Depth = 1;
	dstLocation.PlacedFootprint.Footprint.RowPitch = width * 4;

	frameCommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

	hr = frameCommandList->Close();
	if (FAILED(hr))
	{
		frameCommandList->Release();
		frameAllocator->Release();
		return;
	}

	// Execute and wait for GPU
	ID3D12CommandList* commandLists[] = { frameCommandList };
	g_captureCommandQueue->ExecuteCommandLists(1, commandLists);
	
	g_fenceValue++;
	hr = g_captureCommandQueue->Signal(g_fence, g_fenceValue);
	if (FAILED(hr))
	{
		frameCommandList->Release();
		frameAllocator->Release();
		return;
	}

	hr = g_fence->SetEventOnCompletion(g_fenceValue, g_fenceEvent);
	if (FAILED(hr))
	{
		frameCommandList->Release();
		frameAllocator->Release();
		return;
	}

	if (WaitForSingleObject(g_fenceEvent, 1000) != WAIT_OBJECT_0)
	{
		frameCommandList->Release();
		frameAllocator->Release();
		return;
	}

	// Read D3D12 buffer and transfer to D3D11 shared texture
	void* mappedData = nullptr;
	D3D12_RANGE readRange = { 0, static_cast<SIZE_T>(width * height * 4) };
	hr = g_d3d12StagingBuffer->Map(0, &readRange, &mappedData);
	
	if (SUCCEEDED(hr))
	{
		D3D11_MAPPED_SUBRESOURCE d3d11Mapped = {};
		hr = g_d3d11Context->Map(g_d3d11StagingTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3d11Mapped);
		
		if (SUCCEEDED(hr))
		{
			uint8_t* srcData = static_cast<uint8_t*>(mappedData);
			uint8_t* dstData = static_cast<uint8_t*>(d3d11Mapped.pData);
			
			// Copy with vertical flip (DirectX coordinates are inverted)
			for (int y = 0; y < height; ++y)
			{
				uint8_t* srcRow = srcData + ((height - 1 - y) * width * 4);
				memcpy(dstData, srcRow, width * 4);
				dstData += d3d11Mapped.RowPitch;
			}

			g_d3d11Context->Unmap(g_d3d11StagingTexture, 0);
			g_d3d11Context->CopyResource(g_d3d11SharedTexture, g_d3d11StagingTexture);

			// Update shared memory for NUI
			g_renderData->handle = g_d3d11SharedHandle;
			g_renderData->width = width;
			g_renderData->height = height;
			g_renderData->requested = false;
		}
		
		D3D12_RANGE writtenRange = { 0, 0 };
		g_d3d12StagingBuffer->Unmap(0, &writtenRange);
	}
	
	// Clean up frame resources
	frameCommandList->Release();
	frameAllocator->Release();
}

static uint32_t FindVulkanMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(g_vkPhysicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	trace("^1[Vulkan Frame Capture] Failed to find suitable memory type!\n");
	return 0;
}

static bool InitializeVulkanCaptureResources(int width, int height)
{
	if (!g_vkDevice || g_vkDevice == VK_NULL_HANDLE)
	{
		return false;
	}

	if (!g_vkPhysicalDevice || g_vkPhysicalDevice == VK_NULL_HANDLE)
	{
		return false;
	}

	if (!g_vkGraphicsQueue || g_vkGraphicsQueue == VK_NULL_HANDLE)
	{
		return false;
	}

	if (g_vkCommandPool == VK_NULL_HANDLE)
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = g_vkGraphicsQueueFamily;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VkResult result = vkCreateCommandPool(g_vkDevice, &poolInfo, nullptr, &g_vkCommandPool);
		if (result != VK_SUCCESS || g_vkCommandPool == VK_NULL_HANDLE)
		{
			trace("^1[Vulkan Frame Capture] Failed to create command pool: %d\n", result);
			return false;
		}
	}

	if (g_vkIntermediateImage == VK_NULL_HANDLE || g_vkIntermediateImageWidth != width || g_vkIntermediateImageHeight != height)
	{
		if (g_vkIntermediateImage != VK_NULL_HANDLE)
		{
			vkDestroyImage(g_vkDevice, g_vkIntermediateImage, nullptr);
			g_vkIntermediateImage = VK_NULL_HANDLE;
		}
		if (g_vkIntermediateImageMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(g_vkDevice, g_vkIntermediateImageMemory, nullptr);
			g_vkIntermediateImageMemory = VK_NULL_HANDLE;
		}

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkResult result = vkCreateImage(g_vkDevice, &imageInfo, nullptr, &g_vkIntermediateImage);
		if (result != VK_SUCCESS)
		{
			trace("^1[Vulkan Frame Capture] Failed to create intermediate image: %d\n", result);
			return false;
		}

		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(g_vkDevice, g_vkIntermediateImage, &memReq);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = FindVulkanMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		result = vkAllocateMemory(g_vkDevice, &allocInfo, nullptr, &g_vkIntermediateImageMemory);
		if (result != VK_SUCCESS)
		{
			trace("^1[Vulkan Frame Capture] Failed to allocate intermediate image memory: %d\n", result);
			vkDestroyImage(g_vkDevice, g_vkIntermediateImage, nullptr);
			g_vkIntermediateImage = VK_NULL_HANDLE;
			return false;
		}

		result = vkBindImageMemory(g_vkDevice, g_vkIntermediateImage, g_vkIntermediateImageMemory, 0);
		if (result != VK_SUCCESS)
		{
			trace("^1[Vulkan Frame Capture] Failed to bind intermediate image memory: %d\n", result);
			return false;
		}

		g_vkIntermediateImageWidth = width;
		g_vkIntermediateImageHeight = height;
	}

	int requiredBufferSize = width * height * 4;
	if (g_vkStagingBuffer == VK_NULL_HANDLE || g_vkStagingBufferSize < requiredBufferSize)
	{
		if (g_vkStagingBuffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(g_vkDevice, g_vkStagingBuffer, nullptr);
			g_vkStagingBuffer = VK_NULL_HANDLE;
		}
		if (g_vkStagingBufferMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(g_vkDevice, g_vkStagingBufferMemory, nullptr);
			g_vkStagingBufferMemory = VK_NULL_HANDLE;
		}

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = requiredBufferSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto result = vkCreateBuffer(g_vkDevice, &bufferInfo, nullptr, &g_vkStagingBuffer);
		if (result != VK_SUCCESS)
		{
			trace("^1[Vulkan Frame Capture] Failed to create staging buffer: %d\n", result);
			return false;
		}

		VkMemoryRequirements memReq;
		vkGetBufferMemoryRequirements(g_vkDevice, g_vkStagingBuffer, &memReq);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = FindVulkanMemoryType(memReq.memoryTypeBits, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		result = vkAllocateMemory(g_vkDevice, &allocInfo, nullptr, &g_vkStagingBufferMemory);
		if (result != VK_SUCCESS)
		{
			trace("^1[Vulkan Frame Capture] Failed to allocate staging buffer memory: %d\n", result);
			vkDestroyBuffer(g_vkDevice, g_vkStagingBuffer, nullptr);
			g_vkStagingBuffer = VK_NULL_HANDLE;
			return false;
		}

		result = vkBindBufferMemory(g_vkDevice, g_vkStagingBuffer, g_vkStagingBufferMemory, 0);
		if (result != VK_SUCCESS)
		{
			trace("^1[Vulkan Frame Capture] Failed to bind staging buffer memory: %d\n", result);
			return false;
		}

		g_vkStagingBufferSize = requiredBufferSize;
	}

	if (g_vkCopyFence == VK_NULL_HANDLE)
	{
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = 0;

		auto result = vkCreateFence(g_vkDevice, &fenceInfo, nullptr, &g_vkCopyFence);
		if (result != VK_SUCCESS)
		{
			trace("^1[Vulkan Frame Capture] Failed to create fence: %d\n", result);
			return false;
		}
	}

	if (g_vkReusableCommandBuffer == VK_NULL_HANDLE)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = g_vkCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		auto result = vkAllocateCommandBuffers(g_vkDevice, &allocInfo, &g_vkReusableCommandBuffer);
		if (result != VK_SUCCESS)
		{
			trace("^1[Vulkan Frame Capture] Failed to allocate reusable command buffer: %d\n", result);
			return false;
		}
	}

	return true;
}

static void TransferVulkanDataToD3D11(uint8_t* vulkanPixels, int width, int height)
{
	if (!g_d3d11Device)
	{
		if (!InitializeD3D11Device(width, height))
		{
			return;
		}
	}
	else
	{
		D3D11_TEXTURE2D_DESC desc;
		g_d3d11StagingTexture->GetDesc(&desc);
		
		if (desc.Width != width || desc.Height != height)
		{
			if (g_d3d11SharedHandle)
			{
				g_d3d11SharedHandle = nullptr;
			}
			if (g_d3d11SharedTexture)
			{
				g_d3d11SharedTexture->Release();
				g_d3d11SharedTexture = nullptr;
			}
			if (g_d3d11StagingTexture)
			{
				g_d3d11StagingTexture->Release();
				g_d3d11StagingTexture = nullptr;
			}
			
			D3D11_TEXTURE2D_DESC stagingDesc = {};
			stagingDesc.Width = width;
			stagingDesc.Height = height;
			stagingDesc.MipLevels = 1;
			stagingDesc.ArraySize = 1;
			stagingDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			stagingDesc.SampleDesc.Count = 1;
			stagingDesc.Usage = D3D11_USAGE_DYNAMIC;
			stagingDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			HRESULT hr = g_d3d11Device->CreateTexture2D(&stagingDesc, nullptr, &g_d3d11StagingTexture);
			if (FAILED(hr))
			{
				return;
			}

			D3D11_TEXTURE2D_DESC sharedDesc = {};
			sharedDesc.Width = width;
			sharedDesc.Height = height;
			sharedDesc.MipLevels = 1;
			sharedDesc.ArraySize = 1;
			sharedDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			sharedDesc.SampleDesc.Count = 1;
			sharedDesc.Usage = D3D11_USAGE_DEFAULT;
			sharedDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			sharedDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

			hr = g_d3d11Device->CreateTexture2D(&sharedDesc, nullptr, &g_d3d11SharedTexture);
			if (FAILED(hr))
			{
				g_d3d11StagingTexture->Release();
				g_d3d11StagingTexture = nullptr;
				return;
			}

			IDXGIResource* dxgiResource = nullptr;
			hr = g_d3d11SharedTexture->QueryInterface(__uuidof(IDXGIResource), (void**)&dxgiResource);
			if (SUCCEEDED(hr))
			{
				hr = dxgiResource->GetSharedHandle(&g_d3d11SharedHandle);
				dxgiResource->Release();
			}
		}
	}

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = g_d3d11Context->Map(g_d3d11StagingTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (FAILED(hr))
	{
		return;
	}

	uint8_t* dst = (uint8_t*)mapped.pData;
	for (int y = 0; y < height; y++)
	{
		int srcY = height - 1 - y;
		memcpy(dst + y * mapped.RowPitch, 
		       vulkanPixels + srcY * width * 4, 
		       width * 4);
	}

	g_d3d11Context->Unmap(g_d3d11StagingTexture, 0);
	g_d3d11Context->CopyResource(g_d3d11SharedTexture, g_d3d11StagingTexture);

	auto nuiRenderData = HostSharedData<GameRenderData>("CfxGameRenderHandle");
	nuiRenderData->handle = g_d3d11SharedHandle;
	nuiRenderData->width = width;
	nuiRenderData->height = height;
}

GFX_EXPORT void CaptureVulkanSwapchainImage(void* swapchainPtr, uint32_t imageIndex, int width, int height)
{
	VkSwapchainKHR swapchain = (VkSwapchainKHR)swapchainPtr;
	
	if (!g_vkCommandPool)
	{
		if (!InitializeVulkanCaptureResources(width, height))
		{
			return;
		}
	}

	// Check if previous copy finished
	if (g_vkCopyInProgress)
	{
		VkResult fenceStatus = vkGetFenceStatus(g_vkDevice, g_vkCopyFence);
		if (fenceStatus == VK_SUCCESS)
		{
			void* mappedData = nullptr;
			VkResult result = vkMapMemory(g_vkDevice, g_vkStagingBufferMemory, 0, g_vkStagingBufferSize, 0, &mappedData);
			if (result == VK_SUCCESS)
			{
				TransferVulkanDataToD3D11((uint8_t*)mappedData, width, height);
				vkUnmapMemory(g_vkDevice, g_vkStagingBufferMemory);
			}
			
			g_vkCopyInProgress = false;
			vkResetFences(g_vkDevice, 1, &g_vkCopyFence);
		}
	}
	
	// Capture only once per second
	static auto lastCaptureTime = std::chrono::steady_clock::now();
	auto currentTime = std::chrono::steady_clock::now();
	auto elapsedSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastCaptureTime).count() / 1000.0;
	
	if (elapsedSeconds < 1.0)
	{
		return;
	}
	
	// If copy is in progress, skip
	if (g_vkCopyInProgress)
	{
		return;
	}
	
	lastCaptureTime = currentTime;

	// Cache swapchain images
	if (g_cachedSwapchain != swapchain)
	{
		uint32_t imageCount = 0;
		VkResult result = vkGetSwapchainImagesKHR(g_vkDevice, swapchain, &imageCount, nullptr);
		if (result != VK_SUCCESS || imageCount == 0)
		{
			return;
		}

		g_cachedSwapchainImages.resize(imageCount);
		result = vkGetSwapchainImagesKHR(g_vkDevice, swapchain, &imageCount, g_cachedSwapchainImages.data());
		if (result != VK_SUCCESS)
		{
			return;
		}
		
		g_cachedSwapchain = swapchain;
	}

	if (imageIndex >= g_cachedSwapchainImages.size())
	{
		return;
	}

	VkImage swapchainImage = g_cachedSwapchainImages[imageIndex];

	// Reset and reuse command buffer instead of allocating new one
	VkCommandBuffer cmdBuffer = g_vkReusableCommandBuffer;
	
	VkResult result = vkResetCommandBuffer(cmdBuffer, 0);
	if (result != VK_SUCCESS)
	{
		return;
	}

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	result = vkBeginCommandBuffer(cmdBuffer, &beginInfo);
	if (result != VK_SUCCESS)
	{
		return;
	}

	// Transition intermediate image to TRANSFER_DST
	VkImageMemoryBarrier intermediateBarrier = {};
	intermediateBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	intermediateBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	intermediateBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	intermediateBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	intermediateBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	intermediateBarrier.image = g_vkIntermediateImage;
	intermediateBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	intermediateBarrier.subresourceRange.baseMipLevel = 0;
	intermediateBarrier.subresourceRange.levelCount = 1;
	intermediateBarrier.subresourceRange.baseArrayLayer = 0;
	intermediateBarrier.subresourceRange.layerCount = 1;
	intermediateBarrier.srcAccessMask = 0;
	intermediateBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &intermediateBarrier
	);

	// Transition swapchain to TRANSFER_SRC
	VkImageMemoryBarrier swapchainBarrier = {};
	swapchainBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	swapchainBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	swapchainBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	swapchainBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	swapchainBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	swapchainBarrier.image = swapchainImage;
	swapchainBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	swapchainBarrier.subresourceRange.baseMipLevel = 0;
	swapchainBarrier.subresourceRange.levelCount = 1;
	swapchainBarrier.subresourceRange.baseArrayLayer = 0;
	swapchainBarrier.subresourceRange.layerCount = 1;
	swapchainBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	swapchainBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &swapchainBarrier
	);

	// Copy swapchain → intermediate
	VkImageCopy copyRegion = {};
	copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.srcSubresource.mipLevel = 0;
	copyRegion.srcSubresource.baseArrayLayer = 0;
	copyRegion.srcSubresource.layerCount = 1;
	copyRegion.srcOffset = {0, 0, 0};
	copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.dstSubresource.mipLevel = 0;
	copyRegion.dstSubresource.baseArrayLayer = 0;
	copyRegion.dstSubresource.layerCount = 1;
	copyRegion.dstOffset = {0, 0, 0};
	copyRegion.extent = {(uint32_t)width, (uint32_t)height, 1};

	vkCmdCopyImage(
		cmdBuffer,
		swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		g_vkIntermediateImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &copyRegion
	);

	// Restore swapchain to PRESENT_SRC
	swapchainBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	swapchainBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	swapchainBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	swapchainBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &swapchainBarrier
	);

	// Transition intermediate to TRANSFER_SRC
	intermediateBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	intermediateBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	intermediateBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	intermediateBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &intermediateBarrier
	);

	// Copy image → staging buffer
	VkBufferImageCopy bufferCopyRegion = {};
	bufferCopyRegion.bufferOffset = 0;
	bufferCopyRegion.bufferRowLength = 0;
	bufferCopyRegion.bufferImageHeight = 0;
	bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferCopyRegion.imageSubresource.mipLevel = 0;
	bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
	bufferCopyRegion.imageSubresource.layerCount = 1;
	bufferCopyRegion.imageOffset = {0, 0, 0};
	bufferCopyRegion.imageExtent = {(uint32_t)width, (uint32_t)height, 1};

	vkCmdCopyImageToBuffer(
		cmdBuffer,
		g_vkIntermediateImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		g_vkStagingBuffer,
		1, &bufferCopyRegion
	);

	vkEndCommandBuffer(cmdBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	result = vkQueueSubmit(g_vkGraphicsQueue, 1, &submitInfo, g_vkCopyFence);
	if (result != VK_SUCCESS)
	{
		return;
	}
	
	g_vkCopyInProgress = true;
}

static void(*origEndDraw)(void*);
static void WrapEndDraw(void* cxt)
{
	// pattern near vtbl call: 4C 8B 46 08 44 0F  B7 4E 1A 48 8B 0C F8 (non-inlined in new)
	(*(void(__fastcall**)(__int64, void*))(**(uint64_t**)sgaDriver + 0x328))(*(uint64_t*)sgaDriver, cxt);

	// get swapchain backbuffer
	void* rt[1];
	rt[0] = (*(void*(__fastcall**)(__int64))(**(uint64_t**)sgaDriver + g_swapchainBackbufferOffset))(*(uint64_t*)sgaDriver);

	setRTs(cxt, 1, rt, true);
	setDSs(cxt, nullptr, 0, 0);
	(*(void(__fastcall**)(__int64, void*, uint64_t, uint64_t, uint64_t, char, char))(**(uint64_t**)sgaDriver + 0x318))(*(uint64_t*)sgaDriver, cxt, NULL, NULL, NULL, 1, 0);

	// Vulkan capture is handled in RenderHooks.cpp
	if (rt[0] && GetCurrentGraphicsAPI() == GraphicsAPI::D3D12)
	{
		int width, height;
		GetGameResolution(width, height);
		
		CaptureFrame_D3D12(rt[0], width, height);
	}

	InvokeRender();
	// end draw
	(*(void(__fastcall**)(__int64, void*))(**(uint64_t**)sgaDriver + 0x328))(*(uint64_t*)sgaDriver, cxt);

	origEndDraw(cxt);
}

static LPWSTR GetCommandLineWHook()
{
	static wchar_t str[8192];
	wcscpy(str, GetCommandLineW());

	if (!wcsstr(str, L"sgadriver"))
	{
		wcscat(str, L" -sgadriver=d3d12");
	}

	return str;
}

static void* g_d3d12Driver;
static void* g_vkDriver;

GraphicsAPI GetCurrentGraphicsAPI()
{
	if (*sgaDriver == g_d3d12Driver)
	{
		return GraphicsAPI::D3D12;
	}
	else if (*sgaDriver == g_vkDriver)
	{
		return GraphicsAPI::Vulkan;
	}

	return GraphicsAPI::Unknown;
}

void** g_d3d12Device;
VkDevice* g_vkHandle;

void SetVulkanDeviceHandles(void* device, void* physicalDevice, void* queue, uint32_t queueFamily)
{
	g_vkDevice = (VkDevice)device;
	g_vkPhysicalDevice = (VkPhysicalDevice)physicalDevice;
	g_vkGraphicsQueue = (VkQueue)queue;
	g_vkGraphicsQueueFamily = queueFamily;
}

void* GetGraphicsDriverHandle()
{
	switch (GetCurrentGraphicsAPI())
	{
	case GraphicsAPI::D3D12:
		return *g_d3d12Device;
	case GraphicsAPI::Vulkan:
		return *g_vkHandle;
	default:
		return nullptr;
	}
}

namespace rage::sga
{
	void Driver_Create_ShaderResourceView(rage::sga::Texture* texture, const rage::sga::TextureViewDesc& desc)
	{
		(*(void(__fastcall**)(__int64, void*, void*, const void*))(**(uint64_t**)sgaDriver + 256i64))(*(uint64_t*)sgaDriver, *(char**)((char*)texture + 48), texture, &desc);
	}

	void Driver_Destroy_Texture(rage::sga::Texture* texture)
	{
		(*(void(__fastcall**)(__int64, void*))(**(uint64_t**)sgaDriver + 440i64))(*(uint64_t*)sgaDriver, texture);
	}

	GraphicsContext* GraphicsContext::GetCurrent()
	{
		return reinterpret_cast<GraphicsContext*>(get_sgaGraphicsContext());
	}
}

static hook::thiscall_stub<int(rage::sga::ext::DynamicResource*)> _dynamicResource_GetResourceIdx([]()
{
	return hook::get_call(hook::get_pattern("0F B7 56 6A 8B C0 4C 8B 04 C3 65", -5));
});

static hook::thiscall_stub<void(rage::sga::ext::DynamicResource*, rage::sga::GraphicsContext*, const rage::sga::MapData&)> _dynamicResource_UnmapBase([]()
{
	return hook::get_call(hook::get_pattern("4C 8D 45 B0 33 D2 48 8B CF E8 ? ? ? ? 4C 8D", 9));
});

static hook::thiscall_stub<void(rage::sga::ext::DynamicTexture2*, rage::sga::GraphicsContext*, const rage::sga::MapData&)> _dynamicTexture2_UnmapInternal([]()
{
	return hook::get_call(hook::get_pattern("4C 8D 45 B0 33 D2 48 8B CF E8 ? ? ? ? 4C 8D", 0x17));
});

static hook::thiscall_stub<int(rage::sga::ext::DynamicTexture2*, rage::sga::GraphicsContext*, rage::sga::MapData&)> _dynamicTexture2_Map([]()
{
	return hook::get_call(hook::get_pattern("48 89 5D E0 48 89 5D E8 E8 ? ? ? ? 48 8B CF", 8));
});

static hook::thiscall_stub<void(rage::sga::ext::DynamicTexture2*, rage::sga::GraphicsContext*)> _dynamicTexture2_MakeReady([]()
{
	return hook::get_call(hook::get_pattern("48 8B 14 2A E8 ? ? ? ? 48 8D 5B 08", 4));
});

static hook::thiscall_stub<void(rage::sga::ext::DynamicTexture2*, int flags, void* unk_nullptr, const rage::sga::ImageParams& imageParams, int bufferType, uint32_t flags1, void* memInfo, uint32_t flags2, int cpuAccessType, void* clearValue)> _dynamicTexture2_Init([]()
{
	return hook::get_call(hook::get_pattern("C7 44 24 28 02 00 00 00 C6 44 24 20 01 E8 ? ? ? ? 48 8B CB E8", 13));
});

static hook::thiscall_stub<void(rage::sga::ext::DynamicTexture2*)> _dynamicTexture2_dtor([]()
{
	// this is actually rage::sga::ext::DynamicTextureUav::~dtor
	//return hook::pattern("48 8D 59 60 BD 04 00 00 00 48 8B 3B 48").count(2).get(0).get<void>(-0x17);

	return hook::get_pattern("BE 04 00 00 00 48 8B F9 8B EE", -0x14);
});

namespace rage::sga::ext
{
DynamicResource::DynamicResource()
{
	memset(m_pad, 0, sizeof(m_pad));
}

int DynamicResource::GetResourceIdx()
{
	return _dynamicResource_GetResourceIdx(this);
}

void DynamicResource::UnmapBase(GraphicsContext* context, const MapData& mapData)
{
	return _dynamicResource_UnmapBase(this, context, mapData);
}

DynamicTexture2::~DynamicTexture2()
{
	_dynamicTexture2_dtor(this);
}

void DynamicTexture2::MakeReady(GraphicsContext* context)
{
	return _dynamicTexture2_MakeReady(this, context);
}

void DynamicTexture2::Init(int flags, void* unk_nullptr, const ImageParams& imageParams, int bufferType, uint32_t flags1, void* memInfo, uint32_t flags2, int cpuAccessType, void* clearValue)
{
	return _dynamicTexture2_Init(this, flags, unk_nullptr, imageParams, bufferType, flags1, memInfo, flags2, cpuAccessType, clearValue);
}

Texture* DynamicTexture2::GetTexture()
{
	return ((Texture**)this)[GetResourceIdx()];
}

bool DynamicTexture2::Map(GraphicsContext* context, MapData& mapData)
{
	return _dynamicTexture2_Map(this, context, mapData);
}

void DynamicTexture2::Unmap(GraphicsContext* context, const MapData& mapData)
{
	UnmapBase(context, mapData);
	UnmapInternal(context, mapData);
}

void DynamicTexture2::UnmapInternal(GraphicsContext* context, const MapData& mapData)
{
	return _dynamicTexture2_UnmapInternal(this, context, mapData);
}
}

static bool ShouldUsePipelineCache(const char* pipelineCachePrefix)
{
	// #TODO: check for ERR_GFX_STATE or similar failures last launch?
	return true;
}

static HookFunction hookFunction([]()
{
	hook::jump(hook::get_pattern("48 83 EC 28 48 8D 15 ? ? ? ? 48 2B D1 8A 01"), ShouldUsePipelineCache);

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("48 8B CB E8 ? ? ? ? 48 8B 0D ? ? ? ? 0F 57 ED", -0x1D), WrapEndDraw, (void**)&origEndDraw);

	g_sgaGraphicsContextOffset = *hook::get_pattern<uint32_t>("48 8B 0C D8 48 8B 14 0E 41 C6 40 18 00 C6 82", -21);

	// rage::sga::RS_NoBackfaceCull
	stockStates[RasterizerStateNoCulling] = hook::get_address<uint16_t*>(hook::get_pattern("48 8D 4D BF 88 05 ? ? ? ? C6 45 BF 02", 6));

	stockStates[DepthStencilStateNoDepth] = hook::get_address<uint16_t*>(hook::get_pattern("48 8D 4D BF 88 05 ? ? ? ? C6 45 BF 02", -186));
	stockStates[SamplerStatePoint] = (uint16_t*)&pointSampler;

	// rage::sga::BS_Default
	stockStates[BlendStateNoBlend] = hook::get_address<uint16_t*>(hook::get_pattern("48 8D 4D BF 88 05 ? ? ? ? C6 45 BF 02", 60));

	// rage::sga::BS_Normal
	stockStates[BlendStateDefault] = hook::get_address<uint16_t*>(hook::get_pattern("48 8D 4D BF 88 05 ? ? ? ? C6 45 BF 02", 167));

	// rage::sga::BS_AlphaAdd
	//stockStates[BlendStatePremultiplied] = hook::get_address<uint16_t*>(hook::get_pattern("48 8D 4D BF 88 05 ? ? ? ? C6 45 BF 02", 347));
	stockStates[BlendStatePremultiplied] = stockStates[BlendStateDefault];

	diffSS = hook::get_address<decltype(diffSS)>(hook::get_pattern("66 C7 45 60 15 03 C6 45 62 03", 17));
	sgaDriver = hook::get_address<decltype(sgaDriver)>(hook::get_pattern("C6 82 ? ? 00 00 01 C6 82 ? ? 00 00 01 48 8B 0D", 17));

	g_textureFactory = hook::get_address<decltype(g_textureFactory)>(hook::get_pattern("48 8D 54 24 50 C7 44 24 50 80 80 00 00 48 8B C8", 0x25));

	g_renderThreadTlsIndex = *hook::get_pattern<uint32_t>("42 09 0C 02 BA 01 00 00 00 3B CA 0F 44 C2 88 05", -15);

	g_d3d12Driver = hook::get_address<void*>(hook::get_pattern("75 04 33 C0 EB 1A E8", 28));

	g_vkDriver = hook::get_address<void*>(hook::get_pattern("33 C0 EB 2F 41 B8 80 00 00 00", 47));

	g_d3d12Device = hook::get_address<decltype(g_d3d12Device)>(hook::get_pattern("48 8B 01 FF 50 78 48 8B 0B 48 8D", -7));
	g_vkHandle = hook::get_address<decltype(g_vkHandle)>(hook::get_pattern("8D 50 41 8B CA 44 8B C2 F3 48 AB 48 8B 0D", 14));

	{
		auto location = hook::get_pattern<char>("83 25 ? ? ? ? 00 83 25 ? ? ? ? 00 D1 F8 89 05", -0x26);
		rage::g_WindowWidth = hook::get_address<int*>(location + 6);
		rage::g_WindowHeight = hook::get_address<int*>(location + 0x3E);
	}

	// #TODORDR: badly force d3d12 sga driver (vulkan crashes on older Windows 10?)
	g_commandQueuePtr = hook::get_address<ID3D12CommandQueue**>(hook::get_pattern("4C 8D 0D ? ? ? ? 4C 89 65", 3));

	hook::iat("kernel32.dll", GetCommandLineWHook, "GetCommandLineW");

	MH_EnableHook(MH_ALL_HOOKS);
});
