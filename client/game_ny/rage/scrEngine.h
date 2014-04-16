#pragma once
#include "pgCollection.h"
#include "scrThread.h"

namespace rage
{
class GAMESPEC_EXPORT scrEngine
{
public:
	static pgPtrCollection<GtaThread>* GetThreadCollection();

	static void SetInitHook(void(*hook)(void*));

	static uint32_t IncrementThreadId();

	// gets the active thread
	static scrThread* GetActiveThread();

	// sets the currently running thread
	static void SetActiveThread(scrThread* thread);

	// adds a precreated custom thread to the runtime and starts it
	static void CreateThread(GtaThread* thread);
};
}