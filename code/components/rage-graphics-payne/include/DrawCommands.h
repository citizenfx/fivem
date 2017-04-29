#pragma once

#include "grcTexture.h"

#ifdef COMPILING_RAGE_GRAPHICS_PAYNE
#define GAMESPEC_EXPORT_VMT __declspec(dllexport) __declspec(novtable)
#define GAMESPEC_EXPORT __declspec(dllexport)
#else
// no dllimport for importing a vtable as it will be translated to a local vtable
#define GAMESPEC_EXPORT_VMT __declspec(novtable)
#define GAMESPEC_EXPORT __declspec(dllimport)
#endif

void GAMESPEC_EXPORT EnqueueGenericDrawCommand(void(*cb)(uint32_t, uint32_t), uint32_t* arg1, uint32_t* arg2);

bool GAMESPEC_EXPORT IsOnRenderThread();

void GAMESPEC_EXPORT SetTexture(rage::grcTexture* texture);

void GAMESPEC_EXPORT PushUnlitImShader();
void GAMESPEC_EXPORT BeginImVertices(int type, int count);
void GAMESPEC_EXPORT AddImVertex(float x, float y, float z, float nX, float nY, float nZ, uint32_t color, float u, float v);
void GAMESPEC_EXPORT DrawImVertices();
void GAMESPEC_EXPORT PopUnlitImShader();

void GAMESPEC_EXPORT PushDrawBlitImShader();

void GAMESPEC_EXPORT PopDrawBlitImShader();

void GAMESPEC_EXPORT SetRenderState(int, int);

void GAMESPEC_EXPORT SetTextureGtaIm(rage::grcTexture* texture);
void GAMESPEC_EXPORT DrawImSprite(float x1, float y1, float x2, float y2, float z, float u1, float v1, float u2, float v2, uint32_t* color, int subShader);

extern
#ifdef COMPILING_RAGE_GRAPHICS_PAYNE
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	fwEvent<> OnGrcCreateDevice;

extern
#ifdef COMPILING_RAGE_GRAPHICS_PAYNE
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	fwEvent<> OnPostFrontendRender;

#ifdef COMPILING_RAGE_GRAPHICS_PAYNE
__declspec(dllexport)
#endif
	void GetGameResolution(int& width, int& height);

#define grcCullModeNone 0