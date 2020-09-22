#include <StdInc.h>
#include <DrawCommands.h>
#include <grcTexture.h>

#include <MinHook.h>

#include <Hooking.h>

static rage::grcTextureFactory* g_textureFactory;
static rage::grcTexture* g_noneTexture;

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
	assert(!"none");

	return g_noneTexture;
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
	return *(void**)(*(uintptr_t*)(__readgsqword(88)) + g_sgaGraphicsContextOffset);
}

static intptr_t* gtaImShader;// = (intptr_t*)0x143D96A60;
static intptr_t* gtaImTechnique;// = (intptr_t*)0x143F1303C; // CS_BLIT

static HookFunction hf([]()
{
	gtaImShader = hook::get_address<intptr_t*>(hook::get_pattern("41 B9 10 00 00 00 48 8B 0D ? ? ? ? F3 0F 7F 44 24 20", 9));
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

static int32_t g_renderThreadTlsIndex = 1052;

bool IsOnRenderThread()
{
	char* moduleTls = *(char**)__readgsqword(88);

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

	static std::once_flag of;

	std::call_once(of, []()
	{
		OnGrcCreateDevice();
	});

	OnPostFrontendRender();
}

static hook::cdecl_stub<void(void*, void*, bool)> setDSs([]()
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
	setSRs(get_sgaGraphicsContext(), x, y, z, w);
}

static uint64_t** sgaDriver;

static void(*origEndDraw)(void*);
static void WrapEndDraw(void* cxt)
{
	// pattern near vtbl call: 4C 8B 46 08 44 0F  B7 4E 1A 48 8B 0C F8 (non-inlined in new)
	//(*(void(__fastcall**)(__int64, void*))(**(uint64_t**)sgaDriver + 896i64))(*(uint64_t*)sgaDriver, cxt);
	(*(void(__fastcall**)(__int64, void*))(**(uint64_t**)sgaDriver + 0x328))(*(uint64_t*)sgaDriver, cxt);

	// get swapchain backbuffer
	void* rt[1];
	//rt[0] = (*(void* (__fastcall**)(__int64))(**(uint64_t**)sgaDriver + 1984i64))(*(uint64_t*)sgaDriver);
	//rt[0] = (*(void* (__fastcall**)(__int64))(**(uint64_t**)sgaDriver + 1992i64))(*(uint64_t*)sgaDriver);
	rt[0] = (*(void*(__fastcall**)(__int64))(**(uint64_t**)sgaDriver + 0x778))(*(uint64_t*)sgaDriver);

	// pattern near vtbl call:
	//41 B1 01 BA 01 00 00 00 48 8B 0C D8

	static auto ds = hook::get_address<int*>(hook::get_pattern("44 8B CE 48 89 05 ? ? ? ? 41 B8 02", 19));

	setRTs(cxt, 1, rt, true);
	setDSs(cxt, *(void**)ds, true);
	//(*(void(__fastcall**)(__int64, void*, uint64_t, uint64_t, uint64_t, char, char))(**(uint64_t**)sgaDriver + 880i64))(*(uint64_t*)sgaDriver, cxt, NULL, NULL, NULL, 1, 0);
	(*(void(__fastcall**)(__int64, void*, uint64_t, uint64_t, uint64_t, char, char))(**(uint64_t**)sgaDriver + 0x318))(*(uint64_t*)sgaDriver, cxt, NULL, NULL, NULL, 1, 0);
	InvokeRender();
	// end draw
	//(*(void(__fastcall**)(__int64, void*))(**(uint64_t**)sgaDriver + 896i64))(*(uint64_t*)sgaDriver, cxt);
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
}

static HookFunction hookFunction([]()
{
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
	sgaDriver = hook::get_address<decltype(sgaDriver)>(hook::get_pattern("C6 82 ? ? 00 00 01 C6 82 ? ? 00 00 01", 17));

	g_textureFactory = hook::get_address<decltype(g_textureFactory)>(hook::get_pattern("48 8D 54 24 50 C7 44 24 50 80 80 00 00 48 8B C8", 0x25));

	g_d3d12Driver = hook::get_address<void*>(hook::get_pattern("83 E9 01 74 ? 83 F9 02 75 ? 48 8B CF E8", -4));
	g_vkDriver = hook::get_address<void*>(hook::get_pattern("B9 03 00 00 00 48 83 3D ? ? ? ? 00 0F", 25));

	g_d3d12Device = hook::get_address<decltype(g_d3d12Device)>(hook::get_pattern("48 8B 01 FF 50 78 48 8B 0B 48 8D", -7));
	g_vkHandle = hook::get_address<decltype(g_vkHandle)>(hook::get_pattern("8D 50 41 8B CA 44 8B C2 F3 48 AB 48 8B 0D", 14));

	{
		auto location = hook::get_pattern<char>("83 25 ? ? ? ? 00 83 25 ? ? ? ? 00 D1 F8 89 05", -0x26);
		rage::g_WindowWidth = hook::get_address<int*>(location + 6);
		rage::g_WindowHeight = hook::get_address<int*>(location + 0x3E);
	}

	// #TODORDR: badly force d3d12 sga driver (vulkan crashes on older Windows 10?)
	hook::iat("kernel32.dll", GetCommandLineWHook, "GetCommandLineW");

	MH_EnableHook(MH_ALL_HOOKS);
});
