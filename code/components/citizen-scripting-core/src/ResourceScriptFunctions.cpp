/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ScriptEngine.h>

#include <Resource.h>
#include <ResourceManager.h>
#include <fxScripting.h>

static InitFunction initFunction([] ()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_CURRENT_RESOURCE_NAME", [] (fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;
		
		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			if (resource)
			{
				context.SetResult(resource->GetName().c_str());
				return;
			}
		}

		context.SetResult(nullptr);
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_DUPLICITY_VERSION", [](fx::ScriptContext& context)
	{
		context.SetResult(
#ifdef IS_FXSERVER
			true
#else
			false
#endif
		);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INSTANCE_ID", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			context.SetResult(runtime->GetInstanceId());
		}
		else
		{
			context.SetResult(0);
		}
	});

	static std::vector<fx::Resource*> resources;

	fx::ScriptEngine::RegisterNativeHandler("GET_NUM_RESOURCES", [](fx::ScriptContext& context)
	{
		resources.clear();

		auto manager = fx::ResourceManager::GetCurrent();
		manager->ForAllResources([&] (fwRefContainer<fx::Resource> resource)
		{
			resources.push_back(resource.GetRef());
		});

		context.SetResult(resources.size());
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_RESOURCE_BY_FIND_INDEX", [](fx::ScriptContext& context)
	{
		int i = context.GetArgument<int>(0);
		if (i < 0 || i >= resources.size())
		{
			context.SetResult(nullptr);
			return;
		}

		context.SetResult(resources[i]->GetName().c_str());
	});
});
