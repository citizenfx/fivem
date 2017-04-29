/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ToolComponentHelpers.h"

#include <boost/filesystem/path.hpp>

static void Subprocess_HandleArguments(boost::program_options::wcommand_line_parser& parser, std::function<void()> cb)
{
	boost::program_options::options_description desc;

	desc.add_options()
		("cake", boost::program_options::value<std::vector<std::string>>()->required(), "");

	boost::program_options::positional_options_description positional;
	positional.add("cake", -1);

	parser.options(desc).
		   positional(positional).
		   allow_unregistered();

	cb();
}

std::wstring g_origProcess;

static void Subprocess_Run(const boost::program_options::variables_map& map)
{
	auto args = map["cake"].as<std::vector<std::string>>();

	boost::filesystem::path programPath(args[0]);

	auto parentPath = programPath.parent_path();
	SetCurrentDirectory(parentPath.wstring().c_str());

	trace("sub! %s\n", GetCommandLineA());

	g_origProcess = programPath.wstring();
	ToolMode_LaunchGame(programPath.wstring().c_str());
}

DWORD ToolGetModuleFileNameW(LPWSTR fileName, DWORD nSize)
{
	wcsncpy(fileName, g_origProcess.c_str(), nSize);

	return wcslen(fileName);
}

static FxToolCommand rosSubprocess("ros:subprocess", Subprocess_HandleArguments, Subprocess_Run);