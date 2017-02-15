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
	else if (fileExt == L".wdd")
	{
		wprintf(L"converting drawable dictionary %s...\n", path.filename().c_str());

		auto drawable = (rage::ny::pgDictionary<rage::ny::gtaDrawable>*)bm->blocks[0].data;
		rage::convert<rage::five::pgDictionary<rage::five::gtaDrawable>*>(drawable);

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

	auto& entries = map["filename"].as<std::vector<boost::filesystem::path>>();

	for (auto& filePath : entries)
	{
		ConvertFile(filePath);
	}
}

static FxToolCommand formatsConvert("formats:convert", FormatsConvert_HandleArguments, FormatsConvert_Run);