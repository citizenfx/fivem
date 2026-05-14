#include "StdInc.h"

#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"

#include <atArray.h>

#include <xmmintrin.h>
#include <smmintrin.h>
#include <DirectXMath.h>

enum CVisualEffectsRenderMode : uint32_t
{
	RM_DEFAULT = 0,
	RM_MIRROR_REFLECTION = 1,
	RM_WATER_REFLECTION = 2,
	RM_CUBE_REFLECTION = 3,
	RM_SEETHROUGH = 4,
	RM_NUM = 5,
};

enum eLodLightCategory : uint32_t
{
	LODLIGHT_CATEGORY_SMALL = 0,
	LODLIGHT_CATEGORY_MEDIUM = 1,
	LODLIGHT_CATEGORY_LARGE = 2,
	LODLIGHT_CATEGORY_COUNT = 3,
};

struct distLightsParam
{
	char m_Padding[0x18];

	float m_StreetLightHdrIntensity;

	char m_Padding2[0x18];

	float m_HourStart;
	float m_HourEnd;
	float m_StreetLightHourStart;
	float m_StreetLightHourEnd;

	char m_Padding3[0x4c];
};
static_assert(sizeof(distLightsParam) == 0x90, "distLightsParam has wrong size!");

struct DistantLightElementStruct
{
	DirectX::XMFLOAT3 m_Position;
	uint32_t m_Color;
};
static_assert(sizeof(DistantLightElementStruct) == 0x10, "DistantLightElementStruct has wrong size!");

static bool* g_CRenderer_DisableArtificialLights;

static distLightsParam* g_DistLightsParam;
static float* g_WaterReflectionDistLightFadeout;

static uint64_t* g_CLodLightManager_CurrentFrameInfo;

static bool* g_CLodLights_Enabled;
static bool* g_CLodLights_CanLoad;

static hook::cdecl_stub<void()> g_CreateVertexBuffers([]
{
	return hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 40 80 3D ? ? ? ? 00 0F 85");
});

static hook::cdecl_stub<void(CVisualEffectsRenderMode, float, float, float, int, void*)> g_RenderBufferBegin([]
{
	return hook::get_pattern("48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 55 41 56 41 57 48 8D 68 ? 48 81 EC ? ? ? ? 0F 29 70 ? F3 0F 10 35");
});

static hook::cdecl_stub<void(bool)> g_RenderBuffer([]
{
	return hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 57 48 83 EC ? 8B 1D");
});

static hook::cdecl_stub<DistantLightElementStruct*(uint32_t)> g_ReserveRenderBufferSpace([]
{
	return hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 83 3D ? ? ? ? ? 44 8B 15");
});

static hook::cdecl_stub<float(float, float, float, float)> g_CalculateTimeFade([]
{
	return hook::get_pattern("48 8B C4 48 81 EC ? ? ? ? 66 0F 6E 2D");
});

static hook::cdecl_stub<float(uint32_t)> g_GetRenderLodLightCoronaRange([]
{
	return hook::get_pattern("48 8B 05 ? ? ? ? F3 0F 10 0D ? ? ? ? F3 0F 10 05");
});

class grcViewport
{
public:
	char m_Padding[0xc0];
	DirectX::XMMATRIX m_Camera44;
};
static_assert(sizeof(grcViewport) == 0x100, "grcViewport has wrong size!");

static grcViewport* g_grcViewport_Current;

class CDistantLODLight
{
public:
	char m_Padding[0x8];
	atArray<DirectX::XMFLOAT3> m_Positions;
	atArray<uint32_t> m_Colors;

	uint16_t m_NumStreetLights;
	uint16_t m_Category;
};
static_assert(sizeof(CDistantLODLight) == 0x30, "CDistantLODLight has wrong size!");

class CDistantLODLightBucket
{
public:
	CDistantLODLight* m_Light;
	uint16_t m_MapDataIndex;
	uint16_t m_LightCount;
};
static_assert(sizeof(CDistantLODLightBucket) == 0x10, "CDistantLODLightBucket has wrong size!");

