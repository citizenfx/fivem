#include "StdInc.h"
#include "scrEngine.h"
#include "CrossLibraryInterfaces.h"

fwEvent<> rage::scrEngine::OnScriptInit;

namespace rage
{
pgPtrCollection<GtaThread>* scrEngine::GetThreadCollection()
{
	return reinterpret_cast<pgPtrCollection<GtaThread>*>(0x1983310);
}

/*void scrEngine::SetInitHook(void(*hook)(void*))
{
	g_hooksDLL->SetHookCallback(StringHash("scrInit"), hook);
}*/

static scrThread*& scrActiveThread = *(scrThread**)0x1849AE0;

scrThread* scrEngine::GetActiveThread()
{
	return scrActiveThread;
}

void scrEngine::SetActiveThread(scrThread* thread)
{
	scrActiveThread = thread;
}

static uint32_t& scrThreadId = *(uint32_t*)0x1849ADC;
static uint32_t& scrThreadCount = *(uint32_t*)0x1849AF8;

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

		if (scrThreadId == 0)
		{
			scrThreadId++;
		}

		context->ThreadId = scrThreadId;

		scrThreadId++;
		scrThreadCount++;

		collection->set(slot, thread);
	}
}

void WRAPPER RegisterNative(uint32_t hash, scrEngine::NativeHandler handler) { EAXJMP(0x5A6200); }

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
	});
});

scrEngine::NativeHandler scrEngine::GetNativeHandler(uint32_t hash)
{
	scrEngine::NativeHandler returnValue = nullptr;

	__asm
	{
		push esi
		mov esi, hash
		mov eax, 5A76D0h

		call eax

		pop esi

		mov returnValue, eax
	}

	return returnValue;
}
}