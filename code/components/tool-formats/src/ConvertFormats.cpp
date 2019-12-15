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

#define RAGE_FORMATS_GAME payne
#define RAGE_FORMATS_GAME_PAYNE
#include <gtaDrawable.h>
#include <phBound.h>

#undef RAGE_FORMATS_GAME_PAYNE
#undef RAGE_FORMATS_GAME
#define RAGE_FORMATS_GAME ny
#define RAGE_FORMATS_GAME_NY
#include <gtaDrawable.h>
#include <phBound.h>
#include <fragType.h>

#undef RAGE_FORMATS_GAME_NY
#undef RAGE_FORMATS_GAME
#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#include <gtaDrawable.h>
#include <phBound.h>
#include <fragType.h>

#undef RAGE_FORMATS_GAME_FIVE
#undef RAGE_FORMATS_GAME
#define RAGE_FORMATS_GAME rdr3
#define RAGE_FORMATS_GAME_RDR3
#include <gtaDrawable.h>
#include <phBound.h>
#include <fragType.h>

#include <convert/gtaDrawable_ny_five.h>
#include <convert/phBound_ny_five.h>

#include <convert/gtaDrawable_payne_five.h>
#include <convert/phBound_payne_five.h>

#include <convert/gtaDrawable_rdr3_five.h>
#include <convert/phBound_rdr3_five.h>

#include <optional>

static std::optional<boost::filesystem::path> g_wbnFile;

namespace rage::ny
{
	rage::ny::BlockMap* UnwrapRSC5(const wchar_t* fileName);
}

namespace rage::payne
{
	rage::payne::BlockMap* UnwrapRSC5(const wchar_t* fileName);
}

namespace rage::rdr3
{
	rage::rdr3::BlockMap* UnwrapRSC8(const wchar_t* fileName);
}

template<typename T>
static bool OutputFile(const T&& callback, int fileVersion, const std::wstring& fileName)
{
	auto bm = rage::five::pgStreamManager::BeginPacking();

	callback();

	rage::five::pgStreamManager::EndPacking();

	FILE* f = _wfopen(fileName.c_str(), L"wb");

	if (!f)
	{
		printf("... couldn't open output file for writing.\n");
		return false;
	}

	size_t outputSize = 0;

	bm->Save(fileVersion, [&](const void* d, size_t s)
	{
		fwrite(d, 1, s, f);

		outputSize += s;
	});

	wprintf(L"written %s successfully - compressed size %d\n", boost::filesystem::path(fileName).filename().c_str(), outputSize);

	fclose(f);

	return true;
}

template<typename TOutput, typename TInput, typename TBlockMap>
static bool AutoConvert(TBlockMap blockMap, const std::wstring& fileName, int fileVersion, const wchar_t* fileExtOverride = nullptr, const std::function<void(TOutput*)>& postCb = {})
{
	std::wstring fileExt = std::wstring(wcsrchr(fileName.c_str(), L'.'));

	if (fileExtOverride)
	{
		fileExt = fileExtOverride;
	}

	std::wstring outFileName(fileName);
	outFileName = outFileName.substr(0, outFileName.find_last_of('.')) + L".y" + fileExt.substr(2);
	
	return OutputFile([&]()
	{
		auto tgt = rage::convert<TOutput*>((TInput*)blockMap->blocks[0].data);

		if (postCb)
		{
			postCb(tgt);
		}
	}, fileVersion, outFileName);
}

static void ConvertBoundDict(rage::ny::pgDictionary<rage::ny::phBound>* dictionary, const std::wstring& fileName)
{
	std::wstring dirName = boost::filesystem::path(fileName).parent_path().wstring();

	if (!dirName.empty())
	{
		dirName += L"\\";
	}

	if (dictionary->GetCount())
	{
		for (auto& bound : *dictionary)
		{
			uint32_t hash = bound.first;
			
			OutputFile([&]()
			{
				rage::convert<rage::five::phBoundComposite*>(bound.second);
			}, 43, fmt::sprintf(L"%s0x%08x.ybn", dirName, hash));
		}
	}
}

struct GameConfig_NY
{
	static auto UnwrapRSC(const wchar_t* fileName)
	{
		return rage::ny::UnwrapRSC5(fileName);
	}

	using StreamManager = rage::ny::pgStreamManager;
	using TBound = rage::ny::datOwner<rage::ny::phBound>;
	using TDrawable = rage::ny::gtaDrawable;
	using TTxd = rage::ny::pgDictionary<rage::ny::grcTexturePC>;
	using TDwd = rage::ny::pgDictionary<rage::ny::gtaDrawable>;
};

struct GameConfig_Payne
{
	static auto UnwrapRSC(const wchar_t* fileName)
	{
		return rage::payne::UnwrapRSC5(fileName);
	}

	using StreamManager = rage::payne::pgStreamManager;
	using TBound = rage::payne::phBound;
	using TDrawable = rage::payne::gtaDrawable;
	using TTxd = rage::payne::pgDictionary<rage::payne::grcTexturePC>;
	using TDwd = rage::payne::pgDictionary<rage::payne::gtaDrawable>;
};

