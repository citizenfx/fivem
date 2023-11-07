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
}
