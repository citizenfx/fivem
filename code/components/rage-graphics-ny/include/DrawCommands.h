/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <d3d9.h>
#include "grcTexture.h"

#ifdef COMPILING_RAGE_GRAPHICS_NY
#define DC_EXPORT_VMT __declspec(dllexport) __declspec(novtable)
#define DC_EXPORT __declspec(dllexport)
#else
// no dllimport for importing a vtable as it will be translated to a local vtable
#define DC_EXPORT_VMT __declspec(novtable)
#define DC_EXPORT __declspec(dllimport)
#endif

class DC_EXPORT_VMT CBaseDC
{
public:
	~CBaseDC();

	virtual void Render() = 0;

	void* operator new(size_t size, int a2);

	void operator delete(void* memory, int a2);

	void Enqueue();
};

class DC_EXPORT_VMT CImplementedDC : public CBaseDC
{
protected:
	CImplementedDC();

public:
	virtual ~CImplementedDC();

	virtual void Render();
};

#if unused
class DC_EXPORT_VMT CDrawSpriteDC : public CImplementedDC
{
private:
	char pad[44];

public:
	CDrawSpriteDC(const float* bottomLeft, const float* topLeft, const float* bottomRight, const float* topRight, uint32_t color, rage::grcTexture* texture);
};
#endif

class DC_EXPORT_VMT CGenericDC : public CImplementedDC
{
private:
	char pad[8];

public:
	CGenericDC(void(*cb)());
};

class DC_EXPORT_VMT CGenericDC1Arg : public CImplementedDC
{
private:
	char pad[12];

public:
	CGenericDC1Arg(void(*cb)(int arg), int* arg);
};

class DC_EXPORT_VMT CGenericDC2Args : public CImplementedDC
{
private:
	char pad[16];

public:
	CGenericDC2Args(void (*cb)(int arg, int arg2), int* arg, int* arg2);
};

bool DC_EXPORT IsOnRenderThread();

void DC_EXPORT SetTexture(rage::grcTexture* texture);
void DC_EXPORT PushUnlitImShader();
void DC_EXPORT PopUnlitImShader();

namespace rage
{
	void DC_EXPORT grcBegin(int type, int count);
	void DC_EXPORT grcVertex(float x, float y, float z, float nX, float nY, float nZ, uint32_t color, float u, float v);
	void DC_EXPORT grcEnd();
}
void DC_EXPORT PushDrawBlitImShader();

void DC_EXPORT PopDrawBlitImShader();

void DC_EXPORT SetRenderState(int, int);

void DC_EXPORT SetTextureGtaIm(rage::grcTexture* texture);
void DC_EXPORT DrawImSprite(float x1, float y1, float x2, float y2, float z, float u1, float v1, float u2, float v2, uint32_t* color, int subShader);
void DC_EXPORT EnqueueGenericDrawCommand(void(*cb)(uintptr_t, uintptr_t), uintptr_t* arg1, uintptr_t* arg2);

void DC_EXPORT SetScissorRect(int, int, int, int);

DC_EXPORT IDirect3DDevice9* GetD3D9Device();

extern
	#ifdef COMPILING_RAGE_GRAPHICS_NY
	__declspec(dllexport)
	#else
	__declspec(dllimport)
	#endif
	fwEvent<> OnGrcBeginScene;

extern
	#ifdef COMPILING_RAGE_GRAPHICS_NY
	__declspec(dllexport)
	#else
	__declspec(dllimport)
	#endif
	fwEvent<> OnGrcEndScene;

extern
	#ifdef COMPILING_RAGE_GRAPHICS_NY
	__declspec(dllexport)
	#else
	__declspec(dllimport)
	#endif
	fwEvent<> OnGrcCreateDevice;

#ifdef COMPILING_RAGE_GRAPHICS_NY
__declspec(dllexport)
#endif
	void GetGameResolution(int& width, int& height);

extern DC_EXPORT fwEvent<> OnPostFrontendRender;
extern DC_EXPORT fwEvent<bool&> DoWeIgnoreTheFrontend;


#define _HAS_DWITF 1

#define grcCullModeNone 0
