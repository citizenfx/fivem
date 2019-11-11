#include <StdInc.h>
#include <DrawCommands.h>
#include <grcTexture.h>

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

grcTexture* grcTextureFactory::createImage(const char* name, grcTextureReference* reference, void* createParams)
{
	return _create(this, name, reference, createParams);
}

grcTexture* grcTextureFactory::GetNoneTexture()
{
	assert(!"none");

	return g_noneTexture;
}
}

static hook::cdecl_stub<void(void*)> drawMatrix_ctor([]()
{
	return (void*)0x1425B6F14;
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
	return (void*)0x140192904;
});

static grcViewport* g_imMatrix;

static hook::cdecl_stub<void(void*, void*, bool)> setScreenSpaceMatrix([]()
{
	return (void*)0x140586144;
});

// rage::grmShader::BeginDraw
static hook::thiscall_stub<void(intptr_t, int, int, int32_t)> setSubShader([]()
{
	return (void*)0x14050CA70;
});

// rage::sga::Effect::BeginTechnique
static hook::thiscall_stub<void(intptr_t, void* cxt, int)> setSubShaderUnk([]()
{
	return (void*)0x1404EF9E0;
});

static void* get_sgaGraphicsContext()
{
	return *(void**)(*(uintptr_t*)(__readgsqword(88)) + 5040);
}

static intptr_t* gtaImShader = (intptr_t*)0x143D96A60;
static intptr_t* gtaImTechnique = (intptr_t*)0x143F1303C; // CS_BLIT

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
	intptr_t blit = ((int(*)(intptr_t, const char*))0x142619834)(*gtaImShader, "rage_unlit_draw");

	// shader methods: set subshader?
	setSubShader(*gtaImShader, 0, 0, blit);//*gtaImTechnique);
	setSubShaderUnk(*gtaImShader, get_sgaGraphicsContext(), 0);
}

static hook::cdecl_stub<void()> popImShaderAndResetParams([]
{
	return (void*)0x140538AF0;
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
	return (void*)0x14058C3A8;
});

static hook::cdecl_stub<void(const float*, const float*)> setImGenParams([]()
{
	return (void*)0x140589B38;
});

