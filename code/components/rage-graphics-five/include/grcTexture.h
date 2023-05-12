#pragma once

#include <d3d11.h>
#include <CrossBuildRuntime.h>
#include <XBRVirtual.h>

#define _HAS_GRCTEXTURE_MAP 1

#ifdef COMPILING_RAGE_GRAPHICS_FIVE
#define GFX_EXPORT_VMT __declspec(dllexport) __declspec(novtable)
#define GFX_EXPORT __declspec(dllexport)
#else
// no dllimport for importing a vtable as it will be translated to a local vtable
#define GFX_EXPORT_VMT __declspec(novtable)
#define GFX_EXPORT __declspec(dllimport)
#endif

namespace rage
{
struct grcLockedTexture
{
	int level;
	void* pBits;
	int pitch;
	int pad;
	int width;
	int height;
	int format;
	int numSubLevels;
};

enum class grcLockFlags : int
{
	Read = 1,
	Write = 2,
	Unknown = 4,
	WriteDiscard = 8,
	NoOverwrite = 16
};

DEFINE_ENUM_FLAG_OPERATORS(grcLockFlags);

class grcTexture : XBR_VIRTUAL_BASE_2802(0)
{
public:
	XBR_VIRTUAL_DTOR(grcTexture)

	XBR_VIRTUAL_METHOD(bool, m_4, ())

	XBR_VIRTUAL_METHOD(int, m_8, ())

	XBR_VIRTUAL_METHOD(void, m_C, ())

	XBR_VIRTUAL_METHOD(int, m_10, ())

	XBR_VIRTUAL_METHOD(uint16_t, GetWidth, ())

	XBR_VIRTUAL_METHOD(uint16_t, GetHeight, ())

	XBR_VIRTUAL_METHOD(uint16_t, GetDepth, ())

	// unknown?
	XBR_VIRTUAL_METHOD(uint8_t, GetLevels, ())

	XBR_VIRTUAL_METHOD(void, m_24, ())

	XBR_VIRTUAL_METHOD(bool, m_28, ())

	XBR_VIRTUAL_METHOD(void, m_2C, (intptr_t))

	XBR_VIRTUAL_METHOD(void, m_30, (void*))

	XBR_VIRTUAL_METHOD(void, m_34, (void*))

	XBR_VIRTUAL_METHOD(void, m_unk, ())

	XBR_VIRTUAL_METHOD(rage::grcTexture*, m_38, ())

	XBR_VIRTUAL_METHOD(rage::grcTexture*, m_3C, ())

	XBR_VIRTUAL_METHOD(bool, m_40, ())

	XBR_VIRTUAL_METHOD(int, m_44, ())

	XBR_VIRTUAL_METHOD(int, m_48, ())

	XBR_VIRTUAL_METHOD(int, m_4C, ())

	XBR_VIRTUAL_METHOD(int, m_50, ())

	XBR_VIRTUAL_METHOD(int, m_54, ())

	XBR_VIRTUAL_METHOD(int, m_unk2, ())

	XBR_VIRTUAL_METHOD(int, m_unk3, ())

	XBR_VIRTUAL_METHOD(bool, Map, (int numSubLevels, int subLevel, grcLockedTexture* lockedTexture, grcLockFlags flags))

	XBR_VIRTUAL_METHOD(void, Unmap, (grcLockedTexture * lockedTexture))

public:
	static bool GFX_EXPORT IsRenderSystemColorSwapped();

	// D3D11 specific
private:
	char m_pad[48];

public:
	ID3D11Resource* texture;

private:
	char m_pad2[56];

public:
	ID3D11ShaderResourceView* srv;
};

class grcRenderTarget : public grcTexture
{

};

#define FORMAT_A8R8G8B8 40

struct grcTextureReference
{
	uint16_t width;
	uint16_t height;
	int format; // also some flags in higher bits
	uint8_t type;
private:
	uint8_t pad;
	uint16_t pad2;
public:

	uint16_t stride;
	uint16_t depth;

	uint8_t* pixelData;

	uintptr_t pad3; // 24

	grcTextureReference* nextMipLevel;
	grcTextureReference* nextMajorLevel;
};

struct grcManualTextureDef
{
	// bit flag:
	// | 1 -> dynamic texture (no internal staging texture)
	// | 2 -> unknown
	int isStaging;
	int unk1;
	// flag, sorta:
	// == 0 -> immutable
	// == 1 -> writable?
	// == 2 -> other weird case
	int usage;
	char pad[12];
	int isRenderTarget;
	char pad2[8];
	int arraySize;
	char pad3[16];
};

class grcTextureFactory
{
public:
	virtual ~grcTextureFactory() = 0;

	virtual grcTexture* unk_8() = 0;

	virtual grcTexture* createManualTexture(short width, short height, int format, const void* initialData, bool unused, const grcManualTextureDef* templ) = 0;

	virtual grcTexture* createImage(grcTextureReference* texture, void* unkTemplate) = 0;

private:
	virtual void v0() = 0;
	virtual void v1() = 0;

public:
	virtual int TranslateFormatToParamFormat(int format) = 0;

private:
	virtual void v2() = 0;
	virtual void v3() = 0;
	virtual void v4() = 0;
	virtual void v5() = 0;
	virtual void v6() = 0;
	virtual void v7() = 0;
	virtual void v8() = 0;
	virtual void v9() = 0;
	virtual void v10() = 0;
	virtual void v11() = 0;

public:
	virtual grcTexture* createFromNativeTexture(const char* name, ID3D11Resource* nativeResource, void* a3) = 0;

	virtual void v12() = 0;

	virtual void PushRenderTarget(void* a2, grcRenderTarget* rt, void* a4, int a5, bool a6, int a7) = 0;

	virtual void PopRenderTarget(void* a2, void* a3_clear) = 0;

public:
	static GFX_EXPORT grcTexture* GetNoneTexture();

	static GFX_EXPORT grcTextureFactory* getInstance();
};

class GFX_EXPORT grcResourceCache
{
public:
	static grcResourceCache* GetInstance();

	void QueueDelete(void* graphicsResource);

	void FlushQueue();

	size_t GetUsedPhysicalMemory();

	size_t GetTotalPhysicalMemory();

	size_t _getAndUpdateAvailableMemory(bool virt, bool spare);
};
}

void GFX_EXPORT ClearRenderTarget(bool a1, int value1, bool a2, float value2, bool a3, int value3);
GFX_EXPORT rage::grcRenderTarget* CreateRenderTarget(int idx, const char* name, int usage3, int width, int height, int format /* or bit count */, void* metadata, uint8_t a, rage::grcRenderTarget* last);