struct GameConfig_RDR3
{
	static auto UnwrapRSC(const wchar_t* fileName)
	{
		return rage::rdr3::UnwrapRSC8(fileName);
	}

	using StreamManager = rage::rdr3::pgStreamManager;
	using TBound = rage::rdr3::phBound;
	using TDrawable = rage::rdr3::gtaDrawable;
	using TTxd = rage::rdr3::pgDictionary<rage::rdr3::grcTexturePC>;
	using TDwd = rage::rdr3::pgDictionary<rage::rdr3::gtaDrawable>;
};

template<typename TConfig>
static void ConvertFile(const boost::filesystem::path& path)
{
	std::wstring fileNameStr = path.wstring();
	const wchar_t* fileName = fileNameStr.c_str();

	auto bm = TConfig::UnwrapRSC(fileName);

	if (!bm)
	{
		wprintf(L"couldn't open input file %s...\n", path.filename().c_str());
		return;
	}

	std::wstring fileExt = std::wstring(wcsrchr(fileName, L'.'));

	TConfig::StreamManager::SetBlockInfo(bm);

	int fileVersion = 0;

	if (fileExt == L".wbn" || fileExt == L".obn")
	{
		wprintf(L"converting bound %s...\n", path.filename().c_str());

		AutoConvert<rage::five::phBoundComposite, typename TConfig::TBound>(bm, fileName, 43);
	}
	else if (fileExt == L".wbd")
	{
		wprintf(L"converting bound dictionary %s...\n", path.filename().c_str());

		ConvertBoundDict((rage::ny::pgDictionary<rage::ny::phBound>*)bm->blocks[0].data, fileName);
		//AutoConvert<rage::five::pgDictionary<rage::five::phBound>, rage::ny::pgDictionary<rage::ny::phBound>>(bm, fileName, 43);
	}
	else if (fileExt == L".wft")
	{
		wprintf(L"converting fragment type %s...\n", path.filename().c_str());

		AutoConvert<rage::five::gtaDrawable, rage::ny::fragType>(bm, fileName, 162, L".ydr");
	}
	else if (fileExt == L".wdr" || fileExt == L".odr")
	{
		wprintf(L"converting drawable %s...\n", path.filename().c_str());

		if (g_wbnFile)
		{
			AutoConvert<rage::five::gtaDrawable, rage::ny::gtaDrawable>(bm, fileName, 165, nullptr, [&](rage::five::gtaDrawable* dr)
			{
				auto wbd = TConfig::UnwrapRSC((*g_wbnFile).wstring().c_str());
				auto bd = (rage::ny::pgDictionary<rage::ny::phBound>*)wbd->blocks[0].data;

				TConfig::StreamManager::SetBlockInfo(wbd);

				std::string baseName = boost::filesystem::path(fileName).filename().string();
				baseName = baseName.substr(0, baseName.find_last_of('.'));

				auto ivb = bd->Get(baseName.c_str());

				if (ivb)
				{
					auto bound = rage::convert<rage::five::phBoundComposite*>(ivb);

					dr->SetBound(bound);
				}

				for (int i = 0; i < wbd->physicalLen + wbd->virtualLen; i++)
				{
					delete wbd->blocks[i].data;
				}

				delete wbd;
			});
		}
		else
		{
			AutoConvert<rage::five::gtaDrawable, typename TConfig::TDrawable>(bm, fileName, 165);
		}
	}
	else if (fileExt == L".wdd")
	{
		wprintf(L"converting drawable dictionary %s...\n", path.filename().c_str());

		AutoConvert<rage::five::pgDictionary<rage::five::gtaDrawable>, typename TConfig::TDwd>(bm, fileName, 165);
	}
	else if (fileExt == L".wtd" || fileExt == L".otd")
	{
		wprintf(L"converting txd %s...\n", path.filename().c_str());

		AutoConvert<rage::five::pgDictionary<rage::five::grcTexturePC>, typename TConfig::TTxd>(bm, fileName, 13);
	}
	else
	{
		printf("unknown file extension...\n");

		return;
	}

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
		("filename", boost::program_options::wvalue<std::vector<boost::filesystem::path>>()->required(), "The path of the file to convert.")
		("wbd_file", boost::program_options::wvalue<boost::filesystem::path>(), "A .wbn file to use for drawable bounds.")
		("bound_x", boost::program_options::wvalue<float>(), "The X offset to use for converting static bounds.")
		("bound_y", boost::program_options::wvalue<float>(), "The Y offset to use for converting static bounds.")
		("game", boost::program_options::wvalue<std::string>()->default_value("ny"), "The game ID to use. Currently supported: [ny, payne].");

	boost::program_options::positional_options_description positional;
	positional.add("filename", -1);

	parser.options(desc).
		positional(positional);	

	cb();
}

#include <zlib.h>

