#pragma once

#include <d3d11.h>

#define _HAS_GRCTEXTURE_MAP 1

#ifdef COMPILING_RAGE_GRAPHICS_FIVE
#define GAMESPEC_EXPORT_VMT __declspec(dllexport) __declspec(novtable)
#define GAMESPEC_EXPORT __declspec(dllexport)
#else
// no dllimport for importing a vtable as it will be translated to a local vtable
#define GAMESPEC_EXPORT_VMT __declspec(novtable)
#define GAMESPEC_EXPORT __declspec(dllimport)
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

class grcTexture
{
public:
	virtual ~grcTexture() = 0;

	virtual bool m_4() = 0;

	virtual int m_8() = 0;

	virtual void m_C() = 0;

	virtual int m_10() = 0;

	virtual uint16_t GetWidth() = 0;

	virtual uint16_t GetHeight() = 0;

	virtual uint16_t GetDepth() = 0;

	// unknown?
	virtual uint8_t GetLevels() = 0;

	virtual void m_24() = 0;

	virtual bool m_28() = 0;

	virtual void m_2C(intptr_t) = 0;

	virtual void m_30(void*) = 0;

	virtual void m_34(void*) = 0;

	virtual void m_unk() = 0;

	virtual rage::grcTexture* m_38() = 0;

	virtual rage::grcTexture* m_3C() = 0;

	virtual bool m_40() = 0;

	virtual int m_44() = 0;

	virtual int m_48() = 0;

	virtual int m_4C() = 0;

	virtual int m_50() = 0;

	virtual int m_54() = 0;

	virtual int m_unk2() = 0;

	virtual int m_unk3() = 0;

	virtual bool Map(int numSubLevels, int subLevel, grcLockedTexture* lockedTexture, grcLockFlags flags) = 0;

	virtual void Unmap(grcLockedTexture* lockedTexture) = 0;

public:
	static bool GAMESPEC_EXPORT IsRenderSystemColorSwapped();

	// D3D11 specific
private:
	char m_pad[48];

public:
	ID3D11Resource* texture;
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
	int isStaging;
	char pad[32];
	int arraySize;
	char pad2[16];
};

class grcTextureFactory
{
public:
	virtual ~grcTextureFactory() = 0;

	virtual grcTexture* unk_8() = 0;

	virtual grcTexture* createManualTexture(short width, short height, int format, void* unknown, bool, const grcManualTextureDef* templ) = 0;

	virtual grcTexture* createImage(grcTextureReference* texture, void* unkTemplate) = 0;

private:
	virtual void v0() = 0;
	virtual void v1() = 0;

public:
	virtual int TranslateFormatToParamFormat(int format) = 0;

public:
	static GAMESPEC_EXPORT grcTexture* GetNoneTexture();

	static GAMESPEC_EXPORT grcTextureFactory* getInstance();
};
}

extern
#ifdef COMPILING_RAGE_GRAPHICS_PAYNE
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	fwEvent<> OnD3DPostReset;

extern
#ifdef COMPILING_RAGE_GRAPHICS_PAYNE
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	fwEvent<> OnPostFrontEndRender;

void GAMESPEC_EXPORT ClearRenderTarget(bool a1, int value1, bool a2, float value2, bool a3, int value3);