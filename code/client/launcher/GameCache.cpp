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

#if defined(LAUNCHER_PERSONALITY_MAIN) || defined(COMPILING_GLUE)
#include <CfxLocale.h>
#include <openssl/sha.h>
#endif

#include <KnownFolders.h>
#include <shlobj.h>

#undef interface
#include "InstallerExtraction.h"
#include <array>
#include <filesystem>

#if defined(LAUNCHER_PERSONALITY_MAIN) || defined(COMPILING_GLUE)
#define CURL_STATICLIB
#include <curl/curl.h>
#include <curl/easy.h>
#endif

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

	// overridden local filename
	std::wstring localFileOverride;

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

	GameCacheEntry(const char* filename, const GameCacheEntry& prototype)
		: filename(filename), checksums(prototype.checksums), remotePath(prototype.remotePath), localSize(prototype.localSize), remoteSize(prototype.remoteSize),
			archivedFile(prototype.archivedFile), deltas(prototype.deltas), localFileOverride(prototype.localFileOverride)
	{

	}

	// methods
	bool IsPrimitiveFile() const
	{
		return std::string_view{ filename }.find("ros_") == 0 || std::string_view{ filename }.find("launcher/") == 0;
	}

	bool IsDownloadable() const
	{
		return _strnicmp(remotePath, "nope:", 5) != 0;
	}

	std::wstring GetCacheFileName(std::string_view checksum = {}) const
	{
		std::string filenameBase = filename;

		if (IsPrimitiveFile())
		{
			return MakeRelativeCitPath(ToWide(va("data\\game-storage\\%s", filenameBase.c_str())));
		}

		std::replace(filenameBase.begin(), filenameBase.end(), '/', '+');

		return MakeRelativeCitPath(ToWide(va("data\\game-storage\\%s_%s", filenameBase.c_str(), checksum.empty() ? checksums[0] : checksum)));
	}

	void SetLocalName(const std::wstring& str)
	{
		localFileOverride = str;
	}

	std::wstring GetRemoteBaseName() const
	{
		std::string remoteNameBase = remotePath;

		size_t slashIndex = remoteNameBase.find_last_of('/') + 1;

		return MakeRelativeCitPath(ToWide("data\\game-storage\\" + remoteNameBase.substr(slashIndex)));
	}

	std::wstring GetLocalFileName() const
	{
		if (!localFileOverride.empty())
		{
			return localFileOverride;
		}

		if (_strnicmp(filename, "launcher/", 9) == 0)
		{
			static auto mtlPath = ([]()
			{
				wchar_t rootBuf[1024] = { 0 };
				DWORD rootLength = sizeof(rootBuf);

				RegGetValue(HKEY_LOCAL_MACHINE,
				L"SOFTWARE\\WOW6432Node\\Rockstar Games\\Launcher", L"InstallFolder",
				RRF_RT_REG_SZ, nullptr, rootBuf, &rootLength);
				
				return std::wstring{ rootBuf };
			})();

			return mtlPath + L"\\" + ToWide(&filename[9]);
		}

		if (_strnicmp(filename, "ros_", 4) == 0)
		{
			static auto scPath = ([]() -> std::wstring
			{
				LPWSTR rootPath;
				if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, nullptr, &rootPath)))
				{
					std::wstring pathRef = rootPath;
					CoTaskMemFree(rootPath);

					return pathRef;
				}

				wchar_t rootBuf[1024] = { 0 };
				DWORD rootLength = sizeof(rootBuf);

				RegGetValue(HKEY_LOCAL_MACHINE,
				L"SOFTWARE\\WOW6432Node\\Rockstar Games\\Rockstar Games Social Club", L"InstallFolder",
				RRF_RT_REG_SZ, nullptr, rootBuf, &rootLength);
			})();

			return scPath + L"\\" + ToWide(strchr(filename, L'/') + 1);
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
	std::string_view from{
		reinterpret_cast<const char*>(fromChecksum.data()), 20
	};

	std::string_view to{
		reinterpret_cast<const char*>(toChecksum.data()), 20
	};

	return fmt::sprintf("%x_%x", std::hash<decltype(from)>()(from), std::hash<decltype(to)>()(to));
}

// Returns file size in bytes,
// returns -1 if file does not exist or any other error occured
inline std::uintmax_t GetFileSize2(const std::filesystem::path& filename)
{
	std::error_code error;
	return std::filesystem::file_size(filename, error);
}

