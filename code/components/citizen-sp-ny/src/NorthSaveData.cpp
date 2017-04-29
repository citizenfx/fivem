/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

#include <shlwapi.h>
#include <shlobj.h>

#pragma comment(lib, "shlwapi.lib")

static void AppendPathComponent(std::wstring& value, const wchar_t* component)
{
	value += component;

	if (GetFileAttributes(value.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		CreateDirectoryW(value.c_str(), nullptr);
	}
}

// get a xliveless-compatible save game directory
static std::wstring GetOriginalSavePath()
{
	PWSTR documentsPath;

	// get the Documents folder
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &documentsPath)))
	{
		// put it into a string and free it
		std::wstring savePath(documentsPath);

		CoTaskMemFree(documentsPath);
		
		// append the R* bits
		AppendPathComponent(savePath, L"\\Rockstar Games");
		AppendPathComponent(savePath, L"\\GTA IV");
		AppendPathComponent(savePath, L"\\savegames");

		// append a final separator
		savePath += L"\\";

		// and return the path
		return savePath;
	}

	// if not working, panic
	FatalError("Could not get Documents folder path for save games.");
}

// get our Citizen save game directory
static std::wstring GetCitizenSavePath()
{
	PWSTR saveBasePath;

	// get the 'Saved Games' shell directory
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_SavedGames, 0, nullptr, &saveBasePath)))
	{
		// create a STL string and free the used memory
		std::wstring savePath(saveBasePath);

		CoTaskMemFree(saveBasePath);

		// append our path components
		AppendPathComponent(savePath, L"\\CitizenFX");
		AppendPathComponent(savePath, L"\\GTA4");

		// append a final separator
		savePath += L"\\";

		// and return the path
		return savePath;
	}

	return GetOriginalSavePath();
}

static bool GetSavePathFunc(int, char* buffer, const char* fileName)
{
	// get the base directory
	std::wstring savePath = GetCitizenSavePath();

	// and the save path
	wcstombs(buffer, savePath.c_str(), MAX_PATH - 1);
	buffer[MAX_PATH] = '\0';

	// and PathAppend to it
	PathAppendA(buffer, fileName);
	
	return true;
}

static void MigrateSaveGames()
{
	std::wstring origSavePath = GetOriginalSavePath();
	std::wstring citizenSavePath = GetCitizenSavePath();

	if (origSavePath == citizenSavePath)
	{
		return;
	}

	for (int i = 0; i <= 14; i++)
	{
		// get both filenames
		std::wstring origFileName = va(L"%sSGTA4%02d", origSavePath.c_str(), i);
		std::wstring citizenFileName = va(L"%sSGTA4%02d", citizenSavePath.c_str(), i);
		
		// if the original file exists
		if (GetFileAttributes(origFileName.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			// and if the Citizen file doesn't exist...
			if (GetFileAttributes(citizenFileName.c_str()) == INVALID_FILE_ATTRIBUTES)
			{
				// copy the original file to the Citizen directory
				CopyFile(origFileName.c_str(), citizenFileName.c_str(), TRUE);
			}
		}
	}
}

static void __declspec(naked) DoLoadGame()
{
	__asm
	{
		mov dword ptr [esp + 4 + 8], 1

		push 4205B0h
		retn
	}
}

bool g_preventSaveLoading = false;

static void AttemptSaveLoadOnFrame()
{
	if (!g_preventSaveLoading)
	{
		((void(*)())0x421160)();
	}
}

static HookFunction hookFunction([] ()
{
	// patch the save game directory to point to 'our' directory
	hook::jump(0x5B0110, GetSavePathFunc);

	// copy any saved games from the original directory to our directory, making sure to not overwrite any existent saved games
	MigrateSaveGames();

	// make save loading pass 'true' for 'reload even if episode did not change' - not reloading breaks ModelInfoParents, though reloading might break other things (no double reload)?
	//hook::call(0x4211A9, DoLoadGame);

	// save game loading 'switcharoo' with the 'pause game' flag second meaning of the 'load save' flag
	hook::call(0x4213B7, AttemptSaveLoadOnFrame);
});