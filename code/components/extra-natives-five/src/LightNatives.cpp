#include "StdInc.h"
#include <ScriptEngine.h>
#include <Hooking.h>
#include <GameInit.h>
#include <Hooking.Stubs.h>
#include <Streaming.h>
#include <rageVectors.h>
#include <EntitySystem.h>

#define MATH_PI 3.14159265358979323846f
#define MATH_DEG2RAD (MATH_PI / 180.0f)

class CLight;

static hook::cdecl_stub<void(CLight*, uint64_t unk1, char unk2)> CLight_drawLight([]()
{
    return hook::get_pattern("48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 4C 89 60 20 55 41 56 41 57 48 8D 68 A1 48 81 EC A0 00 00 00 0F 29 70 D8 45");
});

class CLight
{
public:
    rage::Vector3 coords;
    rage::Vector3 color;

    rage::Vector3 direction;
    rage::Vector3 tanDirection;

    rage::Vector4 volColorIntensityOuter;

    int clipRectX;
    int clipRectY;
    int clipRectWidth;
    int clipRectHeight;

    uint32_t lightType;
    uint32_t flags;
    float intensity;
    uint32_t timeFlags : 24;
    uint32_t extraFlags : 8;

    int32_t texDictId;
    uint32_t textureHash;

    float volIntensity;
    float volSizeScale;
    float volOuterExponent;

    size_t shadowTrackingID;
    int32_t shadowIndex;

    rage::fwInteriorLocation interior;

    float radius;
    float params[13];

    uint8_t fadeDistance;
    uint8_t shadowFadeDistance;
    uint8_t specularFadeDistance;
    uint8_t volumetricFadeDistance;

    float shadowNearClip;
    float shadowBlur;
    float shadowDepthBiasScale;

    float alpha;

    void* unk;
    char unk2[0xC8];

	CLight()
	{
		coords = { 0.0f, 0.0f, 0.0f };
		color = { 1.0f, 1.0f, 1.0f };

		direction = { 0.0f, 0.0f, 1.0f };
		tanDirection = { 0.0f, 1.0f, 0.0f };

		volColorIntensityOuter = { 1.0f, 1.0f, 1.0f, 1.0f };

		clipRectX = -1;
		clipRectY = -1;
		clipRectWidth = -1;
		clipRectHeight = -1;

		lightType = 1;
		flags = (1 << 1);
		intensity = 1.0f;
		timeFlags = (1 << 24) - 1;
		extraFlags = 0;

		texDictId = -1;
		textureHash = 0;

		volIntensity = 1.0f;
		volSizeScale = 1.0f;
		volOuterExponent = 1.0f;

		shadowTrackingID = 0;
		shadowIndex = -1;

		radius = 1.0f;

		for (int i = 0; i < 13; ++i)
			params[i] = 0.0f;

		params[0] = 8.0f;

		fadeDistance = 0U;
		shadowFadeDistance = 0U;
		specularFadeDistance = 0U;
		volumetricFadeDistance = 0U;

		shadowNearClip = 0.01f;
		shadowBlur = 0.0f;
		shadowDepthBiasScale = 1.0f;

		alpha = 1.0f;

		unk = nullptr;
		memset(unk2, 0, sizeof(unk2));
	}
};

static CLight g_light;