// Checks if file exists
inline bool DoesFileExist(const std::filesystem::path& filename)
{
	std::error_code error;
	return std::filesystem::exists(filename, error);
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
	{ "ros_2090/socialclub.dll", "AE14687363C0FB5A8B086B4EB24D5A6E2D5161B9", "https://content.cfx.re/mirrors/ros/2.0.9.0/socialclub.dll", 5287320 },
	{ "ros_2090/socialclub.pak", "D70F269F7EBBA3A13AA2871BAFA58212B01E6280", "https://content.cfx.re/mirrors/ros/2.0.9.0/socialclub.pak", 4996 },

	// RDR3 expects these to exist for SC SDK init to succeed
#ifdef IS_RDR3
	{ "ros_2090/SocialClubD3D12Renderer.dll", "73A1421E35B5ED105FA9AF8445F62F0A42EE3C41", "https://content.cfx.re/mirrors/ros/2.0.9.0/SocialClubD3D12Renderer.dll", 415128 },
	{ "ros_2090/SocialClubVulkanLayer.dll", "572E95099825B507079349A2B24BBAE4C1567B84", "https://content.cfx.re/mirrors/ros/2.0.9.0/SocialClubVulkanLayer.dll", 476056 },
	{ "ros_2090/SocialClubVulkanLayer.json", "5DA071BDE81BF96C8939978343C6B5B93730CB39", "https://content.cfx.re/mirrors/ros/2.0.9.0/SocialClubVulkanLayer.json", 339 },
#endif

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

template<int Size>
static std::string FormatHexString(const std::array<uint8_t, Size>& arr)
{
	static const char charTable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	char stringBuffer[(Size * 2) + 1] = { 0 };

	for (size_t i = 0; i < Size; i++)
	{
		stringBuffer[i * 2] = charTable[(arr[i] >> 4) & 0xF];
		stringBuffer[i * 2 + 1] = charTable[arr[i] & 0xF];
	}

	return stringBuffer;
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

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do 
		{
			// try parsing the file name
			std::string fileName;
			std::string fileHash;

			if (ParseCacheFileName(ToNarrow(findData.cFileName).c_str(), fileName, fileHash))
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
					if (entry.localSize != 0)
					{
						bool shouldAddForCheck = false;

						if (entry.IsDownloadable())
						{
							// Add entry for check if cache file does not exist or it's size doesn't match expected (remoteSize)
							shouldAddForCheck = entry.remoteSize != GetFileSize2(entry.GetCacheFileName());
						}
						else
						{
							// Add entry for check if local file does not exist
							shouldAddForCheck = !DoesFileExist(entry.GetLocalFileName());
						}

						if (shouldAddForCheck)
						{
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
				if (entry.localSize == GetFileSize2(entry.GetCacheFileName()))
				{
					found = true;
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

static const char* const kByteStringsUnlocalized[] = {
	" B",
	" kB",
	" MB",
	" GB",
	" TB",
	" PB"
};

static std::wstring FormatBytes(int64_t bytes)
{
	double unit_amount = static_cast<double>(bytes);
	size_t dimension = 0;
	const int kKilo = 1024;
	while (unit_amount >= kKilo && dimension < std::size(kByteStringsUnlocalized) - 1)
	{
		unit_amount /= kKilo;
		dimension++;
	}

	if (bytes != 0 && dimension > 0 && unit_amount < 100)
	{
		return ToWide(fmt::sprintf("%.1lf%s", unit_amount,
		kByteStringsUnlocalized[dimension]));
	}
	else
	{
		return ToWide(fmt::sprintf("%.0lf%s", unit_amount,
		kByteStringsUnlocalized[dimension]));
	}
}


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
		if (!entry.first.IsDownloadable())
		{
			shouldAllow = false;
			badEntries += entry.first.filename;
			badEntries += "\n";
		}

		// if it's a local file...
		if (entry.second)
		{
			localSize += entry.first.localSize;

			detailStr << entry.first.filename << L" (local, " << FormatBytes(entry.first.localSize) << L")\n";
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

			detailStr << entry.first.remotePath << L" (download, " << FormatBytes(entry.first.remoteSize) << L")\n";
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
		taskDialogConfig.pszContent = va(gettext(L"The local %s game data is outdated, and needs to be updated. This will copy %s of data from the local disk, and download %s of data from the internet.\nDo you wish to continue?"), PRODUCT_NAME, FormatBytes(localSize), FormatBytes(remoteSize));
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

static void BumpDownloadCount(const std::shared_ptr<baseDownload>& download, const std::string& key)
{
	DWORD count = 0;
	DWORD countLen = sizeof(count);
	RegGetValueW(HKEY_CURRENT_USER, L"Software\\CitizenFX\\DownloadCount", ToWide(key).c_str(), RRF_RT_REG_DWORD, NULL, &count, &countLen);

	++count;
	download->count = count;

	RegSetKeyValueW(HKEY_CURRENT_USER, L"Software\\CitizenFX\\DownloadCount", ToWide(key).c_str(), REG_DWORD, &count, sizeof(count));
}

#include "ZlibDecompressPlugin.h"

static bool PerformUpdate(const std::vector<GameCacheEntry>& entries)
{
	// create UI
	UI_DoCreation();
	CL_InitDownloadQueue();

	// hash local files for those that *do* exist, add those that don't match to the download queue and add those that do match to be copied locally
	std::set<std::string> referencedFiles; // remote URLs that we already requested
	std::vector<GameCacheEntry> extractedEntries; // entries to extract from an archive

	// entries for notification purposes
	std::vector<std::pair<GameCacheEntry, bool>> notificationEntries;

	uint64_t fileStart = 0;
	uint64_t fileTotal = 0;

	for (auto& entry : entries)
	{
		if (entry.IsDownloadable())
		{
			struct _stat64 stat;
			if (_wstat64(entry.GetLocalFileName().c_str(), &stat) >= 0)
			{
				fileTotal += stat.st_size;
			}
		}
	}

	std::vector<std::tuple<DeltaEntry, GameCacheEntry>> theseDeltas;

	for (const auto& baseEntry : entries)
	{
		auto entryPtr = &baseEntry;
		const auto& deltaEntries = baseEntry.deltas;

		// try to get the smallest local entry
		GameCacheEntry newEntry("", {}, "", 0);

		{
			std::vector<std::tuple<int64_t, std::wstring>> presentDeltas;

			for (const auto& deltaEntry : deltaEntries)
			{
				auto localName = baseEntry.GetCacheFileName(FormatHexString(deltaEntry.fromChecksum));

				if (GetFileAttributesW(localName.c_str()) != INVALID_FILE_ATTRIBUTES)
				{
					presentDeltas.emplace_back(int64_t(deltaEntry.dlSize), localName);
				}
			}

			if (!presentDeltas.empty())
			{
				std::sort(presentDeltas.begin(), presentDeltas.end());

				newEntry = baseEntry;
				newEntry.SetLocalName(std::get<1>(presentDeltas[0]));

				entryPtr = &newEntry;
			}
		}

		// continue on
		const auto& entry = *entryPtr;

		// check if the file is outdated
		std::vector<std::array<uint8_t, 20>> hashes;

		for (auto& checksum : entry.checksums)
		{
			hashes.push_back(ParseHexString<20>(checksum));
		}

		for (auto& deltaEntry : deltaEntries)
		{
			hashes.push_back(deltaEntry.fromChecksum);
		}
		
		std::array<uint8_t, 20> outHash;
		bool fileOutdated = false;
		
		if (entry.IsDownloadable())
		{
			UI_UpdateText(0, gettext(L"Verifying game content...").c_str());

			fileOutdated = CheckFileOutdatedWithUI(entry.GetLocalFileName().c_str(), hashes, &fileStart, fileTotal, &outHash);
		}
		else
		{
			// Non-downloadable files just get a size check, no whole hash check
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

		// if not, copy it from the local filesystem (we're abusing the download code here a lot)
		if (!fileOutdated)
		{
			// should we 'nope' this file?
			if (!entry.IsDownloadable())
			{
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
					std::string escapedUrl;

					{
						auto curl = curl_easy_init();

						char* escapedUrlRaw = curl_easy_escape(curl, ToNarrow(entry.GetLocalFileName()).c_str(), 0);
						escapedUrl = escapedUrlRaw;

						curl_free(escapedUrlRaw);
						curl_easy_cleanup(curl);
					}

					CL_QueueDownload(va("file:///%s", escapedUrl), ToNarrow(entry.GetCacheFileName()).c_str(), entry.localSize, compressionAlgo_e::None);

					notificationEntries.push_back({ entry, true });
				}
				else
				{
					for (auto& deltaEntry : deltaEntries)
					{
						if (outHash == deltaEntry.fromChecksum)
						{
							auto download = CL_QueueDownload(deltaEntry.remoteFile.c_str(), ToNarrow(deltaEntry.GetLocalFileName()).c_str(), deltaEntry.dlSize, compressionAlgo_e::None);
							BumpDownloadCount(download, fmt::sprintf("%s_delta_%s", FormatHexString(deltaEntry.toChecksum), FormatHexString(deltaEntry.fromChecksum)));

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
				std::string localFileName = (entry.archivedFile) ? ToNarrow(entry.GetRemoteBaseName()) : ToNarrow(entry.GetCacheFileName());
				const char* remotePath = entry.remotePath;

				if (_strnicmp(remotePath, "http", 4) != 0)
				{
					remotePath = va("rockstar:%s", entry.remotePath);
				}

				// if the file isn't of the original size
				auto download = CL_QueueDownload(remotePath, localFileName.c_str(), entry.remoteSize, ((entry.remoteSize != entry.localSize && !entry.archivedFile) ? compressionAlgo_e::XZ : compressionAlgo_e::None));
				BumpDownloadCount(download, entry.checksums[0]);

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

	UI_UpdateText(0, gettext(L"Updating game storage...").c_str());

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
						entry->streamImport = nullptr;
						FILE* f = _wfopen(fn.c_str(), L"rb");

						if (!f)
						{
							return false;
						}

						_fseeki64(f, 0, SEEK_END);
						entry->streamImport = (void*)f;
						entry->streamSize = _ftelli64(f);

						entry->read = [](const hpatch_TStreamInput* entry, hpatch_StreamPos_t at, uint8_t* begin, uint8_t* end) -> hpatch_BOOL {
							auto size = end - begin;

							FILE* f = (FILE*)entry->streamImport;
							_fseeki64(f, at, SEEK_SET);

							return (fread(begin, 1, size, f) == size);
						};

						return true;
					};

					UI_UpdateText(1, va(L"Patching %s", ToWide(entry.filename)));

					auto outSize = entry.localSize;

					auto openWrite = [outSize](hpatch_TStreamOutput* entry, const std::wstring& fn)
					{
						entry->streamImport = nullptr;
						FILE* f = _wfopen(fn.c_str(), L"wb");

						if (!f)
						{
							return false;
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
							_fseeki64(f, at, SEEK_SET);

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

						return true;
					};

					auto doClose = [](auto* entry) 
					{
						if (entry->streamImport)
						{
							fclose((FILE*)entry->streamImport);
							entry->streamImport = nullptr;
						}
					};

					auto theFile = entry.GetCacheFileName();
					auto tmpFile = theFile + L".tmp";

					retval = retval && openRead(&oldFile, entry.GetLocalFileName());
					retval = retval && openRead(&deltaFile, deltaEntry.GetLocalFileName());
					retval = retval && openWrite(&outFile, tmpFile);

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
						UI_DisplayError(va(L"Could not patch %s. Do you have enough free disk space on all drives? (~2 GB)", ToWide(entry.filename)));

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
extern bool gameCacheReplaceExecutable;

inline int GetTargetGameBuild()
{
	return gameCacheTargetBuild;
}

inline int GetReplaceExecutable()
{
	return gameCacheReplaceExecutable;
}
#else
inline int GetTargetGameBuild()
{
	return xbr::GetRequestedGameBuild();
}

inline bool GetReplaceExecutable()
{
	return xbr::GetReplaceExecutable();
}
#endif

template<int Build>
bool IsTargetGameBuild()
{
	return GetTargetGameBuild() == Build;
}

template<int Build>
bool IsTargetGameBuildOrGreater()
{
	return GetTargetGameBuild() >= Build;
}

static std::map<int, std::map<std::string, GameCacheEntry>> g_entriesToLoadPerBuild = {
#ifdef GTA_FIVE
	{
		3323,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "059bcf06de5a683ad39f8d24543cae80a988b4cb", "https://content.cfx.re/mirrors/patches_fivem/3323/GTA5.exe", 57496560 }
			},
			{
				"update.rpf",
				{ 
					"update/update.rpf", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://content.cfx.re/mirrors/patches_fivem/3323/update.rpf", 1423288320,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_1604_to_3323.update.rpf.hdiff", 932235268 } /* diff sha1: ea4f44780ed788a6c0acf0c09e1d1f96c3cc84ff */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_2060_to_3323.update.rpf.hdiff", 704310918 } /* diff sha1: 2bd7c551d7d9ded134c7b88556c83b0e1a9437e9 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_2189_to_3323.update.rpf.hdiff", 663014659 } /* diff sha1: bef7ecca334d5dc3887749b80f653f6f7071017d */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_2372_to_3323.update.rpf.hdiff", 646437484 } /* diff sha1: cb0ef5e15d897d42d843974135af5afcf13a6912 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_2545_to_3323.update.rpf.hdiff", 507689034 } /* diff sha1: d821a26c0e436f024850ee621d810ee3c6da41b0 */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_2612_to_3323.update.rpf.hdiff", 507688645 } /* diff sha1: 8b1648690f13fb96e42a27cf55e808b35eb4527f */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_2699_to_3323.update.rpf.hdiff", 462961012 } /* diff sha1: cd2bbded7f26f63cbf30d8607bb5a28034b7c46b */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_2802_to_3323.update.rpf.hdiff", 406279747 } /* diff sha1: ec590139ddddd6f4c4df2754ce6e96119b022ce4 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_2944_to_3323.update.rpf.hdiff", 393108441 } /* diff sha1: be7846a48dd84f2fe0c75a7693091d69e0674006 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_3095_to_3323.update.rpf.hdiff", 63784896 } /* diff sha1: 46d8201479456cf560b74ed95f12557516e75e0d */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_3179_to_3323.update.rpf.hdiff", 63784896 } /* diff sha1: 46d8201479456cf560b74ed95f12557516e75e0d */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_3258_to_3323.update.rpf.hdiff", 232538 } /* diff sha1: 62ba41cf38e55fad1de2e4679787cae91a0dcdd2 */
					}
				}
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://content.cfx.re/mirrors/patches_fivem/3323/update2.rpf", 416063488,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_2612_to_3323.update2.rpf.hdiff", 345380507 } /* diff sha1: b1c69025dbd7bdf2233613f74def7d094b342d79 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_2699_to_3323.update2.rpf.hdiff", 343671960 } /* diff sha1: 7cdbeb852cb14b00c3d13da7c0c61b73a9cd7a04 */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_2802_to_3323.update2.rpf.hdiff", 343371839 } /* diff sha1: d663b68258da5662dcbd1cd2a281cbc13cc4b357 */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_2944_to_3323.update2.rpf.hdiff", 343112220 } /* diff sha1: 196689f39a02ddfd8a89c599c0c2a0e3aa9dd64d */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_3095_to_3323.update2.rpf.hdiff", 340833331 } /* diff sha1: af00ec37780727a25e02b555c4904b525241a2c4 */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_3179_to_3323.update2.rpf.hdiff", 340835870 } /* diff sha1: d2006d94b6edeb0b495baf1f3c841340d25086e7 */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://content.cfx.re/mirrors/patches_fivem/3323/diffs/from_3258_to_3323.update2.rpf.hdiff", 327858356 } /* diff sha1: 82bd2cf214f88f25c01ab31b7a0237203ddfbcda */
					}
				}
			}
		}
	},
	{
		3258,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "17183412df26a019386ffd5097df697d9041bb3d", "https://content.cfx.re/mirrors/patches_fivem/3258/GTA5.exe", 56066032 }
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://content.cfx.re/mirrors/patches_fivem/3258/update.rpf", 1423288320,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_1604_to_3258.update.rpf.hdiff", 932235213 } /* diff sha1: 0c56b47728ae7f3b7b3ed19fcd940a9de9e0336d */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_2060_to_3258.update.rpf.hdiff", 704311288 } /* diff sha1: 104f4370c10d294e6aad8d7a11d0ac740945d050 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_2189_to_3258.update.rpf.hdiff", 663014595 } /* diff sha1: a86a8943032ef74edf65ef6eaa68148b16ab1c7e */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_2372_to_3258.update.rpf.hdiff", 646437117 } /* diff sha1: 7e4b9115d91c33b396b75ee897595e87f5571f90 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_2545_to_3258.update.rpf.hdiff", 507688763 } /* diff sha1: 92ad3f81f9888bdf5fb3390dc1cfcfa33016bdbc */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_2612_to_3258.update.rpf.hdiff", 507688310 } /* diff sha1: ddc065ad6845b0f0eca85d124016dbdd32599c62 */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_2699_to_3258.update.rpf.hdiff", 462960589 } /* diff sha1: eef31a26c45de0f63a6a35b0d9d3215d59e4c4a3 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_2802_to_3258.update.rpf.hdiff", 406279730 } /* diff sha1: 7a8216f59fe7fd73409768f273050b1797316087 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_2944_to_3258.update.rpf.hdiff", 393108418 } /* diff sha1: d5a358eb14bb6ceb0f0cd93a318418b61b225580 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_3095_to_3258.update.rpf.hdiff", 63780343 } /* diff sha1: 7390c645e320807503b25aa86642daa6835c8a09 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_3179_to_3258.update.rpf.hdiff", 63780343 } /* diff sha1: 7390c645e320807503b25aa86642daa6835c8a09 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_3323_to_3258.update.rpf.hdiff", 232538 } /* diff sha1: 0a8f2f2cedf997b849c571bf9fd7ab0165e81faf */
					}
				}
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://content.cfx.re/mirrors/patches_fivem/3258/update2.rpf", 416053248,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_2612_to_3258.update2.rpf.hdiff", 345380170 } /* diff sha1: 2556e1bbfc0685e721e872d5c7f26e6b07fcfaf1 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_2699_to_3258.update2.rpf.hdiff", 343680669 } /* diff sha1: e0ff2108c7201f6fa3a4a52a55ac5bd5209fe1f3 */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_2802_to_3258.update2.rpf.hdiff", 343376273 } /* diff sha1: 4fe9189dcbedbfda93ad7926139ea7e85fee3bcf */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_2944_to_3258.update2.rpf.hdiff", 343101974 } /* diff sha1: adf4ddaa6eb0e12fded154e8a89e028f8e4ed1a5 */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_3095_to_3258.update2.rpf.hdiff", 340810631 } /* diff sha1: 93af9830292c7380b2ebce4b399f077203089d65 */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_3179_to_3258.update2.rpf.hdiff", 340801650 } /* diff sha1: 27c126578d8860ae1fa3329df776083b67d39517 */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://content.cfx.re/mirrors/patches_fivem/3258/diffs/from_3323_to_3258.update2.rpf.hdiff", 327846154 } /* diff sha1: 24e881657fec709df192f3ed4832cfa377eb3d44 */
					}
				}
			}
		}
	},
	{
		3179,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "cf2b853ae2125a26e636daa99f6377b05baaad8a", "https://content.cfx.re/mirrors/patches_fivem/3179/GTA5.exe", 55367152 }
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3179/update.rpf", 1416300544,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_1604_to_3179.update.rpf.hdiff", 923117905 } /* diff sha1: 1b22013721291d5bd65872bd9d179fc01a695fa3 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_2060_to_3179.update.rpf.hdiff", 694772098 } /* diff sha1: e3e7071607195ebda498e945b07bc174caba90a5 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_2189_to_3179.update.rpf.hdiff", 653422887 } /* diff sha1: 7b546f88560c86405cf43c7b4c5bc5c9d0bb4b50 */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_2372_to_3179.update.rpf.hdiff", 636414267 } /* diff sha1: c04b08faa8eabbf161f4b7b27d44c114738d668c */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_2545_to_3179.update.rpf.hdiff", 496735908 } /* diff sha1: 4123b2c63d466ee53fb3b8f8e9306bdf44d2d08d */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_2612_to_3179.update.rpf.hdiff", 496733750 } /* diff sha1: 53ee1f7b28816eed1f9899b7d09a6547ee9fbbb9 */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_2699_to_3179.update.rpf.hdiff", 452905479 } /* diff sha1: 0932832b87a22c963165da8d9cf16b8faede1cd2 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_2802_to_3179.update.rpf.hdiff", 386841388 } /* diff sha1: c11d4796c020236b173cb01eb99cade83d049319 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_2944_to_3179.update.rpf.hdiff", 341954506 } /* diff sha1: 62e7153e824e585739105e3610f16b2518356e64 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_3258_to_3179.update.rpf.hdiff", 56823148 } /* diff sha1: 5b581fa05751c05ec24fd92da7c3644d4a234b74 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_3323_to_3179.update.rpf.hdiff", 56828311 } /* diff sha1: a3979a12f02688bb65b68a9d86c4d3c5e1990372 */
					}
				}
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://content.cfx.re/mirrors/patches_fivem/3179/update2.rpf", 403941376,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_2612_to_3179.update2.rpf.hdiff", 333197023 } /* diff sha1: b3954312af1c0a580f547364a4dd73514cd15828 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_2699_to_3179.update2.rpf.hdiff", 331500185 } /* diff sha1: 63f69d3d5f75784f13d4b7cac1544385f111dc1e */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_2802_to_3179.update2.rpf.hdiff", 331176908 } /* diff sha1: f7e02f4b87d5ccbfc577817c004d5e82b28f6884 */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_2944_to_3179.update2.rpf.hdiff", 330898163 } /* diff sha1: 451e7b646e9f0b7a62e23d6fae193cc09cdc564c */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_3095_to_3179.update2.rpf.hdiff", 313464391 } /* diff sha1: 191fe3dde2ab0d31118769a65488c6561cfd6cf0 */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_3258_to_3179.update2.rpf.hdiff", 328689523 } /* diff sha1: fba401a95e81cbda8f88a861fb747f2774fad663 */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://content.cfx.re/mirrors/patches_fivem/3179/diffs/from_3323_to_3179.update2.rpf.hdiff", 328713490 } /* diff sha1: 0d9ef440d1f9991f0ade93fe7f9b0b1d1eeb12b7 */
					}
				}
			}
		}
	},
	{
		3095,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "17a074bb8eaca5bd8df863de84869a4ab023e1eb", "https://content.cfx.re/mirrors/patches_fivem/3095/GTA5.exe", 49634800 }
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3095/update.rpf", 1416300544,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_1604_to_3095.update.rpf.hdiff", 923117905 } /* diff sha1: 1b22013721291d5bd65872bd9d179fc01a695fa3 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_2060_to_3095.update.rpf.hdiff", 694772098 } /* diff sha1: e3e7071607195ebda498e945b07bc174caba90a5 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_2189_to_3095.update.rpf.hdiff", 653422887 } /* diff sha1: 7b546f88560c86405cf43c7b4c5bc5c9d0bb4b50 */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_2372_to_3095.update.rpf.hdiff", 636414267 } /* diff sha1: c04b08faa8eabbf161f4b7b27d44c114738d668c */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_2545_to_3095.update.rpf.hdiff", 496735908 } /* diff sha1: 4123b2c63d466ee53fb3b8f8e9306bdf44d2d08d */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_2612_to_3095.update.rpf.hdiff", 496733750 } /* diff sha1: 53ee1f7b28816eed1f9899b7d09a6547ee9fbbb9 */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_2699_to_3095.update.rpf.hdiff", 452905479 } /* diff sha1: 0932832b87a22c963165da8d9cf16b8faede1cd2 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_2802_to_3095.update.rpf.hdiff", 386841388 } /* diff sha1: c11d4796c020236b173cb01eb99cade83d049319 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_2944_to_3095.update.rpf.hdiff", 341954506 } /* diff sha1: 62e7153e824e585739105e3610f16b2518356e64 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_3258_to_3095.update.rpf.hdiff", 56823148 } /* diff sha1: 5b581fa05751c05ec24fd92da7c3644d4a234b74 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_3323_to_3095.update.rpf.hdiff", 56828311 } /* diff sha1: a3979a12f02688bb65b68a9d86c4d3c5e1990372 */
					}
				}
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://content.cfx.re/mirrors/patches_fivem/3095/update2.rpf", 403945472,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_2612_to_3095.update2.rpf.hdiff", 333194048 } /* diff sha1: 7e3c826eb759abaa35b68c1be25fa6e673ad9802 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_2699_to_3095.update2.rpf.hdiff", 331503702 } /* diff sha1: fc111e310931d769d8dfce514d1b567f261ddf40 */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_2802_to_3095.update2.rpf.hdiff", 331190538 } /* diff sha1: 11683ea488d00b1e3e9bfb733fa9f032cee9b4b0 */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_2944_to_3095.update2.rpf.hdiff", 330911852 } /* diff sha1: b018bd07b4d4f3ff3670cd0523cdf2d9ef9466fb */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_3179_to_3095.update2.rpf.hdiff", 313461829 } /* diff sha1: c0beaa52ddda5bd084ad1512cdbef79f4749ec31 */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_3258_to_3095.update2.rpf.hdiff", 328700911 } /* diff sha1: 31048725810d4bf433da58c984577c51a03c9412 */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://content.cfx.re/mirrors/patches_fivem/3095/diffs/from_3323_to_3095.update2.rpf.hdiff", 328715458 } /* diff sha1: a9a916d7ae704a2d1b162c5cbe88273ae64d272f */
					}
				}
			}
		}
	},
	{
		2944,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "4d968a0754d59d30b29cd7b01a06e4685a5fa49c", "https://content.cfx.re/mirrors/patches_fivem/2944/GTA5.exe", 49828848 }
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://content.cfx.re/mirrors/patches_fivem/2944/update.rpf", 1087019008,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_1604_to_2944.update.rpf.hdiff", 596587088 } /* diff sha1: 3de54ae4dc5a3d27a4e5621c7690d19f0f6fc432 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_2060_to_2944.update.rpf.hdiff", 368280541 } /* diff sha1: 258f4ee6773872199f664e9c55f8243b0298c48d */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_2189_to_2944.update.rpf.hdiff", 326614727 } /* diff sha1: 1bea466fcccfc2b5275995b85786afc3a952a6e3 */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_2372_to_2944.update.rpf.hdiff", 307384637 } /* diff sha1: 92d41ca77ab60a2de950da2f206a8469415f829b */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_2545_to_2944.update.rpf.hdiff", 166552358 } /* diff sha1: a20129a7d79d56dc57e477adcf69583f30188d88 */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_2612_to_2944.update.rpf.hdiff", 166551505 } /* diff sha1: c65fad96304ee0205dd9efee508e04b53d50d895 */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_2699_to_2944.update.rpf.hdiff", 121830364 } /* diff sha1: 3e3b0549f13e0debbc37cfab480a2b902316c199 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_2802_to_2944.update.rpf.hdiff", 56081281 } /* diff sha1: eb75888e835e3ef58aa736cbb36747194b0eeea6 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_3095_to_2944.update.rpf.hdiff", 15617655 } /* diff sha1: 5b98a4a7ff869b724d477d03b5b4bd7037468b74 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_3179_to_2944.update.rpf.hdiff", 15617655 } /* diff sha1: 5b98a4a7ff869b724d477d03b5b4bd7037468b74 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_3258_to_2944.update.rpf.hdiff", 59814458 } /* diff sha1: 6877cf0cac02ee1ce085e500cb9a3530617aa63d */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_3323_to_2944.update.rpf.hdiff", 59814456 } /* diff sha1: e6767ca11d311070115de3ffb64c0841a1a7770e */
					}
				}
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "a3181d68a532950da5c584100b35f79eaca7c884", "https://content.cfx.re/mirrors/patches_fivem/2944/update2.rpf", 352088064,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "a3181d68a532950da5c584100b35f79eaca7c884", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_2612_to_2944.update2.rpf.hdiff", 280860829 } /* diff sha1: 1ec47c39f3c5a504e7ca7d89221eea17335fa555 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "a3181d68a532950da5c584100b35f79eaca7c884", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_2699_to_2944.update2.rpf.hdiff", 277845689 } /* diff sha1: 00c967b55a0d8c299bc265f0979541b3eb49bb8f */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "a3181d68a532950da5c584100b35f79eaca7c884", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_2802_to_2944.update2.rpf.hdiff", 277310322 } /* diff sha1: 677514fc14030cdb8645b2b14bae383ba3a841df */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "a3181d68a532950da5c584100b35f79eaca7c884", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_3095_to_2944.update2.rpf.hdiff", 279052198 } /* diff sha1: a9155d4a892c21f9e1589c1b04ea7c53618e4d3e */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "a3181d68a532950da5c584100b35f79eaca7c884", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_3179_to_2944.update2.rpf.hdiff", 279043729 } /* diff sha1: b445466b21226c260c76c18b43000e922f763148 */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "a3181d68a532950da5c584100b35f79eaca7c884", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_3258_to_2944.update2.rpf.hdiff", 279135120 } /* diff sha1: 4f5cc49e5c8a2bb825efcee77c5090bf842987ed */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "a3181d68a532950da5c584100b35f79eaca7c884", "https://content.cfx.re/mirrors/patches_fivem/2944/diffs/from_3323_to_2944.update2.rpf.hdiff", 279135010 } /* diff sha1: 063bbc27788d848246ec183f46e93e0a9d89af30 */
					}
				}
			}
		}
	},
	{
		2802,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "ebb6c144c5befe3529235deccbd8f59d6ce1a76c", "https://content.cfx.re/mirrors/patches_fivem/2802/GTA5.exe", 46709592 }
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "66388a381347511b7b28aaf91741615e45008e8b", "https://content.cfx.re/mirrors/patches_fivem/2802/update.rpf", 1079308288,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "66388a381347511b7b28aaf91741615e45008e8b", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_1604_to_2802.update.rpf.hdiff", 586455637 } /* diff sha1: 5acad3e7e9c44e87f886c8b554395b24a749fa21 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "66388a381347511b7b28aaf91741615e45008e8b", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_2060_to_2802.update.rpf.hdiff", 356615804 } /* diff sha1: cdd476afd0846a50821c5afde019f4e69cec4f9c */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "66388a381347511b7b28aaf91741615e45008e8b", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_2189_to_2802.update.rpf.hdiff", 314177287 } /* diff sha1: be29acd02d02a7b18f8d1829e00a9d9e225b24ea */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "66388a381347511b7b28aaf91741615e45008e8b", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_2372_to_2802.update.rpf.hdiff", 294843255 } /* diff sha1: be53e3444cec1fe69b8b2d8afe8e509397d47ff3 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "66388a381347511b7b28aaf91741615e45008e8b", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_2545_to_2802.update.rpf.hdiff", 153796192 } /* diff sha1: d6e8425fab4b55118bcc3fbd94dfa6c70b17bf82 */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "66388a381347511b7b28aaf91741615e45008e8b", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_2612_to_2802.update.rpf.hdiff", 153795013 } /* diff sha1: 823f2df6a209a6a1dd5d06edcf74402868baedfc */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "66388a381347511b7b28aaf91741615e45008e8b", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_2699_to_2802.update.rpf.hdiff", 107750632 } /* diff sha1: ef20902617372028b4caac304b42d38a5d3eec8d */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "66388a381347511b7b28aaf91741615e45008e8b", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_2944_to_2802.update.rpf.hdiff", 48422944 } /* diff sha1: 8baed7a1e37c6cc37be64e6ba5ebae246c2087dc */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "66388a381347511b7b28aaf91741615e45008e8b", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_3095_to_2802.update.rpf.hdiff", 52843554 } /* diff sha1: 19a5a38d4cc02ec5b086dcd8999cd83bed81e145 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "66388a381347511b7b28aaf91741615e45008e8b", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_3179_to_2802.update.rpf.hdiff", 52843554 } /* diff sha1: 19a5a38d4cc02ec5b086dcd8999cd83bed81e145 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "66388a381347511b7b28aaf91741615e45008e8b", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_3258_to_2802.update.rpf.hdiff", 65327489 } /* diff sha1: 9dcfaf73965e139e6420b3049205fdc792654167 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "66388a381347511b7b28aaf91741615e45008e8b", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_3323_to_2802.update.rpf.hdiff", 65327479 } /* diff sha1: 36b634accceba01c1f42e227ded30ed40d9bfd23 */
					}
				}
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://content.cfx.re/mirrors/patches_fivem/2802/update2.rpf", 344610816,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_2612_to_2802.update2.rpf.hdiff", 273248338 } /* diff sha1: 0f392e33b0270e1cdabbff1a30e21e172db2e592 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_2699_to_2802.update2.rpf.hdiff", 270218937 } /* diff sha1: 54bb6ba1f80437fd8a9742c25d1a1b5075d93f9f */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_2944_to_2802.update2.rpf.hdiff", 269824187 } /* diff sha1: fc8d8fc5d8eac3cd48d6c7b03c4b6356816d2324 */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_3095_to_2802.update2.rpf.hdiff", 271850052 } /* diff sha1: e4115d62fdd9f23df93d5daa718431a24883976a */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_3179_to_2802.update2.rpf.hdiff", 271841345 } /* diff sha1: 08bca2abc165d9f44f4ab969b0d5118eef8882f2 */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_3258_to_2802.update2.rpf.hdiff", 271934438 } /* diff sha1: ab1440c9acd248d1219b1fee619ba496c9db8635 */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://content.cfx.re/mirrors/patches_fivem/2802/diffs/from_3323_to_2802.update2.rpf.hdiff", 271926188 } /* diff sha1: 82177a7cf2f395ba599d81bac3023e78a9a6c830 */
					}
				}
			}
		}
	},
	{
		2699,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "b9f3960ca0c7c05aab23d3b1d158309bc085fbbe", "https://content.cfx.re/mirrors/patches_fivem/2699/GTA5.exe", 61111680 }
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://content.cfx.re/mirrors/patches_fivem/2699/update.rpf", 1073854464,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_1604_to_2699.update.rpf.hdiff", 577779656 } /* diff sha1: 00cd98e09e02b24a3dbfb52c272e3e78e945fc4e */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_2060_to_2699.update.rpf.hdiff", 346805883 } /* diff sha1: 180cc0d3498dba3fb09118a670b2185ca8d324e6 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_2189_to_2699.update.rpf.hdiff", 304782654 } /* diff sha1: 1f33549326258481701e1c909e768d60cfdf6c9c */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_2372_to_2699.update.rpf.hdiff", 285467525 } /* diff sha1: e36f07ae02019e2f313d8f089a61a48b3545f556 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_2545_to_2699.update.rpf.hdiff", 144813901 } /* diff sha1: 997cba235f0925e686220200d7a6758b83784e73 */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_2612_to_2699.update.rpf.hdiff", 144812091 } /* diff sha1: e5636168e11f31204b6aa7bc26a2f29a15df2ea6 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_2802_to_2699.update.rpf.hdiff", 102324935 } /* diff sha1: 9e52c963dd498af0abe560241673e2e9963d917f */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_2944_to_2699.update.rpf.hdiff", 108745603 } /* diff sha1: 590e601cbc82f332f7619b78dbb93a3d72d7c741 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_3095_to_2699.update.rpf.hdiff", 113480643 } /* diff sha1: f57949a6c14e71cfa119d7aacb128796c8fcd2e3 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_3179_to_2699.update.rpf.hdiff", 113480643 } /* diff sha1: f57949a6c14e71cfa119d7aacb128796c8fcd2e3 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_3258_to_2699.update.rpf.hdiff", 116581930 } /* diff sha1: ade215b3b763433132d0636948ae0af442a5fdfc */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_3323_to_2699.update.rpf.hdiff", 116581929 } /* diff sha1: c6cd8ce18160bbc2f10a3bf8753f78772b740a9f */
					}
				}
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://content.cfx.re/mirrors/patches_fivem/2699/update2.rpf", 324530176,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_2612_to_2699.update2.rpf.hdiff", 252956098 } /* diff sha1: 6a8df16ec61595298907a9204272567a4c9e1a2f */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_2802_to_2699.update2.rpf.hdiff", 250133852 } /* diff sha1: 547a19410dd0f7cb91816b028405cde27b4caa9a */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_2944_to_2699.update2.rpf.hdiff", 250283500 } /* diff sha1: 76f859c0f2bc7b7c38d1235ee70fc2baf50f6f98 */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_3095_to_2699.update2.rpf.hdiff", 252084865 } /* diff sha1: b574885cb096adfbf15bb776eac029bc5423add9 */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_3179_to_2699.update2.rpf.hdiff", 252085404 } /* diff sha1: 91ee0b5cbd819ae914fb61b2f47768593c2e5add */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_3258_to_2699.update2.rpf.hdiff", 252153440 } /* diff sha1: 1ab962c656573f190634897a3209b49fc35c9e05 */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://content.cfx.re/mirrors/patches_fivem/2699/diffs/from_3323_to_2699.update2.rpf.hdiff", 252145327 } /* diff sha1: 17ec0c0f36846a6da8fec744a46379d73640c922 */
					}
				}
			}
		}
	},
	{
		2612,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "d423086fd7a7721b8be77cfb9a4f8826784b284b", "https://content.cfx.re/mirrors/patches_fivem/2612/GTA5.exe", 60351952 }
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "80f9bd028e5bc781f641fe210a88579eff827989", "https://content.cfx.re/mirrors/patches_fivem/2612/update.rpf", 1056649216,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "80f9bd028e5bc781f641fe210a88579eff827989", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_1604_to_2612.update.rpf.hdiff", 560353786 } /* diff sha1: 71780b7e6701c9efe313c8252345ab88df4f3734 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "80f9bd028e5bc781f641fe210a88579eff827989", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_2060_to_2612.update.rpf.hdiff", 330036916 } /* diff sha1: 9b67a7840d9c33a4354b168fb580876d1281d30a */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "80f9bd028e5bc781f641fe210a88579eff827989", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_2189_to_2612.update.rpf.hdiff", 287033107 } /* diff sha1: 939717a2b3b3b4b25512b0eeb354cf557abc928a */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "80f9bd028e5bc781f641fe210a88579eff827989", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_2372_to_2612.update.rpf.hdiff", 266987849 } /* diff sha1: b1d38bb2ee75d5989201cfb58edcf2f841224c3d */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "80f9bd028e5bc781f641fe210a88579eff827989", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_2545_to_2612.update.rpf.hdiff", 1840945 } /* diff sha1: 35c6b1e1edb8a73e8156bf6d19f3d5aaee5c01a2 */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "80f9bd028e5bc781f641fe210a88579eff827989", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_2699_to_2612.update.rpf.hdiff", 127701528 } /* diff sha1: fa76dab95d4b3a24c9670b1636bcb1fc38030364 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "80f9bd028e5bc781f641fe210a88579eff827989", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_2802_to_2612.update.rpf.hdiff", 131292485 } /* diff sha1: 586b0f78de285133a3234cea71ae87a29174b825 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "80f9bd028e5bc781f641fe210a88579eff827989", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_2944_to_2612.update.rpf.hdiff", 136386970 } /* diff sha1: 69a7f2dfa365deee03f273cd45edfef63887d414 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "80f9bd028e5bc781f641fe210a88579eff827989", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_3095_to_2612.update.rpf.hdiff", 140238453 } /* diff sha1: ec4226c26fe0ab951a064dafc8953d2cf7cd9071 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "80f9bd028e5bc781f641fe210a88579eff827989", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_3179_to_2612.update.rpf.hdiff", 140238453 } /* diff sha1: ec4226c26fe0ab951a064dafc8953d2cf7cd9071 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "80f9bd028e5bc781f641fe210a88579eff827989", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_3258_to_2612.update.rpf.hdiff", 144229149 } /* diff sha1: 13ff3b3e6c80627fed2039877f245e05574c1ffc */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "80f9bd028e5bc781f641fe210a88579eff827989", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_3323_to_2612.update.rpf.hdiff", 144229138 } /* diff sha1: ccf193169dd665ef4105911fb370d96260a142ab */
					}
				}
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://content.cfx.re/mirrors/patches_fivem/2612/update2.rpf", 312209408,
					{
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_2699_to_2612.update2.rpf.hdiff", 240637812 } /* diff sha1: 847791743b223286abf5cee5ab7f773c895c95c6 */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_2802_to_2612.update2.rpf.hdiff", 240847147 } /* diff sha1: 34671a51615c1255de4095d47137d81da26a4adf */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_2944_to_2612.update2.rpf.hdiff", 240982514 } /* diff sha1: 6bfe4bce142e697d83a039918f99eaf5d4ec3938 */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_3095_to_2612.update2.rpf.hdiff", 241447960 } /* diff sha1: 4ea67e0f9fadbe27c3a8b9c7aace9fdfd7f1fc4c */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_3179_to_2612.update2.rpf.hdiff", 241446618 } /* diff sha1: 886224aa434a960e471de884fa884f12fc09c998 */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_3258_to_2612.update2.rpf.hdiff", 241517010 } /* diff sha1: 44d6248a1b8865c763334db3ffc53a01ef29f5d1 */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://content.cfx.re/mirrors/patches_fivem/2612/diffs/from_3323_to_2612.update2.rpf.hdiff", 241507622 } /* diff sha1: e50895e7fec427a21102ba1aa4fe5175300e2652 */
					}
				}
			}
		}
	},
	{
		2545,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "517556bb548880362c18d502361ce374070994c2", "https://content.cfx.re/mirrors/patches_fivem/2545/GTA5.exe", 59988376 }
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches_fivem/2545/update.rpf", 1366638592,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches_fivem/2545/diffs/from_1604_to_2545.update.rpf.hdiff", 804698938 } /* diff sha1: be441ec10c867d1a7e5af0bbdbfa5163ec4d620c */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches_fivem/2545/diffs/from_2060_to_2545.update.rpf.hdiff", 573692039 } /* diff sha1: a5c4796b5373c2a9a2273d8a344e27d80f6dcc69 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches_fivem/2545/diffs/from_2189_to_2545.update.rpf.hdiff", 529825735 } /* diff sha1: 7436ea395e4de45d215a1c7098c48bd10d2364dd */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches_fivem/2545/diffs/from_2372_to_2545.update.rpf.hdiff", 509055770 } /* diff sha1: 84c9a2bfcd86d994f5c9a8fa5087c9f4dc0d7c6d */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches_fivem/2545/diffs/from_2612_to_2545.update.rpf.hdiff", 311774185 } /* diff sha1: 621c16a1d9a3be46c2663dde02bd58f1716c5445 */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches_fivem/2545/diffs/from_2699_to_2545.update.rpf.hdiff", 437636032 } /* diff sha1: 1d52798a772db28de7020738ea6aef9b45112eb0 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches_fivem/2545/diffs/from_2802_to_2545.update.rpf.hdiff", 441227939 } /* diff sha1: 23df563de60020ec4edb6192ab4925ae67af2ae6 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches_fivem/2545/diffs/from_2944_to_2545.update.rpf.hdiff", 446324800 } /* diff sha1: 0eef4d6c08abd9834bf306ac4e172033bac0f694 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches_fivem/2545/diffs/from_3095_to_2545.update.rpf.hdiff", 450172651 } /* diff sha1: 483f1b9d493818afe9fe32d01aa23d265b2f5cd7 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches_fivem/2545/diffs/from_3179_to_2545.update.rpf.hdiff", 450172651 } /* diff sha1: 483f1b9d493818afe9fe32d01aa23d265b2f5cd7 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches_fivem/2545/diffs/from_3258_to_2545.update.rpf.hdiff", 454165325 } /* diff sha1: 63f68a67be0eb06b7ec00cdda882ef07e6d57b28 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches_fivem/2545/diffs/from_3323_to_2545.update.rpf.hdiff", 454165319 } /* diff sha1: 3f501ce62c7a3856406101a2a35ee2e133fa618a */
					}
				}
			}
		}
	},
	{
		2372,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "470235e04299b02aa3aef834ef1ff834cac2327f", "https://content.cfx.re/mirrors/patches_fivem/2372/GTA5.exe", 59716912 }
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches_fivem/2372/update.rpf", 1132066816,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches_fivem/2372/diffs/from_1604_to_2372.update.rpf.hdiff", 562253582 } /* diff sha1: 49c8a15a9314e71b3931a2c7602f8cd9688fcbd2 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches_fivem/2372/diffs/from_2060_to_2372.update.rpf.hdiff", 329760934 } /* diff sha1: f3f6b96d648583e4427392ccf1df36cb1d435a3b */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches_fivem/2372/diffs/from_2189_to_2372.update.rpf.hdiff", 284412077 } /* diff sha1: 5870dc5116a7039476da7d714939ac1ec2e5f010 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches_fivem/2372/diffs/from_2545_to_2372.update.rpf.hdiff", 276106385 } /* diff sha1: 81b60c9be8f749c7bdd2b4dc6d80bda3913045c0 */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches_fivem/2372/diffs/from_2612_to_2372.update.rpf.hdiff", 343976392 } /* diff sha1: 6d00a65b77838cab4c2cc64cd4fd3eb69f75725d */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches_fivem/2372/diffs/from_2699_to_2372.update.rpf.hdiff", 345348050 } /* diff sha1: cd5650fc698852250ab39855ad43c3edb59b5c05 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches_fivem/2372/diffs/from_2802_to_2372.update.rpf.hdiff", 349288020 } /* diff sha1: 2b83c01cbcfa112e62882159b7178527b3aaffe5 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches_fivem/2372/diffs/from_2944_to_2372.update.rpf.hdiff", 354164718 } /* diff sha1: 6699a31eb6be4b331d52383322079139aa2e8968 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches_fivem/2372/diffs/from_3095_to_2372.update.rpf.hdiff", 356862631 } /* diff sha1: 6aa88c4943d6e36071b9ede4bd9b9652b5f76d14 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches_fivem/2372/diffs/from_3179_to_2372.update.rpf.hdiff", 356862631 } /* diff sha1: 6aa88c4943d6e36071b9ede4bd9b9652b5f76d14 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches_fivem/2372/diffs/from_3258_to_2372.update.rpf.hdiff", 359924636 } /* diff sha1: ecd71f547ba9d45088c61cb80c0981bf173db964 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches_fivem/2372/diffs/from_3323_to_2372.update.rpf.hdiff", 359924634 } /* diff sha1: b8f623b99c598fcaaf7f669741c29dee8c629346 */
					}
				}
			}
		}
	},
	{
		2189,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "fcd5fd8a9f99f2e08b0cab5d500740f28a75b75a", "https://content.cfx.re/mirrors/patches_fivem/2189/GTA5.exe", 63124096 }
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches_fivem/2189/update.rpf", 1276805120,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches_fivem/2189/diffs/from_1604_to_2189.update.rpf.hdiff", 562451691 } /* diff sha1: 16be09623e52097da007a166fba80921d7ce5863 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches_fivem/2189/diffs/from_2060_to_2189.update.rpf.hdiff", 296656003 } /* diff sha1: d9e50da86b802e382b65bfa53a28648a92fe79d8 */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches_fivem/2189/diffs/from_2372_to_2189.update.rpf.hdiff", 429153146 } /* diff sha1: 608466df488ba54851190d99d0bdc20160ff6815 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches_fivem/2189/diffs/from_2545_to_2189.update.rpf.hdiff", 441617306 } /* diff sha1: 3428f7544fd574acbb95c46b2889400f21e260b8 */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches_fivem/2189/diffs/from_2612_to_2189.update.rpf.hdiff", 508757790 } /* diff sha1: 9b640d294575a5ddcf680c8aab0a019d3b33229d */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches_fivem/2189/diffs/from_2699_to_2189.update.rpf.hdiff", 509396786 } /* diff sha1: bc5037ea0e7e80de9bae1d4d903d769b1bf0179b */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches_fivem/2189/diffs/from_2802_to_2189.update.rpf.hdiff", 513356064 } /* diff sha1: 0dbfc1422fe085d4654311358cf47204ae8360b9 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches_fivem/2189/diffs/from_2944_to_2189.update.rpf.hdiff", 518134363 } /* diff sha1: 23cb0ec2ad7aed1f58bd64ce37c1f7c1d7e590ad */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches_fivem/2189/diffs/from_3095_to_2189.update.rpf.hdiff", 518604133 } /* diff sha1: 73b7e835643658012708e6c280eb24c354fa2b0b */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches_fivem/2189/diffs/from_3179_to_2189.update.rpf.hdiff", 518604133 } /* diff sha1: 73b7e835643658012708e6c280eb24c354fa2b0b */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches_fivem/2189/diffs/from_3258_to_2189.update.rpf.hdiff", 521239366 } /* diff sha1: ab61a067d7afaab16e4769def84d6c0588bb7bd4 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches_fivem/2189/diffs/from_3323_to_2189.update.rpf.hdiff", 521239331 } /* diff sha1: 2875855cfc9d2aebd7aaeac7536a2548c8136bca */
					}
				}
			}
		}
	},
	{
		2060,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "741c8b91ef57140c023d8d29e38aab599759de76", "https://content.cfx.re/mirrors/patches_fivem/2060/GTA5.exe", 60589184 }
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches_fivem/2060/update.rpf", 1229002752,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches_fivem/2060/diffs/from_1604_to_2060.update.rpf.hdiff", 512193764 } /* diff sha1: 67030f9da5c2d9be1e3187e3efcec1c02c8b2af4 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches_fivem/2060/diffs/from_2189_to_2060.update.rpf.hdiff", 249363428 } /* diff sha1: 7168f459a2f2a75e3fd253f6c1b5fe1cfa93e6db */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches_fivem/2060/diffs/from_2372_to_2060.update.rpf.hdiff", 427205591 } /* diff sha1: 49344c636938b690173344c2e8210d8b12bbc296 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches_fivem/2060/diffs/from_2545_to_2060.update.rpf.hdiff", 438194552 } /* diff sha1: afe305848d46ac47c56a7c7d7e0f3089734bffe5 */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches_fivem/2060/diffs/from_2612_to_2060.update.rpf.hdiff", 504469096 } /* diff sha1: a00b8440d3135267f4097932d34d043cc970d54f */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches_fivem/2060/diffs/from_2699_to_2060.update.rpf.hdiff", 504126319 } /* diff sha1: 4c86479be22672bc3497c6da2a4a6151a568d122 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches_fivem/2060/diffs/from_2802_to_2060.update.rpf.hdiff", 508501688 } /* diff sha1: 02d1f7276d1f93fcc204d7b7e1e31db7058a33ca */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches_fivem/2060/diffs/from_2944_to_2060.update.rpf.hdiff", 512509365 } /* diff sha1: d69b4ba86899e0692217d4173ba1ba68f6816908 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches_fivem/2060/diffs/from_3095_to_2060.update.rpf.hdiff", 512663221 } /* diff sha1: 8fccc8eecc4e44533eac1b8be577dc4260224ec2 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches_fivem/2060/diffs/from_3179_to_2060.update.rpf.hdiff", 512663221 } /* diff sha1: 8fccc8eecc4e44533eac1b8be577dc4260224ec2 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches_fivem/2060/diffs/from_3258_to_2060.update.rpf.hdiff", 515246812 } /* diff sha1: 535f49a489f7c3618a0174f514c017851bb4f7fd */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches_fivem/2060/diffs/from_3323_to_2060.update.rpf.hdiff", 515246794 } /* diff sha1: bfcf008b95313770edc03662a16d30b7bb245f26 */
					}
				}
			}
		}
	},
	{
		1604,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "8939c8c71aa98ad7ca6ac773fae1463763c420d8", "https://content.cfx.re/mirrors/patches_fivem/1604/GTA5.exe", 72484280 }
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches_fivem/1604/update.rpf", 966805504,
					{
						{ "736f1cb26e59167f302c22385463d231cce302d3", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches_fivem/1604/diffs/from_2060_to_1604.update.rpf.hdiff", 252578178 } /* diff sha1: a6b119525e18bb37cbf65b23d51317cd47177188 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches_fivem/1604/diffs/from_2189_to_1604.update.rpf.hdiff", 255544048 } /* diff sha1: 29d31e6a9d964662a490682b07ecd3ac7f776913 */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches_fivem/1604/diffs/from_2372_to_1604.update.rpf.hdiff", 400087270 } /* diff sha1: ca3064d9bb8108b626e5e1b7c747c29cca597587 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches_fivem/1604/diffs/from_2545_to_1604.update.rpf.hdiff", 409505316 } /* diff sha1: 92998e2e6d977815f0a73137275119f7dda2a68c */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches_fivem/1604/diffs/from_2612_to_1604.update.rpf.hdiff", 475094324 } /* diff sha1: 1982eb4530c60c9cdaef64c719d8383cd75954ee */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches_fivem/1604/diffs/from_2699_to_1604.update.rpf.hdiff", 475411202 } /* diff sha1: 574d2f5f091ce6e59036fb74cf5ad785252e10af */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches_fivem/1604/diffs/from_2802_to_1604.update.rpf.hdiff", 478650625 } /* diff sha1: 8f8ad96b2939ddf43c9c014af3592a396330f4d4 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches_fivem/1604/diffs/from_2944_to_1604.update.rpf.hdiff", 481127813 } /* diff sha1: ed3677dc68d2c1db95a90b3fd184ef8ed1c82d0c */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches_fivem/1604/diffs/from_3095_to_1604.update.rpf.hdiff", 481316317 } /* diff sha1: 4cdcfc8e677169799861f5a97247bc7152d75768 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches_fivem/1604/diffs/from_3179_to_1604.update.rpf.hdiff", 481316317 } /* diff sha1: 4cdcfc8e677169799861f5a97247bc7152d75768 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches_fivem/1604/diffs/from_3258_to_1604.update.rpf.hdiff", 483477046 } /* diff sha1: dc84f0bd06326be309bd4b409c354903690ac196 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches_fivem/1604/diffs/from_3323_to_1604.update.rpf.hdiff", 483477045 } /* diff sha1: 4a826e8ab5034b13835c5e303cce9b2f9a718b77 */
					}
				}
			}
		}
	}
