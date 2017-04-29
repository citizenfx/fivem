#include "StdInc.h"
#include "DrawCommands.h"
#include "Hooking.h"
#include "Rect.h"

#define PURECALL() __asm { jmp _purecall }

static hook::cdecl_stub<void(void(*)(uint32_t, uint32_t), uint32_t*, uint32_t*)> enqueueGenericDC2Args([] ()
{
	return hook::get_call(hook::pattern("89 08 C7 44 24 18 FF FF FF FF E8").count(1).get(0).get<void>(10));
});

void EnqueueGenericDrawCommand(void(*cb)(uint32_t, uint32_t), uint32_t* arg1, uint32_t* arg2)
{
	enqueueGenericDC2Args(cb, arg1, arg2);
}

bool IsOnRenderThread()
{
	/*DWORD* gtaTLS = *(DWORD**)(__readfsdword(44) + (4 * *(uint32_t*)0x17237B0));
	return !gtaTLS[306];*/

	return false;
}

//void WRAPPER SetTexture(rage::grcTexture* texture) { EAXJMP(0x627FB0); }

static hook::cdecl_stub<void()> drawImVertices([] ()
{
	// reuse a pattern from below for performance
	return hook::get_call(hook::pattern("5F 5E 75 05 E8 ? ? ? ? 8B 0D").get(0).get<void>(4));
});

static hook::cdecl_stub<void(int, int)> beginImVertices([] ()
{
	// fairly annoying pattern :(
	return hook::pattern("89 86 40 0C 00 00 C7 86 38 0C 00 00").get(0).get<void>(-0x9A);
});

static hook::cdecl_stub<void(float, float, float, float, float, float, uint32_t, float, float)> addImVertex([] ()
{
	return hook::pattern("F3 0F 10 44 24 04 8D 51 24 89 90 40 0C 00 00").get(0).get<void>(-0x3D);
});

void BeginImVertices(int type, int count)
{
	beginImVertices(type, count);
}

void AddImVertex(float x, float y, float z, float nX, float nY, float nZ, uint32_t color, float u, float v)
{
	// colors are the other way around in Payne again, so RGBA-swap we go
	color = (color & 0xFF00FF00) | _rotl(color & 0x00FF00FF, 16);

	addImVertex(x, y, z, nX, nY, nZ, color, u, v);
}

void DrawImVertices()
{
	drawImVertices();
}

/*void WRAPPER PushUnlitImShader() { EAXJMP(0x852540); }
void WRAPPER PopUnlitImShader() { EAXJMP(0x852570); }*/

static hook::cdecl_stub<void()> setScreenSpaceMatrix([] ()
{
	return hook::pattern("57 6A 01 33 FF 33 C0 E8").count(1).get(0).get<void>();
});

static hook::thiscall_stub<void(intptr_t, int, int, intptr_t)> setSubShader([] ()
{
	// there's 3 in the current game, but this part matches for all
	return hook::get_call(hook::pattern("33 FF 33 C0 E8 ? ? ? ? A1 ? ? ? ? 8B 0D").get(0).get<void>(0x1A));
});

static hook::thiscall_stub<void(intptr_t, int)> setSubShaderUnk([] ()
{
	// there's 3 in the current game, but this part matches for all
	return hook::get_call(hook::pattern("33 FF 33 C0 E8 ? ? ? ? A1 ? ? ? ? 8B 0D").get(0).get<void>(0x1A + 12));
});

static intptr_t* gtaImShader;
static intptr_t* gtaImTechnique;

static HookFunction shaderIdHookFunc([] ()
{
	gtaImShader = *hook::pattern("33 FF 33 C0 E8 ? ? ? ? A1 ? ? ? ? 8B 0D").get(0).get<intptr_t*>(16);
	gtaImTechnique = *hook::pattern("33 FF 33 C0 E8 ? ? ? ? A1 ? ? ? ? 8B 0D").get(0).get<intptr_t*>(10);
});

void PushDrawBlitImShader()
{
	// set screen space stuff
	setScreenSpaceMatrix();

	// shader methods: set subshader?
	setSubShader(*gtaImShader, 0, 0, *gtaImTechnique);
	setSubShaderUnk(*gtaImShader, 0);
}

static hook::thiscall_stub<void(intptr_t)> popSubShader([] ()
{
	return hook::get_call(hook::pattern("5F 5E 75 05 E8 ? ? ? ? 8B 0D").get(0).get<void>(15));
});

static hook::thiscall_stub<void(intptr_t)> popSubShaderUnk([] ()
{
	return hook::get_call(hook::pattern("5F 5E 75 05 E8 ? ? ? ? 8B 0D").get(0).get<void>(15 + 0xB));
});

void PopDrawBlitImShader()
{
	popSubShaderUnk(*gtaImShader);

	popSubShader(*gtaImShader);
}

static hook::cdecl_stub<void(rage::grcTexture*)> setTextureGtaIm([] ()
{
	return hook::get_call(hook::pattern("6A 17 E8 ? ? ? ? 83 C4 10 8B 56 38 52 E8").count(1).get(0).get<void>(14));
});

void SetTextureGtaIm(rage::grcTexture* texture)
{
	setTextureGtaIm(texture);
}

//void WRAPPER DrawImSprite(float x1, float y1, float x2, float y2, float z, float u1, float v1, float u2, float v2, uint32_t* color, int subShader) { EAXJMP(0x852CE0); }
void DrawImSprite(float x1, float y1, float x2, float y2, float z, float u1, float v1, float u2, float v2, uint32_t* colorPtr, int subShader)
{
	SetRenderState(0, 0);
	SetRenderState(2, 0);

	PushDrawBlitImShader();

	uint32_t color = *colorPtr;

	// this swaps ABGR (as CRGBA is ABGR in little-endian) to ARGB by rotating left
	color = (color & 0xFF00FF00) | _rotl(color & 0x00FF00FF, 16);

	CRect rect(x1, y1, x2, y2);

	BeginImVertices(4, 4);

	AddImVertex(rect.fX1, rect.fY1, z, 0.0f, 0.0f, -1.0f, color, u1, v1);
	AddImVertex(rect.fX2, rect.fY1, z, 0.0f, 0.0f, -1.0f, color, u2, v1);
	AddImVertex(rect.fX1, rect.fY2, z, 0.0f, 0.0f, -1.0f, color, u1, v2);
	AddImVertex(rect.fX2, rect.fY2, z, 0.0f, 0.0f, -1.0f, color, u2, v2);

	DrawImVertices();

	PopDrawBlitImShader();
}

void GetGameResolution(int& resX, int& resY)
{
	//resX = *(int*)0xFDCEAC;
	//resY = *(int*)0xFDCEB0;

	resX = 2560;
	resY = 1440;
}

static hook::cdecl_stub<void(int, int)> setRenderState([] ()
{
	// NOTE WHEN IMPLEMENTING THIS:
	// THERE IS A DIFFERENCE BETWEEN INTERNAL SETTER (E.G. CULLSTATE = 6) AND EXTERNAL SETTER (E.G. CULLSTATE = 0 -> 6)
	
	// this is internal setter in payne
	//return hook::get_call(hook::pattern("6A 01 6A 17 E8 ? ? ? ? 6A 01 6A 18 E8").count(2).get(0).get<void>(4));

	// external setter
	return hook::get_call(hook::pattern("6A 01 00 00 6A 08 6A 10 E8").count(1).get(0).get<void>(8));
});

//void WRAPPER SetRenderState(int rs, int val) { EAXJMP(0x62D2D0); }
void SetRenderState(int rs, int val)
{
	return setRenderState(rs, val);
}