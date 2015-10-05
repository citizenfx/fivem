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

#include <unordered_set>

fwEvent<> rage::scrEngine::OnScriptInit;
fwEvent<bool&> rage::scrEngine::CheckNativeScriptAllowed;

static rage::pgPtrCollection<GtaThread>* scrThreadCollection;
static uint32_t activeThreadTlsOffset;

static uint32_t* scrThreadId;
static uint32_t* scrThreadCount;

static rage::scriptHandlerMgr* g_scriptHandlerMgr;

struct NativeRegistration
{
	NativeRegistration* nextRegistration;
	rage::scrEngine::NativeHandler handlers[7];
	uint32_t numEntries;
	uint64_t hashes[7];
};

static NativeRegistration** registrationTable;

static std::unordered_set<GtaThread*> g_ownedThreads;

namespace rage
{
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
	char* moduleTls = *(char**)__readgsqword(88);

	return *reinterpret_cast<scrThread**>(moduleTls + activeThreadTlsOffset);
}

void scrEngine::SetActiveThread(scrThread* thread)
{
	char* moduleTls = *(char**)__readgsqword(88);

	*reinterpret_cast<scrThread**>(moduleTls + activeThreadTlsOffset) = thread;
}

//static uint32_t& scrThreadId = *(uint32_t*)0x1849ADC;
//static uint32_t& scrThreadCount = *(uint32_t*)0x1849AF8;

void scrEngine::CreateThread(GtaThread* thread)
{
	// get a free thread slot
	auto collection = GetThreadCollection();
	int slot = 0;

	for (auto& thread : *collection)
	{
		auto context = thread->GetContext();

		if (context->ThreadId == 0)
		{
			break;
		}

		slot++;
	}

	// did we get a slot?
	if (slot == collection->count())
	{
		return;
	}

	{
		auto context = thread->GetContext();
		thread->Reset((*scrThreadCount) + 1, nullptr, 0);

		if (*scrThreadId == 0)
		{
			(*scrThreadId)++;
		}

		context->ThreadId = *scrThreadId;

		(*scrThreadId)++;
		(*scrThreadCount)++;

		collection->set(slot, thread);

		g_ownedThreads.insert(thread);

		// attach script to the GTA script handler manager
		static void* scriptHandler;

		if (!scriptHandler)
		{
			g_scriptHandlerMgr->AttachScript(thread);
			scriptHandler = thread->GetScriptHandler();
		}
		else
		{
			thread->SetScriptHandler(scriptHandler);
		}
	}
}

void scrEngine::RegisterNativeHandler(const char* nativeName, NativeHandler handler)
{
	// no-op
}

/*void WRAPPER RegisterNative(uint32_t hash, scrEngine::NativeHandler handler) { EAXJMP(0x5A6200); }

static std::vector<std::pair<uint32_t, scrEngine::NativeHandler>> g_nativeHandlers;

void scrEngine::RegisterNativeHandler(const char* nativeName, NativeHandler handler)
{
	g_nativeHandlers.push_back(std::make_pair(HashString(nativeName), handler));
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
	});
});*/

uint64_t MapNative(uint64_t inNative);

scrEngine::NativeHandler scrEngine::GetNativeHandler(uint64_t hash)
{
	uint64_t origHash = hash;
	hash = MapNative(hash);

	NativeRegistration* table = registrationTable[hash & 0xFF];

	for (; table; table = table->nextRegistration)
	{
		for (int i = 0; i < table->numEntries; i++)
		{
			if (hash == table->hashes[i])
			{
				// temporary workaround for marking scripts as network script not storing the script handler
				if (origHash == 0xD1110739EEADB592)
				{
					static scrEngine::NativeHandler hashHandler = table->handlers[i];

					return [] (rage::scrNativeCallContext* context)
					{
						hashHandler(context);

						GtaThread* thread = static_cast<GtaThread*>(GetActiveThread());
						void* handler = thread->GetScriptHandler();

						if (handler)
						{
							for (auto& ownedThread : g_ownedThreads)
							{
								if (ownedThread != thread)
								{
									ownedThread->SetScriptHandler(handler);
								}
							}
						}
					};
				}
				// mean, mean people, those cheaters
				else if (origHash == 0xB69317BF5E782347 || origHash == 0xA670B3662FAFFBD0)
				{
					return [] (rage::scrNativeCallContext* context)
					{
						uint32_t ped = NativeInvoke::Invoke<0x43A66C31C68491C0, uint32_t>(-1);

						NativeInvoke::Invoke<0x621873ECE1178967, int>(ped, -8192.0f, -8192.0f, 500.0f);
					};
				}
				else if (origHash == 0xAAA34F8A7CB32098)
				{
					static scrEngine::NativeHandler hashHandler = table->handlers[i];

					return [](rage::scrNativeCallContext* context)
					{
						uint32_t arg = context->GetArgument<uint32_t>(0);

						uint32_t ped = NativeInvoke::Invoke<0x43A66C31C68491C0, uint32_t>(-1);

						if (ped == arg)
							hashHandler(context);
						else
							NativeInvoke::Invoke<0x621873ECE1178967, int>(ped, -8192.0f, -8192.0f, 500.0f);
					};
				}

				return table->handlers[i];
			}
		}
	}

	return nullptr;
}
}

static int JustNoScript(GtaThread* thread)
{
	if (g_ownedThreads.find(thread) != g_ownedThreads.end())
	{
		thread->Run(0);

		// attempt to fix up threads with an unknown thread ID
		for (auto& script : g_ownedThreads)
		{
			if (script != thread && script->GetContext()->ThreadId == 0)
			{
				script->GetContext()->ThreadId = *scrThreadId;
				(*scrThreadId)++;

				script->SetScriptHandler(thread->GetScriptHandler());
			}
		}
	}

	return thread->GetContext()->State;
}

static HookFunction hookFunction([] ()
{
	char* location = hook::pattern("48 8B C8 EB 03 48 8B CB 48 8B 05").count(1).get(0).get<char>(11);

	scrThreadCollection = reinterpret_cast<decltype(scrThreadCollection)>(location + *(int32_t*)location + 4);

	activeThreadTlsOffset = *hook::pattern("48 8B 04 D0 4A 8B 14 00 48 8B 01 F3 44 0F 2C 42 20").count(1).get(0).get<uint32_t>(-4);

	location = hook::pattern("FF 40 5C 8B 15 ? ? ? ? 48 8B").count(1).get(0).get<char>(5);

	scrThreadId = reinterpret_cast<decltype(scrThreadId)>(location + *(int32_t*)location + 4);

	location -= 9;

	scrThreadCount = reinterpret_cast<decltype(scrThreadCount)>(location + *(int32_t*)location + 4);

	location = hook::pattern("76 61 49 8B 7A 40 48 8D 0D").count(1).get(0).get<char>(9);

	registrationTable = reinterpret_cast<decltype(registrationTable)>(location + *(int32_t*)location + 4);

	location = hook::pattern("74 17 48 8B C8 E8 ? ? ? ? 48 8D 0D").count(1).get(0).get<char>(13);

	g_scriptHandlerMgr = reinterpret_cast<decltype(g_scriptHandlerMgr)>(location + *(int32_t*)location + 4);

	// temp: kill stock scripts
	hook::jump(hook::pattern("48 83 EC 20 80 B9 46 01  00 00 00 8B FA").count(1).get(0).get<void>(-0xB), JustNoScript);
});
