#pragma once

#include <grcTexture.h>

#ifdef COMPILING_RAGE_GRAPHICS_RDR3
#define GFX_EXPORT DLL_EXPORT
#else
#define GFX_EXPORT DLL_IMPORT
#endif

void GFX_EXPORT PushDrawBlitImShader();

void GFX_EXPORT PopDrawBlitImShader();

bool GFX_EXPORT IsOnRenderThread();

void GFX_EXPORT EnqueueGenericDrawCommand(void(*cb)(uintptr_t, uintptr_t), uintptr_t* arg1, uintptr_t* arg2);

GFX_EXPORT void SetTextureGtaIm(rage::sga::Texture* texture);

// do we? this is SGA...
#define _HAVE_GRCORE_NEWSTATES 1

namespace rage
{
	void GFX_EXPORT grcBegin(int drawMode, int count);
	void GFX_EXPORT grcVertex(float x, float y, float z, float nX, float nY, float nZ, uint32_t color, float u, float v);
	void GFX_EXPORT grcEnd();
}

uint32_t GFX_EXPORT GetRasterizerState();
uint32_t GFX_EXPORT GetBlendState();
uint32_t GFX_EXPORT GetDepthStencilState();

void GFX_EXPORT SetRasterizerState(uint32_t state);
void GFX_EXPORT SetBlendState(uint32_t state);
void GFX_EXPORT SetDepthStencilState(uint32_t state);

enum StateType
{
	RasterizerStateDefault,
	RasterizerStateNoCulling,
	BlendStateNoBlend,
	BlendStateDefault,
	BlendStatePremultiplied,
	DepthStencilStateNoDepth,
	SamplerStatePoint, // from GFx
	StateTypeMax
};

uint32_t GFX_EXPORT GetStockStateIdentifier(StateType state);

uint32_t GFX_EXPORT GetImDiffuseSamplerState();
void GFX_EXPORT SetImDiffuseSamplerState(uint32_t samplerStateIdentifier);

void GFX_EXPORT GetGameResolution(int& x, int& y);

// for 2D draws using CS_BLIT
bool GFX_EXPORT TransformToScreenSpace(float* verts2d, int len);

void GFX_EXPORT SetScissorRect(int x, int y, int z, int w);

extern GFX_EXPORT fwEvent<> OnPostFrontendRender;
extern GFX_EXPORT fwEvent<> OnGrcCreateDevice;

enum class GraphicsAPI
{
	Unknown,
	Vulkan,
	D3D12
};

extern GFX_EXPORT GraphicsAPI GetCurrentGraphicsAPI();

// VK context or D3D12 device
extern GFX_EXPORT void* GetGraphicsDriverHandle();
