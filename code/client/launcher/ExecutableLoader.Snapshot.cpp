#include <StdInc.h>
#include "ExecutableLoader.h"

#include <Hooking.h>

#include <CrossBuildRuntime.h>

#include <openssl/sha.h>

#if defined(GTA_FIVE) || defined(IS_RDR3)
inline static uintptr_t GetLauncherTriggerEP()
{
	if (getenv("CitizenFX_ToolMode"))
	{
		if (wcsstr(GetCommandLineW(), L"launcher.exe"))
		{
			// launcher.exe with sha256 hash 0dbf58119cdd2d67e6ecee1e31be3f19827444df978f7df747064a870736bce4
			return 0x14020b70c;
		}
	}

	return 0;
}
#endif

#ifdef GTA_FIVE
inline uintptr_t GetTriggerEP()
{
	if (auto ep = GetLauncherTriggerEP(); ep != 0)
	{
		return ep;
	}

	if (Is372())
	{
		return 0x141623FC8;
	}

	if (xbr::IsGameBuild<2802>())
	{
		return 0x1417E6648;
	}

	if (xbr::IsGameBuild<2699>())
	{
		return 0x1417D3600;
	}

	if (xbr::IsGameBuild<2612>())
	{
		return 0x1417DB78C;
	}

	if (xbr::IsGameBuild<2545>())
	{
		return 0x1417DB000;
	}

	if (xbr::IsGameBuild<2372>())
	{
		return 0x1417C9104;
	}

	if (xbr::IsGameBuild<2189>())
	{
		return 0x1417ACE74;
	}

	if (Is2060())
	{
		return 0x141796A34;
	}

	return 0x14175DE00;
}

// 1604
// 1868 now...!
// 2060 realities
#define TRIGGER_EP (GetTriggerEP())
#elif defined(IS_RDR3)
inline uintptr_t GetTriggerEP()
{
	if (auto ep = GetLauncherTriggerEP(); ep != 0)
	{
		return ep;
	}

	if (xbr::IsGameBuild<1355>())
	{
		return 0x142DE455C; // 1355.18
	}

	if (xbr::IsGameBuild<1436>())
	{
		return 0x142E13DA4; // 1436.31
	}

	if (xbr::IsGameBuild<1491>())
	{
		return 0x142E32334; // 1491.16
	}

	return 0x142E0F92C; // 1311.20
}

#define TRIGGER_EP (GetTriggerEP())
#elif defined(GTA_NY)
// .43
#define TRIGGER_EP 0xDF8F2B
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
	return (T*)((uint8_t*)hook::get_adjusted(
#ifdef _M_AMD64
	0x140000000
#else
	0x400000
#endif
		+ rva));
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

		FILE* f = _wfopen(MakeRelativeCitPath(fmt::sprintf(L"data\\cache\\executable_snapshot_%x.bin", ntHeader->OptionalHeader.AddressOfEntryPoint)).c_str(), L"wb");

		SHA256_CTX sha;
		SHA256_Init(&sha);

		auto write = [&f, &sha](const void* data, size_t size)
		{
			if (f)
			{
				fwrite(data, 1, size, f);
			}

			SHA256_Update(&sha, data, size);
		};

		for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
		{
			write(GetTargetRVA<void>(section->VirtualAddress), section->SizeOfRawData);

			++section;
		}

		if (f)
		{
			uint8_t endHash[256 / 8];
			SHA256_Final(endHash, &sha);

			fwrite(endHash, 1, sizeof(endHash), f);

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
		context->Dr3 = (DWORD_PTR)hook::get_adjusted(TRIGGER_EP);
	}, nullptr);
}

static void* g_dumpEP;
static std::wstring g_dumpFileName;

static void UnapplyRelocations(bool a)
{
	constexpr uintptr_t base =
#ifdef _M_AMD64
	0x140000000
#else
	0x400000
#endif
	;

	IMAGE_DOS_HEADER* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(hook::get_adjusted(base));

	IMAGE_NT_HEADERS* ntHeader = GetTargetRVA<IMAGE_NT_HEADERS>(dosHeader->e_lfanew);

	IMAGE_DATA_DIRECTORY* relocationDirectory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

	IMAGE_BASE_RELOCATION* relocation = GetTargetRVA<IMAGE_BASE_RELOCATION>(relocationDirectory->VirtualAddress);
	IMAGE_BASE_RELOCATION* endRelocation = reinterpret_cast<IMAGE_BASE_RELOCATION*>((char*)relocation + relocationDirectory->Size);

	intptr_t relocOffset = static_cast<intptr_t>(hook::get_adjusted(base)) - base;

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
		context->Dr3 = (DWORD_PTR)ep;
	}, nullptr);
}

struct FileDeleter
{
	inline void operator()(FILE* f)
	{
		if (f)
		{
			fclose(f);
		}
	}
};

bool ExecutableLoader::LoadSnapshot(IMAGE_NT_HEADERS* ntHeader)
{
	std::wstring snapBaseName = MakeRelativeCitPath(fmt::sprintf(L"data\\cache\\executable_snapshot_%x.bin", ntHeader->OptionalHeader.AddressOfEntryPoint));

	if (GetFileAttributesW(snapBaseName.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		DoCreateSnapshot();
		return false;
	}

	IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeader);

	SHA256_CTX sha;
	SHA256_Init(&sha);
	uint8_t compareHash[256 / 8];

	{
		auto f = std::unique_ptr<FILE, FileDeleter>(_wfopen(snapBaseName.c_str(), L"rb"));

		if (!f)
		{
			DoCreateSnapshot();
			return false;
		}

		for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
		{
			auto targetPtr = GetTargetRVA<void>(section->VirtualAddress);
			auto targetSize = section->SizeOfRawData;

			DWORD oldProtect;
			VirtualProtect(targetPtr, targetSize, PAGE_EXECUTE_READWRITE, &oldProtect);

			if (fread(targetPtr, 1, targetSize, f.get()) != targetSize)
			{
				DoCreateSnapshot();
				return false;
			}

			SHA256_Update(&sha, targetPtr, targetSize);
			++section;
		}

		if (fread(compareHash, 1, sizeof(compareHash), f.get()) != sizeof(compareHash))
		{
			DoCreateSnapshot();
			return false;
		}
	}

	uint8_t readHash[256 / 8];
	SHA256_Final(readHash, &sha);

	if (memcmp(readHash, compareHash, sizeof(compareHash)) != 0)
	{
		DoCreateSnapshot();
		return false;
	}

	DWORD oldProtect;
	VirtualProtect(ntHeader, 0x1000, PAGE_READWRITE, &oldProtect);

	// no-adjust
	ntHeader->OptionalHeader.AddressOfEntryPoint = TRIGGER_EP - 
#ifdef _M_AMD64
		0x140000000
#else
		0x400000
#endif
		;

	return true;
}
