/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/
#include "StdInc.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "Hooking.h"
#include "DrawCommands.h"

static HMODULE LoadLibraryAHook(const char* libName)
{
	if (strcmp(libName, "dxgi.dll") == 0)
	{
		auto refPath = MakeRelativeGamePath(L"dxgi.dll");

		if (GetFileAttributes(refPath.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			auto hMod = LoadLibraryW(refPath.c_str());

			if (hMod)
			{
				return hMod;
			}
		}
	}

	return LoadLibraryA(libName);
}

/*
	The solution to the LoopBackTCP error is to load this before the game is started...
*/
void ScanForReshades() {
	bool found = false;

	try {
		boost::filesystem::path game_path(MakeRelativeGamePath("").c_str());
		boost::filesystem::directory_iterator it(game_path), end;
		std::vector<wchar_t*> reshadeFiles = std::vector<wchar_t*>({
			L"d3d8.dll",
			L"d3d9.dll",
			L"d3d10.dll",
			L"d3d11.dll",
			L"dxgi.dll"
		});

		// Try loading all dll files in the directory, that are in the list
		while (it != end){
			if (it->path().extension() == ".dll") {
				for (auto itt = reshadeFiles.begin(); itt != reshadeFiles.end(); ++itt) {
					if (*itt != nullptr && *itt != L"") {
						if (wcsicmp(it->path().filename().c_str(), *itt) == 0) {
							LoadLibrary(it->path().c_str()); //I would put a break here but what if they also have enbseries?
							trace("Loaded %s!\n", it->path().filename().string());

							found = true;
						}
					}
				}
			}
			it++;
		}
	}
	catch (...) {}

	if (found)
	{
		hook::iat("kernel32.dll", LoadLibraryAHook, "LoadLibraryA");
	}
}

static HookFunction initFunction([]() {
	ScanForReshades();
});
