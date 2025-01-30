/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "scrEngine.h"
#include "CrossLibraryInterfaces.h"

#include <CrossBuildRuntime.h>
#include "Hooking.h"

#include <sysAllocator.h>

#include <MinHook.h>
#include <ICoreGameInit.h>

#include <unordered_set>
#include <unordered_map>

static bool storyMode;

#if __has_include("scrEngineStubs.h")
#include <scrEngineStubs.h>
#else
inline void HandlerFilter(void* handler)
{

}
#endif

static std::unordered_map<uint64_t, int> g_nativeBlockedBeforeBuild = {
	// Natives that are banned on all builds.

	{0x9BAE5AD2508DF078, std::numeric_limits<int>::max()}, // SET_INSTANCE_PRIORITY_MODE (prop density lowering)
	{0x5A5F40FE637EB584, std::numeric_limits<int>::max()}, // STRING_TO_INT
	{0xE80492A9AC099A93, std::numeric_limits<int>::max()}, // CLEAR_BIT
	{0x8EF07E15701D61ED, std::numeric_limits<int>::max()}, // SET_BITS_IN_RANGE
	{0x933D6A9EEC1BACD0, std::numeric_limits<int>::max()}, // SET_BIT
	{0x213AEB2B90CBA7AC, std::numeric_limits<int>::max()}, // _COPY_MEMORY

	// DATAFILE namespace

	{0x6CC86E78358D5119, std::numeric_limits<int>::max()}, // DATAFILE_CLEAR_WATCH_LIST
	{0xD27058A1CA2B13EE, std::numeric_limits<int>::max()}, // DATAFILE_CREATE
	{0x9AB9C1CFC8862DFB, std::numeric_limits<int>::max()}, // DATAFILE_DELETE
	{0x8F5EA1C01D65A100, std::numeric_limits<int>::max()}, // DATAFILE_DELETE_REQUESTED_FILE
	{0xC55854C7D7274882, std::numeric_limits<int>::max()}, // DATAFILE_FLUSH_MISSION_HEADER
	{0x906B778CA1DC72B6, std::numeric_limits<int>::max()}, // DATAFILE_GET_FILE_DICT
	{0x15FF52B809DB2353, std::numeric_limits<int>::max()}, // DATAFILE_HAS_LOADED_FILE_DATA
	{0xF8CC1EBE0B62E29F, std::numeric_limits<int>::max()}, // DATAFILE_HAS_VALID_FILE_DATA
	{0xBEDB96A7584AA8CF, std::numeric_limits<int>::max()}, // DATAFILE_IS_SAVE_PENDING
	{0xFCCAE5B92A830878, std::numeric_limits<int>::max()}, // DATAFILE_IS_VALID_REQUEST_ID
	{0xC5238C011AF405E4, std::numeric_limits<int>::max()}, // DATAFILE_LOAD_OFFLINE_UGC
	{0x22DA66936E0FFF37, std::numeric_limits<int>::max()}, // DATAFILE_SELECT_ACTIVE_FILE
	{0x01095C95CD46B624, std::numeric_limits<int>::max()}, // DATAFILE_SELECT_CREATOR_STATS
	{0xA69AC4ADE82B57A4, std::numeric_limits<int>::max()}, // DATAFILE_SELECT_UGC_DATA
	{0x52818819057F2B40, std::numeric_limits<int>::max()}, // DATAFILE_SELECT_UGC_PLAYER_DATA
	{0x9CB0BFA7A9342C3D, std::numeric_limits<int>::max()}, // DATAFILE_SELECT_UGC_STATS
	{0x83BCCE3224735F05, std::numeric_limits<int>::max()}, // DATAFILE_START_SAVE_TO_CLOUD
	{0x2ED61456317B8178, std::numeric_limits<int>::max()}, // DATAFILE_STORE_MISSION_HEADER
	{0x4DFDD9EB705F8140, std::numeric_limits<int>::max()}, // DATAFILE_UPDATE_SAVE_TO_CLOUD
	{0xAD6875BBC0FC899C, std::numeric_limits<int>::max()}, // DATAFILE_WATCH_REQUEST_ID

	{0xF8B0F5A43E928C76, std::numeric_limits<int>::max()}, // DATAARRAY_ADD_BOOL
	{0x6889498B3E19C797, std::numeric_limits<int>::max()}, // DATAARRAY_ADD_DICT
	{0x57A995FD75D37F56, std::numeric_limits<int>::max()}, // DATAARRAY_ADD_FLOAT
	{0xCABDB751D86FE93B, std::numeric_limits<int>::max()}, // DATAARRAY_ADD_INT
	{0x2F0661C155AEEEAA, std::numeric_limits<int>::max()}, // DATAARRAY_ADD_STRING
	{0x407F8D034F70F0C2, std::numeric_limits<int>::max()}, // DATAARRAY_ADD_VECTOR
	{0x50C1B2874E50C114, std::numeric_limits<int>::max()}, // DATAARRAY_GET_BOOL
	{0x065DB281590CEA2D, std::numeric_limits<int>::max()}, // DATAARRAY_GET_COUNT
	{0x8B5FADCC4E3A145F, std::numeric_limits<int>::max()}, // DATAARRAY_GET_DICT
	{0xC0C527B525D7CFB5, std::numeric_limits<int>::max()}, // DATAARRAY_GET_FLOAT
	{0x3E5AE19425CD74BE, std::numeric_limits<int>::max()}, // DATAARRAY_GET_INT
	{0xD3F2FFEB8D836F52, std::numeric_limits<int>::max()}, // DATAARRAY_GET_STRING
	{0x3A0014ADB172A3C5, std::numeric_limits<int>::max()}, // DATAARRAY_GET_TYPE
	{0x8D2064E5B64A628A, std::numeric_limits<int>::max()}, // DATAARRAY_GET_VECTOR

	{0x6AD0BD5E087866CB, std::numeric_limits<int>::max()},
	{0xA6EEF01087181EDD, std::numeric_limits<int>::max()},
	{0xDBF860CF1DB8E599, std::numeric_limits<int>::max()},

	// Natives that were introduces after a certain build and are closely coupled with the DLC content.
	// When running new game executable with old DLC set - we have to explicitly disable these natives.

	{0x5E1460624D194A38, 2189}, // SET_USE_ISLAND_MAP
	{0x7E3F55ED251B76D3, 2189}, // _LOAD_GLOBAL_WATER_TYPE
	{0x9A9D1BA639675CF1, 2189} // SET_ISLAND_ENABLED
};

