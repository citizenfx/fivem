// retarget_pe.cpp : Defines the entry point for the console application.
//

#include <peframework.h>

#include <algorithm>

#include <windows.h>

typedef UINT8 UBYTE;

#include <fstream>

typedef enum _UNWIND_OP_CODES {
	UWOP_PUSH_NONVOL = 0, /* info == register number */
	UWOP_ALLOC_LARGE,     /* no info, alloc size in next 2 slots */
	UWOP_ALLOC_SMALL,     /* info == size of allocation / 8 - 1 */
	UWOP_SET_FPREG,       /* no info, FP = RSP + UNWIND_INFO.FPRegOffset*16 */
	UWOP_SAVE_NONVOL,     /* info == register number, offset in next slot */
	UWOP_SAVE_NONVOL_FAR, /* info == register number, offset in next 2 slots */
	UWOP_SAVE_XMM128,     /* info == XMM reg number, offset in next slot */
	UWOP_SAVE_XMM128_FAR, /* info == XMM reg number, offset in next 2 slots */
	UWOP_PUSH_MACHFRAME   /* info == 0: no error-code, 1: error-code */
} UNWIND_CODE_OPS;

typedef union _UNWIND_CODE {
	struct {
		UBYTE CodeOffset;
		UBYTE UnwindOp : 4;
		UBYTE OpInfo : 4;
	};
	USHORT FrameOffset;
} UNWIND_CODE, *PUNWIND_CODE;

#define UNW_FLAG_EHANDLER  0x01  
#define UNW_FLAG_UHANDLER  0x02  
#define UNW_FLAG_CHAININFO 0x04  

typedef struct _UNWIND_INFO {
	UBYTE Version : 3;
	UBYTE Flags : 5;
	UBYTE SizeOfProlog;
	UBYTE CountOfCodes;
	UBYTE FrameRegister : 4;
	UBYTE FrameOffset : 4;
	UNWIND_CODE UnwindCode[1];
	/*  UNWIND_CODE MoreUnwindCode[((CountOfCodes + 1) & ~1) - 1];
	*   union {
	*       OPTIONAL ULONG ExceptionHandler;
	*       OPTIONAL ULONG FunctionEntry;
	*   };
	*   OPTIONAL ULONG ExceptionData[]; */
} UNWIND_INFO, *PUNWIND_INFO;

int main(int argc, const char** argv)
{
	if (argc < 3)
	{
		printf("invalid args\n");
		return 0;
	}

	PEFile peFile;

	{
		std::filebuf fbuf;
		fbuf.open(argv[1], std::ios::binary | std::ios_base::in);

		std::iostream stlStream(&fbuf);

		PEStreamSTL peStream(&stlStream);

		peFile.LoadFromDisk(&peStream);
	}

	PEFile sourceFile;

	{
		std::filebuf fbuf;
		fbuf.open(argv[2], std::ios::binary | std::ios_base::in);

		std::iostream stlStream(&fbuf);

		PEStreamSTL peStream(&stlStream);

		sourceFile.LoadFromDisk(&peStream);
	}

	PEFile::PESection newSection;
	newSection.shortName = ".unwind";

	// start off by counting the amount of unwind infos so we can allocate them in one fell swoop
	size_t unwindInfoSize = 0;

	for (auto& runtimeData : sourceFile.exceptRFs)
	{
		auto& unwindData = runtimeData.unwindInfoRef;
		auto unwindSection = unwindData.GetSection();
		unwindSection->stream.Seek(unwindData.GetSectionOffset());

		UNWIND_INFO info;
		unwindSection->stream.ReadStruct(info);

		unwindInfoSize += sizeof(UNWIND_INFO) + (sizeof(UNWIND_CODE) * info.CountOfCodes);
		unwindInfoSize = (unwindInfoSize + 3) & ~3;
	}

	PEFile::PESectionAllocation globalAlloc;
	newSection.Allocate(globalAlloc, unwindInfoSize);

	size_t offset = 0;

	for (auto& runtimeData : sourceFile.exceptRFs)
	{
		auto& unwindData = runtimeData.unwindInfoRef;
		auto unwindSection = unwindData.GetSection();
		unwindSection->stream.Seek(unwindData.GetSectionOffset());

		UNWIND_INFO info;
		unwindSection->stream.ReadStruct(info);

		info.Flags &= ~UNW_FLAG_CHAININFO;

		size_t baseOffset = offset;

		globalAlloc.WriteToSection(&info, sizeof(info), offset);
		offset += sizeof(info);

		for (int i = 0; i < info.CountOfCodes; i++)
		{
			UNWIND_CODE code;
			unwindSection->stream.ReadStruct(code);

			globalAlloc.WriteToSection(&code, sizeof(code), offset);
			offset += sizeof(code);
		}

		offset = (offset + 3) & ~3;

		PEFile::PERuntimeFunction newRuntimeData;
		newRuntimeData.beginAddrRef = peFile.ResolveRVAToRef(runtimeData.beginAddrRef.GetRVA());
		newRuntimeData.endAddrRef = peFile.ResolveRVAToRef(runtimeData.endAddrRef.GetRVA());
		newRuntimeData.unwindInfoRef = globalAlloc.ToReference(baseOffset, sizeof(UNWIND_INFO) + (sizeof(UNWIND_CODE) * info.CountOfCodes));

		peFile.exceptRFs.push_back(std::move(newRuntimeData));
	}

	peFile.exceptAllocEntry = {};

	std::sort(peFile.exceptRFs.begin(), peFile.exceptRFs.end(), [](const auto& left, const auto& right)
	{
		return (left.beginAddrRef.GetRVA() < right.beginAddrRef.GetRVA());
	});
	
	if (newSection.IsEmpty() == false)
	{
		newSection.Finalize();
		peFile.AddSection(std::move(newSection));
	}

	peFile.CommitDataDirectories();
	
	std::fstream stlStreamOut(argv[1], std::ios::binary | std::ios::out);

	if (!stlStreamOut.good())
	{
		return -18;
	}

	PEStreamSTL peOutStream(&stlStreamOut);
	peFile.WriteToStream(&peOutStream);

    return 0;
}

