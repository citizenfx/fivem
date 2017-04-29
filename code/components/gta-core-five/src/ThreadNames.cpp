/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

static HANDLE CreateThreadWrapper(_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes, _In_ SIZE_T dwStackSize, _In_ LPTHREAD_START_ROUTINE lpStartAddress,
								  _In_opt_ __drv_aliasesMem LPVOID lpParameter, _In_ DWORD dwCreationFlags, _Out_opt_ LPDWORD lpThreadId)
{
	// find the name parameter by frobbling the parent stack
	char* parentStackPtr = reinterpret_cast<char*>(_AddressOfReturnAddress());
	char* threadName = *reinterpret_cast<char**>(parentStackPtr + 0x50 /* offset from base pointer to argument */ + 0x60 /* offset from function stack frame stack to base pointer */ + 8 /* return address offset */);

	// create metadata for passing to the thread
	struct WrapThreadMeta
	{
		char* threadName;
		LPTHREAD_START_ROUTINE origRoutine;
		void* originalData;
	};

	WrapThreadMeta* parameter = new WrapThreadMeta{ threadName, lpStartAddress, lpParameter };

	// create a thread with 'our' callback
	HANDLE hThread = CreateThread(lpThreadAttributes, dwStackSize, [] (void* arguments)
	{
		// get and free metadata
		WrapThreadMeta* metaPtr = reinterpret_cast<WrapThreadMeta*>(arguments);
		WrapThreadMeta meta = *metaPtr;
		delete metaPtr;

		// set thread name, if any
		if (meta.threadName)
		{
			SetThreadName(-1, meta.threadName);
		}

		// invoke original thread start
		return meta.origRoutine(meta.originalData);
	}, parameter, dwCreationFlags, lpThreadId);

	return hThread;
}

static HookFunction hookFunction([] ()
{
	// RAGE thread creation function: CreateThread call
	void* createThread = hook::pattern("48 89 44 24 28 33 C9 44 89 7C 24 20").count(1).get(0).get<void>(12);

	hook::nop(createThread, 6); // as it's an indirect call
	hook::call(createThread, CreateThreadWrapper);
});