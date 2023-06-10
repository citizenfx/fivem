#include "StdInc.h"
#include "DrawCommands.h"
#include "Hooking.h"
#include "CfxRect.h"

#include <CrossBuildRuntime.h>

#define PURECALL() __asm { jmp _purecall }

namespace rage
{
	struct dlDrawCommandBuffer
	{
		void AddDrawCommand(int type);
	};
}

static hook::thiscall_stub<void(rage::dlDrawCommandBuffer*, int)> addDrawCommand([] ()
{
	return hook::pattern("48 8B D9 44 23 91 38 18 00 00 41 81").count(1).get(0).get<void>(-0xC);
});

static hook::cdecl_stub<void*(uintptr_t)> allocDrawCommand([] ()
{
	return hook::pattern("48 8B D9 4D 63 82 38 18 00 00 49").count(1).get(0).get<void>(-0xD);
});

class DrawCommand
{
private:
	uint32_t m_size;
	uint32_t m_pad;

public:
	inline void SetSize(int size)
	{
		m_size &= 0xFFF00000;
		m_size |= 0x33000;
		m_size |= size >> 2;
	}
};

class GenericTwoArgDrawCommand : public DrawCommand
{
private:
	void(*m_callback)(void*);

	char m_pad[16];

	void(*m_userCallback)(uintptr_t, uintptr_t);

	uintptr_t m_data1;
	
	uintptr_t m_data2;

private:
	void InvokeCommand()
	{
		m_userCallback(m_data1, m_data2);
	}

public:
	void Initialize(void(*userCB)(uintptr_t, uintptr_t), uintptr_t* data1, uintptr_t* data2)
	{
		m_callback = [] (void* data)
		{
			((GenericTwoArgDrawCommand*)(((char*)data) - 32))->InvokeCommand();
		};

		m_userCallback = userCB;
		m_data1 = *data1;
		m_data2 = *data2;
	}
};

template<typename T>
T* AllocateDrawCommand()
{
	T* dc = (T*)allocDrawCommand(sizeof(T));
	dc->SetSize(sizeof(T));

	return dc;
}

void rage::dlDrawCommandBuffer::AddDrawCommand(int type)
{
	return addDrawCommand(this, type);
}

static rage::dlDrawCommandBuffer** g_drawCommandBuffer;

void EnqueueGenericDrawCommand(void(*cb)(uintptr_t, uintptr_t), uintptr_t* arg1, uintptr_t* arg2)
{
	if (!IsOnRenderThread())
	{
		(*g_drawCommandBuffer)->AddDrawCommand(8);

		GenericTwoArgDrawCommand* dc = AllocateDrawCommand<GenericTwoArgDrawCommand>();
		dc->Initialize(cb, arg1, arg2);
	}
	else
	{
		cb(*arg1, *arg2);
	}
}

static int32_t g_renderThreadTlsIndex;

bool IsOnRenderThread()
{
	char* moduleTls = hook::get_tls();

	return (*reinterpret_cast<int32_t*>(moduleTls + g_renderThreadTlsIndex) & 2) != 0;
}

//void WRAPPER SetTexture(rage::grcTexture* texture) { EAXJMP(0x627FB0); }

static hook::cdecl_stub<void()> drawImVertices([] ()
{
	return hook::get_call(hook::pattern("E9 ? ? ? ? E8 ? ? ? ? 8B 7C 24  58 44").count(1).get(0).get<void>(5));
});

static hook::cdecl_stub<void(int, int)> beginImVertices([] ()
{
	return hook::pattern("48 8D 44 24 60 45 8D 41 24 8B D3 8B CF").count(1).get(0).get<void>(-0x5E);
});

static hook::cdecl_stub<void(float, float, float, float, float, float, uint32_t, float, float)> addImVertex([] ()
{
	return hook::pattern("F3 41 0F 11 01 F3 0F 10 44 24 28 F3 41").count(1).get(0).get<void>(-0x30);
});

namespace rage
{
	void grcBegin(int type, int count)
	{
		beginImVertices(type, count);
	}

