/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#pragma once

#define NSIS_MAX_STRLEN 1024
#define NSIS_CONFIG_VISIBLE_SUPPORT
#define NSIS_SUPPORT_CODECALLBACKS
#define NSIS_SUPPORT_RENAME
#define NSIS_SUPPORT_FNUTIL
#define NSIS_SUPPORT_FILE
#define NSIS_SUPPORT_DELETE
#define NSIS_SUPPORT_MESSAGEBOX
#define NSIS_SUPPORT_RMDIR
#define NSIS_SUPPORT_STROPTS
#define NSIS_SUPPORT_ENVIRONMENT
#define NSIS_SUPPORT_INTOPTS
#define NSIS_SUPPORT_STACK
#include "nsis_fileform.h"

struct InstallerInterface
{
	// getters
	std::function<std::vector<section>()> getSections;

	// process a section and call the callback for every entry
	std::function<void(section, std::function<void(const entry& entry)>)> processSection;

	// add a file to the index
	std::function<void(const entry& entry, const std::wstring& outPath)> addFile;

	// get a string
	std::function<std::wstring(int)> getString;
};

bool ExtractInstallerFile(const std::wstring& installerFile, const std::function<void(const InstallerInterface&)>& fileFindCallback);