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

#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE

#include <fragType.h>

rage::five::BlockMap* UnwrapRSC7(const wchar_t* fileName, rage::five::ResourceFlags* flags);

static void RunFragment(const boost::filesystem::path& path)
{
	std::wstring fileNameStr = path.wstring();
	const wchar_t* fileName = fileNameStr.c_str();

	rage::five::ResourceFlags flags;
	rage::five::BlockMap* bm = UnwrapRSC7(fileName, &flags);

	if (!bm)
	{
		wprintf(L"couldn't open input file %s...\n", path.filename().c_str());
		return;
	}

	rage::five::pgStreamManager::SetBlockInfo(bm);

	auto fragType = (rage::five::fragType*)bm->blocks[0].data;

	{
		std::vector<rage::five::rmcDrawable*> drawables;
		drawables.push_back(fragType->GetPrimaryDrawable());

		for (size_t i = 0; i < fragType->GetNumDrawables(); i++)
		{
			drawables.push_back(fragType->GetDrawable(i));
		}

		auto physLod = fragType->GetLodGroup()->GetLod(0);

		for (size_t i = 0; i < physLod->GetNumChildren(); i++)
		{
			drawables.push_back(physLod->GetChild(i)->GetDrawable());

			auto dr = physLod->GetChild(i)->GetDrawable2();

			if (dr)
			{
				drawables.push_back(physLod->GetChild(i)->GetDrawable2());
			}
		}

		for (auto& dr : drawables)
		{
			const auto& mp = dr->GetLodGroup().GetMaxPoint();

			if (dr->GetLodGroup().GetModel(1) != nullptr)
			{
				dr->GetLodGroup().SetModel(1, nullptr);
				dr->GetLodGroup().SetModel(2, nullptr);
			}
		}
	}

	auto p2 = path;
	p2 = p2.replace_extension(".new.yft");

	FILE* f = _wfopen(p2.wstring().c_str(), L"wb");

	if (!f)
	{
		printf("... couldn't open output file for writing.\n");
		return;
	}

	size_t outputSize = 0;

	bm->Save(162, [&](const void* d, size_t s)
	{
		fwrite(d, 1, s, f);

		outputSize += s;
	}, &flags);

	printf("written successfully - out size %d\n", outputSize);

	fclose(f);

	for (int i = 0; i < bm->physicalLen + bm->virtualLen; i++)
	{
		delete bm->blocks[i].data;
	}

	delete bm;
}

static void HandleArguments(boost::program_options::wcommand_line_parser& parser, std::function<void()> cb)
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

static void Run(const boost::program_options::variables_map& map)
{
	if (map.count("filename") == 0)
	{
		printf("Usage:\n\n   fivem formats:fraglod *.yft...\n");
		return;
	}

	auto& entries = map["filename"].as<std::vector<boost::filesystem::path>>();

	for (auto& filePath : entries)
	{
		RunFragment(filePath);
	}
}

static FxToolCommand command("formats:fraglod", HandleArguments, Run);