/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"

#if defined(LAUNCHER_PERSONALITY_MAIN) || defined(LAUNCHER_PERSONALITY_GAME) || defined(COMPILING_GLUE)
#include <CfxState.h>
#include <HostSharedData.h>
#include <CfxLocale.h>

#if defined(LAUNCHER_PERSONALITY_MAIN) || defined(COMPILING_GLUE)
#include <openssl/sha.h>
#endif

#include <KnownFolders.h>
#include <shlobj.h>

#undef interface
#include "InstallerExtraction.h"
#include <array>

#include <Error.h>

#if defined(GTA_FIVE) || defined(IS_RDR3) || defined(GTA_NY)
struct GameCacheEntry;

struct DeltaEntry
{
	std::array<uint8_t, 20> fromChecksum;
	std::array<uint8_t, 20> toChecksum;
	std::string remoteFile;
	uint64_t dlSize;

	std::string GetFileName() const;
	GameCacheEntry MakeEntry() const;

	inline std::wstring GetLocalFileName() const
	{
		return MakeRelativeCitPath(ToWide("data\\game-storage\\" + GetFileName()));
	}

	DeltaEntry(std::string_view fromChecksum, std::string_view toChecksum, const std::string& remoteFile, uint64_t dlSize);
};

// entry for a cached-intent file
struct GameCacheEntry
{
	// local filename to map from
	const char* filename;

	// checksum (SHA1, typically) to validate as
	std::vector<const char*> checksums;

	// remote path on ROS service to use
	const char* remotePath;

	// file to extract from any potential archive
	const char* archivedFile;

	// local size of the file
	size_t localSize;

	// remote size of the archive file
	size_t remoteSize;

	// delta sets
	std::vector<DeltaEntry> deltas;

	// constructor
	GameCacheEntry(const char* filename, const char* checksum, const char* remotePath, size_t localSize, std::initializer_list<DeltaEntry> deltas = {})
		: filename(filename), checksums({ checksum }), remotePath(remotePath), localSize(localSize), remoteSize(localSize), archivedFile(nullptr), deltas(deltas)
	{

	}

	GameCacheEntry(const char* filename, const char* checksum, const char* remotePath, size_t localSize, size_t remoteSize, std::initializer_list<DeltaEntry> deltas = {})
		: filename(filename), checksums({ checksum }), remotePath(remotePath), localSize(localSize), remoteSize(remoteSize), archivedFile(nullptr), deltas(deltas)
	{
	}

	GameCacheEntry(const char* filename, const char* checksum, const char* remotePath, const char* archivedFile, size_t localSize, size_t remoteSize, std::initializer_list<DeltaEntry> deltas = {})
		: filename(filename), checksums({ checksum }), remotePath(remotePath), localSize(localSize), remoteSize(remoteSize), archivedFile(archivedFile), deltas(deltas)
	{

	}

	GameCacheEntry(const char* filename, std::initializer_list<const char*> checksums, const char* remotePath, size_t localSize, std::initializer_list<DeltaEntry> deltas = {})
		: filename(filename), checksums(checksums), remotePath(remotePath), localSize(localSize), remoteSize(localSize), archivedFile(nullptr), deltas(deltas)
	{

	}

	GameCacheEntry(const char* filename, std::initializer_list<const char*> checksums, const char* remotePath, const char* archivedFile, size_t localSize, size_t remoteSize, std::initializer_list<DeltaEntry> deltas = {})
		: filename(filename), checksums(checksums), remotePath(remotePath), localSize(localSize), remoteSize(remoteSize), archivedFile(archivedFile), deltas(deltas)
	{

	}

	// methods
	bool IsPrimitiveFile() const
	{
		return std::string(filename).find("ros_") == 0 || std::string(filename).find("launcher/") == 0;
	}

	std::wstring GetCacheFileName() const
	{
		std::string filenameBase = filename;

		if (IsPrimitiveFile())
		{
			return MakeRelativeCitPath(ToWide(va("data\\game-storage\\%s", filenameBase.c_str())));
		}

		std::replace(filenameBase.begin(), filenameBase.end(), '/', '+');

		return MakeRelativeCitPath(ToWide(va("data\\game-storage\\%s_%s", filenameBase.c_str(), checksums[0])));
	}

	std::wstring GetRemoteBaseName() const
	{
		std::string remoteNameBase = remotePath;

		size_t slashIndex = remoteNameBase.find_last_of('/') + 1;

		return MakeRelativeCitPath(ToWide("data\\game-storage\\" + remoteNameBase.substr(slashIndex)));
	}

	std::wstring GetLocalFileName() const
	{
		using namespace std::string_literals;

		if (_strnicmp(filename, "launcher/", 9) == 0)
		{
			wchar_t rootBuf[1024] = { 0 };
			DWORD rootLength = sizeof(rootBuf);

			RegGetValue(HKEY_LOCAL_MACHINE,
				L"SOFTWARE\\WOW6432Node\\Rockstar Games\\Launcher", L"InstallFolder",
				RRF_RT_REG_SZ, nullptr, rootBuf, &rootLength);

			return rootBuf + L"\\"s + ToWide(&filename[9]);
		}

		if (_strnicmp(filename, "ros_", 4) == 0)
		{
			LPWSTR rootPath;
			if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, nullptr, &rootPath)))
			{
				std::wstring pathRef = rootPath;
				CoTaskMemFree(rootPath);

				return pathRef + L"\\Rockstar Games\\Social Club\\"s + ToWide(strchr(filename, L'/') + 1);
			}

			wchar_t rootBuf[1024] = { 0 };
			DWORD rootLength = sizeof(rootBuf);

			RegGetValue(HKEY_LOCAL_MACHINE,
				L"SOFTWARE\\WOW6432Node\\Rockstar Games\\Rockstar Games Social Club", L"InstallFolder",
				RRF_RT_REG_SZ, nullptr, rootBuf, &rootLength);

			return rootBuf + L"\\"s + ToWide(&filename[9]);
		}

		return MakeRelativeGamePath(ToWide(filename));
	}
};

GameCacheEntry DeltaEntry::MakeEntry() const
{
	return GameCacheEntry{ remoteFile.c_str(), "0000000000000000000000000000000000000000", remoteFile.c_str(), size_t(dlSize) };
}

std::string DeltaEntry::GetFileName() const
{
	std::basic_string_view<uint8_t> from{
		fromChecksum.data(), 20
	};

	std::basic_string_view<uint8_t> to{
		toChecksum.data(), 20
	};

	return fmt::sprintf("%x_%x", std::hash<decltype(from)>()(from), std::hash<decltype(to)>()(to));
}

struct GameCacheStorageEntry
{
	// sha1-sized file checksum
	uint8_t checksum[20];

	// file modification time
	time_t fileTime;
};

