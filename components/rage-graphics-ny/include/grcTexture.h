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
};

#define FORMAT_A8R8G8B8 2

class grcTextureFactory
{
public:
	virtual ~grcTextureFactory() = 0;

	virtual grcTexture* createManualTexture(short width, short height, int format, int unknown, const grcTexture* templ) = 0;

public:
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