#elif IS_RDR3
	{
		1491,
		{
			{
				"RDR2.exe",
				{ "RDR2.exe", "25fd42fd09c1cb0b839943f2a752e906627f177b", "https://content.cfx.re/mirrors/patches_redm/1491.50/RDR2.exe", 89562608 }
			},
			{
				"appdata0_update.rpf",
				{
					"appdata0_update.rpf", "e68cbb4882db0028ba2701c26ed69152ad992c2e", "https://content.cfx.re/mirrors/patches_redm/1491.50/appdata0_update.rpf", 3164623,
					{
						{ "1715741785ce3c28adf9a78633e57f478229bb84", "e68cbb4882db0028ba2701c26ed69152ad992c2e", "https://content.cfx.re/mirrors/patches_redm/1491.50/diffs/from_1311_to_1491.50.appdata0_update.rpf.hdiff", 1914067 } /* diff sha1: 00d25742c884f95922ecbbae37583cc4d67cc5c1 */,
						{ "307609c164e78adaf4e50e993328485e6264803f", "e68cbb4882db0028ba2701c26ed69152ad992c2e", "https://content.cfx.re/mirrors/patches_redm/1491.50/diffs/from_1355_to_1491.50.appdata0_update.rpf.hdiff", 1827754 } /* diff sha1: a9dd1da46bab5bf04ca3835c74497695dec89427 */,
						{ "ba1d727a70fa1c204441c8e3768a1a40b02ef67f", "e68cbb4882db0028ba2701c26ed69152ad992c2e", "https://content.cfx.re/mirrors/patches_redm/1491.50/diffs/from_1436_to_1491.50.appdata0_update.rpf.hdiff", 1099205 } /* diff sha1: 2486abcb82d0559dbb4ebba1e2cecfd63ef9c97f */,
						{ "142c6af7a64f2cae06a8f7ac7ad6ee74967afc49", "e68cbb4882db0028ba2701c26ed69152ad992c2e", "https://content.cfx.re/mirrors/patches_redm/1491.50/diffs/from_1491_to_1491.50.appdata0_update.rpf.hdiff", 859819 } /* diff sha1: de5b59a912f8ec3cdf317acd3f7895fdc30c3bc5 */
					}
				}
			},
			{
				"shaders_x64.rpf",
				{
					"shaders_x64.rpf", "f8ecee595e74c66c5bd02fd87c2947cf475a2614", "https://content.cfx.re/mirrors/patches_redm/1491.50/shaders_x64.rpf", 233921358,
					{
						{ "77bad0ab74cd1ef7c646206ea12152449ec56cdf", "f8ecee595e74c66c5bd02fd87c2947cf475a2614", "https://content.cfx.re/mirrors/patches_redm/1491.50/diffs/from_1311_to_1491.50.shaders_x64.rpf.hdiff", 18993038 } /* diff sha1: 7e4691a2113c4734a934dccbc88032c070308469 */,
						{ "a7a45988a6067964214cc4b3af21797249817469", "f8ecee595e74c66c5bd02fd87c2947cf475a2614", "https://content.cfx.re/mirrors/patches_redm/1491.50/diffs/from_1355_to_1491.50.shaders_x64.rpf.hdiff", 17434866 } /* diff sha1: 36371124827afcf6379c35d97602e4503bfc9768 */,
						{ "f4f06c18701d66958eb6f0ac243c8467033b864b", "f8ecee595e74c66c5bd02fd87c2947cf475a2614", "https://content.cfx.re/mirrors/patches_redm/1491.50/diffs/from_1436_to_1491.50.shaders_x64.rpf.hdiff", 14013870 } /* diff sha1: d0db63b6900d71bbe478ef4e5b1fce52302c5673 */,
						{ "f456cbaf70ff921f77279db5a901c6a6e5807e2e", "f8ecee595e74c66c5bd02fd87c2947cf475a2614", "https://content.cfx.re/mirrors/patches_redm/1491.50/diffs/from_1491_to_1491.50.shaders_x64.rpf.hdiff", 13734156 } /* diff sha1: d01ea64ced67c5a4746656d4bdcf0e9993a04270 */
					}
				}
			},
			{
				"update_1.rpf",
				{
					"update_1.rpf", "8c25d7345b7e69ebaee24ccfea97739ace59ba51", "https://content.cfx.re/mirrors/patches_redm/1491.50/update_1.rpf", 2833741450,
					{
						{ "9e62163f0383aa4eb30a02fa0f5628bbf4538543", "8c25d7345b7e69ebaee24ccfea97739ace59ba51", "https://content.cfx.re/mirrors/patches_redm/1491.50/diffs/from_1436_to_1491.50.update_1.rpf.hdiff", 87253125 } /* diff sha1: 83ceb5c113aa6f5be0bc9df8331fc53fdb3835ec */,
						{ "601a4801f739540bebb2b3e141fda022901a7bd1", "8c25d7345b7e69ebaee24ccfea97739ace59ba51", "https://content.cfx.re/mirrors/patches_redm/1491.50/diffs/from_1491_to_1491.50.update_1.rpf.hdiff", 17843685 } /* diff sha1: 91edd8683f7872968a71036cef74041579e272f7 */
					}
				}
			},
			{
				"update_2.rpf",
				{
					"update_2.rpf", "5a77f9b8cb24e1c3e78ee33c7ed218a32e3d2e32", "https://content.cfx.re/mirrors/patches_redm/1491.50/update_2.rpf", 152046254,
					{
						{ "87323b6d0e1c790972041a034a6f293eb774c84d", "5a77f9b8cb24e1c3e78ee33c7ed218a32e3d2e32", "https://content.cfx.re/mirrors/patches_redm/1491.50/diffs/from_1436_to_1491.50.update_2.rpf.hdiff", 24349275 } /* diff sha1: d53cd8b71a3beba9afdb3bd7f76d1e7731be5ec1 */,
						{ "6b3af948543e7a48013bdec930e8dd586be37266", "5a77f9b8cb24e1c3e78ee33c7ed218a32e3d2e32", "https://content.cfx.re/mirrors/patches_redm/1491.50/diffs/from_1491_to_1491.50.update_2.rpf.hdiff", 18451453 } /* diff sha1: 4e87e70df5c0d073f8b0790d9ff3440f5cc31878 */
					}
				}
			},
			{
				"update_3.rpf",
				{
					"update_3.rpf", "be15563d37c1ab0f655eeebb45f4d30527df950d", "https://content.cfx.re/mirrors/patches_redm/1491.50/update_3.rpf", 132374684,
					{
						{ "a2708ff55294d70bda198a1ff98c1f4b55b0c0df", "be15563d37c1ab0f655eeebb45f4d30527df950d", "https://content.cfx.re/mirrors/patches_redm/1491.50/diffs/from_1436_to_1491.50.update_3.rpf.hdiff", 114270024 } /* diff sha1: 9d7af31be4e61cb86a4268c28daae6ce6d40f2bb */,
						{ "9237da54d2267435fc7d7bf0f3ec054bbeea90a9", "be15563d37c1ab0f655eeebb45f4d30527df950d", "https://content.cfx.re/mirrors/patches_redm/1491.50/diffs/from_1491_to_1491.50.update_3.rpf.hdiff", 114892075 } /* diff sha1: d98c2b3f2cf3e9049e7b2682aeaf3f9252f47af3 */
					}
				}
			},
			{
				"update_4.rpf",
				{
					"update_4.rpf", "853a63af1698a970dfb73295faa76a31e56fe4bd", "https://content.cfx.re/mirrors/patches_redm/1491.50/update_4.rpf", 2015028563,
					{
						{ "4ec55a211e7cb1d68c8fd471cfb049d7690fc9ee", "853a63af1698a970dfb73295faa76a31e56fe4bd", "https://content.cfx.re/mirrors/patches_redm/1491.50/diffs/from_1436_to_1491.50.update_4.rpf.hdiff", 19504197 } /* diff sha1: 9c451a6aee4e8a9d4782a2348ff8a569d71b9337 */,
						{ "503c8de5c16e26afdce502b9aedf1ae16a0e8730", "853a63af1698a970dfb73295faa76a31e56fe4bd", "https://content.cfx.re/mirrors/patches_redm/1491.50/diffs/from_1491_to_1491.50.update_4.rpf.hdiff", 293569 } /* diff sha1: 3f5120c886b76512a5de8843d3e909368a8e64f9 */
					}
				}
			}
		}
	},
	{
		1436,
		{
			{
				"RDR2.exe",
				{ "RDR2.exe", "f998b4863b11793547c09c226ab884e1e26931f2", "https://content.cfx.re/mirrors/patches_redm/1436/RDR2.exe", 89104336 }
			},
			{
				"appdata0_update.rpf",
				{
					"appdata0_update.rpf", "ba1d727a70fa1c204441c8e3768a1a40b02ef67f", "https://content.cfx.re/mirrors/patches_redm/1436/appdata0_update.rpf", 3163551,
					{
						{ "1715741785ce3c28adf9a78633e57f478229bb84", "ba1d727a70fa1c204441c8e3768a1a40b02ef67f", "https://content.cfx.re/mirrors/patches_redm/1436/diffs/from_1311_to_1436.appdata0_update.rpf.hdiff", 1785629 } /* diff sha1: 3dec92f13de7440fed0420ebe12522d9f0bcae9e */,
						{ "307609c164e78adaf4e50e993328485e6264803f", "ba1d727a70fa1c204441c8e3768a1a40b02ef67f", "https://content.cfx.re/mirrors/patches_redm/1436/diffs/from_1355_to_1436.appdata0_update.rpf.hdiff", 1726590 } /* diff sha1: e5375671b7270be06fd55052f78e8afc95a8222d */,
						{ "142c6af7a64f2cae06a8f7ac7ad6ee74967afc49", "ba1d727a70fa1c204441c8e3768a1a40b02ef67f", "https://content.cfx.re/mirrors/patches_redm/1436/diffs/from_1491_to_1436.appdata0_update.rpf.hdiff", 1093833 } /* diff sha1: 1f81d526052eda00d14c1bf601f811c005c8f052 */,
						{ "e68cbb4882db0028ba2701c26ed69152ad992c2e", "ba1d727a70fa1c204441c8e3768a1a40b02ef67f", "https://content.cfx.re/mirrors/patches_redm/1436/diffs/from_1491.50_to_1436.appdata0_update.rpf.hdiff", 1093887 } /* diff sha1: f69e79db5ec8b33f4ea4c96d4d96281ecb2feb45 */
					}
				}
			},
			{
				"shaders_x64.rpf",
				{
					"shaders_x64.rpf", "f4f06c18701d66958eb6f0ac243c8467033b864b", "https://content.cfx.re/mirrors/patches_redm/1436/shaders_x64.rpf", 233898030,
					{
						{ "77bad0ab74cd1ef7c646206ea12152449ec56cdf", "f4f06c18701d66958eb6f0ac243c8467033b864b", "https://content.cfx.re/mirrors/patches_redm/1436/diffs/from_1311_to_1436.shaders_x64.rpf.hdiff", 6183735 } /* diff sha1: 22592c1a70552466d8f34c49484bf5475da399ae */,
						{ "a7a45988a6067964214cc4b3af21797249817469", "f4f06c18701d66958eb6f0ac243c8467033b864b", "https://content.cfx.re/mirrors/patches_redm/1436/diffs/from_1355_to_1436.shaders_x64.rpf.hdiff", 4332901 } /* diff sha1: 5b0ff40e1a76775461903fa9669c950ec0113e46 */,
						{ "f456cbaf70ff921f77279db5a901c6a6e5807e2e", "f4f06c18701d66958eb6f0ac243c8467033b864b", "https://content.cfx.re/mirrors/patches_redm/1436/diffs/from_1491_to_1436.shaders_x64.rpf.hdiff", 265613 } /* diff sha1: 448dad13cdb90971890418041e38db9bc436cf3e */,
						{ "f8ecee595e74c66c5bd02fd87c2947cf475a2614", "f4f06c18701d66958eb6f0ac243c8467033b864b", "https://content.cfx.re/mirrors/patches_redm/1436/diffs/from_1491.50_to_1436.shaders_x64.rpf.hdiff", 14048549 } /* diff sha1: 026725b8698e25c4d23b0f58fa22543f620e29e1 */
					}
				}
			},
			{
				"update_1.rpf",
				{
					"update_1.rpf", "9e62163f0383aa4eb30a02fa0f5628bbf4538543", "https://content.cfx.re/mirrors/patches_redm/1436/update_1.rpf", 2836982634,
					{
						{ "601a4801f739540bebb2b3e141fda022901a7bd1", "9e62163f0383aa4eb30a02fa0f5628bbf4538543", "https://content.cfx.re/mirrors/patches_redm/1436/diffs/from_1491_to_1436.update_1.rpf.hdiff", 89907254 } /* diff sha1: f55dae6db48fd0dda668a3ab7bf45cd8a7601359 */,
						{ "8c25d7345b7e69ebaee24ccfea97739ace59ba51", "9e62163f0383aa4eb30a02fa0f5628bbf4538543", "https://content.cfx.re/mirrors/patches_redm/1436/diffs/from_1491.50_to_1436.update_1.rpf.hdiff", 89940992 } /* diff sha1: 248a15b3bb1577117e45a7d7fa7cc59342f61236 */
					}
				}
			},
			{
				"update_2.rpf",
				{
					"update_2.rpf", "87323b6d0e1c790972041a034a6f293eb774c84d", "https://content.cfx.re/mirrors/patches_redm/1436/update_2.rpf", 152008542,
					{
						{ "6b3af948543e7a48013bdec930e8dd586be37266", "87323b6d0e1c790972041a034a6f293eb774c84d", "https://content.cfx.re/mirrors/patches_redm/1436/diffs/from_1491_to_1436.update_2.rpf.hdiff", 24380905 } /* diff sha1: 0c65b20a6f577d14dd0f9733d03c657542a517c8 */,
						{ "5a77f9b8cb24e1c3e78ee33c7ed218a32e3d2e32", "87323b6d0e1c790972041a034a6f293eb774c84d", "https://content.cfx.re/mirrors/patches_redm/1436/diffs/from_1491.50_to_1436.update_2.rpf.hdiff", 24361478 } /* diff sha1: febfd15d7614f69ac16715a020b13361b843e30d */
					}
				}
			},
			{
				"update_3.rpf",
				{
					"update_3.rpf", "a2708ff55294d70bda198a1ff98c1f4b55b0c0df", "https://content.cfx.re/mirrors/patches_redm/1436/update_3.rpf", 132374108,
					{
						{ "9237da54d2267435fc7d7bf0f3ec054bbeea90a9", "a2708ff55294d70bda198a1ff98c1f4b55b0c0df", "https://content.cfx.re/mirrors/patches_redm/1436/diffs/from_1491_to_1436.update_3.rpf.hdiff", 116090311 } /* diff sha1: 01695c3f9ba37b0c2e3f52aefae433452c3edc2a */,
						{ "be15563d37c1ab0f655eeebb45f4d30527df950d", "a2708ff55294d70bda198a1ff98c1f4b55b0c0df", "https://content.cfx.re/mirrors/patches_redm/1436/diffs/from_1491.50_to_1436.update_3.rpf.hdiff", 114282726 } /* diff sha1: f985ea07d597bed15b490ab0f99776cb0ac0a643 */
					}
				}
			},
			{
				"update_4.rpf",
				{
					"update_4.rpf", "4ec55a211e7cb1d68c8fd471cfb049d7690fc9ee", "https://content.cfx.re/mirrors/patches_redm/1436/update_4.rpf", 2014659811,
					{
						{ "503c8de5c16e26afdce502b9aedf1ae16a0e8730", "4ec55a211e7cb1d68c8fd471cfb049d7690fc9ee", "https://content.cfx.re/mirrors/patches_redm/1436/diffs/from_1491_to_1436.update_4.rpf.hdiff", 18944420 } /* diff sha1: 0f0ea25c6e763c834a0dbeec98773479af6ccaa2 */,
						{ "853a63af1698a970dfb73295faa76a31e56fe4bd", "4ec55a211e7cb1d68c8fd471cfb049d7690fc9ee", "https://content.cfx.re/mirrors/patches_redm/1436/diffs/from_1491.50_to_1436.update_4.rpf.hdiff", 19046666 } /* diff sha1: 418cd83ffa6c39289784aa99596b840ad40d526b */
					}
				}
			}
		}
	},
	{
		1355,
		{
			{
				"RDR2.exe",
				{ "RDR2.exe", "c2fab1d25daef4779aafd2754ec9c593e674e7c3", "https://content.cfx.re/mirrors/patches_redm/1355/RDR2.exe", 84664448 }
			},
			{
				"appdata0_update.rpf",
				{
					"appdata0_update.rpf", "307609c164e78adaf4e50e993328485e6264803f", "https://content.cfx.re/mirrors/patches_redm/1355/appdata0_update.rpf", 3069247,
					{
						{ "1715741785ce3c28adf9a78633e57f478229bb84", "307609c164e78adaf4e50e993328485e6264803f", "https://content.cfx.re/mirrors/patches_redm/1355/diffs/from_1311_to_1355.appdata0_update.rpf.hdiff", 1703692 } /* diff sha1: dbea61ab2e74c2c9c984a16c17233dcfde014af0 */,
						{ "ba1d727a70fa1c204441c8e3768a1a40b02ef67f", "307609c164e78adaf4e50e993328485e6264803f", "https://content.cfx.re/mirrors/patches_redm/1355/diffs/from_1436_to_1355.appdata0_update.rpf.hdiff", 1644378 } /* diff sha1: 24faaa60d4df3d109410d6b58286a1af0d880bba */,
						{ "142c6af7a64f2cae06a8f7ac7ad6ee74967afc49", "307609c164e78adaf4e50e993328485e6264803f", "https://content.cfx.re/mirrors/patches_redm/1355/diffs/from_1491_to_1355.appdata0_update.rpf.hdiff", 1740781 } /* diff sha1: e6683ad9aa9bd9ca302b07e324d73d20829ac2f1 */,
						{ "e68cbb4882db0028ba2701c26ed69152ad992c2e", "307609c164e78adaf4e50e993328485e6264803f", "https://content.cfx.re/mirrors/patches_redm/1355/diffs/from_1491.50_to_1355.appdata0_update.rpf.hdiff", 1740781 } /* diff sha1: 3f83c5e3cf75b833757a7bb3cae39f17b5712986 */
					}
				}
			},
			{
				"shaders_x64.rpf",
				{
					"shaders_x64.rpf", "a7a45988a6067964214cc4b3af21797249817469", "https://content.cfx.re/mirrors/patches_redm/1355/shaders_x64.rpf", 233585710,
					{
						{ "77bad0ab74cd1ef7c646206ea12152449ec56cdf", "a7a45988a6067964214cc4b3af21797249817469", "https://content.cfx.re/mirrors/patches_redm/1355/diffs/from_1311_to_1355.shaders_x64.rpf.hdiff", 1875463 } /* diff sha1: ac5fd94f61158f73719290a79e59bb84050ab5dc */,
						{ "f4f06c18701d66958eb6f0ac243c8467033b864b", "a7a45988a6067964214cc4b3af21797249817469", "https://content.cfx.re/mirrors/patches_redm/1355/diffs/from_1436_to_1355.shaders_x64.rpf.hdiff", 4184409 } /* diff sha1: 75ccc162577c4366e151b1b373b2d72ba871ae80 */,
						{ "f456cbaf70ff921f77279db5a901c6a6e5807e2e", "a7a45988a6067964214cc4b3af21797249817469", "https://content.cfx.re/mirrors/patches_redm/1355/diffs/from_1491_to_1355.shaders_x64.rpf.hdiff", 4369953 } /* diff sha1: dc67cb46f136f6b028d5c27872256c3bba5e3788 */,
						{ "f8ecee595e74c66c5bd02fd87c2947cf475a2614", "a7a45988a6067964214cc4b3af21797249817469", "https://content.cfx.re/mirrors/patches_redm/1355/diffs/from_1491.50_to_1355.shaders_x64.rpf.hdiff", 17310440 } /* diff sha1: 9b08a64a7436dd3983f31a1378757ddc7d6e660e */
					}
				}
			},
			{
				"update.rpf",
				{
					"update.rpf", "5a087ef32e6b30b4fde8bbeda7babc45f2c1cf4d", "https://content.cfx.re/mirrors/patches_redm/1355/update.rpf", 4685758145,
					{
						{ "835a767055cfbf2c2ad86cf4462c7dfb931970fd", "5a087ef32e6b30b4fde8bbeda7babc45f2c1cf4d", "https://content.cfx.re/mirrors/patches_redm/1355/diffs/from_1311_to_1355.update.rpf.hdiff", 1513512317 } /* diff sha1: b528f92872385a37302e367de67d9938de918d33 */
					}
				}
			}
		}
	},
	{
		1311,
		{
			{
				"RDR2.exe",
				{ "RDR2.exe", "ac3c2abd80bfa949279d8e1d32105a3d9345c6c8", "https://content.cfx.re/mirrors/patches_redm/1311/RDR2.exe", 91439232 }
			},
			{
				"appdata0_update.rpf",
				{
					"appdata0_update.rpf", "1715741785ce3c28adf9a78633e57f478229bb84", "https://content.cfx.re/mirrors/patches_redm/1311/appdata0_update.rpf", 3003087,
					{
						{ "307609c164e78adaf4e50e993328485e6264803f", "1715741785ce3c28adf9a78633e57f478229bb84", "https://content.cfx.re/mirrors/patches_redm/1311/diffs/from_1355_to_1311.appdata0_update.rpf.hdiff", 1624654 } /* diff sha1: ebd5f32406de1195d5c02482eb036b78060077f3 */,
						{ "ba1d727a70fa1c204441c8e3768a1a40b02ef67f", "1715741785ce3c28adf9a78633e57f478229bb84", "https://content.cfx.re/mirrors/patches_redm/1311/diffs/from_1436_to_1311.appdata0_update.rpf.hdiff", 1650802 } /* diff sha1: 35291b0d0e2fa862d4a1c52deb954f7a1a182627 */,
						{ "142c6af7a64f2cae06a8f7ac7ad6ee74967afc49", "1715741785ce3c28adf9a78633e57f478229bb84", "https://content.cfx.re/mirrors/patches_redm/1311/diffs/from_1491_to_1311.appdata0_update.rpf.hdiff", 1774323 } /* diff sha1: c06f3cc14eb80628aea0baecdcb510ec027bf80e */,
						{ "e68cbb4882db0028ba2701c26ed69152ad992c2e", "1715741785ce3c28adf9a78633e57f478229bb84", "https://content.cfx.re/mirrors/patches_redm/1311/diffs/from_1491.50_to_1311.appdata0_update.rpf.hdiff", 1775075 } /* diff sha1: 53a91497a26e99a81b47da2e20c484d9032e6eb0 */
					}
				}
			},
			{
				"shaders_x64.rpf",
				{
					"shaders_x64.rpf", "77bad0ab74cd1ef7c646206ea12152449ec56cdf", "https://content.cfx.re/mirrors/patches_redm/1311/shaders_x64.rpf", 233487310,
					{
						{ "a7a45988a6067964214cc4b3af21797249817469", "77bad0ab74cd1ef7c646206ea12152449ec56cdf", "https://content.cfx.re/mirrors/patches_redm/1311/diffs/from_1355_to_1311.shaders_x64.rpf.hdiff", 1817785 } /* diff sha1: 4865ba920795b6bcc9724e2e6312beb69fafd4b2 */,
						{ "f4f06c18701d66958eb6f0ac243c8467033b864b", "77bad0ab74cd1ef7c646206ea12152449ec56cdf", "https://content.cfx.re/mirrors/patches_redm/1311/diffs/from_1436_to_1311.shaders_x64.rpf.hdiff", 5981861 } /* diff sha1: bf0116bd17de03b8eac05ac29c74b0a604ce58c6 */,
						{ "f456cbaf70ff921f77279db5a901c6a6e5807e2e", "77bad0ab74cd1ef7c646206ea12152449ec56cdf", "https://content.cfx.re/mirrors/patches_redm/1311/diffs/from_1491_to_1311.shaders_x64.rpf.hdiff", 6142857 } /* diff sha1: 635021d0b676de04b6ebf30fc3ceda54ca58530a */,
						{ "f8ecee595e74c66c5bd02fd87c2947cf475a2614", "77bad0ab74cd1ef7c646206ea12152449ec56cdf", "https://content.cfx.re/mirrors/patches_redm/1311/diffs/from_1491.50_to_1311.shaders_x64.rpf.hdiff", 18810230 } /* diff sha1: 2c8fa2c0ae76318ba22d683daabf0f83548885aa */
					}
				}
			},
			{
				"update.rpf",
				{
					"update.rpf", "835a767055cfbf2c2ad86cf4462c7dfb931970fd", "https://content.cfx.re/mirrors/patches_redm/1311/update.rpf", 3515071792,
					{
						{ "5a087ef32e6b30b4fde8bbeda7babc45f2c1cf4d", "835a767055cfbf2c2ad86cf4462c7dfb931970fd", "https://content.cfx.re/mirrors/patches_redm/1311/diffs/from_1355_to_1311.update.rpf.hdiff", 354416250 } /* diff sha1: 63abb47155024cffdf6fb86d17162c44880a5b57 */
					}
				}
			}
		}
	}