fwEvent<> rage::scrEngine::OnScriptInit;
fwEvent<bool&> rage::scrEngine::CheckNativeScriptAllowed;

static rage::pgPtrCollection<GtaThread>* scrThreadCollection;
static uint32_t activeThreadTlsOffset;

uint32_t* scrThreadId;
static uint32_t* scrThreadCount;

rage::scriptHandlerMgr* g_scriptHandlerMgr;

static bool g_hasObfuscated;

// see https://github.com/ivanmeler/OpenVHook/blob/b5b4d84e76feb05a988e9d69b6b5c164458341cb/OpenVHook/Scripting/ScriptEngine.cpp#L22
#pragma pack(push, 1)
struct NativeRegistration_obf : public rage::sysUseAllocator
{
private:
	uint64_t nextRegistration1;
	uint64_t nextRegistration2;
public:
	rage::scrEngine::NativeHandler handlers[7];
private:
	uint32_t numEntries1;
	uint32_t numEntries2;
	uint32_t pad;
	uint64_t hashes[7 * 2];

public:
	inline NativeRegistration_obf* getNextRegistration()
	{
		uintptr_t result;
		auto v5 = reinterpret_cast<uintptr_t>(&nextRegistration1);
		auto v12 = 2i64;
		auto v13 = v5 ^ nextRegistration2;
		auto v14 = (char *)&result - v5;
		do
		{
			*(DWORD*)&v14[v5] = v13 ^ *(DWORD*)v5;
			v5 += 4i64;
			--v12;
		} while (v12);

		return reinterpret_cast<NativeRegistration_obf*>(result);
	}

	inline void setNextRegistration(NativeRegistration_obf* registration)
	{
		nextRegistration1 = ((uint64_t)&nextRegistration1 << 32) ^ ((uint32_t)&nextRegistration1 << 0) ^ (uint64_t)registration;
		nextRegistration2 = 0;
	}

	inline uint32_t getNumEntries()
	{
		return ((uintptr_t)&numEntries1) ^ numEntries1 ^ numEntries2;
	}

