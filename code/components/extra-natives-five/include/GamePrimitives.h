#pragma once

#include <DrawCommands.h>

// SSE4 is pretty common
#define _XM_SSE4_INTRINSICS_
#include <DirectXMath.h>

extern fwEvent<> OnDrawSceneEnd;
extern fwEvent<int, int> OnSetUpRenderBuffers;

struct HitFlags
{
	// bit field
	// 1: building
	// 2: animatedBuilding
	// 3: vehicle
	// 4: ped
	// 5: object
	// 6: dummyObject
	// 11: lightEntity
	// 12: compEntity
	uint32_t entityTypeMask = (1 << 1) | (1 << 2);

	// do precise (drawable) testing?
	bool preciseTesting = true;
};

fwEntity* DoMouseHitTest(int mX, int mY, const HitFlags& flags);

inline int GetViewportW()
{
	int w, h;
	GetGameResolution(w, h);

	return w;
}

inline int GetViewportH()
{
	int w, h;
	GetGameResolution(w, h);

	return h;
}

namespace rage
{
struct alignas(16) Vec3V
{
	float x;
	float y;
	float z;
	float pad;

	Vec3V()
		: x(0), y(0), z(0), pad(0)
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
	rage::Vec3V mins;
	rage::Vec3V maxs;
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
	rage::Vec3V start;
	rage::Vec3V end;
};

enum class eSearchVolumeType : int
{
	SphereContains,
	SphereIntersect,
	SphereIntersectPrecise,
	AabbContainsAabb,
	AabbContainsSphere,
	AabbIntersectsSphere,
	AabbIntersectsAabb,
	RayIntersectsAabb
};

struct fwSearchVolume
{
	spdAABB aabb;
	spdSphere sphere;
	spdRay ray;
	eSearchVolumeType type;
};

struct grcViewport
{
	float m_mat1[16];
	float m_mat2[16];
	float m_viewProjection[16];
	float m_inverseView[16];
	char m_pad[64];
	float m_projection[16];
};

struct spdViewport : grcViewport
{
	static spdViewport* GetCurrent();
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

	auto invVP = XMMatrixInverse(NULL, XMLoadFloat4x4((const XMFLOAT4X4*)viewport.m_viewProjection));
	auto inVec = XMVectorSet((viewPos.x * 2.0f) - 1.0f, ((1.0 - viewPos.y) * 2.0f) - 1.0f, viewPos.z, 1.0f);
	auto outCoord = XMVector3TransformCoord(inVec, invVP);

	return {
		XMVectorGetX(outCoord),
		XMVectorGetY(outCoord),
		XMVectorGetZ(outCoord)
	};
}
