#include "StdInc.h"
#include "DrawCommands.h"

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

#define VTABLE_CDrawSpriteDC 0xD55A64

auto CDrawSpriteDC__ctor = (void(__thiscall*)(CDrawSpriteDC*, const float* bottomLeft, const float* topLeft, const float* bottomRight, const float* topRight, uint32_t color, rage::grcTexture* texture))0x7C0F00;

CDrawSpriteDC::CDrawSpriteDC(const float* bottomLeft, const float* topLeft, const float* bottomRight, const float* topRight, uint32_t color, rage::grcTexture* texture)
{
	*(uintptr_t*)this = VTABLE_CDrawSpriteDC;

	CDrawSpriteDC__ctor(this, bottomLeft, topLeft, bottomRight, topRight, color, texture);
}

auto CGenericDC__ctor = (void(__thiscall*)(CGenericDC*, void(*)()))0x44CE70;

CGenericDC::CGenericDC(void(*cb)())
{
	CGenericDC__ctor(this, cb);
}

auto CGenericDC1Arg__ctor = (void(__thiscall*)(CGenericDC1Arg*, void(*)(int), int))0x7BF9B0;

CGenericDC1Arg::CGenericDC1Arg(void(*cb)(int arg), int arg)
{
	CGenericDC1Arg__ctor(this, cb, arg);
}

WRAPPER void* CBaseDC::operator new(size_t size, int a2) { EAXJMP(0x7BDD80); }

void CBaseDC::operator delete(void* ptr)
{
	assert(!"HEY");
}

bool IsOnRenderThread()
{
	DWORD* gtaTLS = *(DWORD**)(__readfsdword(44) + (4 * *(uint32_t*)0x17237B0));
	return !gtaTLS[306];
}

void WRAPPER CBaseDC::Enqueue() { EAXJMP(0x404E90); }

void WRAPPER SetTexture(rage::grcTexture* texture) { EAXJMP(0x627FB0); }

void WRAPPER BeginImVertices(int count, int count2) { EAXJMP(0x627E70); }
void WRAPPER AddImVertex(float x, float y, float z, float nX, float nY, float nZ, uint32_t color, float u, float v) { EAXJMP(0x627EE0); }
void WRAPPER DrawImVertices() { EAXJMP(0x63A4A0); }

void WRAPPER PushUnlitImShader() { EAXJMP(0x852540); }
void WRAPPER PopUnlitImShader() { EAXJMP(0x852570); }

void PushDrawBlitImShader()
{
	// set screen space stuff
	((void(*)(int))0x8528F0)(0);

	// shader methods: set subshader?
	((void(__thiscall*)(intptr_t, int, int, intptr_t))0x633D50)(*(intptr_t*)0x11DE810, 2, 0, *(intptr_t*)0x11DE814);

	((void(__thiscall*)(intptr_t, int))0x633CE0)(*(intptr_t*)0x11DE810, 0);
}

void PopDrawBlitImShader()
{
	((void(*)())0x633D20)();

	((void(__thiscall*)(intptr_t))0x633DC0)(*(intptr_t*)0x11DE810);
}

static void WRAPPER __fastcall SetTextureGtaIm_impl(rage::grcTexture** texture) { EAXJMP(0x852480); }

void SetTextureGtaIm(rage::grcTexture* texture)
{
	SetTextureGtaIm_impl(&texture);
}

void WRAPPER DrawImSprite(float x1, float y1, float x2, float y2, float z, float u1, float v1, float u2, float v2, uint32_t* color, int subShader) { EAXJMP(0x852CE0); }

void GetGameResolution(int& resX, int& resY)
{
	resX = *(int*)0xFDCEAC;
	resY = *(int*)0xFDCEB0;
}

void WRAPPER SetRenderState(int rs, int val) { EAXJMP(0x62D2D0); }