	inline void setNumEntries(uint32_t entries)
	{
		numEntries1 = (uint32_t)&numEntries1 ^ entries;
		numEntries2 = 0;
	}

	inline uint64_t getHash(uint32_t index)
	{
		auto naddr = 16 * index + reinterpret_cast<uintptr_t>(&nextRegistration1) + 0x54;
		auto v8 = 2i64;
		uint64_t nResult;
		auto v11 = (char *)&nResult - naddr;
		auto v10 = naddr ^ *(DWORD*)(naddr + 8);
		do
		{
			*(DWORD *)&v11[naddr] = v10 ^ *(DWORD*)(naddr);
			naddr += 4i64;
			--v8;
		} while (v8);

		return nResult;
	}

	inline void setHash(uint32_t index, uint64_t newHash)
	{
		auto hash = &hashes[index * 2];
		hash[0] = ((uint64_t)hash << 32) ^ ((uint32_t)hash << 0) ^ (uint64_t)newHash;
		hash[1] = 0;
	}
};
#pragma pack(pop)

template<typename TReg>
TReg** registrationTable;

static std::unordered_set<GtaThread*> g_ownedThreads;

bool IsScriptInited();

struct NativeHash
{
public:
	explicit inline NativeHash(uint64_t hash)
	{
		m_hash = hash;
	}

	inline uint64_t GetHash() const
	{
		return m_hash;
	}

	inline bool operator==(const NativeHash& right) const
	{
		return m_hash == right.m_hash;
	}

	inline bool operator!=(const NativeHash& right) const
	{
		return !(*this == right);
	}

private:
	uint64_t m_hash;
};

namespace std
{
template<>
struct hash<NativeHash>
{
	inline size_t operator()(const NativeHash& hash) const
	{
		return hash.GetHash();
	}
};
}

