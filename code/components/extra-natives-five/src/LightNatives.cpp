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

static hook::cdecl_stub<void(CLight* mgr, uint32_t lightType, uint32_t flags, rage::Vector3* cooords, rage::Vector3 color, float intensity, uint32_t timeFlags)> CLight_ctor([]()
{
	return hook::get_pattern("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 40 83 A1");
});

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

	CLight(uint32_t a, uint32_t b, rage::Vector3* c, rage::Vector3 d, float e, uint32_t f)
	{
		CLight_ctor(this, a, b, c, d, e, f);
	}
};

class CLightManager
{
	std::unordered_map<uint32_t, CLight*> lights;
	uint32_t currentHandle = 1;

public:
	uint32_t CreateLight(uint32_t lightType, uint32_t flags, rage::Vector3* pos, rage::Vector3 color, float intensity, uint32_t timeFlags)
	{
		CLight* newLight = new CLight(lightType, flags, pos, color, intensity, timeFlags);
		uint32_t handle = currentHandle;
		currentHandle++;
		lights[handle] = newLight;
		return handle;
	}

	bool DeleteLight(uint32_t handle)
	{
		auto it = lights.find(handle);
		if (it != lights.end())
		{
			delete it->second;
			lights.erase(it);
			return true;
		}
		return false;
	}

	CLight* GetLight(uint32_t handle)
	{
		auto it = lights.find(handle);
		return it != lights.end() ? it->second : nullptr;
	}

	void Reset()
	{
		for (auto& lightPair : lights)
		{
			delete lightPair.second;
		}
		lights = {};
		currentHandle = 1;
	}

	void RenderLights()
	{
		for (auto& lightPair : lights)
		{
			CLight* light = lightPair.second;
			CLight_drawLight(light, 0, 0);
		}
	}
};

static CLightManager lightManager;

