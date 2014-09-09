#include "StdInc.h"
#include "Streaming.h"
#include "Hooking.h"

typedef uint32_t(__thiscall* readBulk_t)(void* this_, uint32_t handle, uint64_t ptr, void* buffer, uint32_t toRead);
static readBulk_t origReadBulk;

uint32_t StreamWorker_Read(uint32_t handle, uint64_t ptr, void* buffer, uint32_t toRead)
{
	int streamThreadNum = ((ptr & (0x4000000000000000))) ? 1 : 0;
	LPOVERLAPPED overlapped = StreamWorker_GetOverlapped(streamThreadNum);

	ptr &= ~(0xC000000000000000);

	overlapped->Offset = (ptr & 0xFFFFFFFF);
	overlapped->OffsetHigh = ptr >> 32;

	if (!ReadFile((HANDLE)handle, buffer, toRead, nullptr, overlapped))
	{
		int err = GetLastError();

		if (err != ERROR_IO_PENDING)
		{
			trace("Stream I/O failed: %i\n", err);
		}

		return -1;
	}

	return overlapped->InternalHigh;
}

class fiDeviceLocal : public rage::fiDevice
{
public:
	uint32_t readBulkImpl(uint32_t handle, uint64_t ptr, void* buffer, uint32_t toRead)
	{
		if (ptr & (0x8000000000000000))
		{
			return StreamWorker_Read(handle, ptr, buffer, toRead);
		}

		return origReadBulk(this, handle, ptr, buffer, toRead);
	}
};

static HookFunction hookFunction([] ()
{
	static_assert(sizeof(StreamRequest) == 1556, "StreamRequest size");
	static_assert(sizeof(StreamingItem) == 28, "StreamingItem size");

	DWORD readBulkLocal;

	__asm mov eax, fiDeviceLocal::readBulkImpl
	__asm mov readBulkLocal, eax

	origReadBulk = *(readBulk_t*)(0xD5A9D4);

	hook::put(0xD5A9D4, readBulkLocal);
	hook::jump(0x5B0E80, StreamWorker_Thread);
});