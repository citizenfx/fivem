/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once
#include <d3d9.h>

namespace rage
{
class grcTexture
{
public:
	virtual ~grcTexture() = 0;

public:
	char pad[16];
	const char* m_pszTextureName;
	IDirect3DTexture9* m_pITexture;
	char pad2[44];
	void* m_pixelData;

public:
	static inline bool IsRenderSystemColorSwapped() { return false; }
};

struct grcManualTextureDef
{
	int isStaging;
	char pad[8];
	int arraySize;
};

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

	uint32_t pad3;

	grcTextureReference* nextMipLevel;
	grcTextureReference* nextMajorLevel;

	int pad4[4];

	float pad5[6];
};

#define FORMAT_A8R8G8B8 2

class grcTextureFactory
{
public:
	virtual ~grcTextureFactory() = 0;

	virtual grcTexture* createManualTexture(short width, short height, int format, int unknown, const grcManualTextureDef* templ) = 0;

	virtual grcTexture* createImage(grcTextureReference* texture, void* unkTemplate) = 0;

public:
	static GAMESPEC_EXPORT grcTexture* GetNoneTexture();

	static GAMESPEC_EXPORT grcTextureFactory* getInstance();
};
}

extern
	#ifdef COMPILING_RAGE_GRAPHICS_NY
	__declspec(dllexport)
	#else
	__declspec(dllimport)
	#endif
	fwEvent<> OnD3DPostReset;

void GAMESPEC_EXPORT ClearRenderTarget(bool a1, int value1, bool a2, float value2, bool a3, int value3);