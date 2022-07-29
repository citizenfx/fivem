/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <ScriptEngine.h>
#include <Hooking.h>
#include <MinHook.h>
#include <Pool.h>
#include <DirectXMath.h>
#include <scrEngine.h>

using Matrix4x4 = DirectX::XMFLOAT4X4;

struct CameraData
{
	char pad_0000[32]; //0x0000
	Matrix4x4 m_matrix1; //0x0020
	char pad_0060[144]; //0x0060
	Matrix4x4 m_matrix2; //0x00F0
	char pad_0130[900]; //0x0130
	uint8_t m_currentMatrix : 1;
};

static HookFunction hookFunction([]()
{
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
});
