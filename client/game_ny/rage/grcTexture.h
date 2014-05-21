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

void GAMESPEC_EXPORT RegisterD3DPostResetCallback(void(*function)());