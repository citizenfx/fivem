/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ComponentLoader.h"
#include <boost/algorithm/string.hpp>

#include <CrossBuildRuntime.h>
#include <ShModeLaunch.h>
#include <filesystem>
#include <wchar.h>

#include <CoreConsole.h>
#include <LaunchMode.h>

#include <Error.h>

#pragma comment(lib, "version.lib")

class ComponentInstance : public Component
{
public:
	virtual bool Initialize();

	virtual bool DoGameLoad(void* module);

	virtual bool Shutdown();
};

bool ComponentInstance::Initialize()
{
	InitFunctionBase::RunAll();

	return true;
}

static bool LoadPEFile(const std::filesystem::path& path, std::vector<uint8_t>& outBuf)
{
	FILE* f = _wfopen(path.c_str(), L"rb");

	if (f)
	{
		fseek(f, 0, SEEK_END);
		outBuf.resize(ftell(f));

		fseek(f, 0, SEEK_SET);
		fread(&outBuf[0], 1, outBuf.size(), f);

		fclose(f);
		return true;
	}

	return false;
}

static bool IsCLRAssembly(const std::vector<uint8_t>& libraryBuffer)
{
	// get the DOS header
	IMAGE_DOS_HEADER* header = (IMAGE_DOS_HEADER*)&libraryBuffer[0];

	if (header->e_magic == IMAGE_DOS_SIGNATURE)
	{
		// get the NT header
		const IMAGE_NT_HEADERS* ntHeader = (const IMAGE_NT_HEADERS*)&libraryBuffer[header->e_lfanew];

		// find the COM+ directory
		auto comPlusDirectoryData = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR];

		if (comPlusDirectoryData.Size > 0)
		{
			if (comPlusDirectoryData.VirtualAddress < ntHeader->OptionalHeader.SizeOfImage)
			{
				return true;
			}
		}
	}

	return false;
}

