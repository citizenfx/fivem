#pragma once

#include <d3d11.h>

#include "grcTexture.h"

#define _HAVE_GRCORE_NEWSTATES 1

#ifdef COMPILING_RAGE_GRAPHICS_FIVE
#define GFXSPEC_EXPORT_VMT __declspec(dllexport) __declspec(novtable)
#define GFXSPEC_EXPORT __declspec(dllexport)
#else
// no dllimport for importing a vtable as it will be translated to a local vtable
#define GFXSPEC_EXPORT_VMT __declspec(novtable)
#define GFXSPEC_EXPORT __declspec(dllimport)
#endif

namespace rage
{
	class grmShaderParameter
	{
	public:
		uint8_t m_pad;
		uint8_t m_type;
		uint16_t m_pad2;
		uint32_t m_pad3;
		void* m_value;
	};

	class grmShaderFx;

	class GFXSPEC_EXPORT grmShaderDef
	{
	public:
		int GetParameter(const char* name);

		int GetTechnique(const char* name);

		void SetParameter(grmShaderFx* shader, int index, const void* data, int size, int count);

		void SetSampler(grmShaderFx* shader, int index, void* sampler);

		void PushPass(int pass, grmShaderFx* shader);

		void PopPass();
	};

	class GFXSPEC_EXPORT grmShaderFx
	{
	public:
		grmShaderParameter* m_parameters;
		grmShaderDef* m_shader;
		uint8_t m_parameterCount;
		uint8_t m_drawBucket;
		uint8_t m_pad;
		uint8_t m_hasComment;
		uint16_t m_parameterSize;
		uint16_t m_parameterDataSize;
		void* m_preset;
		uint32_t m_drawBucketMask; // 1 << (bucket) | 0xFF00
		uint8_t m_instanced;
		uint8_t m_pad2[2];
		uint8_t m_resourceCount;
		uint32_t m_pad3;

	public:
		bool LoadTechnique(const char* name, void* templ, bool a4);

		// return num passes
		int PushTechnique(int technique, bool unk1, int a4);

		void PushPass(int index);

		void PopPass();

		void PopTechnique();

		inline int GetParameter(const char* name)
		{
			return m_shader->GetParameter(name);
		}

		inline int GetTechnique(const char* name)
		{
			return m_shader->GetTechnique(name);
		}

		inline void SetSampler(int index, void* sampler)
		{
			return m_shader->SetSampler(this, index, sampler);
		}

		inline void SetParameter(int index, const void* data, int size, int count)
		{
			return m_shader->SetParameter(this, index, data, size, count);
		}
	};

	class GFXSPEC_EXPORT_VMT grmShaderFactory
	{
	public:
		virtual ~grmShaderFactory() = default;

		virtual grmShaderFx* GetShader() = 0;

		static grmShaderFactory* GetInstance();
	};
}

GFXSPEC_EXPORT ID3D11Device* GetD3D11Device();
GFXSPEC_EXPORT ID3D11DeviceContext* GetD3D11DeviceContext();

void GFXSPEC_EXPORT AddTextureOverride(rage::grcTexture* orig, rage::grcTexture* repl);
void GFXSPEC_EXPORT RemoveTextureOverride(rage::grcTexture* orig);

void GFXSPEC_EXPORT EnqueueGenericDrawCommand(void(*cb)(uintptr_t, uintptr_t), uintptr_t* arg1, uintptr_t* arg2);

bool GFXSPEC_EXPORT IsOnRenderThread();

namespace rage
{
	void GFXSPEC_EXPORT grcBegin(int drawMode, int count);
	void GFXSPEC_EXPORT grcVertex(float x, float y, float z, float nX, float nY, float nZ, uint32_t color, float u, float v);
	void GFXSPEC_EXPORT grcEnd();
}

void GFXSPEC_EXPORT PushDrawBlitImShader();

void GFXSPEC_EXPORT PopDrawBlitImShader();

void GFXSPEC_EXPORT SetTextureGtaIm(rage::grcTexture* texture);
void GFXSPEC_EXPORT DrawImSprite(float x1, float y1, float x2, float y2, float z, float u1, float v1, float u2, float v2, uint32_t* color, int subShader);

uint32_t GFXSPEC_EXPORT CreateRasterizerState(const D3D11_RASTERIZER_DESC* desc);
uint32_t GFXSPEC_EXPORT CreateSamplerState(const D3D11_SAMPLER_DESC* desc);
uint32_t GFXSPEC_EXPORT GetImDiffuseSamplerState();
void GFXSPEC_EXPORT SetImDiffuseSamplerState(uint32_t samplerStateIdentifier);


// new state system
uint32_t GFXSPEC_EXPORT GetRasterizerState();
uint32_t GFXSPEC_EXPORT GetBlendState();
uint32_t GFXSPEC_EXPORT GetDepthStencilState();

void GFXSPEC_EXPORT SetRasterizerState(uint32_t state);
void GFXSPEC_EXPORT SetBlendState(uint32_t state);
void GFXSPEC_EXPORT SetDepthStencilState(uint32_t state);

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

uint32_t GFXSPEC_EXPORT GetStockStateIdentifier(StateType state);

void GFXSPEC_EXPORT GfxForceVsync(bool enabled);

void GFXSPEC_EXPORT SetWorldMatrix(const float* matrix);

extern GFXSPEC_EXPORT fwEvent<> OnGrcCreateDevice;

extern GFXSPEC_EXPORT fwEvent<> OnPostFrontendRender;

extern GFXSPEC_EXPORT fwEvent<> OnTempDrawEntityList;

GFXSPEC_EXPORT void GetGameResolution(int& width, int& height);

#define grcCullModeNone 0

#include <d3d11_1.h>

#include <dxgi1_4.h>
#include <dxgi1_5.h>

extern GFXSPEC_EXPORT fwEvent<IDXGIFactory2*, ID3D11Device*, HWND, DXGI_SWAP_CHAIN_DESC1*, DXGI_SWAP_CHAIN_FULLSCREEN_DESC*, IDXGISwapChain1**> OnTryCreateSwapChain;

extern GFXSPEC_EXPORT fwEvent<bool*> OnRequestInternalScreenshot;
extern GFXSPEC_EXPORT fwEvent<const uint8_t*, int, int> OnInternalScreenshot;