rage::five::BlockMap* UnwrapRSC7(const wchar_t* fileName, rage::five::ResourceFlags* flags)
{
    FILE* f = _wfopen(fileName, L"rb");

    if (!f)
    {
        return nullptr;
    }

    fseek(f, 0, SEEK_END);
    size_t fileLength = ftell(f) - 16;
    fseek(f, 0, SEEK_SET);

    uint32_t magic;
    fread(&magic, 1, sizeof(magic), f);

    if (magic != 0x37435352)
    {
        printf("that's not a RSC7, you silly goose...\n");

        fclose(f);
        return nullptr;
    }

    uint32_t version;
    fread(&version, 1, sizeof(version), f);

	flags->version = version;

    /*if (version != 165)
    {
        printf("not actually a supported file...\n");

        fclose(f);
        return nullptr;
    }*/

    uint32_t flag;

	fread(&flag, 1, sizeof(flag), f);
	flags->virtualFlag = flag;

    uint32_t virtualSize = ((((flag >> 17) & 0x7f) + (((flag >> 11) & 0x3f) << 1) + (((flag >> 7) & 0xf) << 2) + (((flag >> 5) & 0x3) << 3) + (((flag >> 4) & 0x1) << 4)) * (0x2000 << (flag & 0xF)));

    fread(&flag, 1, sizeof(flag), f);
	flags->physicalFlag = flag;

    uint32_t physicalSize = ((((flag >> 17) & 0x7f) + (((flag >> 11) & 0x3f) << 1) + (((flag >> 7) & 0xf) << 2) + (((flag >> 5) & 0x3) << 3) + (((flag >> 4) & 0x1) << 4)) * (0x2000 << (flag & 0xF)));

    std::vector<uint8_t> tempBytes(virtualSize + physicalSize);

    {
        std::vector<uint8_t> tempInBytes(fileLength);
        fread(&tempInBytes[0], 1, fileLength, f);

        size_t destLength = tempBytes.size();
        
        {
            z_stream stream;
            int err;

            stream.next_in = (z_const Bytef *)&tempInBytes[0];
            stream.avail_in = (uInt)tempInBytes.size();

            stream.next_out = &tempBytes[0];
            stream.avail_out = (uInt)destLength;

            stream.zalloc = (alloc_func)0;
            stream.zfree = (free_func)0;

            err = inflateInit2(&stream, -15);
            if (err != Z_OK) return nullptr;

            err = inflate(&stream, Z_FINISH);
            /*if (err != Z_STREAM_END) {
                inflateEnd(&stream);
                if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
                    return nullptr;
                return nullptr;
            }*/
            destLength = stream.total_out;

            err = inflateEnd(&stream);
        }
        //uncompress(&tempBytes[0], (uLongf*)&destLength, &tempInBytes[0], tempInBytes.size());
    }

    char* virtualData = new char[virtualSize + physicalSize];
    memcpy(virtualData, &tempBytes[0], virtualSize + physicalSize);

    char* physicalData = new char[physicalSize];
    memcpy(physicalData, &tempBytes[virtualSize], physicalSize);

    auto bm = new rage::five::BlockMap();
    bm->physicalLen = 1;
    bm->virtualLen = 1;

    bm->blocks[0].data = virtualData;
    bm->blocks[0].offset = 0;
    bm->blocks[0].size = virtualSize + physicalSize;

    bm->blocks[1].data = physicalData;
    bm->blocks[1].offset = 0;
    bm->blocks[1].size = physicalSize;

    return bm;
}

static void FormatsConvert_Run(const boost::program_options::variables_map& map)
{
	if (map.count("filename") == 0)
	{
		printf("Usage:\n\n   fivem formats:convert filename<1-n>...\n\nCurrently, GTA:NY static bounds (wbn), drawables (wdr) and texture dictionaries (wtd) are supported.\nSee your vendor for details.\n");
		return;
	}

	static_assert(sizeof(rage::five::CLightAttr) == 0xA8, "lightattr size");

	{
		auto it = map.find("wbd_file");

		if (it != map.end())
		{
			g_wbnFile = it->second.as<boost::filesystem::path>();
		}
	}

	{
		auto it = map.find("bound_x");

		if (it != map.end())
		{
			g_boundOffset[0] = it->second.as<float>();
		}

		it = map.find("bound_y");

		if (it != map.end())
		{
			g_boundOffset[1] = it->second.as<float>();
		}
	}

	auto& entries = map["filename"].as<std::vector<boost::filesystem::path>>();

	for (auto& filePath : entries)
	{
		if (map["game"].as<std::wstring>() == L"ny")
		{
			ConvertFile<GameConfig_NY>(filePath);
		}
		else if (map["game"].as<std::wstring>() == L"payne")
		{
			ConvertFile<GameConfig_Payne>(filePath);
		}
		else if (map["game"].as<std::wstring>() == L"rdr3")
		{
			ConvertFile<GameConfig_RDR3>(filePath);
		}
	}
}

static FxToolCommand formatsConvert("formats:convert", FormatsConvert_HandleArguments, FormatsConvert_Run);
