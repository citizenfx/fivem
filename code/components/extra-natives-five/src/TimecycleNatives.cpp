#include <StdInc.h>
#include <ScriptEngine.h>
#include <Timecycle.h>

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("ACTIVATE_TIMECYCLE_EDITOR", [=](fx::ScriptContext& context)
	{
		TheTimecycleManager->SetActivateEditor(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TIMECYCLE_MODIFIER_STRENGTH", [=](fx::ScriptContext& context)
	{
		float result = 0.0f;

		if (const auto scriptData = TimecycleManager::GetScriptData())
		{
			result = scriptData->m_primaryModifierStrength;
		}

		context.SetResult<float>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TIMECYCLE_MODIFIER_NAME_BY_INDEX", [=](fx::ScriptContext& context)
	{
		const char* result = "";

		auto index = context.GetArgument<int>(0);

		if (const auto tc = TheTimecycleManager->GetTimecycleByIndex(index))
		{
			result = TheTimecycleManager->GetTimecycleName(*tc).c_str();
		}

		context.SetResult<const char*>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TIMECYCLE_MODIFIER_INDEX_BY_NAME", [=](fx::ScriptContext& context)
	{
		int result = -1;

		auto name = context.CheckArgument<const char*>(0);

		if (const auto tc = TheTimecycleManager->GetTimecycle(HashString(name)))
		{
			result = TheTimecycleManager->GetTimecycleIndex(*tc);
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TIMECYCLE_MODIFIER_COUNT", [=](fx::ScriptContext& context)
	{
		int result = 0;

		if (const auto tcManager = TimecycleManager::GetGameManager())
		{
			result = tcManager->m_modifiers.GetCount();
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TIMECYCLE_VAR_COUNT", [=](fx::ScriptContext& context)
	{
		context.SetResult<int>(TimecycleManager::GetConfigVarInfoCount());
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TIMECYCLE_VAR_NAME_BY_INDEX", [=](fx::ScriptContext& context)
	{
		const char* result = "";

		auto index = context.GetArgument<int>(0);
		
		if (const auto varInfo = TimecycleManager::GetConfigVarInfo(index))
		{
			result = TimecycleManager::GetVarInfoName(*varInfo);
		}

		context.SetResult<const char*>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TIMECYCLE_VAR_DEFAULT_VALUE_BY_INDEX", [=](fx::ScriptContext& context)
	{
		float result = 0.0f;

		auto index = context.GetArgument<int>(0);

		if (const auto varInfo = TimecycleManager::GetConfigVarInfo(index))
		{
			result = varInfo->m_value;
		}

		context.SetResult<float>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TIMECYCLE_MODIFIER_VAR_COUNT", [=](fx::ScriptContext& context)
	{
		int result = 0;

		auto tcName = context.CheckArgument<const char*>(0);

		if (const auto tc = TheTimecycleManager->GetTimecycle(tcName))
		{
			result = tc->m_modData.GetCount();
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TIMECYCLE_MODIFIER_VAR_NAME_BY_INDEX", [=](fx::ScriptContext& context)
	{
		const char* result = "";

		auto tcName = context.CheckArgument<const char*>(0);
		auto varIndex = context.GetArgument<int>(1);

		if (const auto tc = TheTimecycleManager->GetTimecycle(tcName))
		{
			if (const auto modData = TheTimecycleManager->GetTimecycleModDataByIndex(*tc, varIndex))
			{
				if (const auto varInfo = TimecycleManager::GetConfigVarInfo(modData->m_index))
				{
					result = TimecycleManager::GetVarInfoName(*varInfo);
				}
			}
		}

		context.SetResult<const char*>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TIMECYCLE_MODIFIER_VAR", [=](fx::ScriptContext& context)
	{
		bool result = false;

		auto tcName = context.CheckArgument<const char*>(0);
		auto varName = context.GetArgument<const char*>(1);

		if (const auto tc = TheTimecycleManager->GetTimecycle(tcName))
		{
			if (const auto modData = TheTimecycleManager->GetTimecycleModData(*tc, varName))
			{
				*context.GetArgument<float*>(2) = modData->m_value1;
				*context.GetArgument<float*>(3) = modData->m_value2;
				result = true;
			}
		}

		context.SetResult<bool>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_TIMECYCLE_MODIFIER_VAR", [=](fx::ScriptContext& context)
	{
		auto tcName = context.CheckArgument<const char*>(0);
		auto varName = context.CheckArgument<const char*>(1);
		auto value1 = context.GetArgument<float>(2);
		auto value2 = context.GetArgument<float>(3);

		if (const auto tc = TheTimecycleManager->GetTimecycle(tcName))
		{
			if (!TheTimecycleManager->DoesTimecycleHasModData(*tc, varName))
			{
				TheTimecycleManager->AddTimecycleModData(*tc, varName);
			}

			TheTimecycleManager->SetTimecycleModData(*tc, std::string(varName), value1, value2);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("DOES_TIMECYCLE_MODIFIER_HAS_VAR", [=](fx::ScriptContext& context)
	{
		bool result = false;

		auto tcName = context.CheckArgument<const char*>(0);
		auto varName = context.CheckArgument<const char*>(1);

		if (const auto tc = TheTimecycleManager->GetTimecycle(tcName))
		{
			result = TheTimecycleManager->DoesTimecycleHasModData(*tc, varName);
		}

		context.SetResult<bool>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("REMOVE_TIMECYCLE_MODIFIER_VAR", [=](fx::ScriptContext& context)
	{
		auto tcName = context.CheckArgument<const char*>(0);
		auto varName = context.CheckArgument<const char*>(1);

		if (const auto tc = TheTimecycleManager->GetTimecycle(tcName))
		{
			if (TheTimecycleManager->DoesTimecycleHasModData(*tc, varName))
			{
				TheTimecycleManager->RemoveTimecycleModData(*tc, varName);
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("CREATE_TIMECYCLE_MODIFIER", [=](fx::ScriptContext& context)
	{
		int result = -1;

		auto tcName = context.CheckArgument<const char*>(0);
		auto nameLength = strlen(tcName);
		
		if (nameLength >= 2 && nameLength <= 128)
		{
			if (!TimecycleManager::HasTimecycleWithName(tcName))
			{
				if (const auto tc = TheTimecycleManager->CreateTimecycle(tcName))
				{
					result = TheTimecycleManager->GetTimecycleIndex(*tc);
				}
			}
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("REMOVE_TIMECYCLE_MODIFIER", [=](fx::ScriptContext& context)
	{
		auto tcName = context.CheckArgument<const char*>(0);

		if (const auto tc = TheTimecycleManager->GetTimecycle(tcName))
		{
			TheTimecycleManager->RemoveTimecycle(*tc);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("CLONE_TIMECYCLE_MODIFIER", [=](fx::ScriptContext& context)
	{
		int result = -1;

		auto origName = context.CheckArgument<const char*>(0);
		auto cloneName = context.CheckArgument<const char*>(1);
		auto nameLength = strlen(cloneName);

		if (nameLength >= 2 && nameLength <= 128)
		{
			if (const auto source = TheTimecycleManager->GetTimecycle(origName))
			{
				if (const auto clone = TheTimecycleManager->CloneTimecycle(*source, cloneName))
				{
					result = TheTimecycleManager->GetTimecycleIndex(*clone);
				}
			}
		}

		context.SetResult<int>(result);
	});
});
