#include <StdInc.h>
#include <ScriptEngine.h>
#include <Hooking.h>
#include <EntitySystem.h>
#include <DirectXMath.h>
#include <scrEngine.h>

using Matrix4x4 = DirectX::XMFLOAT4X4;

static hook::cdecl_stub<void(void*, uint32_t, Matrix4x4* readMatrix, char)> GetPedBoneTransform([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 0F C2 F7 00 0F 50 C6 83 E0"));
});

static hook::cdecl_stub<void*(rage::fwEntity*)> GetPedCreatureComponent([]()
{
	return hook::get_call(hook::get_pattern("D1 0F 14 FA E8 ? ? ? ? 48 8B C8 4C 8D 44", 4));
});

static HookFunction hookFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_PED_BONE_MATRIX", [](fx::ScriptContext& context)
	{
		rage::fwEntity* ped = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));
		uint32_t boneId = context.GetArgument<int>(1);

		if (ped && boneId)
		{

			void* g_pedCreatureComponent = GetPedCreatureComponent(ped);

			if (g_pedCreatureComponent)
			{

				Matrix4x4 readMatrix;
				GetPedBoneTransform(g_pedCreatureComponent, boneId, &readMatrix, (char)0);

				scrVector* forwardVector = context.GetArgument<scrVector*>(2);
				scrVector* rightVector = context.GetArgument<scrVector*>(3);
				scrVector* upVector = context.GetArgument<scrVector*>(4);
				scrVector* atVector = context.GetArgument<scrVector*>(5);

				auto copyVector = [](const float* in, scrVector* out)
				{
					out->x = in[0];
					out->y = in[1];
					out->z = in[2];
				};

				copyVector(readMatrix.m[0], forwardVector);
				copyVector(readMatrix.m[1], rightVector);
				copyVector(readMatrix.m[2], upVector);
				copyVector(readMatrix.m[3], atVector);
			}
		}
	});
});
