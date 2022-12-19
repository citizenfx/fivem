#include <StdInc.h>
#include <Pool.h>

#include <ScriptEngine.h>

#include <DirectXMath.h>

#include <Hooking.h>
#include <CoreConsole.h>
#include <Resource.h>
#include <fxScripting.h>

#include <nutsnbolts.h>
#include <CustomText.h>

#include <RageParser.h>

static hook::cdecl_stub<void* (const uint32_t& hash, void* typeAssert)> _getCameraMetadata([]()
{
	return hook::get_call(hook::get_pattern("C7 44 24 48 63 1C 9E D7 E8", 8));
});

static std::shared_ptr<ConVar<bool>> g_handbrakeCamConvar;

struct camFollowVehicleCameraMetadataHandBrakeSwingSettings
{
	void* vtbl;
	uint32_t HandBrakeInputEnvelopeRef;
	float SpringConstant;
	float MinLateralSkidSpeed;
	float MaxLateralSkidSpeed;
	float SwingSpeedAtMaxSkidSpeed;
};

static void* g_currentCamMetadata;
static camFollowVehicleCameraMetadataHandBrakeSwingSettings g_lastSettings;
static bool g_lastMetadataState;

static ptrdiff_t GetMetadataOffset(const char* structName, const char* memberName)
{
	auto tcurts = rage::GetStructureDefinition(structName);
	for (auto& member : tcurts->m_members)
	{
		if (member->m_definition->hash == HashRageString(memberName))
		{
			return member->m_definition->offset;
		}
	}

	return 0;
}

static auto GetRelativeHandbrakeMetadata(void* base)
{
	static ptrdiff_t offsetRef = GetMetadataOffset("camFollowVehicleCameraMetadata", "HandBrakeSwingSettings");

	return (camFollowVehicleCameraMetadataHandBrakeSwingSettings*)((char*)base + offsetRef);
}

static auto GetRelativeFirstPersonCamMetadata(void* base)
{
	static ptrdiff_t offsetRef = GetMetadataOffset("camCinematicMountedCameraMetadata", "FirstPersonCamera");

	return (bool*)((char*)base + offsetRef);
}

static void OverrideHandbrakeMetadata(void* metadata, bool overridden)
{
	if (g_lastMetadataState != overridden)
	{
		auto camhb = GetRelativeHandbrakeMetadata(metadata);

		if (overridden)
		{
			g_lastSettings = *camhb;

			camhb->SpringConstant = 0.f;
			camhb->MaxLateralSkidSpeed = 0.f;
			camhb->MinLateralSkidSpeed = 0.f;
			camhb->SwingSpeedAtMaxSkidSpeed = 0.f;
		}
		else
		{
			*camhb = g_lastSettings;
		}

		g_lastMetadataState = overridden;
	}
}

static void* GetCameraMetadataWrap(const uint32_t& hash, void* typeAssert)
{
	auto metadata = _getCameraMetadata(hash, typeAssert);

	if (g_currentCamMetadata)
	{
		OverrideHandbrakeMetadata(g_currentCamMetadata, false);
	}

	g_currentCamMetadata = metadata;

	if (!g_handbrakeCamConvar->GetValue())
	{
		OverrideHandbrakeMetadata(g_currentCamMetadata, true);
	}

	return metadata;
}

static void UpdateCameraMetadataRef()
{
	if (g_currentCamMetadata)
	{
		auto curValue = !(g_handbrakeCamConvar->GetValue());

		if (curValue != g_lastMetadataState)
		{
			OverrideHandbrakeMetadata(g_currentCamMetadata, curValue);
		}
	}
}

using Matrix4x4 = DirectX::XMFLOAT4X4;

struct CameraData
{
	uint8_t m_pad[48];
	Matrix4x4 m_matrix1;
	char m_pad2[160];
	Matrix4x4 m_matrix2;
	char m_pad3[208];
	uint8_t m_currentMatrix : 1;
};

struct scrVector
{
	float x;
	int _pad;
	float y;
	int _pad2;
	float z;
	int _pad3;
};

struct camBaseObject
{
	virtual void DESTROY() = 0;
	virtual bool IsHashTypeOf(int hash) = 0;
	virtual int GetCamHash() = 0;
	virtual int GetHash2() = 0;   // Added in 2189, not unique, maybe a category
	virtual bool CanUpdate() = 0; // Checks realtime - lastUpdatedTime > metadata.TimeoutMS
	virtual void sub_1402D5D94() = 0;
	virtual void UpdateCameraState() = 0;
	virtual void sub_14026DF58() = 0;
	virtual void sub_1402E8C64() = 0;
	virtual void sub_14030443C() = 0;
	virtual bool ReturnMetadataBool() = 0;
};

typedef bool (*camCanUpdateFn)(camBaseObject* thisptr);
static camCanUpdateFn origCamCinematicOnFootIdleContext_CanUpdate;

static std::atomic_bool g_DISABLE_IDLE_CAM = false;

static bool CamCinematicOnFootIdleContext_CanUpdate(camBaseObject* thisptr)
{
	if (!g_DISABLE_IDLE_CAM)
	{
		return origCamCinematicOnFootIdleContext_CanUpdate(thisptr);
	}
	return false;
}

