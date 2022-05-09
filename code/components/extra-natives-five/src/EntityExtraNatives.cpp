#include <StdInc.h>

#include <ScriptEngine.h>
#include <scrBind.h>

#include <Streaming.h>

static const char* GetEntityArchetypeName(int entityHandle)
{
	if (auto entity = rage::fwScriptGuid::GetBaseFromGuid(entityHandle))
	{
		static std::string lastReturn;
		lastReturn = streaming::GetStreamingBaseNameForHash(entity->GetArchetype()->hash);

		return lastReturn.c_str();
	}

	return "";
}

static InitFunction initFunction([]()
{
	scrBindGlobal("GET_ENTITY_ARCHETYPE_NAME", &GetEntityArchetypeName);

	fx::ScriptEngine::RegisterNativeHandler("IS_ENTITY_POSITION_FROZEN", [](fx::ScriptContext& context)
	{
		bool result = false;
		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));

		if (entity)
		{
			auto address = (char*)entity;
			DWORD flag = *(DWORD*)(address + 0x2E);
			result = flag & (1 << 1);
		}

		context.SetResult<bool>(result);
	});

	/* original setter is SET_ENTITY_ONLY_DAMAGED_BY_RELATIONSHIP_GROUP, perhaps this is for entities too? */
	fx::ScriptEngine::RegisterNativeHandler("CAN_ENTITY_ONLY_BE_DAMAGED_BY_A_RELATIONSHIP_GROUP", [](fx::ScriptContext& context)
	{
		bool result = false;
		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));

		if (entity)
		{
			auto address = (char*)entity;
			DWORD flag = *(DWORD*)(address + 0x188);
			result = (flag & (1 << 12)) != 0;

			if (auto relationshipHashPtr = context.GetArgument<int*>(1))
				*relationshipHashPtr = *reinterpret_cast<int*>(address + 0x194);
		}

		context.SetResult<bool>(result);
	});
});
