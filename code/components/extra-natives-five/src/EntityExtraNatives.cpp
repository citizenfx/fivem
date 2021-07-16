#include <StdInc.h>

#include <ScriptEngine.h>
#include <scrBind.h>

#include <Streaming.h>

static std::string GetEntityArchetypeName(int entityHandle)
{
	if (auto entity = rage::fwScriptGuid::GetBaseFromGuid(entityHandle))
	{
		return streaming::GetStreamingBaseNameForHash(entity->GetArchetype()->hash);
	}

	return "";
}

static InitFunction initFunction([]()
{
	scrBindGlobal("GET_ENTITY_ARCHETYPE_NAME", &GetEntityArchetypeName);
});