	void grcVertex(float x, float y, float z, float nX, float nY, float nZ, uint32_t color, float u, float v)
	{
		// colors are the other way around in Payne again, so RGBA-swap we go
		color = (color & 0xFF00FF00) | _rotl(color & 0x00FF00FF, 16);

		addImVertex(x, y, z, nX, nY, nZ, color, u, v);
	}

	void grcEnd()
	{
		drawImVertices();
	}
}

static hook::cdecl_stub<void(void*, void*, bool)> setScreenSpaceMatrix([] ()
{
	return hook::pattern("48 85 C9 48 8B D9 48 8B FA 48 0F 44").count(1).get(0).get<void>(-0xA);
});

static hook::thiscall_stub<void(intptr_t, int, int, int32_t)> setSubShader([] ()
{
	return hook::pattern("4C 63 DA 83 F8 FF 75 07").count(1).get(0).get<void>(-0x6);
});

static hook::thiscall_stub<void(intptr_t, int)> setSubShaderUnk([] ()
{
	return hook::get_call(hook::pattern("F6 D8 1B D2 83 E2 02 E8 ? ? ? ? 41 8B D7").count(1).get(0).get<void>(7));
});

static intptr_t* gtaImShader;
static int32_t* gtaImTechnique;

static HookFunction shaderIdHookFunc([] ()
{
	auto shaderMatch = hook::pattern("F0 00 00 80 3F E8 ? ? ? ? 44 8B 0D").count(1).get(0).get<char>(13);

	gtaImTechnique = (int32_t*)(*(int32_t*)shaderMatch + shaderMatch + 4);

	shaderMatch += 7;

	gtaImShader = (intptr_t*)(*(int32_t*)shaderMatch + shaderMatch + 4);
});

static hook::cdecl_stub<void(void*)> drawMatrix_ctor([]()
{
	return hook::get_pattern("80 3F 33 C9 48 89 8B 10 04 00 00", -12);
});

struct drawMatrix
{
	char pad[1168];

	drawMatrix()
	{
		drawMatrix_ctor(this);
	}
};

static hook::cdecl_stub<void(drawMatrix*)> activateMatrix([]()
{
	return hook::get_pattern("40 8A F2 48 8B D9 75 14", -0x1D);
});

static drawMatrix* g_imMatrix;

void PushDrawBlitImShader()
{
	if (!g_imMatrix)
	{
		g_imMatrix = new drawMatrix();
	}

	// activate matrix
	activateMatrix(g_imMatrix);

	// set screen space stuff
	setScreenSpaceMatrix(nullptr, nullptr, true);

	// shader methods: set subshader?
	setSubShader(*gtaImShader, 0, 0, *gtaImTechnique);
	setSubShaderUnk(*gtaImShader, 0);
}

static hook::thiscall_stub<void(intptr_t)> popSubShader([] ()
{
	return hook::get_call(hook::pattern("48 8B 4F 48 E8 ? ? ? ? 48 8B 4F 48 E8").get(0).get<void>(4));
});

static hook::thiscall_stub<void(intptr_t)> popSubShaderUnk([] ()
{
	return hook::get_call(hook::pattern("48 8B 4F 48 E8 ? ? ? ? 48 8B 4F 48 E8").get(0).get<void>(13));
});

static hook::cdecl_stub<void()> popImShaderAndResetParams([] ()
{
	// 393-
	if (Is372())
	{
		return hook::get_call(hook::pattern("0F 28 D8 E8 ? ? ? ? 48 8D 4C 24 68 E8 ? ? ? ? 48 8B").count(1).get(0).get<void>(13));
	}

	// 463/505+
	return hook::get_call(hook::pattern("F3 0F 11 64 24 20 E8 ? ? ? ? 48 8D 8C 24 98").count(1).get(0).get<void>(19));
});

void PopDrawBlitImShader()
{
	//popSubShaderUnk(*gtaImShader);

	//popSubShader(*gtaImShader);

	popImShaderAndResetParams();
}

// params2 is unused for drawblit, params1 is global color multiplier (default multiplies alpha by 0 :( )
static hook::cdecl_stub<void(const float[4], const float[4])> setImGenParams([] ()
{
	return hook::get_call(hook::pattern("48 83 65 5F 00 48 8D 55 FF 48 8D 4D 0F E8 ? ? ? ? 83 65").count(1).get(0).get<void>(13));
});

