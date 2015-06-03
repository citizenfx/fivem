/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ToolComponentHelpers.h"

#include "IteratorView.h"

#include <boost/filesystem.hpp>

#define RAGE_FORMATS_GAME ny
#define RAGE_FORMATS_GAME_NY
#include <gtaDrawable.h>
#include <phBound.h>

#undef RAGE_FORMATS_GAME_NY
#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#include <gtaDrawable.h>
#include <phBound.h>

#include <convert/gtaDrawable_ny_five.h>
#include <convert/phBound_ny_five.h>

rage::ny::BlockMap* UnwrapRSC5(const wchar_t* fileName);

static void ConvertFile(const boost::filesystem::path& path)
{
	std::wstring fileNameStr = path.wstring();
	const wchar_t* fileName = fileNameStr.c_str();

	rage::ny::BlockMap* bm = UnwrapRSC5(fileName);

	if (!bm)
	{
		wprintf(L"couldn't open input file %s...\n", path.filename().c_str());
		return;
	}

	std::wstring fileExt = std::wstring(wcsrchr(fileName, L'.'));

	rage::ny::pgStreamManager::SetBlockInfo(bm);
	auto bm2 = rage::five::pgStreamManager::BeginPacking();

	int fileVersion = 0;

	if (fileExt == L".wbn")
	{
		wprintf(L"converting bound %s...\n", path.filename().c_str());

		auto bound = (rage::ny::datOwner<rage::ny::phBound>*)bm->blocks[0].data;
		rage::convert<rage::five::phBound*>(bound->GetChild());

		fileVersion = 43;
	}
	else if (fileExt == L".wdr")
	{
		wprintf(L"converting drawable %s...\n", path.filename().c_str());

		auto drawable = (rage::ny::gtaDrawable*)bm->blocks[0].data;
		rage::convert<rage::five::gtaDrawable*>(drawable);

		fileVersion = 165;
	}
	else if (fileExt == L".wtd")
	{
		wprintf(L"converting txd %s...\n", path.filename().c_str());

		auto txd = (rage::ny::pgDictionary<rage::ny::grcTexturePC>*)bm->blocks[0].data;
		rage::convert<rage::five::pgDictionary<rage::five::grcTexturePC>*>(txd);

		fileVersion = 13;
	}
	else
	{
		printf("unknown file extension...\n");

		return;
	}

	rage::five::pgStreamManager::EndPacking();

	std::wstring outFileName(fileName);
	outFileName = outFileName.substr(0, outFileName.length() - 3) + L"y" + fileExt.substr(2);

	FILE* f = _wfopen(outFileName.c_str(), L"wb");

	if (!f)
	{
		printf("... couldn't open output file for writing.\n");
		return;
	}

	size_t outputSize = 0;

	bm2->Save(fileVersion, [&] (const void* d, size_t s)
	{
		fwrite(d, 1, s, f);

		outputSize += s;
	});

	printf("written successfully - compressed size %d\n", outputSize);

	fclose(f);

	for (int i = 0; i < bm->physicalLen + bm->virtualLen; i++)
	{
		delete bm->blocks[i].data;
	}

	delete bm;

}

static void FormatsConvert_HandleArguments(boost::program_options::wcommand_line_parser& parser, std::function<void()> cb)
{
	boost::program_options::options_description desc;

	desc.add_options()
		("filename", boost::program_options::value<std::vector<boost::filesystem::path>>()->required(), "The path of the file to convert.");

	boost::program_options::positional_options_description positional;
	positional.add("filename", -1);

	parser.options(desc).
		positional(positional);	

	cb();
}

static void FormatsConvert_Run(const boost::program_options::variables_map& map)
{
	if (map.count("filename") == 0)
	{
		printf("Usage:\n\n   fivem formats:convert filename<1-n>...\n\nCurrently, GTA:NY static bounds (wbn), drawables (wdr) and texture dictionaries (wtd) are supported.\nSee your vendor for details.\n");
		return;
	}

	auto& entries = map["filename"].as<std::vector<boost::filesystem::path>>();

	for (auto& filePath : entries)
	{
		ConvertFile(filePath);
	}
}

static FxToolCommand formatsConvert("formats:convert", FormatsConvert_HandleArguments, FormatsConvert_Run);