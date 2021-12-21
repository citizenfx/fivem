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
});