static InitFunction initFunction([]()
{

    fx::ScriptEngine::RegisterNativeHandler("PREPARE_LIGHT", [](fx::ScriptContext& ctx)
    {
        uint32_t lightType = ctx.GetArgument<uint32_t>(0);
        uint32_t flags = ctx.GetArgument<uint32_t>(1);
        float x = ctx.GetArgument<float>(2);
        float y = ctx.GetArgument<float>(3);
        float z = ctx.GetArgument<float>(4);
        uint32_t r = ctx.GetArgument<uint32_t>(5);
        uint32_t g = ctx.GetArgument<uint32_t>(6);
        uint32_t b = ctx.GetArgument<uint32_t>(7);
        float intensity = ctx.GetArgument<float>(8);

		g_light.lightType = lightType;
		g_light.flags = flags;
		g_light.coords = { x, y, z };
		g_light.color = { r / 255.0f, g / 255.0f, b / 255.0f };
		g_light.intensity = intensity;
    });

    fx::ScriptEngine::RegisterNativeHandler("DRAW_LIGHT", [](fx::ScriptContext& ctx)
    {
        CLight_drawLight(&g_light, 0, 0);
		g_light = CLight();
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_TYPE", [](fx::ScriptContext& ctx)
    {
        g_light.lightType = ctx.GetArgument<uint32_t>(0);
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_EXTRAFLAGS", [](fx::ScriptContext& ctx)
    {
        g_light.extraFlags = ctx.GetArgument<uint32_t>(0);
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_TEXTURE", [](fx::ScriptContext& ctx)
    {
        std::string textureDict = ctx.CheckArgument<const char*>(0);
        uint32_t textureHash = ctx.GetArgument<uint32_t>(1);

        auto str = streaming::Manager::GetInstance();
        if (!str)
        {
            return;
        }

        auto txdStore = str->moduleMgr.GetStreamingModule("ytd");
        uint32_t id = -1;
        txdStore->FindSlot(&id, textureDict.c_str());

        if (id != 0xFFFFFFFF)
        {
            g_light.textureHash = textureHash;
            g_light.texDictId = id;
        }
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_COORDS", [](fx::ScriptContext& ctx)
    {
        float x = ctx.GetArgument<float>(0);
        float y = ctx.GetArgument<float>(1);
        float z = ctx.GetArgument<float>(2);
        g_light.coords = { x, y, z };
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_COLOR", [](fx::ScriptContext& ctx)
    {
        int32_t r = ctx.GetArgument<int32_t>(0);
        int32_t g = ctx.GetArgument<int32_t>(1);
        int32_t b = ctx.GetArgument<int32_t>(2);
        g_light.color = { r / 255.0f, g / 255.0f, b / 255.0f };
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_DIRECTION", [](fx::ScriptContext& ctx)
    {
        float xDir = ctx.GetArgument<float>(0);
        float yDir = ctx.GetArgument<float>(1);
        float zDir = ctx.GetArgument<float>(2);
        float xTanDir = ctx.GetArgument<float>(3);
        float yTanDir = ctx.GetArgument<float>(4);
        float zTanDir = ctx.GetArgument<float>(5);

        auto dir = DirectX::XMVector3Normalize({ xDir, yDir, zDir });
        g_light.direction.x = DirectX::XMVectorGetX(dir);
        g_light.direction.y = DirectX::XMVectorGetY(dir);
        g_light.direction.z = DirectX::XMVectorGetZ(dir);
        auto tanDir = DirectX::XMVector3Normalize({ xTanDir, yTanDir, zTanDir });
        g_light.tanDirection.x = DirectX::XMVectorGetX(tanDir);
        g_light.tanDirection.y = DirectX::XMVectorGetY(tanDir);
        g_light.tanDirection.z = DirectX::XMVectorGetZ(tanDir);
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_RADIUS", [](fx::ScriptContext& ctx)
    {
        g_light.radius = ctx.GetArgument<float>(0);
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_INTENSITY", [](fx::ScriptContext& ctx)
    {
        g_light.intensity = ctx.GetArgument<float>(0);
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_FALLOFF", [](fx::ScriptContext& ctx)
    {
        float falloff = ctx.GetArgument<float>(0);
        g_light.params[0] = falloff <= 0.0f ? 1e-6f : falloff;
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_CONE", [](fx::ScriptContext& ctx)
    {
        float innerConeAngle = ctx.GetArgument<float>(0);
        float outerConeAngle = ctx.GetArgument<float>(1);

        g_light.params[7] = innerConeAngle;
        g_light.params[8] = outerConeAngle;
        g_light.params[5] = cosf(g_light.params[7] * MATH_DEG2RAD);
        g_light.params[6] = cosf(g_light.params[8] * MATH_DEG2RAD);

        const float cosAngle = g_light.params[6];
        const float tanAngle = sqrtf(1.0f - cosAngle * cosAngle) / cosAngle;

        g_light.params[10] = g_light.radius * cosAngle;
        g_light.params[9] = g_light.params[10] * tanAngle;
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_HEADLIGHT", [](fx::ScriptContext& ctx)
    {
        g_light.params[11] = ctx.GetArgument<float>(0);
        g_light.params[12] = ctx.GetArgument<float>(1);
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_AO", [](fx::ScriptContext& ctx)
    {
        g_light.params[5] = ctx.GetArgument<float>(0);
        g_light.params[6] = ctx.GetArgument<float>(1);
        g_light.params[7] = ctx.GetArgument<float>(2);
        g_light.params[8] = ctx.GetArgument<float>(3);
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_CAPSULE_SIZE", [](fx::ScriptContext& ctx)
    {
        g_light.params[5] = ctx.GetArgument<float>(0);
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_VOLUME_DETAILS", [](fx::ScriptContext& ctx)
    {
        float volIntensity = ctx.GetArgument<float>(0);
        float volSizeScale = ctx.GetArgument<float>(1);
        float r = ctx.GetArgument<float>(2);
        float g = ctx.GetArgument<float>(3);
        float b = ctx.GetArgument<float>(4);
        float i = ctx.GetArgument<float>(5);
        float outerExponent = ctx.GetArgument<float>(6);

        g_light.volIntensity = volIntensity;
        g_light.volSizeScale = volSizeScale;
        g_light.volColorIntensityOuter = { r, b, g, i };
        g_light.volOuterExponent = outerExponent;
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_PLANE", [](fx::ScriptContext& ctx)
    {
        float nx = ctx.GetArgument<float>(0);
        float ny = ctx.GetArgument<float>(1);
        float nz = ctx.GetArgument<float>(2);
        float dist = ctx.GetArgument<float>(3);

        float mtx[16];
        for (int i = 0; i < 16; ++i)
        {
            mtx[i] = ctx.GetArgument<float>(4 + i);
        }

        DirectX::XMVECTOR normal = DirectX::XMVectorSet(nx, ny, nz, 0.0f);
        DirectX::XMMATRIX matrix = DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(mtx));
        DirectX::XMVECTOR transformedNormal = DirectX::XMVector3TransformNormal(normal, matrix);
        DirectX::XMFLOAT3 lightPosFloat3(g_light.coords.x, g_light.coords.y, g_light.coords.z);
        DirectX::XMVECTOR lightPos = DirectX::XMLoadFloat3(&lightPosFloat3);
        DirectX::XMVECTOR planeOrigin = DirectX::XMVectorSubtract(lightPos, DirectX::XMVectorScale(transformedNormal, dist));

        g_light.params[1] = DirectX::XMVectorGetX(transformedNormal);
        g_light.params[2] = DirectX::XMVectorGetY(transformedNormal);
        g_light.params[3] = DirectX::XMVectorGetZ(transformedNormal);
        g_light.params[4] = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(planeOrigin, transformedNormal));
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_FLAGS", [](fx::ScriptContext& ctx)
    {
        g_light.flags = ctx.GetArgument<uint32_t>(0);
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_SHADOW_DETAILS", [](fx::ScriptContext& ctx)
    {
        g_light.shadowTrackingID = ctx.GetArgument<int32_t>(0);
        g_light.shadowNearClip = ctx.GetArgument<float>(1);
        g_light.shadowBlur = ctx.GetArgument<float>(2);
        g_light.shadowDepthBiasScale = ctx.GetArgument<float>(3);
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_CLIP_RECT", [](fx::ScriptContext& ctx)
    {
        g_light.clipRectX = ctx.GetArgument<int>(0);
        g_light.clipRectY = ctx.GetArgument<int>(1);
        g_light.clipRectWidth = ctx.GetArgument<int>(2);
        g_light.clipRectHeight = ctx.GetArgument<int>(3);
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_FADE_DISTANCE", [](fx::ScriptContext& ctx)
    {
        g_light.fadeDistance = ctx.GetArgument<uint8_t>(0);
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_SHADOW_FADE_DISTANCE", [](fx::ScriptContext& ctx)
    {
        g_light.shadowFadeDistance = ctx.GetArgument<uint8_t>(0);
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_SPECULAR_FADE_DISTANCE", [](fx::ScriptContext& ctx)
    {
        g_light.specularFadeDistance = ctx.GetArgument<uint8_t>(0);
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_VOLUMETRIC_FADE_DISTANCE", [](fx::ScriptContext& ctx)
    {
        g_light.volumetricFadeDistance = ctx.GetArgument<uint8_t>(0);
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_INTERIOR", [](fx::ScriptContext& ctx)
    {
        uint16_t interior = ctx.GetArgument<uint8_t>(0);
        bool isPortal = ctx.GetArgument<uint8_t>(1);
        uint16_t roomIndex = ctx.GetArgument<uint8_t>(2);

        g_light.interior = { interior, isPortal, roomIndex };
    });

    fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_ALPHA", [](fx::ScriptContext& ctx)
    {
        g_light.alpha = ctx.GetArgument<float>(0);
    });
});
