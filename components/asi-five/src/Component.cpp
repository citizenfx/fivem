/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ComponentLoader.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <wchar.h>

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

bool ComponentInstance::DoGameLoad(void* module)
{
	HookFunction::RunAll();

	try
	{
		boost::filesystem::path plugins_path(MakeRelativeCitPath(L"plugins"));
		boost::filesystem::directory_iterator it(plugins_path), end;

		// if the directory doesn't exist, we create it
		if (!boost::filesystem::exists(plugins_path))
		{
			boost::filesystem::create_directory(plugins_path);
		}
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

				for (auto itt = blacklistedAsis.begin(); itt != blacklistedAsis.end(); ++itt){
					
					if (*itt != L"")
					{
						if (wcsicmp(it->path().filename().c_str(), itt->c_str()) == 0 || wcsicmp(badFileName.c_str(), itt->c_str()) == 0) {
							bad = true;
							trace("Skipping blacklisted ASI %s - this plugin is not compatible with FiveReborn", it->path().filename().string());
							if (*itt == L"openiv.asi")
							{
								FatalError("You cannot use OpenIV with FiveReborn. Please use clean game RPFs and remove OpenIV.asi from your plugins. Check fivereborn.com on how to use modded files with FiveReborn.");
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

							trace("Executed compat patches on %s (loaded at %016llx).", ToNarrow(shim.first), (uint64_t)module);
						}
					}
				}
			}
			it++;
		}
	}
	catch (...) {}

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