#include "StdInc.h"

static HANDLE WINAPI CreateThreadWrapper(_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes, _In_ SIZE_T dwStackSize, _In_ LPTHREAD_START_ROUTINE lpStartAddress,
								  _In_opt_ __drv_aliasesMem LPVOID lpParameter, _In_ DWORD dwCreationFlags, _Out_opt_ LPDWORD lpThreadId)
{
	// find the name parameter by frobbling the parent stack
	char* parentStackPtr = reinterpret_cast<char*>(_AddressOfReturnAddress());
	char* threadName = *reinterpret_cast<char**>(parentStackPtr + 0x38);

	// create metadata for passing to the thread
	struct WrapThreadMeta
	{
		char* threadName;
		LPTHREAD_START_ROUTINE origRoutine;
		void* originalData;
	};

	WrapThreadMeta* parameter = new WrapThreadMeta{ threadName ? strdup(threadName) : nullptr, lpStartAddress, lpParameter };

	// create a thread with 'our' callback
	HANDLE hThread = CreateThread(lpThreadAttributes, dwStackSize, [](void* arguments)
	{
		// get and free metadata
		WrapThreadMeta* metaPtr = reinterpret_cast<WrapThreadMeta*>(arguments);
		WrapThreadMeta meta = *metaPtr;
		delete metaPtr;

		// set thread name, if any
		if (meta.threadName)
		{
			SetThreadName(-1, meta.threadName);
			free(meta.threadName);
		}

		// invoke original thread start
		return meta.origRoutine(meta.originalData);
	}, parameter, dwCreationFlags, lpThreadId);

	return hThread;
}

static HookFunction hookFunction([] ()
{
	// RAGE thread creation function: CreateThread call
	void* createThread = hook::get_pattern("56 6A 00 C7 44 24 2C 00 00 00 00", 11);

	hook::nop(createThread, 6); // as it's an indirect call
	hook::call(createThread, CreateThreadWrapper);
});