uint64_t* g_MapDataStore_Boxes;

static void (*g_CLodLights_RenderDistantLODLights)(CVisualEffectsRenderMode mode, float intensity);

static void __fastcall CLodLights_RenderDistantLODLights(CVisualEffectsRenderMode mode, float intensityScale)
{
	if (*g_CRenderer_DisableArtificialLights)
		return;

	if (!g_CLodLightManager_CurrentFrameInfo)
		return;

	g_CreateVertexBuffers();

	if (mode == RM_WATER_REFLECTION)
	{
		intensityScale *= (1.0f - *g_WaterReflectionDistLightFadeout);

		if (intensityScale <= 0.0f)
			return;
	}

	if (!*g_CLodLights_Enabled || !*g_CLodLights_CanLoad)
		return;

	const uint8_t alphaNormal = uint8_t(
		g_CalculateTimeFade(
			g_DistLightsParam->m_HourStart - 1.0f,
			g_DistLightsParam->m_HourStart,
			g_DistLightsParam->m_HourEnd - 1.0f,
			g_DistLightsParam->m_HourEnd
		) * 255.0f + 0.5f
	);

	const uint8_t alphaStreet = uint8_t(
		g_CalculateTimeFade(
			g_DistLightsParam->m_StreetLightHourStart - 1.0f,
			g_DistLightsParam->m_StreetLightHourStart,
			g_DistLightsParam->m_StreetLightHourEnd - 1.0f,
			g_DistLightsParam->m_StreetLightHourEnd
		) * 255.0f + 0.5f
	);

	if (!alphaNormal && !alphaStreet)
		return;

	static constexpr int32_t kDistantLodLightBucketSize = 0xaf8;

	uint32_t category = LODLIGHT_CATEGORY_MEDIUM;
	uint64_t bucketOffset = kDistantLodLightBucketSize;

	const DirectX::XMVECTOR camera = g_grcViewport_Current->m_Camera44.r[3];

	const float hdrIntensity = g_DistLightsParam->m_StreetLightHdrIntensity * intensityScale;

	do
	{
		const float range = g_GetRenderLodLightCoronaRange(category);

		g_RenderBufferBegin(mode, range, range, hdrIntensity, 0, nullptr);

		uint32_t bucketIndex = 0;

		uint32_t* bucketList = (uint32_t*)(*g_CLodLightManager_CurrentFrameInfo + bucketOffset + kDistantLodLightBucketSize);

		const uint32_t bucketCount = bucketList[700];

		if (bucketCount)
		{
			const float rangeSquared = range * range;

			const DirectX::XMVECTOR half = DirectX::XMVectorReplicate(0.5f);

			do
			{
				CDistantLODLightBucket* bucket = *(CDistantLODLightBucket**)bucketList;

				const uint32_t totalLights = bucket->m_LightCount;

				if (!totalLights)
				{
					bucketIndex++;
					bucketList += 2;
					continue;
				}

				const uint16_t mapDataIndex = bucket->m_MapDataIndex;

				DirectX::XMVECTOR* aabb = (DirectX::XMVECTOR*)(*g_MapDataStore_Boxes + (32 * mapDataIndex));

				const DirectX::XMVECTOR aabbMin = aabb[0];
				const DirectX::XMVECTOR aabbMax = aabb[1];

				const DirectX::XMVECTOR center = DirectX::XMVectorMultiply(
					DirectX::XMVectorAdd(aabbMin, aabbMax),
					half
				);

				const DirectX::XMVECTOR extents = DirectX::XMVectorMultiply(
					DirectX::XMVectorSubtract(aabbMax, aabbMin),
					half
				);

				const DirectX::XMVECTOR delta = DirectX::XMVectorAdd(
					DirectX::XMVectorAbs(DirectX::XMVectorSubtract(camera, center)),
					extents
				);
				
				CDistantLODLight* lightData = bucket->m_Light;

				const uint32_t numStreetLights = lightData->m_NumStreetLights;

				const float distanceSquared = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(delta));

				if (distanceSquared > rangeSquared)
				{
					uint32_t lightCount = 0;

					if (alphaStreet)
					{
						lightCount += numStreetLights;
					}

					if (alphaNormal)
					{
						lightCount += (totalLights - numStreetLights);
					}

					auto* outputBuffer = g_ReserveRenderBufferSpace(lightCount);
					if (!outputBuffer)
						break;

					// NOTE: using raw pointers here avoids atArray::operator[] overhead in this hot loop
					auto* colors = lightData->m_Colors.m_offset;
					auto* positions = lightData->m_Positions.m_offset;

					if (alphaStreet)
					{
						for (uint32_t i = 0; i < numStreetLights; i++)
						{
							uint32_t color = colors[i];

							const uint32_t a = (alphaStreet * (color >> 24)) >> 8;
							const uint32_t r = color & 0xFF;
							const uint32_t g = color & 0xFF00;
							const uint32_t b = (color >> 16) & 0xFF;

							color = (a << 24) | (r << 16) | g | b;

							auto& position = positions[i];

							outputBuffer->m_Position.x = position.x;
							outputBuffer->m_Position.y = position.y;
							outputBuffer->m_Position.z = position.z;

							outputBuffer->m_Color = color;

							outputBuffer++;
						}
					}

					if (alphaNormal)
					{
						for (uint32_t i = numStreetLights; i < totalLights; i++)
						{
							uint32_t color = colors[i];

							const uint32_t a = (alphaNormal * (color >> 24)) >> 8;
							const uint32_t r = color & 0xFF;
							const uint32_t g = color & 0xFF00;
							const uint32_t b = (color >> 16) & 0xFF;

							color = (a << 24) | (r << 16) | g | b;

							auto& position = positions[i];

							outputBuffer->m_Position.x = position.x;
							outputBuffer->m_Position.y = position.y;
							outputBuffer->m_Position.z = position.z;

							outputBuffer->m_Color = color;

							outputBuffer++;
						}
					}
				}

				++bucketIndex;
				bucketList += 2;

			} while (bucketIndex < bucketCount);
		}

		g_RenderBuffer(true);

		category++;
		bucketOffset += kDistantLodLightBucketSize;
	} while (category < LODLIGHT_CATEGORY_COUNT);
}

