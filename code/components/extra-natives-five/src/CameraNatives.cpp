#include <StdInc.h>
#include <Pool.h>

#include <ScriptEngine.h>

#include <DirectXMath.h>

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

static InitFunction initFunction([]()
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
