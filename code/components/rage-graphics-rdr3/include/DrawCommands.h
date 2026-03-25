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

namespace rage::sga
{
class GFX_EXPORT GraphicsContext
{
public:
	// GetCurrent returns the current GraphicsContext
	static GraphicsContext* GetCurrent();
};

// MapData stores information for a Map/Unmap operation
class MapData
{
private:
	char m_pad[68];

public:
	inline MapData()
	{
		memset(m_pad, 0, sizeof(m_pad));
		m_pad[64] = 1;
	}

	inline void* GetBuffer()
	{
		return *(void**)&m_pad[0];
	}

	inline int GetStride()
	{
		return *(int*)&m_pad[8];
	}
};
}

namespace rage::sga::ext
{
// DynamicResource wraps multiple underlying resources to simplify renaming operations for per-frame mapping
// (since D3D12/Vulkan do not do automatic renaming for Map/Unmap operations)
class GFX_EXPORT DynamicResource
{
private:
	char m_pad[96];

public:
	DynamicResource();

	// GetResourceIdx returns the index of this frame's underlying resource
	int GetResourceIdx();

	// UnmapBase is called by derived Unmap implementations
	void UnmapBase(GraphicsContext* context /* can be null for current */, const MapData& mapData);
};

// DynamicTexture2 is an implementation of DynamicResource for a 2D texture
class GFX_EXPORT DynamicTexture2 : DynamicResource
{
public:
	~DynamicTexture2();

	// Init sets up the underlying resources
	void Init(int flags /* usually 3? */, void* unk_nullptr /* seems to be a lambda argument */, const ImageParams& imageParams, int bufferType, uint32_t flags1, void* memInfo, uint32_t flags2, int cpuAccessType, void* clearValue);

	// Map maps writable memory to the MapData
	bool Map(GraphicsContext* context, MapData& mapData);

	// Unmap finalizes a Map operation and updates the resource
	void Unmap(GraphicsContext* context, const MapData& mapData);

	// MakeReady assigns the DynamicTexture2 to a GraphicsContext
	void MakeReady(GraphicsContext* context /* can *not* be null */);

	// GetTexture gets the current readable texture
	Texture* GetTexture();

private:
	void UnmapInternal(GraphicsContext* context, const MapData& mapData);
};
}
