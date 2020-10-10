/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "scrEngine.h"
#include "CrossLibraryInterfaces.h"
#include "Hooking.h"

#include <LaunchMode.h>

#include <sysAllocator.h>

#include <MinHook.h>
#include <ICoreGameInit.h>

#include <unordered_set>

#if __has_include("scrEngineStubs.h")
#include <scrEngineStubs.h>
#else
inline void HandlerFilter(void* handler)
{

}
#endif

fwEvent<> rage::scrEngine::OnScriptInit;
fwEvent<bool&> rage::scrEngine::CheckNativeScriptAllowed;

static rage::pgPtrCollection<GtaThread>* scrThreadCollection;
//static uint32_t activeThreadTlsOffset;

uint32_t* scrThreadId;
static uint32_t* scrThreadCount;

rage::scriptHandlerMgr* g_scriptHandlerMgr;

static bool g_hasObfuscated;

//#pragma pack(push, 1)
struct NativeRegistration : public rage::sysUseAllocator
{
public:
	NativeRegistration* nextRegistration;
	rage::scrEngine::NativeHandler handlers[7];
	uint32_t numEntries;
	uint64_t hashes[7];

public:
	inline NativeRegistration* getNextRegistration()
	{
		return nextRegistration;
	}

	inline void setNextRegistration(NativeRegistration* registration)
	{
		nextRegistration = registration;
	}

	inline uint32_t getNumEntries()
	{
		return numEntries;
	}

	inline void setNumEntries(uint32_t entries)
	{
		numEntries = entries;
	}

	inline uint64_t getHash(uint32_t index)
	{
		return hashes[index];
	}

	inline void setHash(uint32_t index, uint64_t newHash)
	{
		hashes[index] = newHash;
	}
};
//#pragma pack(pop)

NativeRegistration** registrationTable;

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

static GtaThread* hijackedThreads[512];

