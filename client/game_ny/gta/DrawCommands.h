#pragma once

#include "grcTexture.h"

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

void GAMESPEC_EXPORT SetTexture(rage::grcTexture* texture);

void GAMESPEC_EXPORT PushUnlitImShader();
void GAMESPEC_EXPORT BeginImVertices(int count, int count2);
void GAMESPEC_EXPORT AddImVertex(float x, float y, float z, float nX, float nY, float nZ, uint32_t color, float u, float v);
void GAMESPEC_EXPORT DrawImVertices();
void GAMESPEC_EXPORT PopUnlitImShader();

void GAMESPEC_EXPORT SetTextureGtaIm(rage::grcTexture* texture);
void GAMESPEC_EXPORT DrawImSprite(float x1, float y1, float x2, float y2, float z, float u1, float v1, float u2, float v2, uint32_t* color, int subShader);