struct camCinematicMountedCamera
{
	char pad[144];
	float fov;
	char pad2[412];
	void* metadata;
};

static ConVar<float>* g_customVehicleFPSFov;

static void (*g_origPostMountedCinematicCam)(camCinematicMountedCamera* camera, float a2);
static int* g_fovScaleProfileSetting;

static void PostMountedCinematicCam(camCinematicMountedCamera* camera, float a2)
{
	// if this is a first-person camera, scale the FOV by the profile setting
	if (*GetRelativeFirstPersonCamMetadata(camera->metadata))
	{
		auto oldFOV = camera->fov;
		auto overrideValue = g_customVehicleFPSFov->GetValue();
		
		// >= 1: raw FOV
		// -1: 'old' default FOV only
		if (overrideValue >= 1.0f)
		{
			camera->fov = overrideValue;
		}
		else if (overrideValue > -1.0f)
		{
			auto minFOV = oldFOV * 0.7f;
			auto fovScale = std::max(*g_fovScaleProfileSetting / 10.f, 0.0f);

			camera->fov = ((oldFOV - minFOV) * fovScale) + minFOV;
		}
	}

	return g_origPostMountedCinematicCam(camera, a2);
}

static HookFunction hookFunction([]()
{
	hook::call(hook::get_pattern("48 8D 4D 20 44 89 75 20 E8 ? ? ? ? 48 85 C0", 8), GetCameraMetadataWrap);

	// for FOV for first-person (POV) vehicle cameras
	{
		auto location = hook::get_pattern("48 8B CB F3 0F 11 83 90 00 00 00 48 83 C4 20 5B", 16);
		hook::set_call(&g_origPostMountedCinematicCam, location);
		hook::jump(location, PostMountedCinematicCam);
	}

	g_fovScaleProfileSetting = hook::get_address<int*>(hook::get_pattern("44 8B 05 ? ? ? ? BA E7 00 00 00 48 8B"), 3, 7);

	// to support DISABLE_IDLE_CAMERA
	uintptr_t* camCinematicOnFootIdleContext_vtable = hook::get_address<uintptr_t*>(hook::get_pattern<unsigned char>("48 8D 05 ? ? ? ? 48 89 07 48 8B C7 F3 0F 10 ? ? ? ? 02 F3 0F 11 47 60", 3));

	// 2189 Added another vfunc between
	// 2802 Repalced RTTI methods in the very beginning
	int index = xbr::IsGameBuildOrGreater<2802>() ? 8 : xbr::IsGameBuildOrGreater<2189>() ? 4 : 3;

	origCamCinematicOnFootIdleContext_CanUpdate = (camCanUpdateFn)camCinematicOnFootIdleContext_vtable[index];
	hook::put(&camCinematicOnFootIdleContext_vtable[index], (uintptr_t)CamCinematicOnFootIdleContext_CanUpdate);
});

static InitFunction initFunction([]()
{
	// original string for american: First Person On Foot Field of View
	game::AddCustomText("MO_FPS_FOV", "First Person Field of View");

	fx::ScriptEngine::RegisterNativeHandler("GET_CAM_MATRIX", [](fx::ScriptContext& scriptContext)
	{
		auto camIndex = scriptContext.GetArgument<int>(0);

		auto camPool = rage::GetPoolBase(0xFE12CE88);
		auto cam = camPool->GetAtHandle<CameraData>(camIndex);

		if (cam != nullptr)
		{
			Matrix4x4* readMatrix;

			// get the right matrix
			if (cam->m_currentMatrix)
			{
				readMatrix = &cam->m_matrix2;
			}
			else
			{
				readMatrix = &cam->m_matrix1;
			}

			// write to output
			scrVector* rightVector = scriptContext.GetArgument<scrVector*>(1);
			scrVector* forwardVector = scriptContext.GetArgument<scrVector*>(2);
			scrVector* upVector = scriptContext.GetArgument<scrVector*>(3);
			scrVector* atVector = scriptContext.GetArgument<scrVector*>(4);

			auto copyVector = [](const float* in, scrVector* out)
			{
				out->x = in[0];
				out->y = in[1];
				out->z = in[2];
			};

			copyVector(readMatrix->m[0], rightVector);
			copyVector(readMatrix->m[1], forwardVector);
			copyVector(readMatrix->m[2], upVector);
			copyVector(readMatrix->m[3], atVector);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("DISABLE_IDLE_CAMERA", [](fx::ScriptContext& context)
	{
		bool value = context.GetArgument<bool>(0);
		g_DISABLE_IDLE_CAM = value;

		fx::OMPtr<IScriptRuntime> runtime;
		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			resource->OnStop.Connect([]()
			{
				g_DISABLE_IDLE_CAM = false;
			});
		}
	});

	g_handbrakeCamConvar = std::make_shared<ConVar<bool>>("cam_enableHandbrakeCamera", ConVar_Archive, true);
	g_customVehicleFPSFov = new ConVar<float>("cam_vehicleFirstPersonFOV", ConVar_Archive, 0.0f);
	g_customVehicleFPSFov->GetHelper()->SetConstraints(-1.0f, 130.0f);

	OnMainGameFrame.Connect([]()
	{
		UpdateCameraMetadataRef();
	});
});
