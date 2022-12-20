// hook for the initial mountpoint of filesystem stuff
#include "StdInc.h"
#include <fiDevice.h>
#include <Hooking.h>

#include <MinHook.h>
#include <filesystem>

#include <Error.h>

#include <CrossBuildRuntime.h>

static hook::cdecl_stub<void()> originalMount([]()
{
	return hook::pattern("48 81 EC E0 03 00 00 48 B8 63 6F 6D 6D").count(1).get(0).get<void>(-0x1A);
});

static void (*g_origInitialMount)();

static void CallInitialMount()
{
	// do pre-initial mount
	// the direct obfuscated call seems to be important, starting from 2699.16

	if (xbr::IsGameBuildOrGreater<2802>())
	{
		g_origInitialMount();
	}
	else
	{
		originalMount();
	}

	rage::fiDevice::OnInitialMount();
}

static thread_local std::string currentPack;

static bool(*g_origOpenPackfile)(rage::fiPackfile* packfile, const char* archive, bool a3, int a4, intptr_t a5);

static DWORD PackfileEh(PEXCEPTION_POINTERS ei)
{
	if ((uintptr_t)ei->ExceptionRecord->ExceptionAddress < hook::get_adjusted(0x140000000) ||
		(uintptr_t)ei->ExceptionRecord->ExceptionAddress > hook::get_adjusted(0x146000000))
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

static decltype(&CreateFileW) createFileW;

static HANDLE WINAPI CreateFileWDummy(_In_ LPCWSTR lpFileName, _In_ DWORD dwDesiredAccess, _In_ DWORD dwShareMode, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, _In_ DWORD dwCreationDisposition, _In_ DWORD dwFlagsAndAttributes, _In_opt_ HANDLE hTemplateFile)
{
	return createFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

static void ResetPackfile(const char* archive)
{
	// if this is update.rpf, maybe we should do something about that?
	if (strcmp(archive, "update/update.rpf") == 0)
	{
		createFileW = hook::iat("kernel32.dll", CreateFileWDummy, "CreateFileW");

		HANDLE hFile = createFileW(L"update/update.rpf", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			wchar_t realPath[512];
			GetFinalPathNameByHandleW(hFile, realPath, std::size(realPath), VOLUME_NAME_DOS);
			CloseHandle(hFile);

			// a cache will usually contain update+update.rpf
			if (wcschr(realPath, L'+'))
			{
				std::filesystem::path pathRef(realPath);
				pathRef.replace_filename(L"corrupted_" + pathRef.filename().wstring());

				DeleteFileW(pathRef.wstring().c_str());
				MoveFileW(realPath, pathRef.wstring().c_str());
			}
		}
	}
}

static bool OpenArchiveWrapSeh(rage::fiPackfile* packfile, const char* archive, bool a3, int a4, intptr_t a5)
{
	__try
	{
		bool success = g_origOpenPackfile(packfile, archive, a3, a4, a5);

		if (!success)
		{
			ResetPackfile(archive);
		}

		return success;
	}
	__except (PackfileEh(GetExceptionInformation()))
	{
		ResetPackfile(archive);

		FatalError("Failed to read rage::fiPackfile %s - an exception occurred in game code.", archive);
		return false;
	}
}

static decltype(&OpenArchiveWrapSeh) g_packfileWrap = &OpenArchiveWrapSeh;

void* WrapPackfile(void* newFunc)
{
	auto pfw = g_packfileWrap;
	g_packfileWrap = (decltype(g_packfileWrap))newFunc;
	return pfw;
}

static bool OpenArchiveWrapInner(rage::fiPackfile* packfile, const char* archive, bool a3, int a4, intptr_t a5)
{
	currentPack = archive;

	bool retval = g_packfileWrap(packfile, archive, a3, a4, a5);

	currentPack = "";

	return retval;
}

static bool OpenArchiveWrap(rage::fiPackfile* packfile, const char* archive, bool a3, int a4, intptr_t a5)
{
	bool retval = OpenArchiveWrapInner(packfile, archive, a3, a4, a5);

	if (!retval)
	{
		FatalError("Could not open %s\nPlease try to verify your GTA V files, see http://rsg.ms/verify for more information.\n\nCurrently, the installation directory %s is being used.", archive, ToNarrow(MakeRelativeGamePath(L"")));
	}

	return retval;
}

static void PackfileEncryptionError()
{
	std::string modMessage = "";

	if (!currentPack.empty())
	{
		auto device = rage::fiDevice::GetDevice(currentPack.c_str(), true);
		if (device)
		{
			uint64_t ptr;
			auto h = device->OpenBulk(currentPack.c_str(), &ptr);

			if (h != -1)
			{
				struct Header7
				{
					uint32_t magic;
					uint32_t entryCount;
					uint32_t nameLength;
					uint32_t encryption;
				};

				Header7 packHeader;

				device->ReadBulk(h, ptr, &packHeader, sizeof(packHeader));
				device->CloseBulk(h);

				modMessage = fmt::sprintf("Found encryption type %s. ", std::string_view((char*)&packHeader.encryption, 4));

				if (packHeader.encryption == 0x4E45504F || packHeader.encryption == 0x50584643)
				{
					modMessage += "Since you were using OpenIV, make sure you use the 'mods' folder to place your modified files.\n\n";
				}
			}
		}
	}

	FatalError("Detected modified game files%s.\n"
		"%s"
		"Currently, the installation directory %s is being used.\n\n"
		"Please verify your game files, see http://rsg.ms/verify for more information on doing so.",
		(!currentPack.empty()) ? fmt::sprintf(" (packfile %s)", currentPack) : "",
		modMessage,
		ToNarrow(MakeRelativeGamePath(L"")));
}

static void CorruptedErrorCodes()
{
	FatalError("Corrupted Error Code File.\n"
			   "Currently, the installation directory %s is being used.\n\n"
			   "Please verify your game files, see http://rsg.ms/verify for more information on doing so.",
	ToNarrow(MakeRelativeGamePath(L"")));
}

static HookFunction hookFunction([] ()
{
	// errorcodes loading failure
	{
		auto location = hook::get_pattern<char>("C3 33 D2 33 C9 E8 ? ? ? ? ? 33 D2", 5);
		hook::call(location, CorruptedErrorCodes);
		hook::call(location + 10, CorruptedErrorCodes);
	}

	// increase non-DLC fiDevice mount limit
	{
		// GTA project initialization code, arxan-obfuscated
		auto location = hook::get_pattern<int>("C7 05 ? ? ? ? 64 00 00 00 48 8B", 6);
		hook::put<int>(location, *location * 15); // '1500' mount limit now, instead of '500'
	}

	// patch 2 changed register alloc (2015-04-17)
	{
		auto location = hook::pattern("0F B7 05 ? ? ? ? 48 03 C3 44 88 34 38 66").count(1).get(0).get<void>(0x15);
		hook::set_call(&g_origInitialMount, location);
		hook::call(location, CallInitialMount);
	}

	// don't sort update:/ relative devices before ours
	hook::nop(hook::pattern("C6 80 F0 00 00 00 01 E8 ? ? ? ? E8").count(1).get(0).get<void>(12), 5);

	// fail sanely on missing game packfiles
	{
		MH_Initialize();

		auto matches = hook::pattern("E8 ? ? ? ? 84 C0 75 0A E8 ? ? ? ? 84 C0").count_hint(7);
		MH_CreateHook(hook::get_call(matches.get(0).get<void>(0)), OpenArchiveWrapInner, (void**)&g_origOpenPackfile);

		MH_EnableHook(MH_ALL_HOOKS);

		for (int i = 0; i < matches.size(); i++)
		{
			hook::call(matches.get(i).get<void>(0), OpenArchiveWrap);
		}
	}

	// wrap err_gen_invalid failures
	hook::call(hook::get_pattern("B9 EA 0A 0E BE E8", 5), PackfileEncryptionError);

	// don't crash (forget to call rage::fiDevice::Unmount) on failed DLC text reads
	{
		auto location = hook::get_pattern("41 8B D6 E9 7C 02 00 00", 4);
		*(int*)location -= 0x12;
	}
});
