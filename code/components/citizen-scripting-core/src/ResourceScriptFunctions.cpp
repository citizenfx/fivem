/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ScriptEngine.h>

#include <Resource.h>
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
});