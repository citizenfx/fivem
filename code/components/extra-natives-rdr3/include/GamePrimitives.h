#pragma once

// SSE4 is pretty common
#define _XM_SSE4_INTRINSICS_
#include <DirectXMath.h>

namespace rage
{
struct alignas(16) Vec3V
{
	float x;
	float y;
	float z;
	float pad;

	Vec3V()
		: x(0), y(0), z(0), pad(NAN)
	{
	}

	Vec3V(float x, float y, float z)
		: x(x), y(y), z(z), pad(NAN)
	{
	}
};

struct alignas(16) Vec4V
{
	float x;
	float y;
	float z;
	float w;
};

struct spdAABB
{
	Vec3V mins;
	Vec3V maxs;
};

struct spdSphere
{
	// xyz = center
	// w   = radius
	Vec4V sphere;
};

// fake name
struct spdRay
{
	Vec3V start;
	Vec3V end;
};

struct grcViewport
{
	float m_world[16];
	float m_worldView[16];
	float m_projection[16];
	float m_inverseView[16];
	float m_unknown[16];
	float m_view[16];
};
}

struct CViewportGame
{
public:
	virtual ~CViewportGame() = 0;

private:
	char m_pad[8];

public:
	rage::grcViewport viewport;
};

extern CViewportGame** g_viewportGame;

inline rage::Vec3V Unproject(const rage::grcViewport& viewport, const rage::Vec3V& viewPos)
{
	using namespace DirectX;

	auto composite = XMMatrixMultiply(XMLoadFloat4x4((const XMFLOAT4X4*)&viewport.m_worldView), XMLoadFloat4x4((const XMFLOAT4X4*)&viewport.m_projection));
	auto invVP = XMMatrixInverse(NULL, composite);
	auto inVec = XMVectorSet((viewPos.x * 2.0f) - 1.0f, ((1.0 - viewPos.y) * 2.0f) - 1.0f, viewPos.z, 1.0f);
	auto outCoord = XMVector3TransformCoord(inVec, invVP);

	return {
		XMVectorGetX(outCoord),
		XMVectorGetY(outCoord),
		XMVectorGetZ(outCoord)
	};
}
