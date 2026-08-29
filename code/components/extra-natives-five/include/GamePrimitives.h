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
	float m_world[16];
	float m_worldView[16];
	float m_worldViewProj[16];
	float m_inverseView[16];
	float m_view[16];
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

	// Load view and projection matrices
	XMMATRIX worldView = XMLoadFloat4x4((const XMFLOAT4X4*)&viewport.m_worldView);
	XMMATRIX projection = XMLoadFloat4x4((const XMFLOAT4X4*)&viewport.m_projection);

	// Re-orthonormalize the view matrix to reduce accumulated drift
	worldView.r[0] = XMVector3Normalize(worldView.r[0]);
	worldView.r[1] = XMVector3Normalize(worldView.r[1]);
	worldView.r[2] = XMVector3Normalize(worldView.r[2]);

	// Invert the projection and view matrices separately
	// (doing this is numerically more stable than inverting the combined matrix)
	XMMATRIX invProj = XMMatrixInverse(nullptr, projection);
	XMMATRIX invView = XMMatrixInverse(nullptr, worldView);

	// Convert from screen space (0..1) to NDC (-1..1)
	float ndcX = (viewPos.x * 2.0f) - 1.0f;
	float ndcY = ((1.0f - viewPos.y) * 2.0f) - 1.0f;
	float ndcZ = viewPos.z;

	XMVECTOR inVec = XMVectorSet(ndcX, ndcY, ndcZ, 1.0f);

	// Transform from NDC to view space
	XMVECTOR viewSpace = XMVector4Transform(inVec, invProj);
	viewSpace = XMVectorScale(viewSpace, 1.0f / XMVectorGetW(viewSpace));

	// Transform from view space to world space
	XMVECTOR worldSpace = XMVector4Transform(viewSpace, invView);
	worldSpace = XMVectorScale(worldSpace, 1.0f / XMVectorGetW(worldSpace));

	// Store the final world coordinates
	rage::Vec3V result = {
		XMVectorGetX(worldSpace),
		XMVectorGetY(worldSpace),
		XMVectorGetZ(worldSpace)
	};

	return result;
}
