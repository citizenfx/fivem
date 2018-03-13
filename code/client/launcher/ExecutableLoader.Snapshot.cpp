#include <StdInc.h>
#include "ExecutableLoader.h"

// 1103!!!
// 1290 now
#define TRIGGER_EP 0x1417233F8

// on NT pre-6.3 (or 6.2, even), VEHs can't modify debug registers
// making a new thread for every single block is a bad idea as well, but perf isn't _that_ bad
static void SetDebugBits(std::function<void(CONTEXT*)> cb, CONTEXT* curContext)
{
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, GetCurrentThreadId());

	// this certainly isn't needed on 8.1+
	if (!IsWindows8Point1OrGreater() || !curContext)
	{
		std::thread([=]()
		{
			SuspendThread(hThread);

			CONTEXT ctx;
			ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

			GetThreadContext(hThread, &ctx);

			cb(&ctx);

			SetThreadContext(hThread, &ctx);

			ResumeThread(hThread);
		}).join();
	}

	// apply the same changes on the exception's context record (for Windows OSes that *do* apply debug registers from a VEH)
	if (curContext)
	{
		cb(curContext);
	}

	CloseHandle(hThread);
}

template<typename T>
static inline T* GetTargetRVA(uint32_t rva)
{
	return (T*)((uint8_t*)0x140000000 + rva);
}

static LONG CALLBACK SnapshotVEH(PEXCEPTION_POINTERS pointers)
{
	static bool primed = false;

	if (pointers->ExceptionRecord->ExceptionAddress == (void*)TRIGGER_EP)
	{
		// clear the breakpoint
		SetDebugBits([=](CONTEXT* context)
		{
			context->Dr7 &= ~((3 << 6) | (3 << 22) | (3 << 30));
		}, pointers->ContextRecord);

		// snapshot the process
		IMAGE_DOS_HEADER* header = GetTargetRVA<IMAGE_DOS_HEADER>(0);
		IMAGE_NT_HEADERS* ntHeader = GetTargetRVA<IMAGE_NT_HEADERS>(header->e_lfanew);
		IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeader);

		FILE* f = _wfopen(MakeRelativeCitPath(fmt::sprintf(L"cache\\game\\executable_snapshot_%x.bin", ntHeader->OptionalHeader.AddressOfEntryPoint)).c_str(), L"wb");

		for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
		{
			if (f)
			{
				fwrite(GetTargetRVA<void>(section->VirtualAddress), 1, section->SizeOfRawData, f);
			}

			++section;
		}

		if (f)
		{
			fclose(f);
		}

		return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

void DoCreateSnapshot()
{
	AddVectoredExceptionHandler(1, SnapshotVEH);

	SetDebugBits([](CONTEXT* context)
	{
		// clear debug breakpoint 4 and set 'our' flags
		context->Dr7 &= ~((3 << 6) | (3 << 28) | (3 << 30));
		context->Dr7 |= (1 << 6) | (0 << 28) | (0 << 30);

		// set the address for bp 4
		context->Dr3 = (DWORD64)TRIGGER_EP;
	}, nullptr);
}

static void* g_dumpEP;
static std::wstring g_dumpFileName;

static LONG CALLBACK DumpVEH(PEXCEPTION_POINTERS pointers)
{
	if (pointers->ExceptionRecord->ExceptionAddress == (void*)g_dumpEP)
	{
		// clear the breakpoint
		SetDebugBits([=](CONTEXT* context)
		{
			context->Dr7 &= ~((3 << 6) | (3 << 22) | (3 << 30));
		}, pointers->ContextRecord);

		// snapshot the process
		IMAGE_DOS_HEADER* header = GetTargetRVA<IMAGE_DOS_HEADER>(0);
		IMAGE_NT_HEADERS* ntHeader = GetTargetRVA<IMAGE_NT_HEADERS>(header->e_lfanew);
		IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeader);

		FILE* f = _wfopen(g_dumpFileName.c_str(), L"wb");

		auto zeroPad = [&](int64_t point)
		{
			auto curPos = _ftelli64(f);

			assert(curPos <= point);
			
			for (int i = curPos; i < point; i++)
			{
				uint8_t c = 0;
				fwrite(&c, 1, 1, f);
			}
		};

		// make sections usable
		for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
		{
			if (section[i].Characteristics & IMAGE_SCN_MEM_EXECUTE)
			{
				section[i].Characteristics |= IMAGE_SCN_MEM_WRITE;
			}
		}

		// set EP to be past the packer
		ntHeader->OptionalHeader.AddressOfEntryPoint += 0xD;

		if (f)
		{
			// write header
			fwrite(GetTargetRVA<void>(0), 1, section->PointerToRawData, f);
		}

		for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
		{
			if (f)
			{
				if (section->PointerToRawData != 0)
				{
					zeroPad(section->PointerToRawData);
					fwrite(GetTargetRVA<void>(section->VirtualAddress), 1, section->SizeOfRawData, f);
				}
			}

			++section;
		}

		if (f)
		{
			fclose(f);
		}

		TerminateProcess(GetCurrentProcess(), 4242);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

void DoCreateDump(void* ep, const wchar_t* fileName)
{
	g_dumpEP = ep;
	g_dumpFileName = fileName;

	AddVectoredExceptionHandler(1, DumpVEH);

	SetDebugBits([=](CONTEXT* context)
	{
		// clear debug breakpoint 4 and set 'our' flags
		context->Dr7 &= ~((3 << 6) | (3 << 28) | (3 << 30));
		context->Dr7 |= (1 << 6) | (0 << 28) | (0 << 30);

		// set the address for bp 4
		context->Dr3 = (DWORD64)ep;
	}, nullptr);
}

void ExecutableLoader::LoadSnapshot(IMAGE_NT_HEADERS* ntHeader)
{
	std::wstring snapBaseName = MakeRelativeCitPath(fmt::sprintf(L"cache\\game\\executable_snapshot_%x.bin", ntHeader->OptionalHeader.AddressOfEntryPoint));

	if (GetFileAttributesW(snapBaseName.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		DoCreateSnapshot();
		return;
	}

	IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeader);

	FILE* f = _wfopen(snapBaseName.c_str(), L"rb");

	if (!f)
	{
		return;
	}

	for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
	{
		fread(GetTargetRVA<void>(section->VirtualAddress), 1, section->SizeOfRawData, f);

		++section;
	}

	fclose(f);

	DWORD oldProtect;
	VirtualProtect(ntHeader, 0x1000, PAGE_READWRITE, &oldProtect);

	ntHeader->OptionalHeader.AddressOfEntryPoint = TRIGGER_EP - 0x140000000;
}
