#pragma once

#include "grcTexture.h"

#ifdef COMPILING_RAGE_GRAPHICS_NY
#define GAMESPEC_EXPORT_VMT __declspec(dllexport) __declspec(novtable)
#define GAMESPEC_EXPORT __declspec(dllexport)
#else
// no dllimport for importing a vtable as it will be translated to a local vtable
#define GAMESPEC_EXPORT_VMT __declspec(novtable)
#define GAMESPEC_EXPORT __declspec(dllimport)
#endif

class GAMESPEC_EXPORT_VMT __declspec(novtable) CBaseDC
{
public:
	~CBaseDC();

	virtual void Render() = 0;

	void* operator new(size_t size, int a2);

	void operator delete(void* memory);

	void Enqueue();
};

class GAMESPEC_EXPORT_VMT CImplementedDC : public CBaseDC
{
protected:
	CImplementedDC();

public:
	virtual ~CImplementedDC();

	virtual void Render();
};

class GAMESPEC_EXPORT_VMT CDrawSpriteDC : public CImplementedDC
{
private:
	char pad[44];

public:
	CDrawSpriteDC(const float* bottomLeft, const float* topLeft, const float* bottomRight, const float* topRight, uint32_t color, rage::grcTexture* texture);
};

class GAMESPEC_EXPORT_VMT CGenericDC : public CImplementedDC
{
private:
	char pad[8];

public:
	CGenericDC(void(*cb)());
};

class GAMESPEC_EXPORT_VMT CGenericDC1Arg : public CImplementedDC
{
private:
	char pad[12];

public:
	CGenericDC1Arg(void(*cb)(int arg), int* arg);
};

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
	#ifdef COMPILING_RAGE_GRAPHICS_NY
	__declspec(dllexport)
	#else
	__declspec(dllimport)
	#endif
	fwEvent<> OnGrcBeginScene;

extern
	#ifdef COMPILING_RAGE_GRAPHICS_NY
	__declspec(dllexport)
	#else
	__declspec(dllimport)
	#endif
	fwEvent<> OnGrcEndScene;

extern
	#ifdef COMPILING_RAGE_GRAPHICS_NY
	__declspec(dllexport)
	#else
	__declspec(dllimport)
	#endif
	fwEvent<> OnGrcCreateDevice;

#ifdef COMPILING_RAGE_GRAPHICS_NY
__declspec(dllexport)
#endif
	void GetGameResolution(int& width, int& height);