bool ComponentInstance::DoGameLoad(void* module)
{
	HookFunction::RunAll();


	if (!shmr::IsShMode())
	{
		trace("Skipping .asi file loading - sh mode disabled \n");
		return true;
	}

	try
	{
		std::filesystem::path plugins_path(MakeRelativeCitPath(L"plugins"));

		// if the directory doesn't exist, we create it
		if (!std::filesystem::exists(plugins_path))
		{
			std::filesystem::create_directory(plugins_path);
		}

		std::filesystem::directory_iterator it(plugins_path), end;

		std::vector<std::wstring> blacklistedAsis = std::vector<std::wstring>({
			L"openiv.asi",
			L"scripthookvdotnet.asi",
			L"fspeedometerv.asi"
		});

		std::map<std::wstring, std::function<void(void*)>> compatShims =
		{
			{
				L"pld.asi", [] (void* pldModule)
				{
					// log file writing function
					// a missing null pointer check causes the game to terminate with a C runtime error.
					// the log file contains nothing useful, so this gets removed.
					char* patchAddress = (char*)pldModule + 0x1560;

					if (memcmp(patchAddress, "\x4c\x89\x44\x24\x18", 5) == 0)
					{
						DWORD oldProtect;
						VirtualProtect(patchAddress, 1, PAGE_EXECUTE_READWRITE, &oldProtect);

						*(uint8_t*)patchAddress = 0xC3;

						VirtualProtect(patchAddress, 1, oldProtect, &oldProtect);
					}
				}	
			}
		};

		// load all .asi files in the plugins/ directory
		while (it != end)
		{
			if (it->path().extension() == ".asi")
			{
				bool bad = false;
				std::wstring badFileName;
				std::vector<uint8_t> libraryBuffer;

				DWORD versionInfoSize = GetFileVersionInfoSize(it->path().c_str(), nullptr);
				if (versionInfoSize)
				{
					std::vector<uint8_t> versionInfo(versionInfoSize);

					if (GetFileVersionInfo(it->path().c_str(), 0, versionInfo.size(), &versionInfo[0]))
					{
						void* fixedInfoBuffer;
						UINT fixedInfoSize;

						VerQueryValue(&versionInfo[0], L"\\StringFileInfo\\040904b0\\OriginalFilename", &fixedInfoBuffer, &fixedInfoSize);

						badFileName = std::wstring((wchar_t*)fixedInfoBuffer, fixedInfoSize);
					}
				}

				if (xbr::IsGameBuildOrGreater<2189>())
				{
					bad = true;

					HMODULE hModule = LoadLibraryEx(it->path().c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);

					if (hModule)
					{
						HRSRC hResource = FindResource(hModule, L"FX_ASI_BUILD", MAKEINTRESOURCE(xbr::GetGameBuild()));

						if (hResource)
						{
							auto resSize = SizeofResource(hModule, hResource);
							auto resData = LoadResource(hModule, hResource);

							auto resPtr = static_cast<const char*>(LockResource(resData));
							
							if (resPtr)
							{
								bad = false;
							}
						}

						FreeLibrary(hModule);
					}

					if (bad)
					{
						console::Printf("script:shv", "Unable to load %s - this ASI plugin does not claim to support game build %d. If you have access to its source code, add `FX_ASI_BUILD %d dummy.txt` to the .rc file when building this plugin. If not, contact its maintainer.\n", 
							ToNarrow(it->path().wstring()).c_str(),
							xbr::GetGameBuild(),
							xbr::GetGameBuild()
						);
					}
				}

				if (LoadPEFile(it->path(), libraryBuffer))
				{
					if (!CfxIsSinglePlayer())
					{
						for (auto itt = blacklistedAsis.begin(); itt != blacklistedAsis.end(); ++itt)
						{
							if (*itt != L"")
							{
								if (wcsicmp(it->path().filename().c_str(), itt->c_str()) == 0 || wcsicmp(badFileName.c_str(), itt->c_str()) == 0)
								{
									bad = true;
									trace("Skipping blacklisted ASI %s - this plugin is not compatible with FiveM.\n", it->path().filename().string());
									if (*itt == L"openiv.asi")
									{
										FatalError("You cannot use OpenIV with FiveM. Please use clean game RPFs and remove OpenIV.asi from your plugins. Check fivem.net on how to use modded files with FiveM.");
									}
								}
							}
						}

						if (!bad)
						{
							if (IsCLRAssembly(libraryBuffer))
							{
								trace("Skipping blacklisted CLR assembly %s - this plugin is not compatible with FiveM.\n", it->path().filename().string());

								bad = true;
							}
						}
					}

					// this check is ubiquitous as these older dlls will crash you no matter what
					if (wcsicmp(it->path().filename().c_str(), L"gears.asi") == 0 || wcsicmp(badFileName.c_str(), L"gears.asi") == 0)
					{
						// get the DOS header
						IMAGE_DOS_HEADER* header = (IMAGE_DOS_HEADER*)&libraryBuffer[0];

						if (header->e_magic == IMAGE_DOS_SIGNATURE)
						{
							// get the NT header
							const IMAGE_NT_HEADERS* ntHeader = (const IMAGE_NT_HEADERS*)&libraryBuffer[header->e_lfanew];

							// check timestamp
							auto timeStamp = ntHeader->FileHeader.TimeDateStamp;
							if (timeStamp != 0 && timeStamp <= 0x605FC73B)
							{
								MessageBoxW(NULL, L"This version of the manual transmission plugin ('Gears.asi') is outdated and no longer works with FiveM. Please update this plugin to a newer version or delete it.", L"FiveM", MB_OK | MB_ICONSTOP);

								bad = true;
							}
						}
					}
				}

				if (!bad) {
					void* module = LoadLibrary(it->path().c_str());

					if (!module)
					{
						FatalError("Couldn't load %s", it->path().filename().string());
					}

					for (const auto& shim : compatShims)
					{
						if (wcsicmp(it->path().filename().c_str(), shim.first.c_str()) == 0)
						{
							shim.second(module);

							trace("Executed compat patches on %s (loaded at %016llx).\n", ToNarrow(shim.first), (uint64_t)module);
						}
					}
				}
			}
			it++;
		}
	}
	catch (const std::exception&) {}

	return true;
}

bool ComponentInstance::Shutdown()
{
	return true;
}

extern "C" __declspec(dllexport) Component* CreateComponent()
{
	return new ComponentInstance();
}