#endif
};

std::map<std::string, std::string> UpdateGameCache()
{
#if defined(_M_AMD64)
	std::vector<GameCacheEntry> launcherEntries;

	launcherEntries = {
		{ "launcher/api-ms-win-core-console-l1-1-0.dll", "724F4F91041AD595E365B724A0348C83ACF12BBB", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-console-l1-1-0.dll", 19208 },
		{ "launcher/api-ms-win-core-datetime-l1-1-0.dll", "4940D5B92B6B80A40371F8DF073BF3EB406F5658", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-datetime-l1-1-0.dll", 18696 },
		{ "launcher/api-ms-win-core-debug-l1-1-0.dll", "E7C8A6C29C3158F8B332EEA5C33C3B1E044B5F73", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-debug-l1-1-0.dll", 18696 },
		{ "launcher/api-ms-win-core-errorhandling-l1-1-0.dll", "51CBB7BA47802DC630C2507750432C55F5979C27", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-errorhandling-l1-1-0.dll", 18696 },
		{ "launcher/api-ms-win-core-file-l1-1-0.dll", "9ACBEEF0AC510C179B319CA69CD5378D0E70504D", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-file-l1-1-0.dll", 22280 },
		{ "launcher/api-ms-win-core-file-l1-2-0.dll", "04669214375B25E2DC8A3635484E6EEB206BC4EB", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-file-l1-2-0.dll", 18696 },
		{ "launcher/api-ms-win-core-file-l2-1-0.dll", "402B7B8F8DCFD321B1D12FC85A1EE5137A5569B2", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-file-l2-1-0.dll", 18696 },
		{ "launcher/api-ms-win-core-handle-l1-1-0.dll", "A2E2A40CEA25EA4FD64B8DEAF4FBE4A2DB94107A", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-handle-l1-1-0.dll", 18696 },
		{ "launcher/api-ms-win-core-heap-l1-1-0.dll", "B4310929CCB82DD3C3A779CAB68F1F9F368076F2", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-heap-l1-1-0.dll", 19208 },
		{ "launcher/api-ms-win-core-interlocked-l1-1-0.dll", "F779CDEF9DED19402AA72958085213D6671CA572", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-interlocked-l1-1-0.dll", 18696 },
		{ "launcher/api-ms-win-core-libraryloader-l1-1-0.dll", "47143A66B4A2E2BA019BF1FD07BCCA9CFB8BB117", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-libraryloader-l1-1-0.dll", 19720 },
		{ "launcher/api-ms-win-core-localization-l1-2-0.dll", "9874398548891F6A08FC06437996F84EB7495783", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-localization-l1-2-0.dll", 21256 },
		{ "launcher/api-ms-win-core-memory-l1-1-0.dll", "9C03356CF48112563BB845479F40BF27B293E95E", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-memory-l1-1-0.dll", 19208 },
		{ "launcher/api-ms-win-core-namedpipe-l1-1-0.dll", "CB59F1FE73C17446EB196FC0DD7D944A0CD9D81F", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-namedpipe-l1-1-0.dll", 18696 },
		{ "launcher/api-ms-win-core-processenvironment-l1-1-0.dll", "2745259F4DBBEFBF6B570EE36D224ABDB18719BC", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-processenvironment-l1-1-0.dll", 19720 },
		{ "launcher/api-ms-win-core-processthreads-l1-1-0.dll", "50699041060D14576ED7BACBD44BE9AF80EB902A", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-processthreads-l1-1-0.dll", 20744 },
		{ "launcher/api-ms-win-core-processthreads-l1-1-1.dll", "0BFFB9ED366853E7019452644D26E8E8F236241B", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-processthreads-l1-1-1.dll", 19208 },
		{ "launcher/api-ms-win-core-profile-l1-1-0.dll", "E7E0B18A40A35BD8B0766AC72253DE827432E148", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-profile-l1-1-0.dll", 18184 },
		{ "launcher/api-ms-win-core-rtlsupport-l1-1-0.dll", "24F37D46DFC0EF303EF04ABF9956241AF55D25C9", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-rtlsupport-l1-1-0.dll", 19208 },
		{ "launcher/api-ms-win-core-string-l1-1-0.dll", "637E4A9946691F76E6DEB69BDC21C210921D6F07", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-string-l1-1-0.dll", 18696 },
		{ "launcher/api-ms-win-core-synch-l1-1-0.dll", "5584C189216A17228CCA6CD07037AAA9A8603241", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-synch-l1-1-0.dll", 20744 },
		{ "launcher/api-ms-win-core-synch-l1-2-0.dll", "A9AEBBBB73B7B846B051325D7572F2398F5986EE", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-synch-l1-2-0.dll", 19208 },
		{ "launcher/api-ms-win-core-sysinfo-l1-1-0.dll", "F20AE25484A1C1B43748A1F0C422F48F092AD2C1", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-sysinfo-l1-1-0.dll", 19720 },
		{ "launcher/api-ms-win-core-timezone-l1-1-0.dll", "4BF13DB65943E708690D6256D7DDD421CC1CC72B", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-timezone-l1-1-0.dll", 19208 },
		{ "launcher/api-ms-win-core-util-l1-1-0.dll", "1E1A5AB47E4C2B3C32C81690B94954B7612BB493", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-core-util-l1-1-0.dll", 18696 },
		{ "launcher/api-ms-win-crt-conio-l1-1-0.dll", "49002B58CB0DF2EE8D868DEC335133CF225657DF", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-crt-conio-l1-1-0.dll", 19720 },
		{ "launcher/api-ms-win-crt-convert-l1-1-0.dll", "C84E41FDCC4CA89A76AE683CB390A9B86500D3CA", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-crt-convert-l1-1-0.dll", 22792 },
		{ "launcher/api-ms-win-crt-environment-l1-1-0.dll", "9A4818897251CACB7FE1C6FE1BE3E854985186AD", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-crt-environment-l1-1-0.dll", 19208 },
		{ "launcher/api-ms-win-crt-filesystem-l1-1-0.dll", "78FA03C89EA12FF93FA499C38673039CC2D55D40", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-crt-filesystem-l1-1-0.dll", 20744 },
		{ "launcher/api-ms-win-crt-heap-l1-1-0.dll", "60B4CF246C5F414FC1CD12F506C41A1043D473EE", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-crt-heap-l1-1-0.dll", 19720 },
		{ "launcher/api-ms-win-crt-locale-l1-1-0.dll", "9C1DF49A8DBDC8496AC6057F886F5C17B2C39E3E", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-crt-locale-l1-1-0.dll", 19208 },
		{ "launcher/api-ms-win-crt-math-l1-1-0.dll", "8B35EC4676BD96C2C4508DC5F98CA471B22DEED7", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-crt-math-l1-1-0.dll", 27912 },
		{ "launcher/api-ms-win-crt-multibyte-l1-1-0.dll", "91EEF52C557AEFD0FDE27E8DF4E3C3B7F99862F2", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-crt-multibyte-l1-1-0.dll", 26888 },
		{ "launcher/api-ms-win-crt-private-l1-1-0.dll", "0C33CFE40EDD278A692C2E73E941184FD24286D9", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-crt-private-l1-1-0.dll", 71432 },
		{ "launcher/api-ms-win-crt-process-l1-1-0.dll", "EC96F7BEEAEC14D3B6C437B97B4A18A365534B9B", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-crt-process-l1-1-0.dll", 19720 },
		{ "launcher/api-ms-win-crt-runtime-l1-1-0.dll", "A19ACEFA3F95D1B565650FDBC40EF98C793358E9", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-crt-runtime-l1-1-0.dll", 23304 },
		{ "launcher/api-ms-win-crt-stdio-l1-1-0.dll", "982B5DA1C1F5B9D74AF6243885BCBA605D54DF8C", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-crt-stdio-l1-1-0.dll", 24840 },
		{ "launcher/api-ms-win-crt-string-l1-1-0.dll", "7F389E6F2D6E5BEB2A3BAF622A0C0EA24BC4DE60", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-crt-string-l1-1-0.dll", 24840 },
		{ "launcher/api-ms-win-crt-time-l1-1-0.dll", "EE815A158BAACB357D9E074C0755B6F6C286B625", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-crt-time-l1-1-0.dll", 21256 },
		{ "launcher/api-ms-win-crt-utility-l1-1-0.dll", "EAA07829D012206AC55FB1AF5CC6A35F341D22BE", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/api-ms-win-crt-utility-l1-1-0.dll", 19208 },
		{ "launcher/Launcher.exe", "F259DE45C50F399D3E278FD39401EF51A3CC031A", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/Launcher.exe", 48490288 },
		{ "launcher/Launcher.rpf", "237682874D921209CDBDB16E257C65A9480BAD94", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/Launcher.rpf", 858112 },
		{ "launcher/LauncherPatcher.exe", "BFD3A153979C2CED11F6F8BFDBE767AD502F4655", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/LauncherPatcher.exe", 508208 },
		{ "launcher/mtl_libovr.dll", "0FF4CEDA9DE3B63C4DE6E1626009D5ED5A475C96", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/mtl_libovr.dll", 178584 },
		{ "launcher/offline.pak", "53F93E488AA5482C187641CE85164F7C5A1ED8B2", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/offline.pak", 1597382 },
		{ "launcher/RockstarService.exe", "FCA2A3393CEDB7DE49C6ABAD69F2ACC7354DFD66", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/RockstarService.exe", 2017072 },
		{ "launcher/RockstarSteamHelper.exe", "8E10781C248612A0F00A2BBFA828FC110978E751", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/RockstarSteamHelper.exe", 1063216 },
		{ "launcher/ucrtbase.dll", "4189F4459C54E69C6D3155A82524BDA7549A75A6", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/ucrtbase.dll", 1016584 },
		{ "launcher/ThirdParty/Epic/EOSSDK-Win64-Shipping-1.13.1.dll", "9176F6D58E46153342D7B065D279636DF8298603", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/ThirdParty/Epic/EOSSDK-Win64-Shipping-1.13.1.dll", 23390688 },
		{ "launcher/ThirdParty/Epic/EOSSDK-Win64-Shipping.dll", "AF01787DDB7DE00239EDC62D33E0B20C0BE80037", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/ThirdParty/Epic/EOSSDK-Win64-Shipping.dll", 9971968 },
		{ "launcher/ThirdParty/Steam/steam_api64.dll", "BD014660F7978A07BA2F99B6CF0621D678602663", "https://content.cfx.re/mirrors/mtl/1.0.53.576/launcher/ThirdParty/Steam/steam_api64.dll", 121256 }, 
	};

	for (const auto& entry : launcherEntries)
	{
		g_requiredEntries.push_back(entry);
	}
#endif

#if defined(COMPILING_GLUE)
	g_requiredEntries.clear();
#endif

	// cross-build toggle
#ifdef GTA_FIVE

	// Either the feature flag for the new build system with single executable is not set.
	// Or we are loading the game build that does not require any overrides.
	if (GetReplaceExecutable() || GetTargetGameBuild() >= xbr::GetDefaultGameBuild())
	{
		for (auto [_, entry]: g_entriesToLoadPerBuild[GetTargetGameBuild()])
		{
			g_requiredEntries.push_back(entry);
		}
	}
	else
	{
		// Download files for the latest stable executable build because that's what we run regardless of the requested version.
		for (auto [_, entry]: g_entriesToLoadPerBuild[xbr::GetDefaultGameBuild()])
		{
			g_requiredEntries.push_back(entry);
		}

		// Load update.rpf files for the old game builds.
		// The "override/update/update.rpf" doesn't exists among the original files.
		// However we set it here so mapping between MakeRelativeGamePath("override\\update\\update.rpf") and MakeRelativeCitPath("data\\game-storage\\override+update+update.rpf_*") is established.
		// We will mount MakeRelativeCitPath("data\\game-storage\\override+update+update.rpf_*") in UpdateRpfMount.cpp to override some files when old game build is requested.
		if (GetTargetGameBuild() != 1)
		{
			g_requiredEntries.push_back({"override/update/update.rpf", g_entriesToLoadPerBuild[GetTargetGameBuild()].at("update.rpf")});
		}
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

	if (IsTargetGameBuildOrGreater<2372>())
	{
		g_requiredEntries.push_back({ "update/x64/dlcpacks/mptuner/dlc.rpf", "7a7521b3396701f4fe8ae51347c6206c46306648", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 3482869760 });
	}

	if (IsTargetGameBuildOrGreater<2545>())
	{
		g_requiredEntries.push_back({ "update/x64/dlcpacks/mpsecurity/dlc.rpf", "27c8100da2537472ad012df036a95da08188d54a", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 1978963968 });
		g_requiredEntries.push_back({ "update/x64/dlcpacks/mpsecurity/dlc1.rpf", "82f34009966d790a2987c70a2872a5658a71f198", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 1308874752 });
	}

	if (IsTargetGameBuildOrGreater<2612>())
	{
		g_requiredEntries.push_back({ "update/x64/dlcpacks/mpg9ec/dlc.rpf", "011114b746a4d5a830241a174b3e16eb2f63f224", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 1847296 });
	}

	if (IsTargetGameBuildOrGreater<2699>())
	{
		g_requiredEntries.push_back({ "update/x64/dlcpacks/mpsum2/dlc.rpf", "5cb63b0939a716e899fa1f514b73a14ca4b58129", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 1245167616 });
	}

	if (IsTargetGameBuildOrGreater<2802>())
	{
		g_requiredEntries.push_back({ "update/x64/dlcpacks/mpchristmas3/dlc.rpf", "500440406ee1aa825ce2371699b127fce460d9a2", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 1822871552 });
	}

	if (IsTargetGameBuildOrGreater<2944>())
	{
		g_requiredEntries.push_back({ "update/x64/dlcpacks/mp2023_01/dlc.rpf", "11519d20c34a5f34d06252078b41e28275dbc67b", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 809424896 });
	}

	if (IsTargetGameBuildOrGreater<3095>())
	{
		g_requiredEntries.push_back({ "update/x64/dlcpacks/mp2023_02/dlc.rpf", "22afecbf20f46f1a871f442b2822b120bb41fbbf", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 1604741120 });
	}
#elif IS_RDR3
	for (auto [_, entry]: g_entriesToLoadPerBuild[GetTargetGameBuild()])
	{
		g_requiredEntries.push_back(entry);
	}

	if (IsTargetGameBuild<1355>())
	{
		g_requiredEntries.push_back({ "x64/dlcpacks/mp008/dlc.rpf", "66a50ed07293b92466423e1db5eed159551d8c25", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 487150980 });
	}

	if (IsTargetGameBuild<1436>() || IsTargetGameBuild<1491>())
	{
		g_requiredEntries.push_back({ "x64/dlcpacks/mp009/dlc.rpf", "7ae2012968709d6d1079c88ee40369f4359778bf", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 494360763 });
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
			retval.insert({ origFileName, ToNarrow(entry.GetCacheFileName()) });
		}
	}

	return retval;
}
#endif
#endif
