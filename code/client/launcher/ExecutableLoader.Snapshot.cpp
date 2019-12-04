#include <StdInc.h>
#include "ExecutableLoader.h"

#include <Hooking.h>

// 1604 now...!
#ifdef GTA_FIVE
#define TRIGGER_EP 0x14175DE00
#elif defined(IS_RDR3)
// 1207.58
//#define TRIGGER_EP 0x142D55C2C

// 1207.69
//#define TRIGGER_EP 0x142D5B8FC

// 1207.77
//#define TRIGGER_EP 0x142D5F80C

// 1207.80
#define TRIGGER_EP 0x142D601AC
#else
#define TRIGGER_EP 0xDECEA5ED
#endif

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
	return (T*)((uint8_t*)hook::get_adjusted(0x140000000 + rva));
}

static void UnapplyRelocations(bool a);

static LONG CALLBACK SnapshotVEH(PEXCEPTION_POINTERS pointers)
{
	static bool primed = false;

	if (pointers->ExceptionRecord->ExceptionAddress == (void*)hook::get_adjusted(TRIGGER_EP))
	{
		// clear the breakpoint
		SetDebugBits([=](CONTEXT* context)
		{
			context->Dr7 &= ~((3 << 6) | (3 << 22) | (3 << 30));
		}, pointers->ContextRecord);

		// remove relocations
		UnapplyRelocations(false);

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

		UnapplyRelocations(true);

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
		context->Dr3 = (DWORD64)hook::get_adjusted(TRIGGER_EP);
	}, nullptr);
}

static void* g_dumpEP;
static std::wstring g_dumpFileName;

static void UnapplyRelocations(bool a)
{
	IMAGE_DOS_HEADER* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(hook::get_adjusted(0x140000000));

	IMAGE_NT_HEADERS* ntHeader = GetTargetRVA<IMAGE_NT_HEADERS>(dosHeader->e_lfanew);

	IMAGE_DATA_DIRECTORY* relocationDirectory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

	IMAGE_BASE_RELOCATION* relocation = GetTargetRVA<IMAGE_BASE_RELOCATION>(relocationDirectory->VirtualAddress);
	IMAGE_BASE_RELOCATION* endRelocation = reinterpret_cast<IMAGE_BASE_RELOCATION*>((char*)relocation + relocationDirectory->Size);

	intptr_t relocOffset = static_cast<intptr_t>(hook::get_adjusted(0x140000000)) - 0x140000000;

	if (relocOffset == 0)
	{
		return;
	}

	// loop
	while (true)
	{
		// are we past the size?
		if (relocation >= endRelocation)
		{
			break;
		}

		// is this an empty block?
		if (relocation->SizeOfBlock == 0)
		{
			break;
		}

		// go through each and every relocation
		size_t numRelocations = (relocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(uint16_t);
		uint16_t * relocStart = reinterpret_cast<uint16_t*>(relocation + 1);

		for (size_t i = 0; i < numRelocations; i++)
		{
			uint16_t type = relocStart[i] >> 12;
			uint32_t rva = (relocStart[i] & 0xFFF) + relocation->VirtualAddress;

			void* addr = GetTargetRVA<void>(rva);
			DWORD oldProtect;
			VirtualProtect(addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);

			if (type == IMAGE_REL_BASED_HIGHLOW)
			{
				if (a)
				{
					*reinterpret_cast<int32_t*>(addr) += relocOffset;
				}
				else
				{
					*reinterpret_cast<int32_t*>(addr) -= relocOffset;
				}
			}
			else if (type == IMAGE_REL_BASED_DIR64)
			{
				if (a)
				{
					*reinterpret_cast<int64_t*>(addr) += relocOffset;
				}
				else
				{
					*reinterpret_cast<int64_t*>(addr) -= relocOffset;
				}
			}
			else if (type != IMAGE_REL_BASED_ABSOLUTE)
			{
				return;
			}

			VirtualProtect(addr, 4, oldProtect, &oldProtect);
		}

		// on to the next one!
		relocation = reinterpret_cast<IMAGE_BASE_RELOCATION*>((char*)relocation + relocation->SizeOfBlock);
	}
}

static LONG CALLBACK DumpVEH(PEXCEPTION_POINTERS pointers)
{
	if (pointers->ExceptionRecord->ExceptionAddress == (void*)g_dumpEP)
	{
		// clear the breakpoint
		SetDebugBits([=](CONTEXT* context)
		{
			context->Dr7 &= ~((3 << 6) | (3 << 22) | (3 << 30));
		}, pointers->ContextRecord);

		// rebase to the original base
		UnapplyRelocations(false);

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
		DWORD oldProtect;
		VirtualProtect(GetTargetRVA<void>(section->VirtualAddress), section->SizeOfRawData, PAGE_EXECUTE_READWRITE, &oldProtect);

		fread(GetTargetRVA<void>(section->VirtualAddress), 1, section->SizeOfRawData, f);

		++section;
	}

	fclose(f);

	DWORD oldProtect;
	VirtualProtect(ntHeader, 0x1000, PAGE_READWRITE, &oldProtect);

	// no-adjust
	ntHeader->OptionalHeader.AddressOfEntryPoint = TRIGGER_EP - 0x140000000;
}