namespace rage
{
static std::unordered_map<NativeHash, scrEngine::NativeHandler> g_fastPathMap;

pgPtrCollection<GtaThread>* scrEngine::GetThreadCollection()
{
	//return reinterpret_cast<pgPtrCollection<GtaThread>*>(0x1983310);
	return scrThreadCollection;
}

/*void scrEngine::SetInitHook(void(*hook)(void*))
{
	g_hooksDLL->SetHookCallback(StringHash("scrInit"), hook);
}*/

//static scrThread*& scrActiveThread = *(scrThread**)0x1849AE0;

scrThread* scrEngine::GetActiveThread()
{
	return *reinterpret_cast<scrThread**>(hook::get_tls() + activeThreadTlsOffset);
}

void scrEngine::SetActiveThread(scrThread* thread)
{
	*reinterpret_cast<scrThread**>(hook::get_tls() + activeThreadTlsOffset) = thread;
}

//static uint32_t& scrThreadId = *(uint32_t*)0x1849ADC;
//static uint32_t& scrThreadCount = *(uint32_t*)0x1849AF8;

static std::vector<std::function<void()>> g_onScriptInitQueue;

void scrEngine::CreateThread(GtaThread* thread)
{
	if (!IsScriptInited())
	{
		g_onScriptInitQueue.push_back([=]()
		{
			CreateThread(thread);
		});
		return;
	}

	// get a free thread slot
	auto collection = GetThreadCollection();
	int slot = 0;

	// first try finding the actual thread
	for (auto& threadCheck : *collection)
	{
		if (threadCheck == thread)
		{
			break;
		}

		slot++;
	}

	if (slot == collection->count())
	{
		slot = 0;

		for (auto& threadCheck : *collection)
		{
			auto context = threadCheck->GetContext();

			if (context->ThreadId == 0)
			{
				break;
			}

			slot++;
		}
	}

	// did we get a slot?
	if (slot == collection->count())
	{
		return;
	}

	{
		auto context = thread->GetContext();

		if (*scrThreadId == 0)
		{
			(*scrThreadId)++;
		}

		context->ThreadId = *scrThreadId;

		(*scrThreadId)++;

		thread->SetScriptName(va("scr_%d", (*scrThreadCount) + 1));
		context->ScriptHash = (*scrThreadCount) + 1;

		(*scrThreadCount)++;

		collection->set(slot, thread);

		g_ownedThreads.insert(thread);
	}
}

uint64_t MapNative(uint64_t inNative);
void ReviveNative(uint64_t inNative);

template<typename TReg>
bool RegisterNativeOverride(uint64_t hash, scrEngine::NativeHandler handler)
{
	TReg*& registration = registrationTable<TReg>[(hash & 0xFF)];

	uint64_t origHash = hash;

	// remove cached fastpath native
	g_fastPathMap.erase(NativeHash{ origHash });

	hash = MapNative(hash);

	auto table = registrationTable<TReg>[hash & 0xFF];

	for (; table; table = table->getNextRegistration())
	{
		for (int i = 0; i < table->getNumEntries(); i++)
		{
			if (hash == table->getHash(i))
			{
				if (g_hasObfuscated)
				{
					handler = (scrEngine::NativeHandler)EncodePointer(handler);
				}

				table->handlers[i] = handler;

				return true;
			}
		}
	}

	return false;
}

template<typename TReg>
void RegisterNativeDo(uint64_t hash, scrEngine::NativeHandler handler)
{
	// re-implemented here as the game's own function is obfuscated
	TReg*& registration = registrationTable<TReg>[(hash & 0xFF)];

	// see if there's somehow an entry by this name already
	if (RegisterNativeOverride<TReg>(hash, handler))
	{
		return;
	}

	if (registration->getNumEntries() == 7)
	{
		TReg* newRegistration = new TReg();
		newRegistration->setNextRegistration(registration);
		newRegistration->setNumEntries(0);

		// should also set the entry in the registration table
		registration = newRegistration;
	}

	// add the entry to the list
	if (g_hasObfuscated)
	{
		handler = (scrEngine::NativeHandler)EncodePointer(handler);
	}

	uint32_t index = registration->getNumEntries();
	registration->setHash(index, hash);
	registration->handlers[index] = handler;

	registration->setNumEntries(index + 1);
}

void RegisterNative(uint64_t hash, scrEngine::NativeHandler handler)
{
	RegisterNativeDo<NativeRegistration_obf>(hash, handler);
}

static std::vector<std::pair<uint64_t, scrEngine::NativeHandler>> g_nativeHandlers;

void scrEngine::RegisterNativeHandler(const char* nativeName, NativeHandler handler)
{
	g_nativeHandlers.push_back(std::make_pair(HashString(nativeName), handler));
}

void scrEngine::RegisterNativeHandler(uint64_t nativeIdentifier, NativeHandler handler)
{
	g_nativeHandlers.push_back(std::make_pair(nativeIdentifier, handler));
}

void scrEngine::ReviveNativeHandler(uint64_t nativeIdentifier, NativeHandler handler)
{
	RegisterNativeHandler(nativeIdentifier, handler);
	if (MapNative(nativeIdentifier) == nativeIdentifier)
	{
		// If two builds share the same maxVersion index, e.g., 2545 and 2612,
		// and a script command was removed, then the native may be found in the
		// mapping table instead of the unmapped table.
		ReviveNative(nativeIdentifier);
	}
}

bool scrEngine::ShouldBlockNative(uint64_t hash)
{
	auto it = g_nativeBlockedBeforeBuild.find(hash);
	return it != g_nativeBlockedBeforeBuild.end() && xbr::GetRequestedGameBuild() < it->second;
}

std::vector<uint64_t> scrEngine::GetBlockedNatives()
{
	std::vector<uint64_t> blockedNatives;
	for (auto [hash, _]: g_nativeBlockedBeforeBuild)
	{
		if (scrEngine::ShouldBlockNative(hash))
		{
			blockedNatives.push_back(hash);
		}
	}
	return blockedNatives;
}

bool scrEngine::GetStoryMode()
{
	return storyMode;
}

static InitFunction initFunction([] ()
{
	scrEngine::OnScriptInit.Connect([] ()
	{
		auto doReg = []()
		{
			for (auto& handler : g_nativeHandlers)
			{
				RegisterNative(handler.first, handler.second);
			}

			// to prevent double registration resulting in a game error
			g_nativeHandlers.clear();
		};

		doReg();

		for (auto& entry : g_onScriptInitQueue)
		{
			entry();
		}

		g_onScriptInitQueue.clear();
	}, 50000);
});

template<typename TReg>
scrEngine::NativeHandler GetNativeHandlerDo(uint64_t origHash, uint64_t hash)
{
	auto table = registrationTable<TReg>[hash & 0xFF];
	scrEngine::NativeHandler handler = nullptr;

	for (; table; table = table->getNextRegistration())
	{
		for (int i = 0; i < table->getNumEntries(); i++)
		{
			if (hash == table->getHash(i))
			{
				handler = (scrEngine::NativeHandler) /*DecodePointer(*/ table->handlers[i] /*)*/;
				HandlerFilter(&handler);
				g_fastPathMap[NativeHash{ origHash }] = handler;
				break;
			}
		}
	}

	return handler;
}

scrEngine::NativeHandler GetNativeHandlerWrap(uint64_t origHash, uint64_t hash)
{
	return GetNativeHandlerDo<NativeRegistration_obf>(origHash, hash);
}

scrEngine::NativeHandler scrEngine::GetNativeHandler(uint64_t hash)
{
	if (auto it = g_fastPathMap.find(NativeHash{ hash }); it != g_fastPathMap.end() && it->second)
	{
		return it->second;
	}

	return GetNativeHandlerWrap(hash, MapNative(hash));
}
}

