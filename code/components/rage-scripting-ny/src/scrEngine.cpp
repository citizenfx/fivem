/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "scrEngine.h"
#include "CrossLibraryInterfaces.h"
#include <Hooking.h>

fwEvent<> rage::scrEngine::OnScriptInit;
fwEvent<bool&> rage::scrEngine::CheckNativeScriptAllowed;

static char* g_threadCollection;

namespace rage
{
pgPtrCollection<GtaThread>* scrEngine::GetThreadCollection()
{
	return *reinterpret_cast<pgPtrCollection<GtaThread>**>(g_threadCollection);
}

/*void scrEngine::SetInitHook(void(*hook)(void*))
{
	g_hooksDLL->SetHookCallback(StringHash("scrInit"), hook);
}*/

scrThread** scrActiveThread;

scrThread* scrEngine::GetActiveThread()
{
	return *scrActiveThread;
}

void scrEngine::SetActiveThread(scrThread* thread)
{
	*scrActiveThread = thread;
}

uint32_t* scrThreadId;
uint32_t* scrThreadCount;

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
		thread->Reset(1, nullptr, 0);

		if (*scrThreadId == 0)
		{
			(*scrThreadId)++;
		}

		context->ThreadId = *scrThreadId;

		(*scrThreadId)++;
		(*scrThreadCount)++;

		collection->set(slot, thread);
	}
}

static hook::cdecl_stub<void(uint32_t, scrEngine::NativeHandler)> _registerNative([]()
{
	return hook::get_pattern("FF 74 24 08 FF 74 24 08 E8 ? ? ? ? 84 C0 75");
});

void RegisterNative(uint32_t hash, scrEngine::NativeHandler handler) 
{ 
	_registerNative(hash, handler);
	//EAXJMP(0x5A6200);
}

static std::vector<std::pair<uint32_t, scrEngine::NativeHandler>> g_nativeHandlers;

void scrEngine::RegisterNativeHandler(const char* nativeName, NativeHandler handler)
{
	g_nativeHandlers.push_back(std::make_pair(HashString(nativeName), handler));
}

void scrEngine::RegisterNativeHandler(size_t nativeHash, NativeHandler handler)
{
	g_nativeHandlers.push_back(std::make_pair(nativeHash, handler));
}

static HookFunction initFunction([] ()
{
	// up the native limit
	hook::put<uint32_t>(hook::get_pattern("68 83 0C 00 00", 1), 9999);

	g_threadCollection = hook::get_pattern<char>("74 16 8B 01 57 FF 50 0C", -11);
	scrThreadId = *(uint32_t**)hook::get_pattern<char>("C7 40 40 00 00 00 00 A1 ? ? ? ? 8B 0C 06", 32);
	scrThreadCount = *(uint32_t**)hook::get_pattern<char>("C7 40 40 00 00 00 00 A1", 43);
	scrActiveThread = *(scrThread***)hook::get_pattern<char>("C1 E1 02 3B CB 74 ? A1", 8);

	scrEngine::OnScriptInit.Connect([] ()
	{
		for (auto& handler : g_nativeHandlers)
		{
			RegisterNative(handler.first, handler.second);
		}

		// to prevent double registration resulting in a game error
		g_nativeHandlers.clear();
	});
});

static hook::thiscall_stub<scrEngine::NativeHandler(void*, uint32_t)> _getNativeHandler([]()
{
	return hook::pattern("56 8B 35 ? ? ? ? 85 F6 75 06").count(2).get(1).get<void>(0);
});

scrEngine::NativeHandler scrEngine::GetNativeHandler(uint32_t hash)
{
	return _getNativeHandler(NULL, hash);
}
}
