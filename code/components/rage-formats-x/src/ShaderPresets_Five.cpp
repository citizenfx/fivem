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

#include <Error.h>

using namespace rage::five;

void grmShaderFx::DoPreset(const char* shaderName, const char* spsName)
{
	wchar_t shaderNameWide[256];
	mbstowcs(shaderNameWide, shaderName, _countof(shaderNameWide));

	auto shaderFile = fxc::ShaderFile::Load(MakeRelativeCitPath(va(L"citizen\\shaders\\win32_40_final\\%s.fxc", shaderNameWide)));

	if (!shaderFile)
	{
		FatalError("Could not find %s.fxc!", shaderName);
	}

	//
	grmShaderFx* shader = this;
	shader->SetSpsName(spsName);
	shader->SetShaderName(shaderName);

	// override the draw bucket if needed
	auto drawBucket = shaderFile->GetGlobalValue("__rage_drawbucket");

	shader->SetDrawBucket(atoi(drawBucket.get_value_or("0").c_str()));
	
	// set parameters
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

	// pre-sort samplers based on register index (certain third-party tools want the samplers to be in register order, not .fxc order)
	std::map<int, int> samplerIndexMapping;

	{
		std::vector<std::pair<int, int>> samplerIndexList;

		int i = 0;

		for (auto& pair : localParameters)
		{
			std::shared_ptr<fxc::ShaderParameter> parameter = pair.second;

			if (parameter->IsSampler())
			{
				samplerIndexList.push_back({ i, parameter->GetRegister() });

				i++;
			}
		}

		std::sort(samplerIndexList.begin(), samplerIndexList.end(), [] (const auto& left, const auto& right)
		{
			return (left.second < right.second);
		});

		i = 0;

		for (auto& pair : samplerIndexList)
		{
			samplerIndexMapping.insert({ pair.first, i });

			i++;
		}
	}
	
	// apply parameters
	for (auto& pair : localParameters)
	{
		std::shared_ptr<fxc::ShaderParameter> parameter = pair.second;

		int idx = (parameter->IsSampler()) ? samplerIdx++ : nonSamplerIdx++;

		if (parameter->IsSampler())
		{
			idx = samplerIndexMapping[idx];
		}

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