static int ReturnTrue()
{
	return true;
}

static int(*g_origReturnTrue)(void* a1, void* a2);

static int ReturnTrueFromScript(void* a1, void* a2)
{
	if (storyMode)
	{
		return g_origReturnTrue(a1, a2);
	}

	return true;
}

static void(*g_CTheScripts__Shutdown)(void);

static void CTheScripts__Shutdown()
{
	g_CTheScripts__Shutdown();

	for (auto& thread : g_ownedThreads)
	{
		thread->Reset(thread->GetContext()->ScriptHash, nullptr, 0);
	}
}

static int(*g_origNoScript)(void*, int);

static int JustNoScript(GtaThread* thread, int a2)
{
	if (storyMode)
	{
		return g_origNoScript(thread, a2);
	}

	if (g_ownedThreads.find(thread) != g_ownedThreads.end())
	{
		thread->Run(0);
	}

	return thread->GetContext()->State;
}

static void(*origStartupScript)();

static void StartupScriptWrap()
{
	for (auto& thread : g_ownedThreads)
	{
		if (!thread->GetContext()->ThreadId)
		{
			rage::scrEngine::CreateThread(thread);
		}
	}

	origStartupScript();
}

static HookFunction hookFunction([] ()
{
	Instance<ICoreGameInit>::Get()->OnSetVariable.Connect([](const std::string& name, bool value)
	{
		if (name == "storyMode")
		{
			storyMode = value;
		}
	});

	char* location = nullptr;

	if (xbr::IsGameBuildOrGreater<3258>())
	{
		location = hook::pattern("48 8B C8 EB ? 33 C9 48 8B 05").count(1).get(0).get<char>(10);
	}
	else if (xbr::IsGameBuildOrGreater<2545>())
	{
		location = hook::pattern("48 8B C8 EB 03 49 8B CD 48 8B 05").count(1).get(0).get<char>(11);
	}
	else
	{
		location = hook::pattern("48 8B C8 EB 03 48 8B CB 48 8B 05").count(1).get(0).get<char>(11);
	}

	scrThreadCollection = reinterpret_cast<decltype(scrThreadCollection)>(location + *(int32_t*)location + 4);

	activeThreadTlsOffset = *hook::pattern("48 8B 04 D0 4A 8B 14 00 48 8B 01 F3 44 0F 2C 42 20").count(1).get(0).get<uint32_t>(-4);

	{
		if (xbr::IsGameBuildOrGreater<3258>())
		{
			scrThreadId = hook::get_address<uint32_t*>(hook::get_pattern("8B 15 ? ? ? ? 48 8B 05 ? ? ? ? FF C2 89 15 ? ? ? ? 48 8B 0C F8", 2));
		}
		else if (xbr::IsGameBuildOrGreater<2612>())
		{
			scrThreadId = hook::get_address<uint32_t*>(hook::get_pattern("8B 15 ? ? ? ? 48 8B 05 ? ? ? ? FF C2 89 15 ? ? ? ? 48 8B 0C D8", 2));
		}
		else if (xbr::IsGameBuildOrGreater<2545>())
		{
			scrThreadId = hook::get_address<uint32_t*>(hook::get_pattern("8B 15 ? ? ? ? 48 8B 05 ? ? ? ? FF C2 89 15 ? ? ? ? E9", 2));
		}
		else if (xbr::IsGameBuildOrGreater<2372>())
		{
			scrThreadId = hook::get_address<uint32_t*>(hook::get_pattern("8B 15 ? ? ? ? 48 8B 05 ? ? ? ? FF C2 89", 2));
		}
		else
		{
			// memory layout dependent
			scrThreadId = hook::get_address<uint32_t*>(hook::get_pattern("33 FF 48 85 C0 74 08 48 8B C8 E8", -9)) - 2;
		}

		location = xbr::IsGameBuildOrGreater<2545>() ? hook::get_pattern<char>("FF 0D ? ? ? ? 48 8B D9 75", 2) : hook::get_pattern<char>("FF 0D ? ? ? ? 48 8B F9", 2);
		scrThreadCount = reinterpret_cast<decltype(scrThreadCount)>(location + *(int32_t*)location + 4);

		location = hook::pattern("76 32 48 8B 53 40").count(1).get(0).get<char>(9);
		registrationTable<NativeRegistration_obf> = reinterpret_cast<decltype(registrationTable<NativeRegistration_obf>)>(location + *(int32_t*)location + 4);
	}

	location = hook::pattern("74 17 48 8B C8 E8 ? ? ? ? 48 8D 0D").count(1).get(0).get<char>(13);

	g_scriptHandlerMgr = reinterpret_cast<decltype(g_scriptHandlerMgr)>(location + *(int32_t*)location + 4);

	// script re-init
	{
		auto location = hook::get_pattern("83 FB FF 0F 84 D6 00 00 00", -0x37);

		MH_Initialize();
		MH_CreateHook(location, StartupScriptWrap, (void**)&origStartupScript);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	{
		MH_Initialize();

		// temp: kill stock scripts
		// NOTE: before removing make sure scrObfuscation in fivem-private can handle opcode 0x2C (NATIVE)
		//hook::jump(hook::pattern("48 83 EC 20 80 B9 ? 01 00 00 00 8B FA").count(1).get(0).get<void>(-0xB), JustNoScript);
		MH_CreateHook(hook::pattern("48 83 EC 20 80 B9 ? 01 00 00 00 8B FA").count(1).get(0).get<void>(-0xB), JustNoScript, (void**)&g_origNoScript);

		// make all CGameScriptId instances return 'true' in matching function (mainly used for 'is script allowed to use this object' checks)
		//hook::jump(hook::pattern("74 3C 48 8B 01 FF 50 10 84 C0").count(1).get(0).get<void>(-0x1A), ReturnTrue);
		if (xbr::IsGameBuildOrGreater<2802>())
		{
			MH_CreateHook(hook::pattern("74 41 48 8B 01 FF 50 10 84 C0").count(1).get(0).get<void>(-0x1A), ReturnTrueFromScript, (void**)&g_origReturnTrue);
		}
		else
		{
			MH_CreateHook(hook::pattern("74 3C 48 8B 01 FF 50 10 84 C0").count(1).get(0).get<void>(-0x1A), ReturnTrueFromScript, (void**)&g_origReturnTrue);
		}

		// replace `startup` initialization with resetting all owned threads
		//hook::jump(hook::get_pattern("48 63 18 83 FB FF 0F 84 D6", -0x34), ResetOwnedThreads);
		//MH_CreateHook(hook::get_pattern("48 63 18 83 FB FF 0F 84 D6", -0x34), ResetOwnedThreads, (void**)&g_origResetOwnedThreads);
		if (xbr::IsGameBuildOrGreater<2802>())
		{
			MH_CreateHook(hook::get_pattern("48 8B 01 FF 50 30 E8 ? ? ? ? E8", -0x61), CTheScripts__Shutdown, (void**)&g_CTheScripts__Shutdown);
		}
		else
		{
			MH_CreateHook(hook::get_pattern("48 8B 0D ? ? ? ? 33 D2 48 8B 01 FF 10", -0x58), CTheScripts__Shutdown, (void**)&g_CTheScripts__Shutdown);
		}

		MH_EnableHook(MH_ALL_HOOKS);
	}
});