namespace rage
{
static std::unordered_map<NativeHash, scrEngine::NativeHandler> g_fastPathMap;

pgPtrCollection<GtaThread>* scrEngine::GetThreadCollection()
{
	return scrThreadCollection;
}

scrThread** g_activeThread;

scrThread* scrEngine::GetActiveThread()
{
	// 1207.58
	return *g_activeThread;
}

void scrEngine::SetActiveThread(scrThread* thread)
{
	*g_activeThread = thread;
}

static std::vector<std::function<void()>> g_onScriptInitQueue;

static thread_local bool g_inScriptReinit;

void scrEngine::CreateThread(GtaThread* thread)
{
	if (!g_inScriptReinit)
	{
		g_onScriptInitQueue.push_back([=]()
		{
			g_inScriptReinit = true;
			CreateThread(thread);
			g_inScriptReinit = false;
		});

		if (!IsScriptInited())
		{
			return;
		}
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

		thread->SetScriptName(va("scr_%d", (*scrThreadId) + 1));
		context->ScriptHash = (*scrThreadId) + 1;

		(*scrThreadCount)++;

		if (!hijackedThreads[slot])
		{
			hijackedThreads[slot] = collection->at(slot);
		}

		collection->set(slot, thread);

		g_ownedThreads.insert(thread);
	}
}

static uint64_t* nHashObf;

static uint64_t MapNative(uint64_t hash)
{
	auto tmp = _rotl64(_rotr64(*nHashObf, 5), 32);
	return tmp ^ _rotr64(_rotl64(~hash, 32), ((uint8_t)tmp & 0x1Fu) + 1);
}

bool RegisterNativeOverride(uint64_t hash, scrEngine::NativeHandler handler)
{
	NativeRegistration*& registration = registrationTable[(hash & 0xFF)];

	uint64_t origHash = hash;

	// remove cached fastpath native
	g_fastPathMap.erase(NativeHash{ origHash });

	NativeRegistration* table = registrationTable[hash & 0xFF];

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

void RegisterNative(uint64_t hash, scrEngine::NativeHandler handler)
{
	hash = MapNative(hash);

	// re-implemented here as the game's own function is obfuscated
	NativeRegistration*& registration = registrationTable[(hash & 0xFF)];

	// see if there's somehow an entry by this name already
	if (RegisterNativeOverride(hash, handler))
	{
		return;
	}

	if (registration->getNumEntries() == 7)
	{
		NativeRegistration* newRegistration = new NativeRegistration();
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

static std::vector<std::pair<uint64_t, scrEngine::NativeHandler>> g_nativeHandlers;

void scrEngine::RegisterNativeHandler(const char* nativeName, NativeHandler handler)
{
	g_nativeHandlers.push_back(std::make_pair(HashString(nativeName), handler));
}

void scrEngine::RegisterNativeHandler(uint64_t nativeIdentifier, NativeHandler handler)
{
	g_nativeHandlers.push_back(std::make_pair(nativeIdentifier, handler));
}

void OnScriptReInit()
{
	for (auto& entry : g_onScriptInitQueue)
	{
		entry();
	}
}

static InitFunction initFunction([] ()
{
	scrEngine::OnScriptInit.Connect([] ()
	{
		for (auto& handler : g_nativeHandlers)
		{
			RegisterNative(handler.first, handler.second);
		}

		// to prevent double registration resulting in a game error
		g_nativeHandlers.clear();
	}, 50000);
});

struct NativeObfuscation
{
	NativeObfuscation()
	{
		for (size_t i = 0; i < 256; i++)
		{
			NativeRegistration* table = registrationTable[i];

			for (; table; table = table->getNextRegistration())
			{
				for (size_t j = 0; j < table->getNumEntries(); j++)
				{
					table->handlers[j] = (scrEngine::NativeHandler)EncodePointer(table->handlers[j]);
				}
			}
		}

		g_hasObfuscated = true;
	}

	~NativeObfuscation()
	{

	}
};

static void EnsureNativeObfuscation()
{
	//static NativeObfuscation nativeObfuscation;
}

scrEngine::NativeHandler scrEngine::GetNativeHandler(uint64_t hash)
{
	EnsureNativeObfuscation();

	scrEngine::NativeHandler handler = nullptr;

	auto it = g_fastPathMap.find(NativeHash{ hash });

	if (it != g_fastPathMap.end())
	{
		handler = it->second;
	}

	uint64_t origHash = hash;

	if (!handler)
	{
		hash = MapNative(hash);

		NativeRegistration* table = registrationTable[hash & 0xFF];

		for (; table; table = table->getNextRegistration())
		{
			for (int i = 0; i < table->getNumEntries(); i++)
			{
				if (hash == table->getHash(i))
				{
					handler = (scrEngine::NativeHandler)/*DecodePointer(*/table->handlers[i]/*)*/;
					HandlerFilter(&handler);

					g_fastPathMap.insert({ NativeHash{ origHash }, handler });

					break;
				}
			}
		}
	}

	if (handler)
	{
		//StringToInt, ClearBit, SetBitsInRange, SetBit, CopyMemory
		if (origHash == 0xF2DD2298B3AF23E2 || origHash == 0x7D1D4A3602B6AD4E || origHash == 0x324DC1CEF57F31E6 || origHash == 0xF73FBE4845C43B5B || origHash == 0xF7AC7DC0DEE7C9BE)
		{
			return [](rage::scrNativeCallContext*)
			{
				// no-op
			};
		}

		return handler;
	}

	return nullptr;
}
}

static int ReturnTrue()
{
	return true;
}

static int(*g_origReturnTrue)(void* a1, void* a2);

static int ReturnTrueFromScript(void* a1, void* a2)
{
	if (Instance<ICoreGameInit>::Get()->HasVariable("storyMode"))
	{
		return g_origReturnTrue(a1, a2);
	}

	return true;
}

static void(*g_origResetOwnedThreads)();

static void ResetOwnedThreads()
{
	if (Instance<ICoreGameInit>::Get()->HasVariable("storyMode"))
	{
		if (g_origResetOwnedThreads)
		{
			return g_origResetOwnedThreads();
		}
	}

	for (auto& thread : g_ownedThreads)
	{
		thread->Reset(thread->GetContext()->ScriptHash, nullptr, 0);
	}
}

static int(*g_origNoScript)(void*, int);
static int numTicks;

static int JustNoScript(GtaThread* thread, int a2)
{
	numTicks++;

	if (Instance<ICoreGameInit>::Get()->HasVariable("storyMode"))
	{
		return g_origNoScript(thread, a2);
	}

	if (g_ownedThreads.find(thread) != g_ownedThreads.end())
	{
		thread->Run(0);
	}

	return thread->GetContext()->State;
}

static void*(*origStartupScript)(void*, void*);

static void* StartupScriptWrap(void* a1, void* a2)
{
	for (auto& thread : g_ownedThreads)
	{
		if (!thread->GetContext()->ThreadId)
		{
			rage::scrEngine::CreateThread(thread);
		}
	}

	ResetOwnedThreads();
	numTicks = 0;

	return origStartupScript(a1, a2);
}

static int ReturnScriptType()
{
	if (!Instance<ICoreGameInit>::Get()->GetGameLoaded() || numTicks < 5)
	{
		return 0xFFFF;
	}

	return 1;
}

static void(*g_origCStreamedScriptHelper__LaunchScript)(void*);

static void CStreamedScriptHelper__LaunchScript_Hook(void* a1)
{
	if (Instance<ICoreGameInit>::Get()->HasVariable("storyMode"))
	{
		return g_origCStreamedScriptHelper__LaunchScript(a1);
	}
}

static int (*g_origscrThread__StartNewScriptWithNameHashAndArgs)(int hash, void* args, int numArgs, int flags);

static int scrThread__StartNewScriptWithNameHashAndArgs_Hook(int hash, void* args, int numArgs, int flags)
{
	if (Instance<ICoreGameInit>::Get()->HasVariable("storyMode"))
	{
		return g_origscrThread__StartNewScriptWithNameHashAndArgs(hash, args, numArgs, flags);
	}

	return 0;
}

static void(*g_origUnkScriptShutdown)(void*);

static void UnkScriptShutdown(void* a1)
{
	for (auto& thread : g_ownedThreads)
	{
		for (int i = 0; i < scrThreadCollection->count(); i++)
		{
			if (scrThreadCollection->at(i) == thread)
			{
				scrThreadCollection->set(i, hijackedThreads[i]);
				(*scrThreadCount)--;

				break;
			}
		}
	}

	g_origUnkScriptShutdown(a1);
}

static HookFunction hookFunction([] ()
{
	//char* location = hook::pattern("48 8B C8 EB 03 48 8B CB 48 8B 05").count(1).get(0).get<char>(11);

	// 1207.58
	scrThreadCollection = hook::get_address<decltype(scrThreadCollection)>(hook::get_pattern("48 8B 05 ? ? ? ? 49 89 0C", 3));

	rage::g_activeThread = hook::get_address<decltype(rage::g_activeThread)>(hook::get_pattern("41 C6 04 06 01 48 8D 45 08 48 89 44 24 20 E8", 34));

	scrThreadId = hook::get_address<decltype(scrThreadId)>(hook::get_pattern("8B 0D ? ? ? ? 3B CA 7D 28 4C 8B 0D", 2));

	scrThreadCount = reinterpret_cast<decltype(scrThreadCount)>(hook::get_address<char*>(hook::get_pattern("48 8B F1 83 2D ? ? ? ? 01 75 16 8B FB 8B CF", 5)) + 1 + 8);

	//location = hook::pattern("76 32 48 8B 53 40").count(1).get(0).get<char>(9);

	registrationTable = hook::get_address<decltype(registrationTable)>(hook::get_pattern("33 D2 83 25 ? ? ? ? 00 41 B8 ? ? ? ? C6 05 ? ? ? ? 00 05", 60));

	//location = hook::pattern("74 17 48 8B C8 E8 ? ? ? ? 48 8D 0D").count(1).get(0).get<char>(13);

	g_scriptHandlerMgr = hook::get_address<decltype(g_scriptHandlerMgr)>(hook::get_pattern("48 8D 96 ? ? 00 00 48 C1 E0 07 48 8D 0D", 14));

	// disable nested script behavior on tick
	hook::put<uint8_t>(hook::get_pattern("74 ? E8 ? ? ? ? 8B 15 ? ? ? ? 48 8D 0D ? ? ? ? E8"), 0xEB);

	// always pretend root script is SP (leads to HUD loading and some other stuff lighting up)
	hook::jump(hook::get_pattern("44 8B 81 D8 06 00 00 85 D2 74", -0xE), ReturnScriptType);

	// native hash obfuscation root for 1311
	rage::nHashObf = hook::get_address<uint64_t*>(hook::get_pattern("41 B8 ? ? ? ? C6 05 ? ? ? ? 00 05 C3 9E", 26));

	// remove our scripts pre-shutdown (since this is INIT_SESSION-time now)
	{
		auto location = hook::get_pattern("53 48 83 EC 20 48 8D 0D ? ? ? ? E8 ? ? ? ? B9 08 00 00 00 E8", 12);
		hook::set_call(&g_origUnkScriptShutdown, location);
		hook::call(location, UnkScriptShutdown);
	}

	// script re-init
	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("48 8B D9 4C 8D 05 ? ? ? ? 4C 0F 45 C0", -0x14), StartupScriptWrap, (void**)&origStartupScript);
		MH_CreateHook(hook::get_pattern("83 79 08 00 48 8B D9 74 38 48 83 79 18 00", -6), CStreamedScriptHelper__LaunchScript_Hook, (void**)&g_origCStreamedScriptHelper__LaunchScript);
		MH_CreateHook(hook::get_pattern("41 B9 00 02 00 00 44  0F 45 C8 41 C1 E0 03", -0x1D), scrThread__StartNewScriptWithNameHashAndArgs_Hook, (void**)&g_origscrThread__StartNewScriptWithNameHashAndArgs);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	if (!CfxIsSinglePlayer())
	{
		MH_Initialize();

		// temp: kill stock scripts
		// NOTE: before removing make sure scrObfuscation in fivem-private can handle opcode 0x2C (NATIVE)
		//hook::jump(hook::pattern("48 83 EC 20 80 B9 46 01  00 00 00 8B FA").count(1).get(0).get<void>(-0xB), JustNoScript);
		MH_CreateHook(hook::pattern("80 B9 1A 07 00 00").count(1).get(0).get<void>(-0xA), JustNoScript, (void**)&g_origNoScript);

		// make all CGameScriptId instances return 'true' in matching function (mainly used for 'is script allowed to use this object' checks)
		//hook::jump(hook::pattern("74 3C 48 8B 01 FF 50 10 84 C0").count(1).get(0).get<void>(-0x1A), ReturnTrue);
		//MH_CreateHook(hook::pattern("74 3C 48 8B 01 FF 50 10 84 C0").count(1).get(0).get<void>(-0x1A), ReturnTrueFromScript, (void**)&g_origReturnTrue);

		// replace `startup` initialization with resetting all owned threads
		//hook::jump(hook::get_pattern("48 63 18 83 FB FF 0F 84 D6", -0x34), ResetOwnedThreads);
		//MH_CreateHook(hook::get_pattern("48 63 18 83 FB FF 0F 84 D6", -0x34), ResetOwnedThreads, (void**)&g_origResetOwnedThreads);

		MH_EnableHook(MH_ALL_HOOKS);
	}
});