// global cache mapping of ROS files to disk files
static std::vector<GameCacheEntry> g_requiredEntries =
{
#if defined(GTA_FIVE)
	//{ "GTA5.exe", "79a272830be65afae4acd520bfbfe176e0b142f3", "https://runtime.fivem.net/patches/GTA_V_Patch_1_0_1737_0.exe", "$/GTA5.exe", 77631632, 1189307336 },
	//{ "update/update.rpf", "8a98e0879b91661dd2aa23f86abdf91ee741b237", "https://runtime.fivem.net/patches/GTA_V_Patch_1_0_1737_0.exe", "$/update/update.rpf", 1171902464, 1189307336 },

	//{ L"update/update.rpf", "c819ecc1df08f3a90bc144fce0bba08bb7b6f893", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfupdate.rpf", 560553984 },
	//{ "update/update.rpf", "319d867a44746885427d9c40262e9d735cd2a169", "Game_EFIGS/GTA_V_Patch_1_0_1011_1.exe", "$/update/update.rpf", 701820928, SIZE_MAX },
	{ "update/x64/dlcpacks/patchday4ng/dlc.rpf", "124c908d82724258a5721535c87f1b8e5c6d8e57", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday4ng/dlc.rpf", 312438784 },
	{ "update/x64/dlcpacks/mpluxe/dlc.rpf", "78f7777b49f4b4d77e3da6db728cb3f7ec51e2fc", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpluxe/dlc.rpf", 226260992 },

	{ "update/x64/dlcpacks/patchday5ng/dlc.rpf", "af3b2a59b4e1e5fd220c308d85753bdbffd8063c", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday5ng/dlc.rpf", 7827456 },
	{ "update/x64/dlcpacks/mpluxe2/dlc.rpf", "1e59e1f05be5dba5650a1166eadfcb5aeaf7737b", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpluxe2/dlc.rpf", 105105408 },

	{ "update/x64/dlcpacks/mpreplay/dlc.rpf", "f5375beef591178d8aaf334431a7b6596d0d793a", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpreplay/dlc.rpf", 429932544 },
	{ "update/x64/dlcpacks/patchday6ng/dlc.rpf", "5d38b40ad963a6cf39d24bb5e008e9692838b33b", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday6ng/dlc.rpf", 31907840 },

	{ "update/x64/dlcpacks/mphalloween/dlc.rpf", "3f960c014e83be00cf8e6b520bbf22f7da6160a4", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmphalloween/dlc.rpf", 104658944 },
	{ "update/x64/dlcpacks/mplowrider/dlc.rpf", "eab744fe959ca29a2e5f36843d259ffc9d04a7f6", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmplowrider/dlc.rpf", 1088813056 },
	{ "update/x64/dlcpacks/patchday7ng/dlc.rpf", "29df23f3539907a4e15f1cdb9426d462c1ad0337", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday7ng/dlc.rpf", 43843584 },
	
	//573
	{ "update/x64/dlcpacks/mpxmas_604490/dlc.rpf", "929e5b79c9915f40f212f1ed9f9783f558242c3d", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpxmas_604490/dlc.rpf", 46061568 },
	{ "update/x64/dlcpacks/mpapartment/dlc.rpf", "e1bed90e750848407f6afbe1db21aa3691bf9d82", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpapartment/dlc.rpf", 636985344 },
	{ "update/x64/dlcpacks/patchday8ng/dlc.rpf", "2f9840c20c9a93b48cfcf61e07cf17c684858e36", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday8ng/dlc.rpf", 365328384 },

	//617
	{ "update/x64/dlcpacks/mpjanuary2016/dlc.rpf", "4f0d5fa835254eb918716857a47e8ce63e158c22", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpjanuary2016/dlc.rpf", 149415936 },
	{ "update/x64/dlcpacks/mpvalentines2/dlc.rpf", "b1ef3b0e4741978b5b04c54c6eca8b475681469a", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpvalentines2/dlc.rpf", 25073664 },

	//678
	{ "update/x64/dlcpacks/mplowrider2/dlc.rpf", "6b9ac7b7b35b56208541692cf544788d35a84c82", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmplowrider2/dlc.rpf", 334028800 },
	{ "update/x64/dlcpacks/patchday9ng/dlc.rpf", "e29c191561d8fa4988a71be7be5ca9c6e1335537", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday9ng/dlc.rpf", 160524288 },

	//757
	{ "update/x64/dlcpacks/mpexecutive/dlc.rpf", "3fa67dd4005993c9a7a66879d9f244a55fea95e9", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpexecutive/dlc.rpf", 801568768 },
	{ "update/x64/dlcpacks/patchday10ng/dlc.rpf", "4140c1f56fd29b0364be42a11fcbccd9e345d6ff", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday10ng/dlc.rpf", 94134272 },

	//791 Cunning Stunts
	{ "update/x64/dlcpacks/mpstunt/dlc.rpf", "c5d338068f72685523a49fddfd431a18c4628f61", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpstunt/dlc.rpf", 348047360 },
	{ "update/x64/dlcpacks/patchday11ng/dlc.rpf", "7941a22c6238c065f06ff667664c368b6dc10711", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday11ng/dlc.rpf", 9955328 },

	//877 Bikers
	{ "update/x64/dlcpacks/mpbiker/dlc.rpf", "52c48252eeed97e9a30efeabbc6623c67566c237", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 1794048000 },
	{ "update/x64/dlcpacks/patchday12ng/dlc.rpf", "4f3f3e88d4f01760648057c56fb109e1fbeb116a", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday12ng/dlc.rpf", 155363328 },

	//DLCPacks12
	{ "update/x64/dlcpacks/mpimportexport/dlc.rpf", "019b1b433d9734ac589520a74dd451d72cbff051", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 915310592 },
	{ "update/x64/dlcpacks/patchday13ng/dlc.rpf", "4fe0ee843e83ef6a7a5f3352b4f6d7eb14d96e0f", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday12ng/dlc.rpf", 144752640 },

	//DLCPacks13
	{ "update/x64/dlcpacks/mpspecialraces/dlc.rpf", "de1a6f688fdf8965e7b9a92691ac34f9c9881742", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 78448640 },
	{ "update/x64/dlcpacks/patchday14ng/dlc.rpf", "078b683deb9b787e523093b9f3bc1bf5d3e7be09", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday12ng/dlc.rpf", 92930048 },

	//DLCPacks14
	{ "update/x64/dlcpacks/mpgunrunning/dlc.rpf", "153bee008c16e0bcc007d76cf97999f503fc9b2a", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 1879756800 },
	{ "update/x64/dlcpacks/patchday15ng/dlc.rpf", "6114122c428e901532ab6577ea7dbe2113126647", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday12ng/dlc.rpf", 47478784 },

	//DLCPacks15
	{ "update/x64/dlcpacks/mpsmuggler/dlc.rpf", "ac6a3501c6e5fc2ac06a60d1bc1bd3eb8683643b", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 973670400 },
	{ "update/x64/dlcpacks/patchday16ng/dlc.rpf", "37fae29af765ff0f2d7a5abd3d40d3a9ea7f357a", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday12ng/dlc.rpf", 12083200 },

	//DLCPacks16
	{ "update/x64/dlcpacks/mpchristmas2017/dlc.rpf", "16f8c031aa79f1e83b7f5ab883df3dbfcda8dddf", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 2406123520 },
	{ "update/x64/dlcpacks/patchday17ng/dlc.rpf", { "7dc8639f1ffa25b3237d01aea1e9975238628952", "c7163e1d8105c87b867b09928ea8346e26b27565", "7dc8639f1ffa25b3237d01aea1e9975238628952" }, "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday12ng/dlc.rpf", 59975680 },

	//DLCPacks17
	{ "update/x64/dlcpacks/mpassault/dlc.rpf", "7c65b096261dd88bd1f952fc6046626f1ca56215", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 314443776 },
	{ "update/x64/dlcpacks/patchday18ng/dlc.rpf", "9e16b7af4a1e58878f0dd16dd86cbd772a8ce9ef", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday12ng/dlc.rpf", 4405248 },

	//DLCPacks18
	{ "update/x64/dlcpacks/mpbattle/dlc.rpf", "80018257a637417b911bd4540938866ae95d0cf5", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 3981039616 },
	{ "update/x64/dlcpacks/mpbattle/dlc1.rpf", "b16fb76065132f5f9af4b2a92431b9f91b670542", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 992296960 },
	{ "update/x64/dlcpacks/patchday19ng/dlc.rpf", "3373311add1eb5ff850e1f3fbb7d15512cbc5b8b", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday12ng/dlc.rpf", 765630464 },

	//DLCPacks19m
	{ "update/x64/dlcpacks/mpchristmas2018/dlc.rpf", "c4cda116420f14a28e5a999740cc53cf53a950ec", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 3247781888 },
	{ "update/x64/dlcpacks/patchday20ng/dlc.rpf", "fbba396a0ede622e08f76c5ced8ac1d6839c0227", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday12ng/dlc.rpf", 457129984 },
#elif defined(IS_RDR3)
	{ "x64/dlcpacks/mp007/dlc.rpf", "f9d085bc889fc89d205c43a63d784d131be3ae8f", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 1425958473 },
	{ "x64/dlcpacks/patchpack007/dlc.rpf", "1847fa67af881ae8f6b88149948db6a181b698ac", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 58027818 },
#endif

#if defined(_M_AMD64)
	{ "launcher/api-ms-win-core-console-l1-1-0.dll", "724F4F91041AD595E365B724A0348C83ACF12BBB", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-console-l1-1-0.dll", 19208 },
	{ "launcher/api-ms-win-core-datetime-l1-1-0.dll", "4940D5B92B6B80A40371F8DF073BF3EB406F5658", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-datetime-l1-1-0.dll", 18696 },
	{ "launcher/api-ms-win-core-debug-l1-1-0.dll", "E7C8A6C29C3158F8B332EEA5C33C3B1E044B5F73", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-debug-l1-1-0.dll", 18696 },
	{ "launcher/api-ms-win-core-errorhandling-l1-1-0.dll", "51CBB7BA47802DC630C2507750432C55F5979C27", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-errorhandling-l1-1-0.dll", 18696 },
	{ "launcher/api-ms-win-core-file-l1-1-0.dll", "9ACBEEF0AC510C179B319CA69CD5378D0E70504D", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-file-l1-1-0.dll", 22280 },
	{ "launcher/api-ms-win-core-file-l1-2-0.dll", "04669214375B25E2DC8A3635484E6EEB206BC4EB", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-file-l1-2-0.dll", 18696 },
	{ "launcher/api-ms-win-core-file-l2-1-0.dll", "402B7B8F8DCFD321B1D12FC85A1EE5137A5569B2", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-file-l2-1-0.dll", 18696 },
	{ "launcher/api-ms-win-core-handle-l1-1-0.dll", "A2E2A40CEA25EA4FD64B8DEAF4FBE4A2DB94107A", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-handle-l1-1-0.dll", 18696 },
	{ "launcher/api-ms-win-core-heap-l1-1-0.dll", "B4310929CCB82DD3C3A779CAB68F1F9F368076F2", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-heap-l1-1-0.dll", 19208 },
	{ "launcher/api-ms-win-core-interlocked-l1-1-0.dll", "F779CDEF9DED19402AA72958085213D6671CA572", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-interlocked-l1-1-0.dll", 18696 },
	{ "launcher/api-ms-win-core-libraryloader-l1-1-0.dll", "47143A66B4A2E2BA019BF1FD07BCCA9CFB8BB117", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-libraryloader-l1-1-0.dll", 19720 },
	{ "launcher/api-ms-win-core-localization-l1-2-0.dll", "9874398548891F6A08FC06437996F84EB7495783", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-localization-l1-2-0.dll", 21256 },
	{ "launcher/api-ms-win-core-memory-l1-1-0.dll", "9C03356CF48112563BB845479F40BF27B293E95E", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-memory-l1-1-0.dll", 19208 },
	{ "launcher/api-ms-win-core-namedpipe-l1-1-0.dll", "CB59F1FE73C17446EB196FC0DD7D944A0CD9D81F", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-namedpipe-l1-1-0.dll", 18696 },
	{ "launcher/api-ms-win-core-processenvironment-l1-1-0.dll", "2745259F4DBBEFBF6B570EE36D224ABDB18719BC", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-processenvironment-l1-1-0.dll", 19720 },
	{ "launcher/api-ms-win-core-processthreads-l1-1-0.dll", "50699041060D14576ED7BACBD44BE9AF80EB902A", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-processthreads-l1-1-0.dll", 20744 },
	{ "launcher/api-ms-win-core-processthreads-l1-1-1.dll", "0BFFB9ED366853E7019452644D26E8E8F236241B", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-processthreads-l1-1-1.dll", 19208 },
	{ "launcher/api-ms-win-core-profile-l1-1-0.dll", "E7E0B18A40A35BD8B0766AC72253DE827432E148", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-profile-l1-1-0.dll", 18184 },
	{ "launcher/api-ms-win-core-rtlsupport-l1-1-0.dll", "24F37D46DFC0EF303EF04ABF9956241AF55D25C9", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-rtlsupport-l1-1-0.dll", 19208 },
	{ "launcher/api-ms-win-core-string-l1-1-0.dll", "637E4A9946691F76E6DEB69BDC21C210921D6F07", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-string-l1-1-0.dll", 18696 },
	{ "launcher/api-ms-win-core-synch-l1-1-0.dll", "5584C189216A17228CCA6CD07037AAA9A8603241", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-synch-l1-1-0.dll", 20744 },
	{ "launcher/api-ms-win-core-synch-l1-2-0.dll", "A9AEBBBB73B7B846B051325D7572F2398F5986EE", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-synch-l1-2-0.dll", 19208 },
	{ "launcher/api-ms-win-core-sysinfo-l1-1-0.dll", "F20AE25484A1C1B43748A1F0C422F48F092AD2C1", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-sysinfo-l1-1-0.dll", 19720 },
	{ "launcher/api-ms-win-core-timezone-l1-1-0.dll", "4BF13DB65943E708690D6256D7DDD421CC1CC72B", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-timezone-l1-1-0.dll", 19208 },
	{ "launcher/api-ms-win-core-util-l1-1-0.dll", "1E1A5AB47E4C2B3C32C81690B94954B7612BB493", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-core-util-l1-1-0.dll", 18696 },
	{ "launcher/api-ms-win-crt-conio-l1-1-0.dll", "49002B58CB0DF2EE8D868DEC335133CF225657DF", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-crt-conio-l1-1-0.dll", 19720 },
	{ "launcher/api-ms-win-crt-convert-l1-1-0.dll", "C84E41FDCC4CA89A76AE683CB390A9B86500D3CA", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-crt-convert-l1-1-0.dll", 22792 },
	{ "launcher/api-ms-win-crt-environment-l1-1-0.dll", "9A4818897251CACB7FE1C6FE1BE3E854985186AD", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-crt-environment-l1-1-0.dll", 19208 },
	{ "launcher/api-ms-win-crt-filesystem-l1-1-0.dll", "78FA03C89EA12FF93FA499C38673039CC2D55D40", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-crt-filesystem-l1-1-0.dll", 20744 },
	{ "launcher/api-ms-win-crt-heap-l1-1-0.dll", "60B4CF246C5F414FC1CD12F506C41A1043D473EE", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-crt-heap-l1-1-0.dll", 19720 },
	{ "launcher/api-ms-win-crt-locale-l1-1-0.dll", "9C1DF49A8DBDC8496AC6057F886F5C17B2C39E3E", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-crt-locale-l1-1-0.dll", 19208 },
	{ "launcher/api-ms-win-crt-math-l1-1-0.dll", "8B35EC4676BD96C2C4508DC5F98CA471B22DEED7", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-crt-math-l1-1-0.dll", 27912 },
	{ "launcher/api-ms-win-crt-multibyte-l1-1-0.dll", "91EEF52C557AEFD0FDE27E8DF4E3C3B7F99862F2", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-crt-multibyte-l1-1-0.dll", 26888 },
	{ "launcher/api-ms-win-crt-private-l1-1-0.dll", "0C33CFE40EDD278A692C2E73E941184FD24286D9", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-crt-private-l1-1-0.dll", 71432 },
	{ "launcher/api-ms-win-crt-process-l1-1-0.dll", "EC96F7BEEAEC14D3B6C437B97B4A18A365534B9B", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-crt-process-l1-1-0.dll", 19720 },
	{ "launcher/api-ms-win-crt-runtime-l1-1-0.dll", "A19ACEFA3F95D1B565650FDBC40EF98C793358E9", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-crt-runtime-l1-1-0.dll", 23304 },
	{ "launcher/api-ms-win-crt-stdio-l1-1-0.dll", "982B5DA1C1F5B9D74AF6243885BCBA605D54DF8C", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-crt-stdio-l1-1-0.dll", 24840 },
	{ "launcher/api-ms-win-crt-string-l1-1-0.dll", "7F389E6F2D6E5BEB2A3BAF622A0C0EA24BC4DE60", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-crt-string-l1-1-0.dll", 24840 },
	{ "launcher/api-ms-win-crt-time-l1-1-0.dll", "EE815A158BAACB357D9E074C0755B6F6C286B625", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-crt-time-l1-1-0.dll", 21256 },
	{ "launcher/api-ms-win-crt-utility-l1-1-0.dll", "EAA07829D012206AC55FB1AF5CC6A35F341D22BE", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/api-ms-win-crt-utility-l1-1-0.dll", 19208 },
	{ "launcher/Launcher.exe", "9AB0848E89FCAA7D1AB34BBC7E6B02461652950A", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/Launcher.exe", 42820736 },
	{ "launcher/Launcher.rpf", "A84622EE990F8CF16F15E4347926FB57C9747018", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/Launcher.rpf", 831488 },
	{ "launcher/mtl_libovr.dll", "3AADE10DBF3C51233AA701AD1E12CD17A9DCB722", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/mtl_libovr.dll", 190952 },
	{ "launcher/offline.pak", "4F52E60E1580CD5F44FFACFCDAEA6A78E08FC29D", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/offline.pak", 2087267 },
	{ "launcher/RockstarService.exe", "4C384774618ACE14700A13E7D35233B6F37B73A9", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/RockstarService.exe", 1631360 },
	{ "launcher/RockstarSteamHelper.exe", "0B957C26F151D33248CE0EBEA91BC9B51DB0B943", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/RockstarSteamHelper.exe", 1131136 },
	{ "launcher/ucrtbase.dll", "4189F4459C54E69C6D3155A82524BDA7549A75A6", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/ucrtbase.dll", 1016584 },
	{ "launcher/ThirdParty/Epic/EOSSDK-Win64-Shipping.dll", "AF01787DDB7DE00239EDC62D33E0B20C0BE80037", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/ThirdParty/Epic/EOSSDK-Win64-Shipping.dll", 9971968 },
	{ "launcher/ThirdParty/Steam/steam_api64.dll", "BD014660F7978A07BA2F99B6CF0621D678602663", "https://content.cfx.re/mirrors/emergency_mirror/launcher_1_0_33_319/launcher/ThirdParty/Steam/steam_api64.dll", 121256 },
	{ "ros_2075/cef_100_percent.pak", "22A96140286FDB004540A2051B93432AA133843D", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/cef_100_percent.pak", 737157 },
	{ "ros_2075/cef_200_percent.pak", "4BB734F61C04BFC68F7E15F128A2853A5F7649EA", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/cef_200_percent.pak", 864626 },
	{ "ros_2075/cef.pak", "09F37AFFF84B2445F0AFA8CBB803D53BADA62080", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/cef.pak", 3659767 },
	{ "ros_2075/chrome_elf.dll", "674B1C6F117F06A18A8B44EC4F35569B3DF048E0", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/chrome_elf.dll", 805848 },
	{ "ros_2075/d3dcompiler_43.dll", "C8A906AD2313E56C9F141398F54A04865045199D", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/d3dcompiler_43.dll", 2106328 },
	{ "ros_2075/d3dcompiler_47.dll", "6F30788B9CE5D8E044986F8F409938B6BC06D29B", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/d3dcompiler_47.dll", 4336600 },
	{ "ros_2075/icudtl.dat", "5CC62646E6C73B4BE276D08719BC5E257AF972BB", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/icudtl.dat", 10326688 },
	{ "ros_2075/libcef.dll", "003C35F14C6BF5EAB25099A3E4989D85B54895BC", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/libcef.dll.xz", 109920216, 35120472 },
	{ "ros_2075/libEGL.dll", "97B4D29104784D39BE41A4DAA66161E1D448DD6F", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/libEGL.dll", 146392 },
	{ "ros_2075/libGLESv2.dll", "C7E82DCBCA90784F1AFAEF4CA9CB7BB363239515", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/libGLESv2.dll", 5431256 },
	{ "ros_2075/natives_blob.bin", "9D60E499A907811A3155E9A07F8645D6C83CB909", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/natives_blob.bin", 83328 },
	{ "ros_2075/scui.pak", "3A03DFA2CECF1E356EB8D080443069ED35A897F1", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/scui.pak", 3401985 },
	{ "ros_2075/snapshot_blob.bin", "49038CDCB26B4DCC37F017787265180478E894B5", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/snapshot_blob.bin", 287384 },
	{ "ros_2075/socialclub.dll", "AACDB1573806E153BAC88F481F48CDC8779E54DE", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/socialclub.dll", 5291480 },
	{ "ros_2075/socialclub.pak", "D70F269F7EBBA3A13AA2871BAFA58212B01E6280", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/socialclub.pak", 4996 },
	{ "ros_2075/SocialClubD3D12Renderer.dll", "E8A4A88F72551B5CC783C1EC3D49F88867C2992F", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/SocialClubD3D12Renderer.dll", 414168 },
	{ "ros_2075/SocialClubHelper.exe", "48358424E4DC16345FA818A5731FC43F534C1714", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/SocialClubHelper.exe", 2629592 },
	{ "ros_2075/SocialClubVulkanLayer.dll", "CFB66FA7C3BD8727494CC712AC086F24ED6AFF20", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/SocialClubVulkanLayer.dll", 471512 },
	{ "ros_2075/SocialClubVulkanLayer.json", "5DA071BDE81BF96C8939978343C6B5B93730CB39", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/SocialClubVulkanLayer.json", 339 },
	{ "ros_2075/v8_context_snapshot.bin", "1834A6FF2B69121D01DA29EB1CB82ED29F493AE2", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/v8_context_snapshot.bin", 688952 },
	{ "ros_2075/locales/am.pak", "C38470C74EC5FA6C39C557A9AE1C62EA8C5949E7", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/am.pak", 308569 },
	{ "ros_2075/locales/ar.pak", "488117B83394AC599F68D89EE6CB4AFDC101617D", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/ar.pak", 309144 },
	{ "ros_2075/locales/bg.pak", "82BB3358F6404168A22618C666736D45B9652C15", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/bg.pak", 354995 },
	{ "ros_2075/locales/bn.pak", "CAEE7C690B81EE2A39D806598891BBE419FED0F0", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/bn.pak", 457090 },
	{ "ros_2075/locales/ca.pak", "4E5A93371D8E69306FF7104EC9E4BA9F1E658DE0", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/ca.pak", 220145 },
	{ "ros_2075/locales/cs.pak", "8E28D1B3E2DCEE254DB0906470FE9EF061F6F301", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/cs.pak", 223622 },
	{ "ros_2075/locales/da.pak", "C69E36ABB056EE8F3F79348ABAC3ACB4394A915D", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/da.pak", 200457 },
	{ "ros_2075/locales/de.pak", "BE21B950998704AC0F6A4F8FE2DFE7831E4DB188", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/de.pak", 218494 },
	{ "ros_2075/locales/el.pak", "1C27383402BF6E116DFE35275C10A258BFB2C1DC", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/el.pak", 388334 },
	{ "ros_2075/locales/en-GB.pak", "1FDFEE5FDC8E4DC1BF797773A4C7B381640E94DB", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/en-GB.pak", 179898 },
	{ "ros_2075/locales/en-US.pak", "C250B53402E3CA81A5B15B4AE9EFBE374D0B40DC", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/en-US.pak", 181262 },
	{ "ros_2075/locales/es-419.pak", "AC9C6DDCAFDED36C1B7341D715D4D5AEC80EF6C4", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/es-419.pak", 216250 },
	{ "ros_2075/locales/es.pak", "7D68745DF63153295BA95362298A6C807E22FB8F", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/es.pak", 219649 },
	{ "ros_2075/locales/et.pak", "0D699B20763B1617466C767422AB165B59FFA44D", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/et.pak", 196109 },
	{ "ros_2075/locales/fa.pak", "C3252469CB0704118DC87EFF963730786EDE9CF8", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/fa.pak", 311518 },
	{ "ros_2075/locales/fi.pak", "A8E758C20BE09FB539F70A0808F16088D75F9412", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/fi.pak", 202474 },
	{ "ros_2075/locales/fil.pak", "849467DF01B5B214DFD5BDE4E6571DD68D368164", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/fil.pak", 222492 },
	{ "ros_2075/locales/fr.pak", "117E0FA329FD8C6CF2C50683112AA0AA9FB78E92", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/fr.pak", 234784 },
	{ "ros_2075/locales/gu.pak", "C0CC8D54E5E10011AEE01445C84FB9F0D5886976", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/gu.pak", 433963 },
	{ "ros_2075/locales/he.pak", "A3DE5BDE7F03F3CE7A49D8C9C3D222D4F1B92E33", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/he.pak", 263008 },
	{ "ros_2075/locales/hi.pak", "C8E80B209AFF6C0672846E45A5CAD70E294372C4", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/hi.pak", 444860 },
	{ "ros_2075/locales/hr.pak", "340B0FE0BFAD72027B5C4851AD0CDD1E88DB8F2E", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/hr.pak", 212148 },
	{ "ros_2075/locales/hu.pak", "AF775FBF31FE24FF03D7F20E54A13EBB704B1243", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/hu.pak", 230497 },
	{ "ros_2075/locales/id.pak", "48EB5000362567230AECE13533A20201BC7E9DA5", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/id.pak", 193889 },
	{ "ros_2075/locales/it.pak", "1AA31C84E57DAFC025C7101FF67686F5CA4C1427", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/it.pak", 212034 },
	{ "ros_2075/locales/ja.pak", "F5E8088157758222A5F12D92468E3278379ACC9D", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/ja.pak", 263635 },
	{ "ros_2075/locales/kn.pak", "A404D241DC5C878EE984B9A020868C3B3A657D48", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/kn.pak", 506652 },
	{ "ros_2075/locales/ko.pak", "CCF76DE8F9B8123534FDBC295EB611084827AE44", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/ko.pak", 221281 },
	{ "ros_2075/locales/lt.pak", "5BF289E0B7FBF3BA76C458CDEE3DA37AE8916FC6", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/lt.pak", 227745 },
	{ "ros_2075/locales/lv.pak", "C99AE49A35A91CB65DEC70C7D047317E7A4990B1", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/lv.pak", 226576 },
	{ "ros_2075/locales/ml.pak", "AF5EE2F04A369FF547A302FC25D8A83625B4834A", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/ml.pak", 545408 },
	{ "ros_2075/locales/mr.pak", "F5B0DAA6B06102EB7CB8E0D3A2B8A7CC8E4EB3B5", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/mr.pak", 440783 },
	{ "ros_2075/locales/ms.pak", "F321C7D05192A568BFA9A3F1F2E1E4F990CCB0FD", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/ms.pak", 200631 },
	{ "ros_2075/locales/nb.pak", "11447469B1E2EA31E5E41175EACBF2688CFDCBFC", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/nb.pak", 197834 },
	{ "ros_2075/locales/nl.pak", "3CF49D1A6C4EAD4DDDE1BDB7B47F27D396DEA174", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/nl.pak", 207112 },
	{ "ros_2075/locales/pl.pak", "3B458BE93A629DD21412C90261C6E0FC75884A01", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/pl.pak", 219794 },
	{ "ros_2075/locales/pt-BR.pak", "6D800996B7AED2CCFD61F862F5972ECD863F301D", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/pt-BR.pak", 213275 },
	{ "ros_2075/locales/pt-PT.pak", "F9510A1E9EB748A1DA710568CFA7E8E7572C8F9D", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/pt-PT.pak", 215681 },
	{ "ros_2075/locales/ro.pak", "3715F4B689B36371BE57F8B3B428463B04E3D589", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/ro.pak", 220783 },
	{ "ros_2075/locales/ru.pak", "B5E70099AB9221C7B48172615E90EEC4651BA962", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/ru.pak", 342198 },
	{ "ros_2075/locales/sk.pak", "5B1145F2AFF82A48ADFC8649468C738729BE11F5", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/sk.pak", 227524 },
	{ "ros_2075/locales/sl.pak", "2BF6609859614FFFF59835CC646F312616CA3221", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/sl.pak", 213458 },
	{ "ros_2075/locales/sr.pak", "DE8301191E06A92FDA4B6AA0173D13144BC9D201", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/sr.pak", 331863 },
	{ "ros_2075/locales/sv.pak", "59BCD61F90A572EE04B653E380F7FB037558EDF7", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/sv.pak", 198993 },
	{ "ros_2075/locales/sw.pak", "E227032C65FD15F95134D5737D6A82153D64F88E", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/sw.pak", 203901 },
	{ "ros_2075/locales/ta.pak", "C55A45300374620C318931CB57AD40A765C310E3", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/ta.pak", 513725 },
	{ "ros_2075/locales/te.pak", "C5F840C16BBB8D881CBBE6DC5ECE0224A2B233FA", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/te.pak", 488193 },
	{ "ros_2075/locales/th.pak", "77115AFC2AACCEAF7E06BFFA204636F5D43896DE", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/th.pak", 414114 },
	{ "ros_2075/locales/tr.pak", "263531664B606FE910BA60E34D2FA588EEA940C6", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/tr.pak", 212733 },
	{ "ros_2075/locales/uk.pak", "D4E839D8839A4E974D0AD0481516CDADB20CC22F", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/uk.pak", 346713 },
	{ "ros_2075/locales/vi.pak", "816C6A6AAB8F0FD3262E55EACB26EA7F02886103", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/vi.pak", 245342 },
	{ "ros_2075/locales/zh-CN.pak", "68887DDD4DCB4AB75A1A4FA7148CE7EECB3B1524", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/zh-CN.pak", 184050 },
	{ "ros_2075/locales/zh-TW.pak", "7B77CAA9C113E815950835EA543F1C46638CB62A", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/locales/zh-TW.pak", 184212 },
	{ "ros_2075/swiftshader/libEGL.dll", "B4FDE410EB5D454A79A2737A719277B7FEF5BB4B", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/swiftshader/libEGL.dll", 155096 },
	{ "ros_2075/swiftshader/libGLESv2.dll", "C6F248A3FB105F14F63459A3F3F8DF5E7AC2BD4B", "https://content.cfx.re/mirrors/emergency_mirror/ros_2075/swiftshader/libGLESv2.dll", 2686424 },

	{ "launcher/LauncherPatcher.exe", "1C6BCE6CDB4B2E1766A67F931A72519CEFF6AEB1", "", "", 0, 0 },
	{ "launcher/index.bin", "85e2cc75d6d07518883ce5d377d3425b74636667", "", "", 0, 0 },
#elif defined(_M_IX86)
	{ "ros_2079_x86/cef_100_percent.pak", "5DF428B8B1D8584F2670A19224B0A3A11368B8F5", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/cef_100_percent.pak", 658266 },
	{ "ros_2079_x86/cef_200_percent.pak", "5FA7D4173D0A43610378AC26E05701B0F9F9222D", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/cef_200_percent.pak", 812521 },
	{ "ros_2079_x86/cef.pak", "743AAAFD06E48CE8006751016E3F9A1D20C528D7", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/cef.pak", 2206428 },
	{ "ros_2079_x86/chrome_elf.dll", "A35C92343290AA283A57BF8FAA233BAACA2AF378", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/chrome_elf.dll", 816520 },
	{ "ros_2079_x86/d3dcompiler_47.dll", "24B863C59A8725A2040070D6CD63B4F0B2501122", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/d3dcompiler_47.dll", 3648904 },
	{ "ros_2079_x86/icudtl.dat", "C8930E95B78DEEF5B7730102ACD39F03965D479A", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/icudtl.dat", 10505952 },
	{ "ros_2079_x86/libcef.dll", "EF40BDD5C7D1BA378F4BD6661E9D617F77F033BF", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/libcef.dll", 104726920 },
	{ "ros_2079_x86/libEGL.dll", "271F1FB5B00882F6E5D30743CD7B43A91C4F4E31", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/libEGL.dll", 319368 },
	{ "ros_2079_x86/libGLESv2.dll", "C601D45C0A4C7571A8252B7263455B84C7A6E80C", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/libGLESv2.dll", 6821768 },
	{ "ros_2079_x86/scui.pak", "3A03DFA2CECF1E356EB8D080443069ED35A897F1", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/scui.pak", 3401985 },
	{ "ros_2079_x86/snapshot_blob.bin", "FD1DF208437FB8A0E36F57F700C8FD412C300786", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/snapshot_blob.bin", 50522 },
	{ "ros_2079_x86/socialclub.dll", "1E5702D3E75E1802D16132CC27942589F9301AA2", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/socialclub.dll", 1693064 },
	{ "ros_2079_x86/socialclub.pak", "D70F269F7EBBA3A13AA2871BAFA58212B01E6280", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/socialclub.pak", 4996 },
	{ "ros_2079_x86/SocialClubHelper.exe", "A6EE9FFFE5436180B341647E06C60FD26A2F32DC", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/SocialClubHelper.exe", 1052040 },
	{ "ros_2079_x86/v8_context_snapshot.bin", "9C351FD39D4F64097B778BF920DB9CACB6884A71", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/v8_context_snapshot.bin", 170474 },
	{ "ros_2079_x86/locales/am.pak", "1BA4F8D3A96D53E236F31315ED94CE7857BE676C", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/am.pak", 385976 },
	{ "ros_2079_x86/locales/ar.pak", "D402FF17B3DEB25C729862367C6A66D4C71064C5", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/ar.pak", 395800 },
	{ "ros_2079_x86/locales/bg.pak", "789DEB5B067B64C336ED501A47EACF7AC28C165C", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/bg.pak", 438902 },
	{ "ros_2079_x86/locales/bn.pak", "D6E4E916D3A5D6B06D7252F5A3EE3546C0D5FA81", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/bn.pak", 570862 },
	{ "ros_2079_x86/locales/ca.pak", "B3A84377C6DFDCD2EC8B76DAE51EF174A3F32161", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/ca.pak", 272754 },
	{ "ros_2079_x86/locales/cs.pak", "42F2E55A50F980D8A7BC6ACA247FEA38187050C4", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/cs.pak", 278073 },
	{ "ros_2079_x86/locales/da.pak", "4EFB233CF6F6D6FE7A30887CCDF2758481F7CEF9", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/da.pak", 249964 },
	{ "ros_2079_x86/locales/de.pak", "E293C63938808FFE58CEF2E911C3E645099122C3", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/de.pak", 271680 },
	{ "ros_2079_x86/locales/el.pak", "6FB999483B51B732797F46433571CD1F99C6C382", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/el.pak", 483371 },
	{ "ros_2079_x86/locales/en-GB.pak", "337681F89B3CC5E069066BE31FE548A7FE1BDC3D", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/en-GB.pak", 222850 },
	{ "ros_2079_x86/locales/en-US.pak", "55D6297A4E9BAC33E1975015592324CE32A426E5", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/en-US.pak", 224948 },
	{ "ros_2079_x86/locales/es-419.pak", "D7CFB264CA28E4310060D4330D0F869F750296EA", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/es-419.pak", 267636 },
	{ "ros_2079_x86/locales/es.pak", "AD9BCEBBC3DFB6B346E6DDA1B24410DAE844FAD0", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/es.pak", 272041 },
	{ "ros_2079_x86/locales/et.pak", "2CC11D3FE483042FABDB0B568F70EC6D3AA89499", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/et.pak", 241972 },
	{ "ros_2079_x86/locales/fa.pak", "5A83F5797DB4C5FD4EFE8E83CD32FCF30F7A579B", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/fa.pak", 387663 },
	{ "ros_2079_x86/locales/fi.pak", "752D5AB53154F4F6CE671C11CDFFAD87E4B2098F", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/fi.pak", 250146 },
	{ "ros_2079_x86/locales/fil.pak", "CB1FE897C559395F042E113162526A1A985F3C54", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/fil.pak", 276980 },
	{ "ros_2079_x86/locales/fr.pak", "9BD0A49B7F93CECC843C4858FE02FC3233B95FB0", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/fr.pak", 293801 },
	{ "ros_2079_x86/locales/gu.pak", "503499E4B8D7EDE7C2546546C708B62315B93534", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/gu.pak", 546805 },
	{ "ros_2079_x86/locales/he.pak", "20DE3DE5EFE18A02E3722503812D1BE7F125BAC7", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/he.pak", 332944 },
	{ "ros_2079_x86/locales/hi.pak", "261DF4775E2E7FF64BBA52D6CC2E3E7E6A744BA6", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/hi.pak", 563160 },
	{ "ros_2079_x86/locales/hr.pak", "CC0EC12EE1A0182FD5095735BDAC50A7968E5941", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/hr.pak", 265276 },
	{ "ros_2079_x86/locales/hu.pak", "3A06F37F8E0DCB4A2C4B9223F4EA630DEC598F30", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/hu.pak", 287478 },
	{ "ros_2079_x86/locales/id.pak", "7648DD9102EC8D1A989F4CFEE5E77A50EEB2543A", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/id.pak", 240937 },
	{ "ros_2079_x86/locales/it.pak", "AF12931047F550A700930292678ED68A2F3AE693", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/it.pak", 263141 },
	{ "ros_2079_x86/locales/ja.pak", "3D69B821559E8B1B7BE61C5E0403013C6E34B472", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/ja.pak", 326307 },
	{ "ros_2079_x86/locales/kn.pak", "BB0B09183BFD4642EDFA4DCDFC6C589D51C91339", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/kn.pak", 637150 },
	{ "ros_2079_x86/locales/ko.pak", "11E8387A32FA7E25669EA58288AAD02240D59115", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/ko.pak", 274716 },
	{ "ros_2079_x86/locales/lt.pak", "B08521D70C17BC493EDE917CF64483C73CC8CD2B", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/lt.pak", 284278 },
	{ "ros_2079_x86/locales/lv.pak", "68E7175DF0FD0781434761C7AF533D44DFE94DD0", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/lv.pak", 283337 },
	{ "ros_2079_x86/locales/ml.pak", "0D881D6C532AFD275DB69B4468742F64BDBCEB57", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/ml.pak", 672973 },
	{ "ros_2079_x86/locales/mr.pak", "578BDC9D3EC2A35CA5338DEC949AD6269A95CF81", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/mr.pak", 539342 },
	{ "ros_2079_x86/locales/ms.pak", "6452CC250C4219EA0130D461942284ABB203105D", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/ms.pak", 249855 },
	{ "ros_2079_x86/locales/nb.pak", "B36849BBB46F448DA1A86C93A777E7600898143E", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/nb.pak", 245307 },
	{ "ros_2079_x86/locales/nl.pak", "63C363F3A6D953D2F516926A60036CD45A2BBC73", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/nl.pak", 255846 },
	{ "ros_2079_x86/locales/pl.pak", "6A9B10202C2A4EABE2DB370478A120303CCEA9E2", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/pl.pak", 276450 },
	{ "ros_2079_x86/locales/pt-BR.pak", "CDDDA77FE5857E7095B0BEF9C95ADDC0E1BFE81E", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/pt-BR.pak", 263825 },
	{ "ros_2079_x86/locales/pt-PT.pak", "B01F54FEAC5D8612FADBD1AA1E1A9AE48686765B", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/pt-PT.pak", 267667 },
	{ "ros_2079_x86/locales/ro.pak", "C5F88F5AA8070D93D1A5AC2F6D5B716A405D6402", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/ro.pak", 273510 },
	{ "ros_2079_x86/locales/ru.pak", "3E87FCC49496F1A375022B18F29C5CA184AC94A1", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/ru.pak", 435040 },
	{ "ros_2079_x86/locales/sk.pak", "696DB99FE09F2B6F04F6755B562DA0698E67EE73", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/sk.pak", 281889 },
	{ "ros_2079_x86/locales/sl.pak", "FC42DE011EB9EB97F76B90191A0AA6763524F257", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/sl.pak", 268266 },
	{ "ros_2079_x86/locales/sr.pak", "A5F59A87CCAECE1EA27B679DC4CD43B226D44652", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/sr.pak", 414419 },
	{ "ros_2079_x86/locales/sv.pak", "52B28217FA8CFB1BA78DA18FF8FABE96AA65C0CC", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/sv.pak", 247090 },
	{ "ros_2079_x86/locales/sw.pak", "F06249989197891C214B827CB6ACA078A8AC52A9", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/sw.pak", 252916 },
	{ "ros_2079_x86/locales/ta.pak", "7B2078915D759278F044DA16C331BC35A8EAE366", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/ta.pak", 644686 },
	{ "ros_2079_x86/locales/te.pak", "81B147F70EFC789597013B862AC7C4C2E932D668", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/te.pak", 606991 },
	{ "ros_2079_x86/locales/th.pak", "17BB58538907D534C950C830451FD09BE8B699ED", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/th.pak", 514784 },
	{ "ros_2079_x86/locales/tr.pak", "5872240D9E0700DDBE160CE5FED55B64A8C58D4E", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/tr.pak", 262379 },
	{ "ros_2079_x86/locales/uk.pak", "403A5AF73EAA7E31FA913B3E92989A3966E84F27", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/uk.pak", 433942 },
	{ "ros_2079_x86/locales/vi.pak", "4CDA92C95B944456682F49F7542A7DF29AFA390D", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/vi.pak", 305558 },
	{ "ros_2079_x86/locales/zh-CN.pak", "297C09A18520F9716D81D612540AC8ED7EBDC42B", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/zh-CN.pak", 227335 },
	{ "ros_2079_x86/locales/zh-TW.pak", "7F67C3A955A99FD31C0A5D5D7B0CD6B404F09BB1", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/locales/zh-TW.pak", 227384 },
	{ "ros_2079_x86/swiftshader/libEGL.dll", "315BE829397C2C65B4401DE0A9F634D2DF864CD4", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/swiftshader/libEGL.dll", 338312 },
	{ "ros_2079_x86/swiftshader/libGLESv2.dll", "E62DA6B61D963AB9CD242C2811AC9D7ADA2613AB", "https://content.cfx.re/mirrors/emergency_mirror/ros_2079_x86/swiftshader/libGLESv2.dll", 3017608 },
#endif

#if defined(_M_IX86)
	// #TODOLIBERTY: ROS 2.0.7.x for 32-bit
#endif
};

static bool ParseCacheFileName(const char* inString, std::string& fileNameOut, std::string& hashOut)
{
	// check if the file name meets the minimum length for there to be a hash
	size_t length = strlen(inString);

	if (length < 44)
	{
		return false;
	}

	// find the file extension
	const char* dotPos = strchr(inString, '.');

	if (!dotPos)
	{
		return false;
	}

	// find the first underscore following the file extension
	const char* underscorePos = strchr(dotPos, '_');

	if (!underscorePos)
	{
		return false;
	}

	// store the file name
	fileNameOut = fwString(inString, underscorePos - inString);

	// check if we have a hash
	const char* hashStart = &inString[length - 41];

	if (*hashStart != '_')
	{
		return false;
	}

	hashOut = hashStart + 1;

	return true;
}

#include <charconv>

template<int Size>
static constexpr std::array<uint8_t, Size> ParseHexString(const std::string_view string)
{
	std::array<uint8_t, Size> retval;

	for (size_t i = 0; i < Size; i++)
	{
		size_t idx = i * 2;
		char byte[3] = { string[idx], string[idx + 1], 0 };

		std::from_chars(&byte[0], &byte[2], retval[i], 16);
	}

	return retval;
}

DeltaEntry::DeltaEntry(std::string_view fromChecksum, std::string_view toChecksum, const std::string& remoteFile, uint64_t dlSize)
	: fromChecksum(ParseHexString<20>(fromChecksum)), toChecksum(ParseHexString<20>(toChecksum)), remoteFile(remoteFile), dlSize(dlSize)
{
}

static std::vector<GameCacheStorageEntry> LoadCacheStorage()
{
	// create the cache directory if needed
	CreateDirectory(MakeRelativeCitPath(L"data").c_str(), nullptr);
	CreateDirectory(MakeRelativeCitPath(L"data\\game-storage").c_str(), nullptr);

	// output buffer
	std::vector<GameCacheStorageEntry> cacheStorage;

	// iterate over files in cache
	WIN32_FIND_DATA findData;

	HANDLE hFind = FindFirstFile(MakeRelativeCitPath(L"data\\game-storage\\*.*").c_str(), &findData);

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do 
		{
			// try parsing the file name
			std::string fileName;
			std::string fileHash;

			if (ParseCacheFileName(converter.to_bytes(findData.cFileName).c_str(), fileName, fileHash))
			{
				// add the entry, if so
				LARGE_INTEGER quadTime;
				quadTime.HighPart = findData.ftLastWriteTime.dwHighDateTime;
				quadTime.LowPart = findData.ftLastWriteTime.dwLowDateTime;

				GameCacheStorageEntry entry;
				entry.fileTime = quadTime.QuadPart / 10000000ULL - 11644473600ULL;
				
				auto checksum = ParseHexString<20>(fileHash.c_str());
				memcpy(entry.checksum, checksum.data(), checksum.size());

				cacheStorage.push_back(entry);
			}
		} while (FindNextFile(hFind, &findData));

		FindClose(hFind);
	}

	// load on-disk storage as well
	{
		if (FILE* f = _wfopen(MakeRelativeCitPath(L"data\\game-storage\\game_files.dat").c_str(), L"rb"))
		{
			// get file length
			int length;
			fseek(f, 0, SEEK_END);
			length = ftell(f);
			fseek(f, 0, SEEK_SET);

			// read into buffer
			std::vector<GameCacheStorageEntry> fileEntries(length / sizeof(GameCacheStorageEntry));
			fread(&fileEntries[0], sizeof(GameCacheStorageEntry), fileEntries.size(), f);

			// close file
			fclose(f);

			// insert into list
			cacheStorage.insert(cacheStorage.end(), fileEntries.begin(), fileEntries.end());
		}
	}

	// return the obtained data
	return cacheStorage;
}

#if defined(LAUNCHER_PERSONALITY_MAIN) || defined(COMPILING_GLUE)
static std::vector<GameCacheEntry> CompareCacheDifferences()
{
	// load the cache storage from disk
	auto storageEntries = LoadCacheStorage();

	// return value
	std::vector<GameCacheEntry> retval;

	// go through each entry and check for validity
	for (auto& entry : g_requiredEntries)
	{
		// find the storage entry associated with the file and check it for validity
		bool found = false;

		for (auto& checksum : entry.checksums)
		{
			auto requiredHash = ParseHexString<20>(checksum);

			for (auto& storageEntry : storageEntries)
			{
				if (std::equal(requiredHash.begin(), requiredHash.end(), storageEntry.checksum))
				{
					// check if the file exists
					std::wstring cacheFileName = entry.GetCacheFileName();

					if (GetFileAttributes(cacheFileName.c_str()) == INVALID_FILE_ATTRIBUTES && (GetFileAttributes(entry.GetLocalFileName().c_str()) == INVALID_FILE_ATTRIBUTES || strncmp(entry.remotePath, "nope:", 5) != 0))
					{
						if (entry.localSize != 0)
						{
							// as it doesn't add to the list
							retval.push_back(entry);
						}
					}

					found = true;

					break;
				}
			}

			if (found)
			{
				break;
			}
		}

		// if no entry was found, add to the list as well
		if (!found)
		{
			if (entry.IsPrimitiveFile())
			{
				auto fileHandle = CreateFileW(entry.GetCacheFileName().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				if (fileHandle != INVALID_HANDLE_VALUE && fileHandle != 0)
				{
					LARGE_INTEGER fs;
					GetFileSizeEx(fileHandle, &fs);

					if (fs.QuadPart == entry.localSize)
					{
						found = true;
					}

					CloseHandle(fileHandle);
				}
			}

			if (entry.localSize == 0)
			{
				found = true;
			}

			if (!found)
			{
				retval.push_back(entry);
			}
		}
	}

	return retval;
}

#include <sstream>

bool ExtractInstallerFile(const std::wstring& installerFile, const std::string& entryName, const std::wstring& outFile);

#include <commctrl.h>

#if defined(COMPILING_GLUE)
extern void TaskDialogEmulated(TASKDIALOGCONFIG* config, int* button, void*, void*);
#endif

static bool ShowDownloadNotification(const std::vector<std::pair<GameCacheEntry, bool>>& entries)
{
	// iterate over the entries
	std::wstringstream detailStr;
	size_t localSize = 0;
	size_t remoteSize = 0;

	bool shouldAllow = true;

	std::string badEntries;
	for (auto& entry : entries)
	{
		// is the file allowed?
		if (_strnicmp(entry.first.remotePath, "nope:", 5) == 0)
		{
			shouldAllow = false;
			badEntries += entry.first.filename;
			badEntries += "\n";
		}

		// if it's a local file...
		if (entry.second)
		{
			localSize += entry.first.localSize;

			detailStr << entry.first.filename << L" (local, " << va(L"%.2f", entry.first.localSize / 1024.0 / 1024.0) << L" MB)\n";
		}
		else
		{
			if (entry.first.remoteSize == SIZE_MAX)
			{
				shouldAllow = false;
				badEntries += entry.first.filename;
				badEntries += "\n";
			}

			remoteSize += entry.first.remoteSize;

			detailStr << entry.first.remotePath << L" (download, " << va(L"%.2f", entry.first.remoteSize / 1024.0 / 1024.0) << L" MB)\n";
		}
	}

	// convert to string
	std::wstring footerString = detailStr.str();

	// remove the trailing newline
	footerString = footerString.substr(0, footerString.length() - 1);

	// show a dialog
	TASKDIALOGCONFIG taskDialogConfig = { 0 };
	taskDialogConfig.cbSize = sizeof(taskDialogConfig);
	taskDialogConfig.hwndParent = UI_GetWindowHandle();
	taskDialogConfig.hInstance = GetModuleHandle(nullptr);
	taskDialogConfig.dwFlags = TDF_EXPAND_FOOTER_AREA;
	taskDialogConfig.dwCommonButtons = TDCBF_YES_BUTTON | TDCBF_NO_BUTTON;
	taskDialogConfig.pszWindowTitle = PRODUCT_NAME L": Game data outdated";
	taskDialogConfig.pszMainIcon = TD_INFORMATION_ICON;
	taskDialogConfig.pszMainInstruction = PRODUCT_NAME L" needs to update the local game data";

	if (shouldAllow)
	{
		taskDialogConfig.pszContent = va(gettext(L"The local %s game data is outdated, and needs to be updated. This will copy %.2f MB of data from the local disk, and download %.2f MB of data from the internet.\nDo you wish to continue?"), PRODUCT_NAME, (localSize / 1024.0 / 1024.0), (remoteSize / 1024.0 / 1024.0));
	}
	else
	{
		const TASKDIALOG_BUTTON buttons[] = {
			{ 42, L"Close" }
		};

		std::wstring badEntriesWide = ToWide(badEntries);

		taskDialogConfig.pszMainInstruction = L"Game files missing";
		taskDialogConfig.pszContent = va(gettext(L"DLC files are missing (or corrupted) in your game installation. Please update or verify the game using Steam, Epic Games Launcher or Rockstar Games Launcher and try again. See http://rsg.ms/verify step 4 for more info.\nCurrently, the game installation in '%s' is being used.\nRelevant files: \n%s"), MakeRelativeGamePath(L""), badEntriesWide.c_str());

		taskDialogConfig.cButtons = 1;
		taskDialogConfig.dwCommonButtons = 0;
		taskDialogConfig.pButtons = buttons;

		footerString = L"";
	}

	taskDialogConfig.pszExpandedInformation = footerString.c_str();
	taskDialogConfig.pfCallback = [] (HWND, UINT type, WPARAM wParam, LPARAM lParam, LONG_PTR data)
	{
		if (type == TDN_BUTTON_CLICKED)
		{
			return S_OK;
		}

		return S_FALSE;
	};

	int outButton;

#if defined(COMPILING_GLUE)
	TaskDialogEmulated(&taskDialogConfig, &outButton, nullptr, nullptr);
#else
	TaskDialogIndirect(&taskDialogConfig, &outButton, nullptr, nullptr);
#endif

	return (outButton != IDNO && outButton != 42);
}

extern void StartIPFS();

#include "ZlibDecompressPlugin.h"

static bool PerformUpdate(const std::vector<GameCacheEntry>& entries)
{
	// create UI
	UI_DoCreation();

	// hash local files for those that *do* exist, add those that don't match to the download queue and add those that do match to be copied locally
	std::set<std::string> referencedFiles; // remote URLs that we already requested
	std::vector<GameCacheEntry> extractedEntries; // entries to extract from an archive

	// entries for notification purposes
	std::vector<std::pair<GameCacheEntry, bool>> notificationEntries;

	uint64_t fileStart = 0;
	uint64_t fileTotal = 0;

	for (auto& entry : entries)
	{
		if (_strnicmp(entry.remotePath, "nope:", 5) != 0)
		{
			struct _stat64 stat;
			if (_wstat64(entry.GetLocalFileName().c_str(), &stat) >= 0)
			{
				fileTotal += stat.st_size;
			}
		}
	}

	bool hadIpfsFile = false;
	std::vector<std::tuple<DeltaEntry, GameCacheEntry>> theseDeltas;

	for (auto& entry : entries)
	{
		// check if the file is outdated
		std::vector<std::array<uint8_t, 20>> hashes;

		for (auto& checksum : entry.checksums)
		{
			hashes.push_back(ParseHexString<20>(checksum));
		}

		const auto& deltaEntries = entry.deltas;

		for (auto& deltaEntry : deltaEntries)
		{
			hashes.push_back(deltaEntry.fromChecksum);
		}
		
		std::array<uint8_t, 20> outHash;
		bool fileOutdated = false;
		
		if (_strnicmp(entry.remotePath, "nope:", 5) != 0)
		{
			// does *not* start with nope: (manual download)
			UI_UpdateText(0, gettext(L"Verifying game content...").c_str());

			fileOutdated = CheckFileOutdatedWithUI(entry.GetLocalFileName().c_str(), hashes, &fileStart, fileTotal, &outHash);
		}
		else
		{
			// 'nope:' files just get a size check, no whole hash check
			if (GetFileAttributes(entry.GetLocalFileName().c_str()) == INVALID_FILE_ATTRIBUTES)
			{
				fileOutdated = true;
			}
			else
			{
				HANDLE hFile = CreateFile(entry.GetLocalFileName().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				if (hFile == INVALID_HANDLE_VALUE)
				{
					fileOutdated = true;
				}
				else
				{
					LARGE_INTEGER fileSize;
					fileSize.QuadPart = 0;

					GetFileSizeEx(hFile, &fileSize);

					CloseHandle(hFile);

					if (fileSize.QuadPart != entry.localSize)
					{
						fileOutdated = true;
					}
				}
			}

			if (!fileOutdated)
			{
				outHash = hashes[0];
			}
		}

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

		// if not, copy it from the local filesystem (we're abusing the download code here a lot)
		if (!fileOutdated)
		{
			// should we 'nope' this file?
			if (_strnicmp(entry.remotePath, "nope:", 5) == 0)
			{
				// *does* start with nope:
				if (FILE* f = _wfopen(MakeRelativeCitPath(L"data\\game-storage\\game_files.dat").c_str(), L"ab"))
				{
					auto hash = outHash;

					GameCacheStorageEntry storageEntry;
					memcpy(storageEntry.checksum, &hash[0], sizeof(storageEntry.checksum));
					storageEntry.fileTime = time(nullptr);

					fwrite(&storageEntry, sizeof(GameCacheStorageEntry), 1, f);

					fclose(f);
				}
			}
			else
			{
				if (outHash == hashes[0])
				{
					CL_QueueDownload(va("file:///%s", converter.to_bytes(entry.GetLocalFileName()).c_str()), converter.to_bytes(entry.GetCacheFileName()).c_str(), entry.localSize, false);

					notificationEntries.push_back({ entry, true });
				}
				else
				{
					for (auto& deltaEntry : deltaEntries)
					{
						if (outHash == deltaEntry.fromChecksum)
						{
							CL_QueueDownload(deltaEntry.remoteFile.c_str(), ToNarrow(deltaEntry.GetLocalFileName()).c_str(), deltaEntry.dlSize, false);

							notificationEntries.push_back({ deltaEntry.MakeEntry(), false });
							theseDeltas.emplace_back(deltaEntry, entry);

							break;
						}
					}
				}
			}
		}
		else
		{
			// else, if it's not already referenced by a queued download...
			if (referencedFiles.find(entry.remotePath) == referencedFiles.end())
			{
				// download it from the rockstar service
				std::string localFileName = (entry.archivedFile) ? converter.to_bytes(entry.GetRemoteBaseName()) : converter.to_bytes(entry.GetCacheFileName());
				const char* remotePath = entry.remotePath;

				if (_strnicmp(remotePath, "http", 4) != 0 && _strnicmp(remotePath, "ipfs", 4) != 0)
				{
					remotePath = va("rockstar:%s", entry.remotePath);
				}

				// if the file isn't of the original size
				CL_QueueDownload(remotePath, localFileName.c_str(), entry.remoteSize, (entry.remoteSize != entry.localSize && !entry.archivedFile));

				if (strncmp(remotePath, "ipfs://", 7) == 0)
				{
					hadIpfsFile = true;
				}

				referencedFiles.insert(entry.remotePath);

				notificationEntries.push_back({ entry, false });
			}

			if (entry.archivedFile && strlen(entry.archivedFile) > 0)
			{
				// if we want an archived file from here, we should *likely* note its existence
				extractedEntries.push_back(entry);
			}
		}
	}

	// notify about entries that will be 'downloaded'
	if (!notificationEntries.empty())
	{
		if (!ShowDownloadNotification(notificationEntries))
		{
			UI_DoDestruction();

			return false;
		}
	}
	else
	{
		return true;
	}

	// start IPFS
	std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");

	bool ipfsPeer = true;

	if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		ipfsPeer = (GetPrivateProfileInt(L"Game", L"DisableIPFSPeer", 0, fpath.c_str()) != 1);
	}

	static HostSharedData<CfxState> initState("CfxInitState");

	if (ipfsPeer && initState->IsMasterProcess() && hadIpfsFile)
	{
		StartIPFS();
	}

	UI_UpdateText(0, gettext(L"Updating game cache...").c_str());

	bool retval = DL_RunLoop();

	// if succeeded, try extracting any entries
	if (retval)
	{
		// sort extracted entries by 'archive' they belong to
		std::sort(extractedEntries.begin(), extractedEntries.end(), [] (const auto& left, const auto& right)
		{
			return strcmp(left.remotePath, right.remotePath) < 0;
		});

		// apply deltas
		if (!theseDeltas.empty())
		{
			for (auto& [ deltaEntry, entry ] : theseDeltas)
			{
				if (retval)
				{
					hpatch_TStreamInput oldFile;
					hpatch_TStreamInput deltaFile;
					hpatch_TStreamOutput outFile;

					auto openRead = [](hpatch_TStreamInput* entry, const std::wstring& fn) 
					{
						FILE* f = _wfopen(fn.c_str(), L"rb");

						if (!f)
						{
							return;
						}

						fseek(f, 0, SEEK_END);
						entry->streamImport = (void*)f;
						entry->streamSize = ftell(f);

						entry->read = [](const hpatch_TStreamInput* entry, hpatch_StreamPos_t at, uint8_t* begin, uint8_t* end) -> hpatch_BOOL {
							auto size = end - begin;

							FILE* f = (FILE*)entry->streamImport;
							fseek(f, at, SEEK_SET);

							return (fread(begin, 1, size, f) == size);
						};
					};

					UI_UpdateText(1, va(L"Patching %s", ToWide(entry.filename)));

					auto outSize = entry.localSize;

					auto openWrite = [outSize](hpatch_TStreamOutput* entry, const std::wstring& fn)
					{
						FILE* f = _wfopen(fn.c_str(), L"wb");

						if (!f)
						{
							return;
						}

						entry->streamImport = (void*)f;
						entry->streamSize = outSize;
						entry->read_writed = NULL;

						static uint64_t numWritten;
						numWritten = 0;

						entry->write = [](const hpatch_TStreamOutput* entry, hpatch_StreamPos_t at, const uint8_t* begin, const uint8_t* end) -> hpatch_BOOL
						{
							auto size = end - begin;

							FILE* f = (FILE*)entry->streamImport;
							fseek(f, at, SEEK_SET);

							numWritten += size;

							static auto ticks = 0;

							if ((ticks % 400) == 0)
							{
								UI_UpdateProgress((numWritten / (double)entry->streamSize) * 100.0);

								MSG msg;

								// poll message loop
								while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
								{
									TranslateMessage(&msg);
									DispatchMessage(&msg);
								}
							}

							ticks++;

							return (fwrite(begin, 1, size, f) == size);
						};
					};

					auto doClose = [](const auto* entry) 
					{
						fclose((FILE*)entry->streamImport);
					};

					auto theFile = entry.GetCacheFileName();
					auto tmpFile = theFile + L".tmp";

					openRead(&oldFile, entry.GetLocalFileName());
					openRead(&deltaFile, deltaEntry.GetLocalFileName());
					openWrite(&outFile, tmpFile);

					retval = retval && patch_decompress(&outFile, &oldFile, &deltaFile, &zlibDecompressPlugin);

					doClose(&oldFile);
					doClose(&deltaFile);
					doClose(&outFile);

					if (retval)
					{
						_wunlink(theFile.c_str());
						_wrename(tmpFile.c_str(), theFile.c_str());
					}
					else
					{
						MessageBoxW(NULL, va(L"Could not patch %s. Do you have enough free disk space on all drives? (~2 GB)", ToWide(entry.filename)), L"Error", MB_OK | MB_ICONSTOP);

						_wunlink(tmpFile.c_str());
					}

					_wunlink(deltaEntry.GetLocalFileName().c_str());
				}
			}
		}

		// batch up entries per archive
		if (!extractedEntries.empty())
		{
			std::string lastArchive = extractedEntries[0].remotePath;
			std::vector<GameCacheEntry> lastEntries;
			std::set<std::string> foundHashes;

			// append a dummy entry
			extractedEntries.push_back(GameCacheEntry{ "", "", "", 0 });

			for (auto& entry : extractedEntries)
			{
				if (lastArchive != entry.remotePath)
				{
					// process each entry
					retval = retval && ExtractInstallerFile(lastEntries[0].GetRemoteBaseName(), [&] (const InstallerInterface& interface)
					{
						// scan for a section
						section targetSection;
						
						for (section section : interface.getSections())
						{
							if (section.code_size > 0)
							{
								targetSection = section;
								break;
							}
						}

						// process the section
						std::wstring currentDirectory;

						auto processFile = [&] (const ::entry& entry)
						{
							// get the base filename
							std::wstring fileName = currentDirectory + L"/" + interface.getString(entry.offsets[1]);

							// append a new filename without double slashes
							{
								std::wstringstream newFileName;
								bool wasSlash = false;

								for (int j = 0; j < fileName.length(); j++)
								{
									wchar_t c = fileName[j];

									if (c == L'/')
									{
										if (!wasSlash)
										{
											newFileName << c;

											wasSlash = true;
										}
									}
									else
									{
										newFileName << c;

										wasSlash = false;
									}
								}

								fileName = newFileName.str();
							}

							// strip the first path separator (variable/instdir stuff)
							fileName = L"$/" + fileName.substr(fileName.find_first_of(L'/', 0) + 1);

							// find an entry (slow linear search, what'chagonnado'aboutit?)
							for (auto& dlEntry : lastEntries)
							{
								if (_wcsicmp(ToWide(dlEntry.archivedFile).c_str(), fileName.c_str()) == 0)
								{
									if (foundHashes.find(dlEntry.checksums[0]) == foundHashes.end())
									{
										std::wstring cacheName = dlEntry.GetCacheFileName();

										if (cacheName.find(L'/') != std::string::npos)
										{
											std::wstring cachePath = cacheName.substr(0, cacheName.find_last_of(L'/'));

											CreateDirectory(cachePath.c_str(), nullptr);
										}

										interface.addFile(entry, cacheName);

										foundHashes.insert(dlEntry.checksums[0]);
									}
								}
							}
						};

						auto processEntry = [&] (const ::entry& entry)
						{
							if (entry.which == EW_CREATEDIR)
							{
								if (entry.offsets[1] != 0)
								{
									// update instdir
									currentDirectory = interface.getString(entry.offsets[0]);

									std::replace(currentDirectory.begin(), currentDirectory.end(), L'\\', L'/');
								}
							}
							else if (entry.which == EW_EXTRACTFILE)
							{
								processFile(entry);
							}
						};

						interface.processSection(targetSection, [&] (const ::entry& entry)
						{
							// call
							if (entry.which == EW_CALL)
							{
								section localSection;
								localSection.code = entry.offsets[0];
								localSection.code_size = 9999999;

								interface.processSection(localSection, processEntry);
							}
							// extract file
							else
							{
								processEntry(entry);
							}
						});
					});

					// append entries to cache storage if it succeeded
					if (retval)
					{
						if (FILE* f = _wfopen(MakeRelativeCitPath(L"data\\game-storage\\game_files.dat").c_str(), L"ab"))
						{
							for (auto& entry : lastEntries)
							{
								auto hash = ParseHexString<20>(entry.checksums[0]);

								GameCacheStorageEntry storageEntry;
								memcpy(storageEntry.checksum, &hash[0], sizeof(storageEntry.checksum));
								storageEntry.fileTime = time(nullptr);

								fwrite(&storageEntry, sizeof(GameCacheStorageEntry), 1, f);
							}

							fclose(f);
						}
					}

					// clear the list
					lastEntries.clear();
					lastArchive = entry.remotePath;
				}

				if (entry.localSize)
				{
					// append cleanly
					lastEntries.push_back(entry);
				}
			}
		}
	}

	// destroy UI
	UI_DoDestruction();

	// failed?
	if (!retval)
	{
		return false;
	}

	return true;
}
#endif

#include <CrossBuildRuntime.h>

#if defined(COMPILING_GLUE)
extern int gameCacheTargetBuild;

template<int Build>
bool IsTargetGameBuild()
{
	return (gameCacheTargetBuild == Build);
}

template<int Build>
bool IsTargetGameBuildOrGreater()
{
	return (gameCacheTargetBuild >= Build);
}
#else
template<int Build>
bool IsTargetGameBuild()
{
	return xbr::IsGameBuild<Build>();
}

template<int Build>
bool IsTargetGameBuildOrGreater()
{
	return xbr::IsGameBuildOrGreater<Build>();
}
#endif

std::map<std::string, std::string> UpdateGameCache()
{
#if defined(COMPILING_GLUE)
	g_requiredEntries.clear();
#endif

	// 1604/1868 toggle
#ifdef GTA_FIVE
	if (Is372())
	{
		g_requiredEntries.push_back({ "GTA5.exe", "ae6e9cc116e8435e4dcfb5b870deee00a8b0904c", "https://runtime.fivem.net/patches/GTA_V_Patch_1_0_372_2.exe", "$/GTA5.exe", 55559560, 399999536 });
		g_requiredEntries.push_back({ "update/update.rpf", "b72884c9af7170908f558ec5d629d805857b80f2", "https://runtime.fivem.net/patches/GTA_V_Patch_1_0_372_2.exe", "$/update/update.rpf", 352569344, 399999536 });
	}
	else if (IsTargetGameBuild<2189>())
	{
		g_requiredEntries.push_back({ "GTA5.exe", "fcd5fd8a9f99f2e08b0cab5d500740f28a75b75a", "https://content.cfx.re/mirrors/patches/2189.0/GTA5.exe", 63124096 });
		g_requiredEntries.push_back({ "update/update.rpf", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches/2189.0/update.rpf", 1276805120,
		{
			{ "748d16b81a34ad317b93cd85e2b088dabdce5cc7", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches/2245_2189_update.hdiff", 193145914 },
			{ "36c5c94274602527f946497553e118d72500c09f", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches/2215_2189_update.hdiff", 21294688 },	
		} });
	}
	else if (IsTargetGameBuild<2060>())
	{
		g_requiredEntries.push_back({ "GTA5.exe", "741c8b91ef57140c023d8d29e38aab599759de76", "https://content.cfx.re/mirrors/patches/2060.2/GTA5.exe", 60589184 });
		g_requiredEntries.push_back({ "update/update.rpf", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches/2060.2/update.rpf", 1229002752,
		{
			{ "748d16b81a34ad317b93cd85e2b088dabdce5cc7", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches/2245_2060_update.hdiff", 249407832 },
			{ "36c5c94274602527f946497553e118d72500c09f", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches/2215_2060_update.hdiff", 249407861 },
			{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches/2189_2060_update.rpf.hdiff", 249363428 },
			{ "2d9756564bece80205165a724536b2fce731c600", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches/2060_1_2060_update.rpf.hdiff", 799273 },
		} });
	}
	else
	{
		g_requiredEntries.push_back({ "GTA5.exe", "8939c8c71aa98ad7ca6ac773fae1463763c420d8", "https://content.cfx.re/mirrors/patches/1604.0/GTA5.exe", 72484280 });
		g_requiredEntries.push_back({ "update/update.rpf", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://runtime.fivem.net/patches/GTA_V_Patch_1_0_1604_0.exe", "$/update/update.rpf", 966805504, 1031302600,
		{
			{ "748d16b81a34ad317b93cd85e2b088dabdce5cc7", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches/2245_1604_update.hdiff", 255586706 },
			{ "36c5c94274602527f946497553e118d72500c09f", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches/2215_1604_update.hdiff", 255586724 },
			{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches/2189_1604_update.rpf.hdiff", 257064151 },
			{ "2d9756564bece80205165a724536b2fce731c600", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches/2060_1_1604_update.rpf.hdiff", 252578172 },
		} });
	}

	if (IsTargetGameBuildOrGreater<2060>())
	{
		g_requiredEntries.push_back({ "update/x64/dlcpacks/mpsum/dlc.rpf", "ffd81a2ce5741b38eae69e47132ddbfc5cfdf9f4", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 980621312 });
	}

	if (IsTargetGameBuildOrGreater<2189>())
	{
		g_requiredEntries.push_back({ "update/x64/dlcpacks/mpheist4/dlc.rpf", "1ddd73a584126793478c835efef9899a1c9d6fe7", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 3452489728 });

		// 2215.0 DLC update
		g_requiredEntries.push_back({ "update/x64/dlcpacks/patchday24ng/dlc.rpf", "f1d3a69dc31f50dd7741dfe5495568af40da4191", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 384018432 });
	}
#elif IS_RDR3
	// 1311/1355 toggle
	if (IsTargetGameBuild<1355>())
	{
		g_requiredEntries.push_back({ "RDR2.exe", "c2fab1d25daef4779aafd2754ec9c593e674e7c3", "ipfs://bafybeigcudahnyogfbavh2fldp5irtm3jxvseysqyarkgibf75wcsmxo4i", 84664448 });
		g_requiredEntries.push_back({ "appdata0_update.rpf", "307609c164e78adaf4e50e993328485e6264803f", "ipfs://bafybeieesm4cgypcfesnlr4n3q5ruxbjywarnarq75bso77nu6chapktbu", 3069247 });
		g_requiredEntries.push_back({ "update.rpf", "5a087ef32e6b30b4fde8bbeda7babc45f2c1cf4d", "ipfs://bafybeie4roojrcitremf2mdsavxqkzjhwmtbj5la3lkr5wreexaybq7q4e", 4685758145 });
		g_requiredEntries.push_back({ "shaders_x64.rpf", "a7a45988a6067964214cc4b3af21797249817469", "ipfs://bafybeib2dprijqsp7xoauqsueboj4xs7kmxiwy4m4uksw2zlgp6w63zqne", 233487310 });
	}
	else
	{
		g_requiredEntries.push_back({ "RDR2.exe", "ac3c2abd80bfa949279d8e1d32105a3d9345c6c8", "ipfs://bafybeihtqz54b4or4xxqyvrih5wi4il7ni72e7qxg6mtt2c47s6bbkgy4q", 91439232 });
		g_requiredEntries.push_back({ "appdata0_update.rpf", "1715741785ce3c28adf9a78633e57f478229bb84", "ipfs://bafybeiapjt7ifvkqrtscmbgghgq2jz7ptpdxvvyqwcrxzqqoqolh45ybwq", 3003087 });
		g_requiredEntries.push_back({ "update.rpf", "835a767055cfbf2c2ad86cf4462c7dfb931970fd", "ipfs://bafybeidgpk6as7ebr4sakax2cy5mulmjr6gbb32mj74i3apbfbjbeoi2ki", 3515071792 });
		g_requiredEntries.push_back({ "shaders_x64.rpf", "77bad0ab74cd1ef7c646206ea12152449ec56cdf", "ipfs://bafybeia5ol2sjowvyfzyncn4wiyp5wtr6tdb6hlq3imbh4ed5v5hgll6jm", 233487310 });
	}

	if (IsTargetGameBuild<1355>())
	{
		g_requiredEntries.push_back({ "x64/dlcpacks/mp008/dlc.rpf", "66a50ed07293b92466423e1db5eed159551d8c25", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 487150980 });
	}
#endif

	// delete bad migration on 2019-01-10 (incorrect update.rpf download URL caused Steam users to fetch 1493.1 instead of 1604.0)
	{
		auto dataPath = MakeRelativeCitPath(L"data\\game-storage\\game_files.dat");
		auto failPath = MakeRelativeCitPath(L"data\\game-storage\\update+update.rpf_fc941d698834e30e40a06a40f6a35b1b18e1c50c");

		struct _stat64i32 statData;
		if (_wstat(failPath.c_str(), &statData) == 0)
		{
			if (statData.st_size == 928075776)
			{
				_wunlink(failPath.c_str());
				_wunlink(dataPath.c_str());
			}
		}
	}

#if defined(LAUNCHER_PERSONALITY_MAIN) || defined(COMPILING_GLUE)
	// check the game executable(s)
	for (auto& entry : g_requiredEntries)
	{
		// if it's a root-directory .exe
		if (entry.filename && strstr(entry.filename, ".exe") && !strstr(entry.filename, "/"))
		{
			auto cacheName = entry.GetCacheFileName();

			if (auto f = _wfopen(cacheName.c_str(), L"rb"); f)
			{
				SHA_CTX ctx;
				SHA1_Init(&ctx);

				uint8_t buffer[32768];

				while (!feof(f))
				{
					int read = fread(buffer, 1, sizeof(buffer), f);
					assert(read >= 0);

					SHA1_Update(&ctx, buffer, read);
				}

				fclose(f);

				uint8_t hash[20];
				SHA1_Final(hash, &ctx);

				auto origCheck = ParseHexString<20>(entry.checksums[0]);
				if (memcmp(hash, origCheck.data(), 20) != 0)
				{
					// delete both the cache metadata and the corrupted file itself
					auto dataPath = MakeRelativeCitPath(L"data\\game-storage\\game_files.dat");

					_wunlink(dataPath.c_str());
					_wunlink(cacheName.c_str());
				}
			}
		}
	}

	// perform a game update
	auto differences = CompareCacheDifferences();

	if (!differences.empty())
	{
		if (!PerformUpdate(differences))
		{
			return {};
		}
	}
#endif

	// get a list of cache files that should be mapped given an updated cache
	std::map<std::string, std::string> retval;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

	for (auto& entry : g_requiredEntries)
	{
		std::string origFileName = entry.filename;

		if (origFileName.find("ros_") == 0)
		{
			origFileName = "Social Club/" + origFileName.substr(origFileName.find_first_of("/\\") + 1);
		}

		if (origFileName.find("launcher/") == 0)
		{
			origFileName = "Launcher/" + origFileName.substr(9);
		}

		if (GetFileAttributes(entry.GetCacheFileName().c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			retval.insert({ origFileName, converter.to_bytes(entry.GetCacheFileName()) });
		}
	}

	return retval;
}
#endif
#endif