void SetTextureGtaIm(rage::sga::Texture* texture)
{
	alignas(16) const float params1[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	alignas(16) const float params2[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	setImGenParams(params1, params2);

	setTextureGtaIm(texture);
}

// 142AC0FDC
static int32_t g_renderThreadTlsIndex = 1052;

bool IsOnRenderThread()
{
	char* moduleTls = *(char**)__readgsqword(88);

	return (*reinterpret_cast<int32_t*>(moduleTls + g_renderThreadTlsIndex) & 2) != 0;
}

static hook::cdecl_stub<void()> drawImVertices([]()
{
	return (void*)0x1425C97D0;
});

static hook::cdecl_stub<void(int, int, int)> beginImVertices([]()
{
	return (void*)0x1425C7E60;
});

static hook::cdecl_stub<void(float, float, float, float, float, float, uint32_t, float, float)> addImVertex([]()
{
	return (void*)0x1425C9944;
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
	return (void*)0x140561574;
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

uint32_t GetImDiffuseSamplerState()
{
	return *(uint8_t*)0x145938978;
}

static hook::cdecl_stub<void(intptr_t, int, uint8_t)> _setSamplerState([]
{
	return (void*)0x141261BF0;
});

void SetImDiffuseSamplerState(uint32_t samplerStateIdentifier)
{
	int sampler = ((int(*)(intptr_t, const char*, int, int))0x142619990)(*gtaImShader, "DiffuseSampler", 2, 1);
	assert(sampler);

	_setSamplerState(*gtaImShader, sampler, samplerStateIdentifier);
}

uint32_t GetRasterizerState()
{
	return *(uint8_t*)((char*)get_sgaGraphicsContext() + 40124);
}

void SetRasterizerState(uint32_t state)
{
	if (GetRasterizerState() != (uint8_t)state)
	{
		char* cxt = (char*)get_sgaGraphicsContext();

		uint32_t o = *(uint32_t*)(cxt + 40384);

		if (*(uint8_t*)(cxt + 40236) == (uint8_t)state)
		{
			*(uint32_t*)(cxt + 40384) &= ~4;
		}
		else
		{
			*(uint32_t*)(cxt + 40384) |= 4;
		}

		*(uint8_t*)((char*)get_sgaGraphicsContext() + 40124) = state;
	}
}

uint32_t GetBlendState()
{
	return *(uint16_t*)((char*)get_sgaGraphicsContext() + 40136);
}

void SetBlendState(uint32_t state)
{
	if (GetBlendState() != state)
	{
		char* cxt = (char*)get_sgaGraphicsContext();

		uint32_t o = *(uint32_t*)(cxt + 40384);
		*(uint32_t*)(cxt + 40384) |= 16;

		*(uint8_t*)((char*)get_sgaGraphicsContext() + 40136) = state;
	}
}

uint32_t GetDepthStencilState()
{
	return *(uint16_t*)((char*)get_sgaGraphicsContext() + 40126);
}

void SetDepthStencilState(uint32_t state)
{
	if (GetDepthStencilState() != state)
	{
		char* cxt = (char*)get_sgaGraphicsContext();

		uint32_t o = *(uint32_t*)(cxt + 40384);

		if (*(uint8_t*)(cxt + 40238) == state)
		{
			*(uint32_t*)(cxt + 40384) &= ~8;
		}
		else
		{
			*(uint32_t*)(cxt + 40384) |= 8;
		}

		*(uint8_t*)((char*)get_sgaGraphicsContext() + 40126) = state;
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

	pointSampler = ((uint8_t(*)(void* state))0x1425F5B30)(&state);

	if (!*(uintptr_t*)((char*)get_sgaGraphicsContext() + 40600))
	{
		//return;
	}

	static std::once_flag of;

	std::call_once(of, []()
	{
		OnGrcCreateDevice();
	});

	OnPostFrontendRender();
}

static hook::cdecl_stub<void(void*, void*, bool)> setDSs([]()
{
	return (void*)0x142625348;
});

static hook::cdecl_stub<void(void*, int, void**, bool)> setRTs([]()
{
	return (void*)0x142625784;
});

static hook::cdecl_stub<void(void*, int, int, int, int)> setSRs([]()
{
	return (void*)0x14058C7DC;
});

void SetScissorRect(int x, int y, int z, int w)
{
	setSRs(get_sgaGraphicsContext(), x, y, z, w);
}

static void(*origEndDraw)(void*);
static void WrapEndDraw(void* cxt)
{
	(*(void(__fastcall**)(__int64, void*))(**(uint64_t**)0x14556ADA8 + 896i64))(*(uint64_t*)0x14556ADA8, cxt);

	// get swapchain backbuffer
	void* rt[1];
	rt[0] = (*(void* (__fastcall**)(__int64))(**(uint64_t**)0x14556ADA8 + 1984i64))(*(uint64_t*)0x14556ADA8);

	setRTs(cxt, 1, rt, true);
	setDSs(cxt, *(void**)0x143D96B68, true);
	(*(void(__fastcall**)(__int64, void*, uint64_t, uint64_t, uint64_t, char, char))(**(uint64_t**)0x14556ADA8 + 880i64))(*(uint64_t*)0x14556ADA8, cxt, NULL, NULL, NULL, 1, 0);
	InvokeRender();
	// end draw
	(*(void(__fastcall**)(__int64, void*))(**(uint64_t**)0x14556ADA8 + 896i64))(*(uint64_t*)0x14556ADA8, cxt);

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

static HookFunction hookFunction([]()
{
	hook::set_call(&origEndDraw, 0x1425FDD84);
	hook::call(0x1425FDD84, WrapEndDraw);

	stockStates[RasterizerStateNoCulling] = (uint16_t*)0x1455624C7;
	stockStates[DepthStencilStateNoDepth] = (uint16_t*)0x1455624E0;
	stockStates[SamplerStatePoint] = (uint16_t*)&pointSampler;
	stockStates[BlendStateNoBlend] = (uint16_t*)0x145562504;
	stockStates[BlendStateDefault] = (uint16_t*)0x14556250C;

	g_textureFactory = hook::get_address<decltype(g_textureFactory)>(hook::get_pattern("EB 03 40 32 ED 83 64 24 30 00 48 8D 0D", 13));

	{
		auto location = hook::get_pattern<char>("83 25 0B ? ? ? ? 83 25 ? ? ? ? 00 D1 F8 89 05", -0x26);
		rage::g_WindowWidth = hook::get_address<int*>(location + 6);
		rage::g_WindowHeight = hook::get_address<int*>(location + 0x3E);
	}

	// #TODORDR: badly force d3d12 sga driver (vulkan crashes on older Windows 10?)
	hook::iat("kernel32.dll", GetCommandLineWHook, "GetCommandLineW");
});
