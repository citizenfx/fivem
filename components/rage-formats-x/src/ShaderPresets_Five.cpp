/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#include <rmcDrawable.h>

#include <ShaderInfo.h>

using namespace rage::five;

void grmShaderFx::DoPreset(const char* shaderName, const char* spsName)
{
	auto shaderFile = fxc::ShaderFile::Load(va("Y:\\common\\shaders\\win32_40_final\\%s.fxc", shaderName));

	//
	grmShaderFx* shader = this;
	shader->SetSpsName(spsName);
	shader->SetShaderName(shaderName);
	shader->SetDrawBucket(0); // TODO: read draw bucket from .fxc

	auto& localParameters = shaderFile->GetLocalParameters();

	std::vector<grmShaderParameterMeta> parameters(localParameters.size());
	std::vector<uint32_t> parameterNames(localParameters.size());
	std::vector<std::vector<uint8_t>> parameterValues(localParameters.size());

	// count the samplers in the parameter list
	auto numSamplers = std::count_if(localParameters.begin(), localParameters.end(), [] (const decltype(*localParameters.begin())& value) // weird workaround to the lack of auto arguments in VS12
	{
		return value.second->IsSampler();
	});

	int samplerIdx = 0;
	int nonSamplerIdx = numSamplers;

	for (auto& pair : localParameters)
	{
		std::shared_ptr<fxc::ShaderParameter> parameter = pair.second;

		int idx = (parameter->IsSampler()) ? samplerIdx++ : nonSamplerIdx++;

		parameterNames[idx] = parameter->GetNameHash();

		if (!parameter->IsSampler())
		{
			parameterValues[idx] = parameter->GetDefaultValue();
			parameters[idx].registerIdx = shaderFile->MapRegister(parameter->GetRegister());
		}
		else
		{
			// TODO: set as sampler reference
			parameters[idx].registerIdx = parameter->GetRegister();
		}

		parameters[idx].isSampler = parameter->IsSampler();
	}

	shader->SetParameters(parameters, parameterNames, parameterValues);
}

namespace rage
{
	FORMATS_EXPORT int g_curGeom;
	FORMATS_EXPORT std::map<int, void*> g_vbMapping;
	FORMATS_EXPORT std::map<int, void*> g_ibMapping;
}