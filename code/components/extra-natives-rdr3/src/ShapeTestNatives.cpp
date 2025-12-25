#include <StdInc.h>
#include <ScriptEngine.h>

#include <Hooking.h>
#include <scrEngine.h>
#include <EntitySystem.h>
#include <ScriptSerialization.h>

enum NativeIdentifiers : uint64_t
{
	GET_SHAPE_TEST_RESULT = 0xEDE8AC7C5108FB1D
};

struct ShapeTestResult
{
	uint32_t shapeTestHandle;
	bool hit;
	scrVector endCoords;
	scrVector surfaceNormal;
	uint64_t entityHit;
};

class ResultData
{
public:
	char pad_0000[108]; // 0x0000
	uint8_t materialIndex; // 0x006C
}; // Size: 0x006D

class CScriptShapeTestResult
{
public:
	char pad_0000[36]; // 0x0000
	uint32_t state; // 0x0024
	char pad_0028[8]; // 0x0028
	ResultData* data; // 0x0030
}; // Size: 0x0038

template<std::size_t Index, typename ReturnType, typename... Args>
inline ReturnType call_virtual(void* instance, Args... args)
{
	using Fn = ReturnType(__thiscall*)(void*, Args...);

	auto function = (*reinterpret_cast<Fn**>(instance))[Index];
	return function(instance, args...);
}

static hook::cdecl_stub<CScriptShapeTestResult*(uint64_t)> g_getCScriptShapeTestResult([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 4C 8B C0 48 85 C0 75 04"));
});

// This information was previously returned into the native, but this is not the case for RDR3
// we need to grab it from the trace result.
uint8_t GetMateralIndexFromTraceHandle(uint64_t rayHandle)
{
	CScriptShapeTestResult* shapeResult = g_getCScriptShapeTestResult(rayHandle);

	if (!shapeResult || !shapeResult->data)
	{
		return 0;
	}

	return shapeResult->data->materialIndex;
}

static HookFunction hookFunction([]()
{
	static void** phMaterialMgr = hook::get_address<void**>(hook::get_pattern("48 8B CB 0F 28 CA F3 0F 59", 22));

	fx::ScriptEngine::RegisterNativeHandler("GET_SHAPE_TEST_RESULT_INCLUDING_MATERIAL", [](fx::ScriptContext& context)
	{
		auto copyVector = [](scrVector* out, scrVector& in)
		{
			out->x = in.x;
			out->y = in.y;
			out->z = in.z;
		};

		ShapeTestResult shapeTestResult = {};
		shapeTestResult.shapeTestHandle = context.GetArgument<uint32_t>(0);

		// Fetch material result before invoke native as it cleans up the shapetest handle
		uint8_t materialIndex = GetMateralIndexFromTraceHandle(shapeTestResult.shapeTestHandle);
		uint32_t status = NativeInvoke::Invoke<GET_SHAPE_TEST_RESULT, uint32_t>(shapeTestResult.shapeTestHandle, &shapeTestResult.hit, &shapeTestResult.endCoords, &shapeTestResult.surfaceNormal, &shapeTestResult.entityHit);

		// Unless the return value is 2, the other return values are undefined.
		if (status != 2)
		{
			return context.SetResult(status);
		}

		char data[128];
		call_virtual<8, uint32_t>(*phMaterialMgr, materialIndex, data, sizeof(data));

		*context.GetArgument<bool*>(1) = shapeTestResult.hit;
		copyVector(context.GetArgument<scrVector*>(2), shapeTestResult.endCoords);
		copyVector(context.GetArgument<scrVector*>(3), shapeTestResult.surfaceNormal);
		*context.GetArgument<uint32_t*>(4) = HashString(data);
		*context.GetArgument<uint32_t*>(5) = shapeTestResult.entityHit;
		context.SetResult<int>(status);
	});
});
