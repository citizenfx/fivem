/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Streaming.h"
#include "Hooking.h"

uint32_t WRAPPER CImgManager::registerIMGFile(const char* name, uint32_t offset, uint32_t size, uint8_t imgNum, uint32_t index, uint32_t resourceType) { EAXJMP(0xBCC2E0); }

struct ImgFileData
{
	char filename[MAX_PATH];
};

static ImgFileData fileData[65535];

struct ImgArchiveData
{
	char pad[0x9C];
	char field_9C;
	char doesIPL;
	char field_9E;
	char field_9F;
};

void PreLoadImgArchives()
{
	// first copy someone's img data to have something to work with
	ImgArchiveData* archiveData = (ImgArchiveData*)(0x121DFD8);

	memcpy(&archiveData[0xFE], &archiveData[0], sizeof(*archiveData));
	archiveData[0xFE].doesIPL = -1;

	// scan IMG entries
	CStreaming::ScanImgEntries();
}

static void __stdcall CreateFakeEntriesOnLoad(int, int)
{
	/*int numModelIndices = *(int*)0x15F73A4;

	for (int i = numModelIndices; i < 27500; i++)
	{
		int dummyIndex = CImgManager::GetInstance()->registerIMGFile("hash:4294967294.wdr", 1, 0, 0xFE, 65535, 110);

		trace("registered fake dummy at %d\n", dummyIndex);
	}*/
}

void __declspec(naked) PreLoadImgArchives1Stub()
{
	__asm
	{
		cmp dword ptr ds:[0121E070h], 0FFFFFFFFh
		jne justGoOn

		call PreLoadImgArchives

	justGoOn:
		push 897BD0h
		retn
	}
}

void __declspec(naked) PreLoadImgArchives2Stub()
{
	__asm
	{
		call PreLoadImgArchives
		
		push 897C50h
		retn
	}
}

typedef uint32_t(__thiscall* openBulk_t)(void* this_, const char* fileName, uint64_t* ptr);
static openBulk_t origOpenBulk;

typedef uint32_t(__thiscall* readBulk_t)(void* this_, uint32_t handle, uint64_t ptr, void* buffer, uint32_t toRead);
static readBulk_t origReadBulk;

#include "StreamingTypes.h"

class fiStreamingDevice : public rage::fiDevice
{
public:
	uint32_t openBulkImpl(const char* fileName, uint64_t* ptr)
	{
		uint32_t handle = _atoi64(&fileName[8]);

		if (handle < (INT32_MAX / 2))
		{
			/*if (handle == (1056 + streamingTypes.types[*(int*)0x15F73A0].startIndex))
			{
				__asm int 3
			}*/

			if (CImgManager::GetInstance()->fileDatas[handle].imgIndex != 0xFE)
			{
				return origOpenBulk(this, fileName, ptr);
			}
		}

		*ptr = 0;

		return CStreaming::OpenImgEntry(handle) | 0x80000000;
	}

	uint32_t readBulkImpl(uint32_t handle, uint64_t ptr, void* buffer, uint32_t toRead)
	{
		if ((handle & 0x80000000) == 0)
		{
			return origReadBulk(this, handle, ptr, buffer, toRead);
		}

		auto imgEntry = CStreaming::GetImgEntry(handle & ~0x80000000);

		return imgEntry->Read(ptr, buffer, toRead);
	}

	uint32_t closeBulkImpl(uint32_t handle)
	{
		if ((handle & 0x80000000) == 0)
		{
			return -1;
		}

		return CStreaming::CloseImgEntry(handle & ~0x80000000);
	}
};

static HookFunction hookFunction([] ()
{
	hook::jump(0x897DD0, PreLoadImgArchives1Stub);
	hook::call(0x897D9E, PreLoadImgArchives2Stub);

	DWORD funcPtr;
	__asm mov funcPtr, offset fiStreamingDevice::openBulkImpl

	origOpenBulk = *(openBulk_t*)(0xD68994);
	hook::put(0xD68994, funcPtr);

	__asm mov funcPtr, offset fiStreamingDevice::readBulkImpl

	origReadBulk = *(readBulk_t*)(0xD689A4);
	hook::put(0xD689A4, funcPtr);

	__asm mov funcPtr, offset fiStreamingDevice::closeBulkImpl
	hook::put(0xD689B8, funcPtr);

	hook::put(0xD68A20, CreateFakeEntriesOnLoad);
});