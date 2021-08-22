/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "DrawCommands.h"
#include <Hooking.h>

#define PURECALL() __asm { jmp _purecall }

CImplementedDC::CImplementedDC()
{
	
}

CImplementedDC::~CImplementedDC()
{
	//PURECALL();
}

// as the compiler is trying to be smart in some instances
void CImplementedDC::Render()
{
	uintptr_t* vtable = *(uintptr_t**)this;

	((void(__thiscall*)(void*))vtable[1])(this);
}

CBaseDC::~CBaseDC()
{
	//PURECALL();
}

#if unused
#define VTABLE_CDrawSpriteDC 0xD55A64

auto CDrawSpriteDC__ctor = (void(__thiscall*)(CDrawSpriteDC*, const float* bottomLeft, const float* topLeft, const float* bottomRight, const float* topRight, uint32_t color, rage::grcTexture* texture))0x7C0F00;

CDrawSpriteDC::CDrawSpriteDC(const float* bottomLeft, const float* topLeft, const float* bottomRight, const float* topRight, uint32_t color, rage::grcTexture* texture)
{
	*(uintptr_t*)this = VTABLE_CDrawSpriteDC;

	CDrawSpriteDC__ctor(this, bottomLeft, topLeft, bottomRight, topRight, color, texture);
}
#endif

static hook::thiscall_stub<void(CGenericDC*, void(*)())> _CGenericDC__ctor([]()
{
	return hook::get_call(hook::get_pattern("6A 00 84 FF 74 6E", 27));
});

CGenericDC::CGenericDC(void(*cb)())
{
	_CGenericDC__ctor(this, cb);
}

static hook::thiscall_stub<void(CGenericDC1Arg*, void(*)(int), int*)> _CGenericDC1Arg__ctor([]()
{
	return (void*)hook::get_call(hook::get_pattern<char>("8B C8 C7 44 24 10 FF FF FF FF E8", 10));
});

CGenericDC1Arg::CGenericDC1Arg(void(*cb)(int arg), int* arg)
{
	static auto loc = *(uintptr_t*)(hook::get_call(hook::get_pattern<char>("8B C8 C7 44 24 10 FF FF FF FF E8", 10)) + 0x2A);

	*(uintptr_t*)this = loc;

	_CGenericDC1Arg__ctor(this, cb, arg);
}

static hook::thiscall_stub<void(CGenericDC2Args*, void (*)(int, int), int*, int*)> _CGenericDC2Args__ctor([]()
{
	return (void*)hook::get_call(hook::get_pattern<char>("C7 44 24 2C 00 00 00 00 E8 ? ? ? ? EB 02", 8));
});

CGenericDC2Args::CGenericDC2Args(void (*cb)(int arg, int arg2), int* arg, int* arg2)
{
	static auto loc = *(uintptr_t*)(hook::get_call(hook::get_pattern<char>("C7 44 24 2C 00 00 00 00 E8 ? ? ? ? EB 02", 8)) + 0x2A);

	*(uintptr_t*)this = loc;

	_CGenericDC2Args__ctor(this, cb, arg, arg2);
}

static hook::cdecl_stub<void*(size_t, int)> _allocDC([]()
{
	return hook::get_pattern("53 56 57 8B 7C 24 10 FF 74 24 14");
});

void* CBaseDC::operator new(size_t size, int a2) 
{ 
	return _allocDC(size, a2);
}

void CBaseDC::operator delete(void* ptr, int a2)
{
	PURECALL();
}

bool IsOnRenderThread()
{
	static auto tlsOffset = *hook::get_pattern<int>("8B 04 88 83 B8 ? ? ? ? 00 0F 84 ? ? ? ? 6A 00", 5);

	DWORD* gtaTLS = *(DWORD**)(__readfsdword(44));
	return !gtaTLS[tlsOffset / 4];
}

static hook::cdecl_stub<void(void*)> _queueDC([]()
{
	return hook::get_pattern("56 57 8B 7C 24 0C 8B CF 8B 07 FF 50 08");
});

void CBaseDC::Enqueue() 
{
	_queueDC(this);
}

static hook::cdecl_stub<void(rage::grcTexture*)> _setTexture([]()
{
	return hook::get_pattern("8B 44 24 04 85 C0 0F 44");
});

void SetTexture(rage::grcTexture* texture) 
{
	_setTexture(texture);
}

static hook::cdecl_stub<void(int, int)> _beginImVertices([]()
{
	return (void*)hook::get_call(hook::get_pattern<char>("F3 0F 10 44 24 28 8B 74 24 34", -0x41) + 0x3C);
});

static hook::cdecl_stub<void(float, float, float, float, float, float, uint32_t, float, float)> _addImVertex([]()
{
	return (void*)hook::get_call(hook::get_pattern<char>("F3 0F 10 44 24 28 8B 74 24 34", -0x41) + 0x9C);
});

static hook::cdecl_stub<void()> _drawImVertices([]()
{
	return (void*)hook::get_call(hook::get_pattern<char>("F3 0F 10 44 24 28 8B 74 24 34", -0x41) + 0x1C3);
});

namespace rage
{
	void grcBegin(int type, int count) 
	{ 
		_beginImVertices(type, count);
	}

	void grcVertexReal(float x, float y, float z, float nX, float nY, float nZ, uint32_t color, float u, float v) 
	{
		_addImVertex(x, y, z, nX, nY, nZ, color, u, v);
	}

	void grcVertex(float x, float y, float z, float nX, float nY, float nZ, uint32_t color, float u, float v)
	{
		int resX;
		int resY;
		GetGameResolution(resX, resY);

		grcVertexReal(x, y, z, nX, nY, nZ, color, u + (0.5f / resX), v + (0.5f / resY));
	}

