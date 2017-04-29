#pragma once

#include <d3d11.h>

#include "grcTexture.h"

#define _HAVE_GRCORE_NEWSTATES 1

#ifdef COMPILING_RAGE_GRAPHICS_FIVE
#define GAMESPEC_EXPORT_VMT __declspec(dllexport) __declspec(novtable)
#define GAMESPEC_EXPORT __declspec(dllexport)
#else
// no dllimport for importing a vtable as it will be translated to a local vtable
#define GAMESPEC_EXPORT_VMT __declspec(novtable)
#define GAMESPEC_EXPORT __declspec(dllimport)
#endif

GAMESPEC_EXPORT ID3D11Device* GetD3D11Device();
GAMESPEC_EXPORT ID3D11DeviceContext* GetD3D11DeviceContext();

void GAMESPEC_EXPORT EnqueueGenericDrawCommand(void(*cb)(uintptr_t, uintptr_t), uintptr_t* arg1, uintptr_t* arg2);

bool GAMESPEC_EXPORT IsOnRenderThread();

void GAMESPEC_EXPORT SetTexture(rage::grcTexture* texture);

void GAMESPEC_EXPORT PushUnlitImShader();
void GAMESPEC_EXPORT BeginImVertices(int type, int count);
void GAMESPEC_EXPORT AddImVertex(float x, float y, float z, float nX, float nY, float nZ, uint32_t color, float u, float v);
void GAMESPEC_EXPORT DrawImVertices();
void GAMESPEC_EXPORT PopUnlitImShader();

void GAMESPEC_EXPORT PushDrawBlitImShader();

void GAMESPEC_EXPORT PopDrawBlitImShader();

void GAMESPEC_EXPORT SetTextureGtaIm(rage::grcTexture* texture);
void GAMESPEC_EXPORT DrawImSprite(float x1, float y1, float x2, float y2, float z, float u1, float v1, float u2, float v2, uint32_t* color, int subShader);

uint32_t GAMESPEC_EXPORT CreateSamplerState(const D3D11_SAMPLER_DESC* desc);
uint32_t GAMESPEC_EXPORT GetImDiffuseSamplerState();
void GAMESPEC_EXPORT SetImDiffuseSamplerState(uint32_t samplerStateIdentifier);


// new state system
uint32_t GAMESPEC_EXPORT GetRasterizerState();
uint32_t GAMESPEC_EXPORT GetBlendState();
uint32_t GAMESPEC_EXPORT GetDepthStencilState();

void GAMESPEC_EXPORT SetRasterizerState(uint32_t state);
void GAMESPEC_EXPORT SetBlendState(uint32_t state);
void GAMESPEC_EXPORT SetDepthStencilState(uint32_t state);

enum StateType
{
	RasterizerStateDefault,
	RasterizerStateNoCulling,
	BlendStateNoBlend,
	BlendStateDefault,
	BlendStatePremultiplied,
	DepthStencilStateNoDepth,
	StateTypeMax
};

uint32_t GAMESPEC_EXPORT GetStockStateIdentifier(StateType state);

extern
#ifdef COMPILING_RAGE_GRAPHICS_FIVE
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	fwEvent<> OnGrcCreateDevice;

extern
#ifdef COMPILING_RAGE_GRAPHICS_FIVE
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	fwEvent<> OnPostFrontendRender;

#ifdef COMPILING_RAGE_GRAPHICS_FIVE
__declspec(dllexport)
#endif
	void GetGameResolution(int& width, int& height);

#define grcCullModeNone 0