static HookFunction initFunction([]
{
	g_CRenderer_DisableArtificialLights = hook::get_address<bool*>(hook::get_pattern("80 3D ? ? ? ? 00 0F 28 D9"), 2, 7);
	
	g_MapDataStore_Boxes = hook::get_address<uint64_t*>(hook::get_pattern("48 03 0D ? ? ? ? 0F 28 61"), 3, 7);

    g_CLodLights_Enabled = hook::get_address<bool*>(hook::get_pattern("80 3D ? ? ? ? 00 48 63 DA 74 ? 80 3D ? ? ? ? 00"), 2, 7);
	g_CLodLights_CanLoad = hook::get_address<bool*>(hook::get_pattern("80 3D ? ? ? ? 00 74 ? C6 05 ? ? ? ? 00 33 C9"), 2, 7);

	g_CLodLightManager_CurrentFrameInfo = hook::get_address<uint64_t*>(hook::get_pattern("4C 8B 25 ? ? ? ? 45 8B FD"), 3, 7);

	g_WaterReflectionDistLightFadeout = hook::get_address<float*>(hook::get_pattern("F3 0F 5C 05 ? ? ? ? F3 44 0F 59 C8 44 0F 2F 0D"), 4, 8);

	g_DistLightsParam = hook::get_address<distLightsParam*>(hook::get_pattern("F3 0F 10 1D ? ? ? ? EB ? F3 0F 10 1D ? ? ? ? 0F 28 C6"), 4, 8);

	g_grcViewport_Current = hook::get_address<grcViewport*>(hook::get_pattern("48 8B 05 ? ? ? ? BB ? ? ? ? BD"), 3, 7);

	g_CLodLights_RenderDistantLODLights = hook::trampoline(hook::get_pattern("48 8B C4 48 89 58 ? 89 48 ? 55"), CLodLights_RenderDistantLODLights);
});