	void grcEnd() 
	{ 
		_drawImVertices();
	}
}

static hook::cdecl_stub<void()> _pushUnlitImShader([]()
{
	return hook::get_call(hook::pattern("E8 ? ? ? ? E8 ? ? ? ? 8D 44 24 10 50 8D 44 24 34").count(2).get(0).get<void>(5));
});

static hook::cdecl_stub<void()> _popUnlitImShader([]()
{
	return hook::get_call(hook::pattern("E8 ? ? ? ? E8 ? ? ? ? 8D 44 24 10 50 8D 44 24 34").count(2).get(0).get<void>(51));
});

void PushUnlitImShader() 
{
	_pushUnlitImShader();
}

void PopUnlitImShader() 
{
	_popUnlitImShader();
}

static hook::cdecl_stub<void(BYTE)> _setScreenSpaceStuff([]()
{
	return hook::get_pattern("C3 55 8B EC 83 E4 F0 83 EC 48 80 3D", 1);
});

static hook::thiscall_stub<void(intptr_t, int, int, intptr_t)> _setSubShader([]()
{
	return (void*)hook::get_call(hook::get_pattern<char>("F3 0F 10 44 24 28 8B 74 24 34", -0x41) + 0x24);
});

static hook::thiscall_stub<void(intptr_t, int)> _setSomethingSubShader([]()
{
	return (void*)hook::get_call(hook::get_pattern<char>("F3 0F 10 44 24 28 8B 74 24 34", -0x41) + 0x33);
});

void PushDrawBlitImShader()
{
	static auto a1 = *(intptr_t**)(hook::get_pattern<char>("F3 0F 10 44 24 28 8B 74 24 34", -0x41) + 0x13);
	static auto a4 = *(intptr_t**)(hook::get_pattern<char>("F3 0F 10 44 24 28 8B 74 24 34", -0x41) + 0x1C);

	// set screen space stuff
	_setScreenSpaceStuff(0);

	// shader methods: set subshader?
	_setSubShader(*a1, 2, 0, *a4);

	_setSomethingSubShader(*a1, 0);
}

static hook::cdecl_stub<void()> _popShaderFn1([]()
{
	return (void*)hook::get_call(hook::get_pattern<char>("F3 0F 10 44 24 28 8B 74 24 34", -0x41) + 0x1CE);
});

static hook::thiscall_stub<void(intptr_t)> _popShaderFn2([]()
{
	return (void*)hook::get_call(hook::get_pattern<char>("F3 0F 10 44 24 28 8B 74 24 34", -0x41) + 0x1DA);
});

void PopDrawBlitImShader()
{
	_popShaderFn1();

	static auto something = *(intptr_t**)(hook::get_pattern<char>("F3 0F 10 44 24 28 8B 74 24 34", -0x41) + 0x1D5);

	_popShaderFn2(*something);
}

static hook::thiscall_stub<void(void*)> _setTextureGtaIm([]()
{
	return hook::get_pattern("56 8B F1 8B 0D ? ? ? ? FF 36 FF 35");
});

void SetTextureGtaIm(rage::grcTexture* texture)
{
	_setTextureGtaIm((void*)&texture);
}

static hook::cdecl_stub<void(float, float, float, float, float, float, float, float, float, uint32_t*, int)> _drawImSprite([]()
{
	return hook::get_pattern("F3 0F 10 44 24 28 8B 74 24 34", -0x41);
});

void DrawImSprite(float x1, float y1, float x2, float y2, float z, float u1, float v1, float u2, float v2, uint32_t* color, int subShader)
{
	_drawImSprite(x1, y1, x2, y2, z, u1, v1, u2, v2, color, subShader);
}

int* g_resolutionX;
int* g_resolutionY;

static HookFunction hookFunc([]() 
{
	static auto location = hook::get_pattern<char>("F3 0F 11 4C 24 60 E8 ? ? ? ? 8B 1D");
	g_resolutionX = *(int**)(location + 0x21);
	g_resolutionY = *(int**)(location + 0x0D);
});

void GetGameResolution(int& resX, int& resY)
{
	if (!g_resolutionX || !g_resolutionY)
	{
		resX = 0;
		resY = 0;
		return;
	}

	resX = *g_resolutionX;
	resY = *g_resolutionY;
}

static hook::cdecl_stub<void(int, int)> _setRenderState([]()
{
	return hook::get_call(hook::get_pattern("6A 02 6A 04 E8 ? ? ? ? 6A 01", 4));
});

void SetRenderState(int rs, int val)
{
	return _setRenderState(rs, val);
}

void EnqueueGenericDrawCommand(void(*cb)(uintptr_t, uintptr_t), uintptr_t* arg1, uintptr_t* arg2)
{
	if (!IsOnRenderThread())
	{
		auto dc = new(0) CGenericDC2Args((void(*)(int, int))cb, (int*)arg1, (int*)arg2);
		dc->Enqueue();
	}
	else
	{
		cb(*arg1, *arg2);
	}
}

void SetScissorRect(int a, int b, int c, int d)
{
	RECT r;
	r.left = a;
	r.top = b;
	r.right = c;
	r.bottom = d;

	GetD3D9Device()->SetScissorRect(&r);
}

IDirect3DDevice9* GetD3D9Device()
{
	IDirect3DDevice9** device = *(IDirect3DDevice9***)(hook::get_call(hook::get_pattern<char>("6A 02 6A 04 E8 ? ? ? ? 6A 01", 4)) + 0x1D);

	return *device;
}

DC_EXPORT fwEvent<> OnPostFrontendRender;