static hook::cdecl_stub<void(rage::grcTexture*)> setTextureGtaIm([] ()
{
	return hook::pattern("48 8B D9 4C 8B C9 48 8B 0D").count(1).get(0).get<void>(-0xD);
});

void SetTextureGtaIm(rage::grcTexture* texture)
{
	const float params1[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const float params2[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	setImGenParams(params1, params2);

	setTextureGtaIm(texture);
}

//void WRAPPER DrawImSprite(float x1, float y1, float x2, float y2, float z, float u1, float v1, float u2, float v2, uint32_t* color, int subShader) { EAXJMP(0x852CE0); }
void DrawImSprite(float x1, float y1, float x2, float y2, float z, float u1, float v1, float u2, float v2, uint32_t* colorPtr, int subShader)
{
	auto oldRasterizerState = GetRasterizerState();
	SetRasterizerState(GetStockStateIdentifier(StateType::RasterizerStateNoCulling));

	auto oldDepthStencilState = GetDepthStencilState();
	SetDepthStencilState(GetStockStateIdentifier(StateType::DepthStencilStateNoDepth));

	PushDrawBlitImShader();

	uint32_t color = *colorPtr;

	// this swaps ABGR (as CRGBA is ABGR in little-endian) to ARGB by rotating left
	color = (color & 0xFF00FF00) | _rotl(color & 0x00FF00FF, 16);

	CRect rect(x1, y1, x2, y2);

	rage::grcBegin(4, 4);

	rage::grcVertex(rect.fX1, rect.fY1, z, 0.0f, 0.0f, -1.0f, color, u1, v1);
	rage::grcVertex(rect.fX2, rect.fY1, z, 0.0f, 0.0f, -1.0f, color, u2, v1);
	rage::grcVertex(rect.fX1, rect.fY2, z, 0.0f, 0.0f, -1.0f, color, u1, v2);
	rage::grcVertex(rect.fX2, rect.fY2, z, 0.0f, 0.0f, -1.0f, color, u2, v2);

	rage::grcEnd();

	PopDrawBlitImShader();

	SetDepthStencilState(oldDepthStencilState);
	SetRasterizerState(oldRasterizerState);
}

static uint32_t* g_resolution;
static uint32_t g_realResolution[2];

void GetGameResolution(int& resX, int& resY)
{
	if (!g_resolution)
	{
		resX = 0;
		resY = 0;
		return;
	}

	resX = g_resolution[0];
	resY = g_resolution[1];

	if (IsOnRenderThread() && (resX != resY))
	{
		g_realResolution[0] = resX;
		g_realResolution[1] = resY;
	}
	else
	{
		resX = g_realResolution[0];
		resY = g_realResolution[1];
	}
}

static hook::cdecl_stub<uint32_t(const D3D11_RASTERIZER_DESC*)> createRasterizerState([]()
{
	return hook::get_pattern("48 8D 4D D7 F3 0F 11 7D FF F3 44 0F", -0x86);
});

static hook::cdecl_stub<uint32_t(const D3D11_DEPTH_STENCIL_DESC*)> createDepthStencilState([]()
{
	return hook::get_pattern("8A C2 48 8B 45 C8 88 55 B0 48 C1 E8 20", -0x69);
});

static hook::cdecl_stub<uint32_t(const D3D11_BLEND_DESC*)> createBlendState([]()
{
	return hook::get_pattern("74 3D 41 0F B7 44 18 0C", -0x47);
});

hook::cdecl_stub<void(uint32_t)> setRasterizerState([] ()
{
	return hook::pattern("74 0D 80 0D ? ? ? ? 02 89 0D ? ? ? ? C3").count(1).get(0).get<void>(-6);
});

static uint32_t* g_nextRasterizerState;
static uint32_t* g_nextBlendState;
static float** g_nextBlendFactor;
static uint32_t* g_nextSampleMask;
static uint32_t* g_nextDepthStencilState;

uint32_t CreateRasterizerState(const D3D11_RASTERIZER_DESC* desc)
{
	return createRasterizerState(desc);
}

uint32_t CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* desc)
{
	return createDepthStencilState(desc);
}

uint32_t CreateBlendState(const D3D11_BLEND_DESC* desc)
{
	return createBlendState(desc);
}

uint32_t GetRasterizerState()
{
	return *g_nextRasterizerState;
}

void SetRasterizerState(uint32_t state)
{
	setRasterizerState(state);
}

hook::cdecl_stub<void(uint32_t, float*, uint32_t)> setBlendState([] ()
{
	return hook::pattern("74 1A 80 0D ? ? ? ? 04 89 0D").count(1).get(0).get<void>(-0x17);
});

uint32_t GetBlendState()
{
	return *g_nextBlendState;
}

void SetBlendState(uint32_t state)
{
	setBlendState(state, *g_nextBlendFactor, *g_nextSampleMask);
}

hook::cdecl_stub<void(uint32_t)> setDepthStencilState([] ()
{
	return hook::pattern("3B C2 74 13 80 0D ? ? ? ? 01 89").count(1).get(0).get<void>(-0x2D);
});

uint32_t GetDepthStencilState()
{
	return *g_nextDepthStencilState;
}

void SetDepthStencilState(uint32_t state)
{
	setDepthStencilState(state);
}

static uint32_t* stockStates[StateTypeMax];

uint32_t GetStockStateIdentifier(StateType state)
{
	if (state >= 0 && state < StateTypeMax)
	{
		return *stockStates[state];
	}

	return 0;
}

static hook::cdecl_stub<uint32_t(void* shaderGroup, void*, int sampler)> getSamplerState([] ()
{
	return hook::get_call(hook::pattern("48 8D 91 88 02 00 00 44 0F 29 54 24 60 44 0F").count(1).get(0).get<void>(25));
});

static hook::cdecl_stub<void(void* shaderGroup, void*, int sampler, uint32_t)> setSamplerState([] ()
{
	return hook::get_call(hook::pattern("48 8D 91 88 02 00 00 44 0F 29 54 24 60 44 0F").count(1).get(0).get<void>(25 + 36));
});

static hook::cdecl_stub<uint32_t(const D3D11_SAMPLER_DESC*)> createSamplerState([] ()
{
	return hook::get_call(hook::pattern("54 03 00 00 00 C7 44 24 58 03 00 00 00").count(1).get(0).get<void>(13));
});

static int* g_imDiffuseSampler;

uint32_t CreateSamplerState(const D3D11_SAMPLER_DESC* desc)
{
	return createSamplerState(desc);
}

uint32_t GetImDiffuseSamplerState()
{
	char* struc = *(char**)((*gtaImShader) + 8);

	return getSamplerState(struc, (void*)(*gtaImShader), *g_imDiffuseSampler);
}

void SetImDiffuseSamplerState(uint32_t samplerStateIdentifier)
{
	char* struc = *(char**)((*gtaImShader) + 8);

	setSamplerState(struc, (void*)(*gtaImShader), *g_imDiffuseSampler, samplerStateIdentifier);
}

static ID3D11Device** g_d3d11Device;
static int g_d3d11DeviceContextOffset;

ID3D11Device* GetD3D11Device()
{
	if (!g_d3d11Device)
	{
		return nullptr;
	}

	return *g_d3d11Device;
}

ID3D11DeviceContext* GetD3D11DeviceContext()
{
	return *(ID3D11DeviceContext**)(hook::get_tls() + g_d3d11DeviceContextOffset);
}

namespace rage
{
	static hook::cdecl_stub<bool(grmShaderFx*, const char*, void*, bool)> _grmShaderFx_LoadTechnique([]()
	{
		return hook::get_pattern("48 8B D9 BA 2E 00 00 00 48 8B CF 45 8A F1", -0x1C);
	});

	static hook::cdecl_stub<int(grmShaderFx*, int, bool, int)> _grmShaderFx_PushTechnique([]()
	{
		return hook::get_pattern("4C 63 DA 83 F8 FF 75 07 0F B6 05", -6);
	});

	grmShaderFactory* grmShaderFactory::GetInstance()
	{
		static auto ptr = hook::get_address<grmShaderFactory**>(hook::get_pattern("84 C0 74 29 48 8B 0D ? ? ? ? 48 8B 01", 7));
		return *ptr;
	}

	static hook::cdecl_stub<void(grmShaderDef*, int, grmShaderFx*)> _grmShaderDef_PushPass([]()
	{
		return hook::get_pattern("8B C2 49 8B F0 4C 8D 0C 40", -0x15);
	});

	static hook::cdecl_stub<void(grmShaderDef*)> _grmShaderDef_PopPass([]()
	{
		return hook::get_call(hook::get_pattern("0F 8C 0C FE FF FF 48 8B CE E8", 9));
	});

	static hook::cdecl_stub<int(grmShaderDef*, const char*)> _grmShaderDef_GetParameter([]()
	{
		return hook::get_call(hook::get_pattern("89 43 40 48 8B 43 08 48 8B 48 08 E8", 11));
	});

	static hook::cdecl_stub<int(grmShaderDef*, const char*)> _grmShaderDef_GetTechnique([]()
	{
		return hook::get_call(hook::get_pattern("41 B0 01 48 8B 49 08 8B E8 E8 ? ? ? ? 48", 9));
	});

	static hook::cdecl_stub<void(grmShaderDef*, grmShaderFx*, int, void*)> _grmShaderDef_SetSampler([]()
	{
		return hook::get_pattern("74 47 41 8D 40 FF 48 63 C8 48 8B 02 48 03 C9", -6);
	});

	static hook::cdecl_stub<void(grmShaderDef*, grmShaderFx*, int, const void*, int, int)> _grmShaderDef_SetParameter([]()
	{
		return hook::get_pattern("74 4E 44 8B 54 24 30 45 85 D2 7E", -3);
	});

	void grmShaderDef::PushPass(int pass, grmShaderFx* shader)
	{
		return _grmShaderDef_PushPass(this, pass, shader);
	}

	void grmShaderDef::PopPass()
	{
		return _grmShaderDef_PopPass(this);
	}

	int grmShaderDef::GetParameter(const char* name)
	{
		return _grmShaderDef_GetParameter(this, name);
	}

	int grmShaderDef::GetTechnique(const char* name)
	{
		return _grmShaderDef_GetTechnique(this, name);
	}

	void grmShaderDef::SetSampler(grmShaderFx* shader, int index, void* sampler)
	{
		return _grmShaderDef_SetSampler(this, shader, index, sampler);
	}

	void grmShaderDef::SetParameter(grmShaderFx* shader, int index, const void* data, int size, int count)
	{
		return _grmShaderDef_SetParameter(this, shader, index, data, size, count);
	}

	bool grmShaderFx::LoadTechnique(const char* name, void* templ, bool a4)
	{
		return _grmShaderFx_LoadTechnique(this, name, templ, a4);
	}

	int grmShaderFx::PushTechnique(int technique, bool unk1, int a4)
	{
		return _grmShaderFx_PushTechnique(this, a4, unk1, technique);
	}

	void grmShaderFx::PushPass(int index)
	{
		m_shader->PushPass(index, this);
	}

	void grmShaderFx::PopPass()
	{
		m_shader->PopPass();
	}

	void grmShaderFx::PopTechnique()
	{
		static void** ptr = hook::get_address<void**>(hook::get_pattern("FF C9 48 C1 E1 05 49 03 09 48 89 0D", 12));
		*ptr = nullptr;
	}
}

static hook::cdecl_stub<void(const float*)> _setWorldMatrix([]()
{
	return hook::get_pattern("0F 28 69 30 0F 28 49 20 0F 28 61 10 0F", -0xB);
});

void SetWorldMatrix(const float* matrix)
{
	_setWorldMatrix(matrix);
}

static HookFunction hookFunction([] ()
{
	char* location = hook::pattern("48 8B 05 ? ? ? ? ? 8B ? 33 D2 48 89 0D").count(1).get(0).get<char>(15);

	g_d3d11Device = (ID3D11Device**)(location + *(int32_t*)location + 4);

	g_d3d11DeviceContextOffset = *(int*)(location - (xbr::IsGameBuildOrGreater<2802>() ? 60 : 59));

	// things
	location = hook::pattern("74 0D 80 0D ? ? ? ? 02 89 0D ? ? ? ? C3").count(1).get(0).get<char>(-4);

	g_nextRasterizerState = (uint32_t*)(*(int32_t*)location + location + 4);

	location = hook::pattern("74 1A 80 0D ? ? ? ? 04 89 0D").count(1).get(0).get<char>(11);

	g_nextBlendState = (uint32_t*)(*(int32_t*)location + location + 4);
	
	location += 6;
	g_nextBlendFactor = (float**)(*(int32_t*)location + location + 4);

	location += 7;
	g_nextSampleMask = (uint32_t*)(*(int32_t*)location + location + 4);

	location = hook::pattern("3B C2 74 13 80 0D ? ? ? ? 01 89").count(1).get(0).get<char>(13);

	g_nextDepthStencilState = (uint32_t*)(*(int32_t*)location + location + 4);

	// sampler state stuff
	location = hook::pattern("48 8B D9 4C 8B C9 48 8B 0D").count(1).get(0).get<char>(-4);

	g_imDiffuseSampler = (int*)(*(int32_t*)location + location + 4);

	// resolution
	location = hook::pattern("C7 05 ? ? ? ? 00 05 00 00").count(1).get(0).get<char>(2);

	g_resolution = (uint32_t*)(*(int32_t*)location + location + 8);

	// states
	location = hook::pattern("44 89 74 24 5C 89 05 ? ? ? ? E8").count(1).get(0).get<char>(7);

	stockStates[RasterizerStateDefault] = (uint32_t*)(*(int32_t*)location + location + 4);

	location += 17;

	stockStates[RasterizerStateNoCulling] = (uint32_t*)(*(int32_t*)location + location + 4);

	location += 20;

	stockStates[BlendStateNoBlend] = (uint32_t*)(*(int32_t*)location + location + 4);

	location = hook::pattern("44 89 7D EC 44 89 7D E0  89 05").count(1).get(0).get<char>(10);

	stockStates[BlendStateDefault] = (uint32_t*)(*(int32_t*)location + location + 4);

	// first one is subtractive; motivation for relying on count >1 is that this is a RAGE function, unlikely to get modified by patches
	location = hook::pattern("48 8D 4D D0 33 D2 44 89 7D EC 89 05").count(2).get(1).get<char>(12);

	stockStates[BlendStatePremultiplied] = (uint32_t*)(*(int32_t*)location + location + 4);

	location = hook::pattern("33 D2 44 89 74 24 20 44 89 74 24 24 89 05").count(1).get(0).get<char>(14);

	stockStates[DepthStencilStateNoDepth] = (uint32_t*)(*(int32_t*)location + location + 4);

	// draw command buffer
	location = hook::pattern("EB 07 48 89 2D ? ? ? ? BA 1E 00 00 00 48").count(1).get(0).get<char>(5);

	g_drawCommandBuffer = (rage::dlDrawCommandBuffer**)(*(int32_t*)location + location + 4);

	location = hook::pattern("42 09 0C 02 BA 01 00 00 00 83 F9 04 0F 44 C2").count(1).get(0).get<char>(-15);

	g_renderThreadTlsIndex = *(int32_t*)location;

	OnGrcCreateDevice.Connect([] ()
	{
		g_realResolution[0] = g_resolution[0];
		g_realResolution[1] = g_resolution[1];
	}, -500);

	// set immediate mode vertex limit to 8x what it was (so, 32 MB)
	{
		auto location = hook::get_pattern<char>("44 89 74 24 74 89 05 ? ? ? ? 89 44", 5);

		auto refloc = hook::get_address<char*>(location + 2);
		hook::put<uint32_t>(refloc, 0x4000000);

		auto otherLoc = hook::get_address<uint32_t*>(hook::get_pattern("48 8D 54 24 60 45 33 C0 FF 50 18 8B 05", 13));
		hook::put<uint32_t>(otherLoc, 0x7FFF8 * 4);

		hook::nop(location, 6); // setter
		hook::put<uint8_t>(location, 0xB8); // write to eax for later
		hook::put<uint32_t>(location + 1, 0x4000000);
	}
});
