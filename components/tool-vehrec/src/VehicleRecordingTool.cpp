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

#include <pgBase.h>
#include <pgContainers.h>

struct VehicleRecordingEntry : public rage::five::pgStreamableBase
{
	uint32_t time;
	uint16_t vel[3];
	uint8_t right[3];
	uint8_t up[3];
	uint8_t steeringAngle;
	uint8_t gasPower;
	uint8_t brakePower;
	uint8_t handbrake;
	float pos[3];
};

class VehicleRecordingFile : public rage::five::pgBase
{
private:
	rage::five::pgArray<VehicleRecordingEntry> m_entries;

public:
	inline void SetEntries(uint16_t count, VehicleRecordingEntry* entries)
	{
		m_entries.SetFrom(entries, count);
	}
};

static void DoFile(const boost::filesystem::path& path)
{
	static_assert(sizeof(VehicleRecordingEntry) == 32, "size");

	std::wstring fileNameStr = path.wstring();
	const wchar_t* fileName = fileNameStr.c_str();

	FILE* f = _wfopen(fileName, L"rb");

	if (!f)
	{
		wprintf(L"couldn't open input file %s...\n", path.filename().c_str());
		return;
	}

	std::vector<VehicleRecordingEntry> entries;

	fseek(f, 0, SEEK_END);
	entries.resize(ftell(f) / sizeof(VehicleRecordingEntry));
	fseek(f, 0, SEEK_SET);

	fread(&entries[0], sizeof(VehicleRecordingEntry), entries.size(), f);

	fclose(f);

	auto bm = rage::five::pgStreamManager::BeginPacking();

	auto file = new(false) VehicleRecordingFile();
	file->SetBlockMap();
	file->SetEntries(entries.size(), &entries[0]);

	rage::five::pgStreamManager::EndPacking();

	std::wstring outFileName(fileName);
	outFileName = outFileName.substr(0, outFileName.length() - 3) + L"yvr";

	f = _wfopen(outFileName.c_str(), L"wb");

	if (!f)
	{
		printf("... couldn't open output file for writing.\n");
		return;
	}

	size_t outputSize = 0;

	bm->Save(1, [&](const void* d, size_t s)
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
		printf("Usage:\n\n   fivem tool:vehrec *.rrr...\n");
		return;
	}

	auto& entries = map["filename"].as<std::vector<boost::filesystem::path>>();

	for (auto& filePath : entries)
	{
		DoFile(filePath);
	}
}

static FxToolCommand command("tool:vehrec", HandleArguments, Run);