/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ScriptEngine.h>

#include <ResourceManager.h>
#include <ResourceScriptingComponent.h>

#include <tbb/concurrent_queue.h>

static fx::OMPtr<IScriptRefRuntime> ValidateAndLookUpRef(const std::string& refString, int32_t* refIdx)
{
	// parse the ref string into its components
	int colonIndex = refString.find_first_of(':');
	int colonIndexEnd = refString.find_first_of(':', colonIndex + 1);

	std::string resourceName = refString.substr(0, colonIndex);
	int instanceId = atoi(refString.substr(colonIndex + 1, colonIndexEnd - colonIndex).c_str());
	int refId = atoi(refString.substr(colonIndexEnd + 1).c_str());

	// get the resource manager and find stuff in it
	fx::ResourceManager* manager = fx::ResourceManager::GetCurrent(false);

	// if there's a resource by that name...
	fwRefContainer<fx::Resource> resource = manager->GetResource(resourceName);

	if (!resource.GetRef())
	{
		return nullptr;
	}

	// ... and it has a scripting component...
	fwRefContainer<fx::ResourceScriptingComponent> scriptingComponent = resource->GetComponent<fx::ResourceScriptingComponent>();

	if (!scriptingComponent.GetRef())
	{
		return nullptr;
	}

	// ... and there's an instance by this instance ID...
	fx::OMPtr<IScriptRuntime> runtime = scriptingComponent->GetRuntimeById(instanceId);

	if (!runtime.GetRef())
	{
		return nullptr;
	}

	// ... check if it's a RefRuntime and return it.
	fx::OMPtr<IScriptRefRuntime> refRuntime;

	if (FX_FAILED(runtime.As(&refRuntime)))
	{
		return nullptr;
	}

	*refIdx = refId;

	return refRuntime;
}

static InitFunction initFunction([] ()
{
	fx::ResourceManager::SetCallRefCallback([](const std::string& ref, const std::string& argumentData)
	{
		int32_t refId;
		fx::OMPtr<IScriptRefRuntime> refRuntime = ValidateAndLookUpRef(ref, &refId);

		if (refRuntime.GetRef())
		{
			char* retvalData;
			uint32_t retvalSize;

			if (FX_SUCCEEDED(refRuntime->CallRef(refId, const_cast<char*>(argumentData.c_str()), argumentData.size(), &retvalData, &retvalSize)))
			{
				return std::string(retvalData, retvalSize);
			}
		}

		return std::string();
	});

	fx::ScriptEngine::RegisterNativeHandler("INVOKE_FUNCTION_REFERENCE", [] (fx::ScriptContext& context)
	{
		int32_t refId;
		fx::OMPtr<IScriptRefRuntime> refRuntime = ValidateAndLookUpRef(context.GetArgument<const char*>(0), &refId);

		if (refRuntime.GetRef())
		{
			char* retvalData;

			refRuntime->CallRef(refId, context.GetArgument<char*>(1), context.GetArgument<uint32_t>(2), &retvalData, context.GetArgument<uint32_t*>(3));

			context.SetResult(retvalData);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("DUPLICATE_FUNCTION_REFERENCE", [] (fx::ScriptContext& context)
	{
		int32_t refId;
		fx::OMPtr<IScriptRefRuntime> refRuntime = ValidateAndLookUpRef(context.GetArgument<const char*>(0), &refId);

		if (refRuntime.GetRef())
		{
			int32_t newRefId;
			refRuntime->DuplicateRef(refId, &newRefId);

			fx::OMPtr<IScriptRuntime> runtime;
			refRuntime.As(&runtime);

			context.SetResult(va("%s:%d:%d", reinterpret_cast<fx::Resource*>(runtime->GetParentObject())->GetName().c_str(), runtime->GetInstanceId(), newRefId));
		}
	});

	static tbb::concurrent_queue<std::string> deleteRefs;

	static auto deleteRef = [](const std::string& ref)
	{
		int32_t refId;
		fx::OMPtr<IScriptRefRuntime> refRuntime = ValidateAndLookUpRef(ref, &refId);

		if (refRuntime.GetRef())
		{
			refRuntime->RemoveRef(refId);
		}
	};

	fx::ScriptEngine::RegisterNativeHandler("DELETE_FUNCTION_REFERENCE", [] (fx::ScriptContext& context)
	{
		std::string ref = context.GetArgument<const char*>(0);

		try
		{
			deleteRef(ref);
		}
		catch (std::runtime_error& error)
		{
			deleteRefs.push(std::move(ref));
		}
	});

	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* manager)
	{
		manager->OnTick.Connect([]()
		{
			std::string str;

			while (deleteRefs.try_pop(str))
			{
				deleteRef(str);
			}
		});
	});
});
