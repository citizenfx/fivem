#include <StdInc.h>

#include <ScriptEngine.h>
#include <scrBind.h>

#include <Streaming.h>
#include <Hooking.h>

static int PedIdOffset;
static uintptr_t CReplayPedExtensionPtr;

class OverlayEntry
{
public:
	uint32_t collection_hash; // 0x0000
	uint32_t overlay_hash; // 0x0004
	char pad_0008[8]; // 0x0008
	float unk_float; // 0x0010
}; // Size: 0x0014

class PedEntry
{
public:
	char pad_0000[184]; // 0x0000
	OverlayEntry entries[87]; // 0x00B8
	uint32_t overlay_count; // 0x0784
	char pad_0788[80]; // 0x0788
}; // Size: 0x07D8

class CReplayPedExtension
{
public:
	PedEntry entries[1]; // Indexed by a value from CPed-class (0x308)
};

class ptrCReplayPedExtension
{
public:
	CReplayPedExtension* ptr; // 0x0000
}; // Size: 0x0008

struct DecorationResult
{
	DecorationResult(uint32_t m_collection_hash, uint32_t m_overlay_hash)
		: collection_hash(m_collection_hash), overlay_hash(m_overlay_hash)
	{
	}

	uint32_t collection_hash;
	uint32_t overlay_hash;

	MSGPACK_DEFINE_ARRAY(collection_hash, overlay_hash)
};

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

static HookFunction hookFunction([]()
{
	PedIdOffset = *hook::get_pattern<int>("00 00 66 85 C0 78 ? 0F BF C8 0F B7 05 ? ? ? ? 3B C8 7D ? 44 8B C1", -0x2);
	CReplayPedExtensionPtr = hook::get_address<uintptr_t>(hook::get_pattern<uintptr_t>("4C 03 05 ? ? ? ? EB 03 4D 8B C3"), 3, 7);

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_DECORATIONS", [](fx::ScriptContext& context)
	{
		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));
		std::vector<DecorationResult> decorationList;

		if (entity && entity->IsOfType<CPed>())
		{
			uint16_t bufferIndex = *(uint16_t*)((uintptr_t)entity + PedIdOffset);
			ptrCReplayPedExtension* decorationSystem = (ptrCReplayPedExtension*)CReplayPedExtensionPtr;
			if (decorationSystem && decorationSystem->ptr)
			{
				PedEntry* decorationPedEntry = &decorationSystem->ptr->entries[bufferIndex];
				if (decorationPedEntry)
				{
					decorationList.reserve(decorationPedEntry->overlay_count);
					for (int i = 0; i < decorationPedEntry->overlay_count; i++)
					{
						decorationList.emplace_back(decorationPedEntry->entries[i].collection_hash, decorationPedEntry->entries[i].overlay_hash);
					}
				}
			}
		}
		context.SetResult(fx::SerializeObject(decorationList));
	});
});

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
});