static void (*g_everyFrame)();
static void Scripts_everyFrame()
{
	g_everyFrame();
	lightManager.RenderLights();
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("CREATE_LIGHT", [](fx::ScriptContext& ctx)
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

		rage::Vector3 pos = { x, y, z };
		rage::Vector3 color = { r / 255.0f, g / 255.0f, b / 255.0f };

		uint32_t handle = lightManager.CreateLight(lightType, flags, &pos, color, intensity, ((1 << 24) - 1));

		ctx.SetResult<uint32_t>(handle);
	});

	fx::ScriptEngine::RegisterNativeHandler("DELETE_LIGHT", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		bool success = lightManager.DeleteLight(handle);

		ctx.SetResult<bool>(success);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_TYPE", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		uint32_t type = ctx.GetArgument<uint32_t>(1);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->lightType = type;
		ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_EXTRAFLAGS", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		uint32_t extraFlags = ctx.GetArgument<uint32_t>(1);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->extraFlags = extraFlags;
		ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_TEXTURE", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		std::string textureDict = ctx.CheckArgument<const char*>(1);
		uint32_t textureHash = ctx.GetArgument<uint32_t>(2);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		auto str = streaming::Manager::GetInstance();
		if (!str)
		{
			return ctx.SetResult<bool>(false);
		}

		auto txdStore = str->moduleMgr.GetStreamingModule("ytd");

		uint32_t id = -1;
		txdStore->FindSlot(&id, textureDict.c_str());

		if (id == 0xFFFFFFFF)
		{
			return ctx.SetResult<bool>(false);
		}

		light->textureHash = textureHash;
		light->texDictId = id;
		ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_COORDS", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		float x = ctx.GetArgument<float>(1);
		float y = ctx.GetArgument<float>(2);
		float z = ctx.GetArgument<float>(3);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->coords = { x, y, z };
		ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_COLOR", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		int32_t r = ctx.GetArgument<int32_t>(1);
		int32_t g = ctx.GetArgument<int32_t>(2);
		int32_t b = ctx.GetArgument<int32_t>(3);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		rage::Vector3 color = { r / 255.0f, g / 255.0f, b / 255.0f };
		light->color = color;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_DIRECTION", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		float xDir = ctx.GetArgument<float>(1);
		float yDir = ctx.GetArgument<float>(2);
		float zDir = ctx.GetArgument<float>(3);
		float xTanDir = ctx.GetArgument<float>(4);
		float yTanDir = ctx.GetArgument<float>(5);
		float zTanDir = ctx.GetArgument<float>(6);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		auto dir = DirectX::XMVector3Normalize({ xDir, yDir, zDir });
		light->direction.x = DirectX::XMVectorGetX(dir);
		light->direction.y = DirectX::XMVectorGetY(dir);
		light->direction.z = DirectX::XMVectorGetZ(dir);
		auto tanDir = DirectX::XMVector3Normalize({ xTanDir, yTanDir, zTanDir });
		light->tanDirection.x = DirectX::XMVectorGetX(tanDir);
		light->tanDirection.y = DirectX::XMVectorGetY(tanDir);
		light->tanDirection.z = DirectX::XMVectorGetZ(tanDir);

		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_RADIUS", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		float radius = ctx.GetArgument<float>(1);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->radius = radius;
		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_INTENSITY", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		float intensity = ctx.GetArgument<float>(1);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->intensity = intensity;

		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_FALLOFF", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		float falloff = ctx.GetArgument<float>(1);

		CLight* light = lightManager.GetLight(handle);
		if (light)
		{
			light->params[0] = falloff <= 0.0f ? 1e-6f : falloff;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_CONE", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		float innerConeAngle = ctx.GetArgument<float>(1);
		float outerConeAngle = ctx.GetArgument<float>(2);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->params[7] = innerConeAngle;
		light->params[8] = outerConeAngle;

		light->params[5] = cosf(light->params[7] * MATH_DEG2RAD);
		light->params[6] = cosf(light->params[8] * MATH_DEG2RAD);

		const float cosAngle = light->params[6];
		const float tanAngle = sqrtf(1.0f - cosAngle * cosAngle) / cosAngle;

		light->params[10] = light->radius * cosAngle;
		light->params[9] = light->params[10] * tanAngle;

		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_HEADLIGHT", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		float offset = ctx.GetArgument<float>(1);
		float split = ctx.GetArgument<float>(2);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->params[11] = offset;
		light->params[12] = split;

		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_AO", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		float x = ctx.GetArgument<float>(1);
		float y = ctx.GetArgument<float>(2);
		float z = ctx.GetArgument<float>(3);
		float intensity = ctx.GetArgument<float>(4);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->params[5] = x;
		light->params[6] = y;
		light->params[7] = z;
		light->params[8] = intensity;

		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_CAPSULE_SIZE", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		float length = ctx.GetArgument<float>(1);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->params[5] = length;

		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_VOLUME_DETAILS", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		float volIntensity = ctx.GetArgument<float>(1);
		float volSizeScale = ctx.GetArgument<float>(2);
		float r = ctx.GetArgument<float>(3);
		float g = ctx.GetArgument<float>(4);
		float b = ctx.GetArgument<float>(5);
		float i = ctx.GetArgument<float>(6);
		float outerExponent = ctx.GetArgument<float>(7);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->volIntensity = volIntensity;
		light->volSizeScale = volSizeScale;
		light->volColorIntensityOuter = {r, b, g, i};
		light->volOuterExponent = outerExponent;

		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_PLANE", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		float nx = ctx.GetArgument<float>(1);
		float ny = ctx.GetArgument<float>(2);
		float nz = ctx.GetArgument<float>(3);
		float dist = ctx.GetArgument<float>(4);

		float mtx[16];
		for (int i = 0; i < 16; ++i)
		{
			mtx[i] = ctx.GetArgument<float>(5 + i);
		}

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		DirectX::XMVECTOR normal = DirectX::XMVectorSet(nx, ny, nz, 0.0f);
		DirectX::XMMATRIX matrix = DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(mtx));
		DirectX::XMVECTOR transformedNormal = DirectX::XMVector3TransformNormal(normal, matrix);
		DirectX::XMFLOAT3 lightPosFloat3(light->coords.x, light->coords.y, light->coords.z);
		DirectX::XMVECTOR lightPos = DirectX::XMLoadFloat3(&lightPosFloat3);
		DirectX::XMVECTOR planeOrigin = DirectX::XMVectorSubtract(lightPos, DirectX::XMVectorScale(transformedNormal, dist));

		light->params[1] = DirectX::XMVectorGetX(transformedNormal);
		light->params[2] = DirectX::XMVectorGetY(transformedNormal);
		light->params[3] = DirectX::XMVectorGetZ(transformedNormal);
		light->params[4] = -DirectX::XMVectorGetX(DirectX::XMVector3Dot(planeOrigin, transformedNormal));

		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_FLAGS", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		uint32_t flags = ctx.GetArgument<uint32_t>(1);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->flags = flags;

		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_SHADOW_DETAILS", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		int32_t shadowId = ctx.GetArgument<int32_t>(1);
		float shadowNearClip = ctx.GetArgument<float>(2);
		float shadowBlur = ctx.GetArgument<float>(3);
		float shadowDepthBiasScale = ctx.GetArgument<float>(4);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->shadowTrackingID = shadowId;
		light->shadowNearClip = shadowNearClip;
		light->shadowBlur = shadowBlur;
		light->shadowDepthBiasScale = shadowDepthBiasScale;

		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_CLIP_RECT", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		int clipX = ctx.GetArgument<int>(1);
		int clipY = ctx.GetArgument<int>(2);
		int clipWidth = ctx.GetArgument<int>(3);
		int clipHeight = ctx.GetArgument<int>(4);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->clipRectX = clipX;
		light->clipRectY = clipY;
		light->clipRectWidth = clipWidth;
		light->clipRectHeight = clipHeight;

		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_FADE_DISTANCE", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		uint8_t lightFade = ctx.GetArgument<uint8_t>(1);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->fadeDistance = lightFade;

		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_SHADOW_FADE_DISTANCE", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		uint8_t shadowFade = ctx.GetArgument<uint8_t>(1);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->shadowFadeDistance = shadowFade;

		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_SPECULAR_FADE_DISTANCE", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		uint8_t specularFade = ctx.GetArgument<uint8_t>(1);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->specularFadeDistance = specularFade;

		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_VOLUMETRIC_FADE_DISTANCE", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		uint8_t volumetricFade = ctx.GetArgument<uint8_t>(1);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->volumetricFadeDistance = volumetricFade;

		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_INTERIOR", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		uint16_t interior = ctx.GetArgument<uint8_t>(1);
		bool isPortal = ctx.GetArgument<uint8_t>(2);
		uint16_t roomIndex = ctx.GetArgument<uint8_t>(3);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		rage::fwInteriorLocation loc{ interior, isPortal, roomIndex };
		light->interior = loc;

		return ctx.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_LIGHT_ALPHA", [](fx::ScriptContext& ctx)
	{
		uint32_t handle = ctx.GetArgument<uint32_t>(0);
		float alpha = ctx.GetArgument<float>(1);

		CLight* light = lightManager.GetLight(handle);
		if (light == nullptr)
		{
			return ctx.SetResult<bool>(false);
		}

		light->alpha = alpha;

		return ctx.SetResult<bool>(true);
	});

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		lightManager.Reset();
	});
});

static HookFunction hookFunction([]
{
	{
		auto location = hook::get_pattern("48 89 5C 24 18 48 89 74 24 20 55 57 41 55 41 56 41 57 48 8D AC");
		g_everyFrame = hook::trampoline(location, &Scripts_everyFrame);
	}
});

