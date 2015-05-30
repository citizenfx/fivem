#include "StdInc.h"

#define RAGE_FORMATS_GAME ny
#define RAGE_FORMATS_GAME_NY
#include <phBound.h>

#ifdef _WIN32
#include <winternl.h>

extern "C" NTSTATUS ZwAllocateVirtualMemory(
	_In_    HANDLE    ProcessHandle,
	_Inout_ PVOID     *BaseAddress,
	_In_    ULONG_PTR ZeroBits,
	_Inout_ PSIZE_T   RegionSize,
	_In_    ULONG     AllocationType,
	_In_    ULONG     Protect
	);


#pragma comment(lib, "ntdll.lib")
#endif

using namespace rage::ny;

void LoadBoundFour()
{
	char* buffers2 = nullptr;
	size_t bufSize = 1089536;

	if (!NT_SUCCESS(ZwAllocateVirtualMemory(GetCurrentProcess(), (void**)&buffers2, 0xFFFFFFF, &bufSize, MEM_COMMIT, PAGE_READWRITE)))
	{
		return;
	}

	ValidateSizePh<phBound, 128>();
	ValidateSizePh<phBoundGeometry, 224>();
	ValidateSizePh<phBVH, 0x58>();

	FILE* f2 = fopen("Y:/dev/ydr/nj_liberty_1.wbn.seg", "rb");
	fread(buffers2, 1, 1089536, f2);
	fclose(f2);

	BlockMap bm2;
	bm2.virtualLen = 1;
	bm2.physicalLen = 0;
	bm2.blocks[0].data = buffers2;
	bm2.blocks[0].offset = 0;
	bm2.blocks[0].size = 1089536;

	pgStreamManager::SetBlockInfo(&bm2);
	datOwner<phBoundComposite>* boundParent = (datOwner<phBoundComposite>*)buffers2;
	boundParent->Resolve(&bm2);

	phBoundComposite* bound = boundParent->GetChild();

	for (uint16_t idx = 0; idx < bound->GetNumChildBounds(); idx++)
	{
		phBoundBVH* childBound = static_cast<phBoundBVH*>(bound->GetChildBound(idx));
		childBound->Resolve(&bm2);

		phBVH* bvh = childBound->GetBVH();

		std::vector<phBoundPoly> polys(childBound->GetNumPolygons());

		memcpy(&polys[0], childBound->GetPolygons(), sizeof(phBoundPoly) * polys.size());

		__debugbreak();
	}

	__debugbreak();
}