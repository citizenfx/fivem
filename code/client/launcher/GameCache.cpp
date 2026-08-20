/*
* This file is part of the Cfx project - https://cfx.re/
*
* See LICENSE in the root of the source tree for information
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
extern int gameCacheDefaultBuild;

inline int GetTargetGameBuild()
{
	return gameCacheTargetBuild;
}

inline int GetDefaultBuild()
{
	return gameCacheDefaultBuild;
}
#else
inline int GetTargetGameBuild()
{
	return xbr::GetRequestedGameBuild();
}

inline int GetDefaultBuild()
{
	return xbr::GetGameBuild();
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
		3889,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "a9bff6962adade85843616fd932df58b13e68036", "https://downloads.cfx-services.net/prod/019fec11-0ff5-7c7c-9b27-07a47ff4407f/GTA5.exe", 47467128 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec1d-8d01-7651-b4e5-e3d3eb0d5be1/update.rpf", 2014193664,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec11-2768-701c-a9f2-1bbce5fca974/from_1604_to_3889.update.rpf.hdiff", 1520607219 } /* diff sha1: 48d5d78b1975eb4f59f750204e768de7f1f94a43 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec12-0edc-7956-9d45-5e0ddb62fdb4/from_2060_to_3889.update.rpf.hdiff", 1292820764 } /* diff sha1: 88c41cc4c9f273922c136e31c37983d42a93a3cc */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec12-dce3-7dab-a681-6ca345b01558/from_2189_to_3889.update.rpf.hdiff", 1250648559 } /* diff sha1: 187f1d933837b9f276594eacb15294ad4a63779a */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec13-a657-7043-8a62-5e81b49fab54/from_2372_to_3889.update.rpf.hdiff", 1235034885 } /* diff sha1: bbb2304749570c8db6cdc443105ee9116f0acda4 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec14-6923-7a64-a947-3bb0741af251/from_2545_to_3889.update.rpf.hdiff", 1096923323 } /* diff sha1: afe98e5a2a0e1b9e1b91d8c9cab3d3145710f920 */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec15-18c3-73f8-a13b-660248ad5732/from_2612_to_3889.update.rpf.hdiff", 1096922209 } /* diff sha1: 12e3bb4f308903855910d49d2be2c2bb4af1edb2 */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec16-18c0-74e5-a806-dda05748a2c7/from_2699_to_3889.update.rpf.hdiff", 1052142402 } /* diff sha1: 38a14676ac37f3f651a01db6b202d2d8b34afc69 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec17-058e-74bf-9385-40f04288a88a/from_2802_to_3889.update.rpf.hdiff", 995682048 } /* diff sha1: 51b56cea521b57634b02e9b2b9a3deea4318e809 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec17-f078-7131-9028-0d786ee83d68/from_2944_to_3889.update.rpf.hdiff", 985614345 } /* diff sha1: da347faa6a42c17b29c4d238858f93b02775c06c */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec18-cf21-7599-bf95-8839a56ebb61/from_3095_to_3889.update.rpf.hdiff", 657759280 } /* diff sha1: f95d40000b28830f7bf8c34e868dc9acaf5a8382 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec18-cf21-7599-bf95-8839a56ebb61/from_3095_to_3889.update.rpf.hdiff", 657759280 } /* diff sha1: f95d40000b28830f7bf8c34e868dc9acaf5a8382 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec19-cdcb-7712-9d5a-a5144db6a1d9/from_3258_to_3889.update.rpf.hdiff", 649812315 } /* diff sha1: 05f86f4472fb1bf1808d3d1369f6147434cd2b71 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec1a-7b6e-7c81-9a93-b36358585e19/from_3323_to_3889.update.rpf.hdiff", 649791573 } /* diff sha1: cfc6847aa27715196079b992dc4c80d482d9925d */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec1b-2fc4-7414-8354-8107e45bfc2a/from_3407_to_3889.update.rpf.hdiff", 637739905 } /* diff sha1: e0d8e768473760e9f4539110b42a36bb2417022b */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec1b-d5c2-78cf-aa4e-87511f93e45c/from_3570_to_3889.update.rpf.hdiff", 611541852 } /* diff sha1: 6f9015557b677edf921f7de08e942da9ee022be1 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec1c-e199-7529-942b-0b9b9ba7d736/from_3751_to_3889.update.rpf.hdiff", 15994670 } /* diff sha1: 1ecc2002187fba12628db6db4f07ab5164e26c98 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "31cbd81373475d5407c20058733ea910cadce13b", "https://downloads.cfx-services.net/prod/019fec1c-e199-7529-942b-0b9b9ba7d736/from_3751_to_3889.update.rpf.hdiff", 15994670 } /* diff sha1: 1ecc2002187fba12628db6db4f07ab5164e26c98 */
					},
				},
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "https://downloads.cfx-services.net/prod/019fec1e-b8d2-7b4d-9bfd-074be5bac619/update2.rpf", 503742464,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "https://downloads.cfx-services.net/prod/019fec15-cb00-76d2-a0b1-c2b6c4f428f9/from_2612_to_3889.update2.rpf.hdiff", 433099044 } /* diff sha1: 454695b720fabcea24f720e8a8c0be8a3f951bd0 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "https://downloads.cfx-services.net/prod/019fec16-ba35-7181-867c-8c908df4117e/from_2699_to_3889.update2.rpf.hdiff", 431413110 } /* diff sha1: 4700e97c087d250b030c3107cb386d1e8274062d */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "https://downloads.cfx-services.net/prod/019fec17-9ee2-7015-bbd0-34cc6449fcf6/from_2802_to_3889.update2.rpf.hdiff", 431110090 } /* diff sha1: a73c0ceb90b50d47c9084c34cf6975e8ee38fbb5 */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "https://downloads.cfx-services.net/prod/019fec18-862b-7146-bcee-f80e97515c96/from_2944_to_3889.update2.rpf.hdiff", 430845481 } /* diff sha1: c22dc154611b7346b36509da517c4b18f4c4e0cc */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "https://downloads.cfx-services.net/prod/019fec19-33c5-7d45-9fa2-69715e139f7d/from_3095_to_3889.update2.rpf.hdiff", 429647926 } /* diff sha1: f39b67f4e30211ba177779b8eab88afdb2a2ebe1 */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "https://downloads.cfx-services.net/prod/019fec19-85db-74c0-8318-5c85746d7317/from_3179_to_3889.update2.rpf.hdiff", 429647943 } /* diff sha1: 69fb9dc5716a58942ffff1b7f2e6a878782ff36f */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "https://downloads.cfx-services.net/prod/019fec1a-38bf-73dd-aec3-10e3f5792c0b/from_3258_to_3889.update2.rpf.hdiff", 429260477 } /* diff sha1: 76603eea78c45891671a19af8ce92cbece4cf275 */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "https://downloads.cfx-services.net/prod/019fec1a-e35e-7671-a28f-6d61507488a6/from_3323_to_3889.update2.rpf.hdiff", 429260483 } /* diff sha1: 47fb90c3c06e0bf0e8c43661e205594ad4d4301c */,
						{ "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "https://downloads.cfx-services.net/prod/019fec1b-913b-74c5-bd4f-07cb9339b9b2/from_3407_to_3889.update2.rpf.hdiff", 428917858 } /* diff sha1: 6fb47bc9c2158f517039405ce6192d1572b59db3 */,
						{ "28095c30975ffaead56ff72dfb6418d19eef38dc", "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "https://downloads.cfx-services.net/prod/019fec1c-3648-7b92-b015-48eac10f5a28/from_3570_to_3889.update2.rpf.hdiff", 428356486 } /* diff sha1: 9fc1178619cf1e78b89a3c87d2a56c7d0213e1ee */,
						{ "e67aad3699a50988dd03d201110346e38ebf5225", "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "https://downloads.cfx-services.net/prod/019fec1c-eb7b-7345-9e3a-4efb8b610e4d/from_3751_to_3889.update2.rpf.hdiff", 427369724 } /* diff sha1: 780563431ae52b8f322d41e0e64c66eaaec9298a */,
						{ "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "https://downloads.cfx-services.net/prod/019fec1d-3b7b-79d2-a959-dbf1fe0c3cff/from_3788_to_3889.update2.rpf.hdiff", 427369722 } /* diff sha1: 34531507f644f9dc92188f21ca82a722e4e21f8f */
					},
				},
			},
		}
	},
	{
		3788,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "82b4575d6a3dad6ecf5460060a97ac51c0be4990", "https://downloads.cfx-services.net/prod/019fec0c-a838-740b-8039-d48f46f2a4ac/GTA5.exe", 49221752 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec0b-1e9e-7a7b-94e0-45876a202c5c/update.rpf", 2010816512,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019febfe-5ada-7634-92b6-0968b5d03ae1/from_1604_to_3751.update.rpf.hdiff", 1517246851 } /* diff sha1: c5192c98ee15dee20ad56b286cabad416e130054 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019febff-60dd-71fe-89f5-ea5f19b05e4d/from_2060_to_3751.update.rpf.hdiff", 1289468906 } /* diff sha1: 4b207375055304a3491f08342a0ccd56e9e4bf01 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec00-3d4e-761d-a0a5-0c0317d3e366/from_2189_to_3751.update.rpf.hdiff", 1247294382 } /* diff sha1: 646a58ecb0936824571642925ab97a8b58ccd5ab */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec01-0ca3-7d10-8886-a6235d5a875c/from_2372_to_3751.update.rpf.hdiff", 1231684939 } /* diff sha1: f553c98ece72cccf29c9167b1a8ffa697762ed46 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec01-e1b3-74a6-adb0-39b7d03113c9/from_2545_to_3751.update.rpf.hdiff", 1093569564 } /* diff sha1: 68b02bcf2c60b49c003daf644cb1f2575e3c531a */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec02-93d4-7270-914b-dfbbbda097e0/from_2612_to_3751.update.rpf.hdiff", 1093568183 } /* diff sha1: b086516115c4dbab752e9c6902f7e014b797e9dc */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec03-a1d5-7903-992c-9f047dc56bad/from_2699_to_3751.update.rpf.hdiff", 1048755655 } /* diff sha1: 03060b0537f826308e95612eb9a6bb063bd9f3fd */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec04-8d9f-71bd-adb8-07f8fced18dd/from_2802_to_3751.update.rpf.hdiff", 992333361 } /* diff sha1: a0e7c5110d30b6bb5a1f7c3abaa138e4dd093942 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec05-7a13-7183-ae2d-b1bdaf7f93a4/from_2944_to_3751.update.rpf.hdiff", 982222965 } /* diff sha1: b5b9bd09d699be40852f7032df7f25a65bf267e1 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec06-6200-7e16-ba9c-78d0b58934c2/from_3095_to_3751.update.rpf.hdiff", 654361091 } /* diff sha1: 0ab4ccb16a59ecd9d340c1902b0854bf9654bc2d */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec06-6200-7e16-ba9c-78d0b58934c2/from_3095_to_3751.update.rpf.hdiff", 654361091 } /* diff sha1: 0ab4ccb16a59ecd9d340c1902b0854bf9654bc2d */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec07-6e4b-7341-b307-ca29401f06e3/from_3258_to_3751.update.rpf.hdiff", 646458713 } /* diff sha1: df3977a7ec8b5b8d3ec7ffa89e46b0195ab661d5 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec08-301a-7db0-ae37-d7f672d7b69f/from_3323_to_3751.update.rpf.hdiff", 646439748 } /* diff sha1: 8c9b3167cb7cbff4e9a961a71102b389274241db */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec08-e5b4-7dea-ae9b-48c126f2375f/from_3407_to_3751.update.rpf.hdiff", 634093722 } /* diff sha1: 7376b10703599ff034fd197164553067e2a8c231 */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec09-9e60-7cea-8488-0aeb8333b4f5/from_3570_to_3751.update.rpf.hdiff", 607732534 } /* diff sha1: 6e2c9512a618e7428a64c82110a03d280d134e9b */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec0a-c907-705e-91a9-3f317e2ffe2f/from_3889_to_3751.update.rpf.hdiff", 12642016 } /* diff sha1: 41494bc170d646f44c2a07d295d996b697dbc142 */
					},
				},
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "https://downloads.cfx-services.net/prod/019fec10-c2c8-763e-a759-f92176e38b5d/update2.rpf", 494157824,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "https://downloads.cfx-services.net/prod/019fec0c-f91a-7b47-912e-660ce4ece93b/from_2612_to_3788.update2.rpf.hdiff", 423514012 } /* diff sha1: 41b9a4118923cda3b5c4bbd1636e98db97cf4ad9 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "https://downloads.cfx-services.net/prod/019fec0d-4e35-731e-8b45-83e310971986/from_2699_to_3788.update2.rpf.hdiff", 421828086 } /* diff sha1: 681e84d5c42ef0840b89ed2ce6561f850c0837de */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "https://downloads.cfx-services.net/prod/019fec0d-a214-74f2-9bae-5ea03e41ec72/from_2802_to_3788.update2.rpf.hdiff", 421525069 } /* diff sha1: 34f693b4ca3142130635d00bd15774d7887f828e */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "https://downloads.cfx-services.net/prod/019fec0d-f176-73cb-bd5d-08b6ae6e45fb/from_2944_to_3788.update2.rpf.hdiff", 421260453 } /* diff sha1: ffafd099d931492c63781302e732b96d9c469d26 */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "https://downloads.cfx-services.net/prod/019fec0e-3be8-7520-bfb1-8753223a238f/from_3095_to_3788.update2.rpf.hdiff", 420062930 } /* diff sha1: e2d10ab574f5bbcc5bd85fb079d76daa39339642 */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "https://downloads.cfx-services.net/prod/019fec0e-85bd-7495-9f64-e81b8731acb7/from_3179_to_3788.update2.rpf.hdiff", 420061425 } /* diff sha1: a485065085dab17d83c11ead3ab14b4b78b1bbeb */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "https://downloads.cfx-services.net/prod/019fec0e-ceea-78d4-8c53-8e39681493b9/from_3258_to_3788.update2.rpf.hdiff", 419675455 } /* diff sha1: 4f06f230cbed19fd165aee14efc7a88ab783ae57 */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "https://downloads.cfx-services.net/prod/019fec0f-19be-7e48-9379-bcb0aa9d94b4/from_3323_to_3788.update2.rpf.hdiff", 419675473 } /* diff sha1: 1f522c43728380d057238d0a1ca4af270a7aca0c */,
						{ "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "https://downloads.cfx-services.net/prod/019fec0f-6418-74f6-ac24-e8311d4816b2/from_3407_to_3788.update2.rpf.hdiff", 419314890 } /* diff sha1: 7fdeba907097cd1eefcc2da7f83fcf73598c6131 */,
						{ "28095c30975ffaead56ff72dfb6418d19eef38dc", "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "https://downloads.cfx-services.net/prod/019fec0f-ae83-7e36-b27a-fa8d64c98bcd/from_3570_to_3788.update2.rpf.hdiff", 418752493 } /* diff sha1: 15dd80ee1e73578661ffa23573e4d36fc59f2085 */,
						{ "e67aad3699a50988dd03d201110346e38ebf5225", "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "https://downloads.cfx-services.net/prod/019fec10-35c6-78ee-a850-8d91719b6a10/from_3751_to_3788.update2.rpf.hdiff", 312131424 } /* diff sha1: 52e4c031efebcd420e679c909ad1303f9e48c764 */,
						{ "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "https://downloads.cfx-services.net/prod/019fec10-6c9e-718b-84ff-09b53ce01dbd/from_3889_to_3788.update2.rpf.hdiff", 417788974 } /* diff sha1: bb9b23af0fe9809d061fae87dd3af494b9f4f729 */
					},
				},
			},
		}
	},
	{
		3751,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "e5fe7bd7c6c26e09e3ce0335c89a2cdc9080b925", "https://downloads.cfx-services.net/prod/019febfe-3eb6-70ae-871b-846902a4be43/GTA5.exe", 49183352 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec0b-1e9e-7a7b-94e0-45876a202c5c/update.rpf", 2010816512,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019febfe-5ada-7634-92b6-0968b5d03ae1/from_1604_to_3751.update.rpf.hdiff", 1517246851 } /* diff sha1: c5192c98ee15dee20ad56b286cabad416e130054 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019febff-60dd-71fe-89f5-ea5f19b05e4d/from_2060_to_3751.update.rpf.hdiff", 1289468906 } /* diff sha1: 4b207375055304a3491f08342a0ccd56e9e4bf01 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec00-3d4e-761d-a0a5-0c0317d3e366/from_2189_to_3751.update.rpf.hdiff", 1247294382 } /* diff sha1: 646a58ecb0936824571642925ab97a8b58ccd5ab */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec01-0ca3-7d10-8886-a6235d5a875c/from_2372_to_3751.update.rpf.hdiff", 1231684939 } /* diff sha1: f553c98ece72cccf29c9167b1a8ffa697762ed46 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec01-e1b3-74a6-adb0-39b7d03113c9/from_2545_to_3751.update.rpf.hdiff", 1093569564 } /* diff sha1: 68b02bcf2c60b49c003daf644cb1f2575e3c531a */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec02-93d4-7270-914b-dfbbbda097e0/from_2612_to_3751.update.rpf.hdiff", 1093568183 } /* diff sha1: b086516115c4dbab752e9c6902f7e014b797e9dc */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec03-a1d5-7903-992c-9f047dc56bad/from_2699_to_3751.update.rpf.hdiff", 1048755655 } /* diff sha1: 03060b0537f826308e95612eb9a6bb063bd9f3fd */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec04-8d9f-71bd-adb8-07f8fced18dd/from_2802_to_3751.update.rpf.hdiff", 992333361 } /* diff sha1: a0e7c5110d30b6bb5a1f7c3abaa138e4dd093942 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec05-7a13-7183-ae2d-b1bdaf7f93a4/from_2944_to_3751.update.rpf.hdiff", 982222965 } /* diff sha1: b5b9bd09d699be40852f7032df7f25a65bf267e1 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec06-6200-7e16-ba9c-78d0b58934c2/from_3095_to_3751.update.rpf.hdiff", 654361091 } /* diff sha1: 0ab4ccb16a59ecd9d340c1902b0854bf9654bc2d */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec06-6200-7e16-ba9c-78d0b58934c2/from_3095_to_3751.update.rpf.hdiff", 654361091 } /* diff sha1: 0ab4ccb16a59ecd9d340c1902b0854bf9654bc2d */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec07-6e4b-7341-b307-ca29401f06e3/from_3258_to_3751.update.rpf.hdiff", 646458713 } /* diff sha1: df3977a7ec8b5b8d3ec7ffa89e46b0195ab661d5 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec08-301a-7db0-ae37-d7f672d7b69f/from_3323_to_3751.update.rpf.hdiff", 646439748 } /* diff sha1: 8c9b3167cb7cbff4e9a961a71102b389274241db */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec08-e5b4-7dea-ae9b-48c126f2375f/from_3407_to_3751.update.rpf.hdiff", 634093722 } /* diff sha1: 7376b10703599ff034fd197164553067e2a8c231 */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec09-9e60-7cea-8488-0aeb8333b4f5/from_3570_to_3751.update.rpf.hdiff", 607732534 } /* diff sha1: 6e2c9512a618e7428a64c82110a03d280d134e9b */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "https://downloads.cfx-services.net/prod/019fec0a-c907-705e-91a9-3f317e2ffe2f/from_3889_to_3751.update.rpf.hdiff", 12642016 } /* diff sha1: 41494bc170d646f44c2a07d295d996b697dbc142 */
					},
				},
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "e67aad3699a50988dd03d201110346e38ebf5225", "https://downloads.cfx-services.net/prod/019fec0c-58b1-7d4b-8855-5cd9484f4618/update2.rpf", 494157824,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "e67aad3699a50988dd03d201110346e38ebf5225", "https://downloads.cfx-services.net/prod/019fec03-4ec7-7726-aab9-9d3380cca179/from_2612_to_3751.update2.rpf.hdiff", 423514001 } /* diff sha1: 985bbc99edbfe23e118d789ba744488cb97aa8f8 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "e67aad3699a50988dd03d201110346e38ebf5225", "https://downloads.cfx-services.net/prod/019fec04-3fe2-736f-ab7f-3eb8d8fdafb8/from_2699_to_3751.update2.rpf.hdiff", 421828074 } /* diff sha1: c897f13b6525a46d5cc625de27ee960c1bdddb64 */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "e67aad3699a50988dd03d201110346e38ebf5225", "https://downloads.cfx-services.net/prod/019fec05-2728-7886-a24b-01e7345cb43c/from_2802_to_3751.update2.rpf.hdiff", 421525071 } /* diff sha1: 1f87a184dc1b6fe29ec384e4a7d8fd1b5f369713 */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "e67aad3699a50988dd03d201110346e38ebf5225", "https://downloads.cfx-services.net/prod/019fec06-1a16-736c-8e68-46a4023a7fde/from_2944_to_3751.update2.rpf.hdiff", 421260450 } /* diff sha1: 69fbbed781daf0f799c9af551a4854fc472b93a7 */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "e67aad3699a50988dd03d201110346e38ebf5225", "https://downloads.cfx-services.net/prod/019fec06-d1af-728b-bdc2-8f8d0928467e/from_3095_to_3751.update2.rpf.hdiff", 420062931 } /* diff sha1: e720d93012b9a2817420858a60edfe87c552c7d7 */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "e67aad3699a50988dd03d201110346e38ebf5225", "https://downloads.cfx-services.net/prod/019fec07-2642-70b4-8b33-79001efc0280/from_3179_to_3751.update2.rpf.hdiff", 420062918 } /* diff sha1: f6fbd3cbe01f926959474de8a6d63313a7cccfc8 */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "e67aad3699a50988dd03d201110346e38ebf5225", "https://downloads.cfx-services.net/prod/019fec07-e872-70d8-869d-8a6109c857ca/from_3258_to_3751.update2.rpf.hdiff", 419675463 } /* diff sha1: d09f820e6c2ccc78a7f3a9bbbcfeb915a9dbddf6 */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "e67aad3699a50988dd03d201110346e38ebf5225", "https://downloads.cfx-services.net/prod/019fec08-9dc0-7a0f-8aaf-c0f7f76509d7/from_3323_to_3751.update2.rpf.hdiff", 419675467 } /* diff sha1: 663a72c6a5142faa76c230ac1832add0da751e4a */,
						{ "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "e67aad3699a50988dd03d201110346e38ebf5225", "https://downloads.cfx-services.net/prod/019fec09-52cb-78ee-8b2d-166389623608/from_3407_to_3751.update2.rpf.hdiff", 419314898 } /* diff sha1: e81d33ddac1ba0c469ea797b4f4732e8c5ef2154 */,
						{ "28095c30975ffaead56ff72dfb6418d19eef38dc", "e67aad3699a50988dd03d201110346e38ebf5225", "https://downloads.cfx-services.net/prod/019fec09-fdde-71aa-8e3f-4b7e4528bf5d/from_3570_to_3751.update2.rpf.hdiff", 418749087 } /* diff sha1: d9f6adfaa8cb4d4edcd2a1e6347d9538721a30ae */,
						{ "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "e67aad3699a50988dd03d201110346e38ebf5225", "https://downloads.cfx-services.net/prod/019fec0a-97c3-763a-9008-927da1adacf8/from_3788_to_3751.update2.rpf.hdiff", 312131424 } /* diff sha1: fdd16615910944eaff81f0203626ed54beaea38d */,
						{ "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "e67aad3699a50988dd03d201110346e38ebf5225", "https://downloads.cfx-services.net/prod/019fec0a-d28b-711d-8780-8a7e6246633c/from_3889_to_3751.update2.rpf.hdiff", 417788977 } /* diff sha1: c08067dcc717997b5aa70148b8bd052a28ac645a */
					},
				},
			},
		}
	},
	{
		3570,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "17d593958471a2224ad42d8234cb77f6832e258e", "https://downloads.cfx-services.net/prod/019febe7-22ba-74a7-b06e-2e91f3b81a50/GTA5.exe", 49242152 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febee-89de-7cbc-ac6f-eaab7ac70983/update.rpf", 1452832768,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febe7-354e-73eb-b710-0a791ceb03f7/from_1604_to_3570.update.rpf.hdiff", 962134423 } /* diff sha1: 53cb2f3c75283e9dbe605d24410013b9e7e05f80 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febe7-bfca-704d-82c4-e793a1d73635/from_2060_to_3570.update.rpf.hdiff", 734372570 } /* diff sha1: 15dada4b255f1b1f3a77ce986ec24f576569c0cc */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febe8-47db-7d47-9ca5-768f4e033738/from_2189_to_3570.update.rpf.hdiff", 693034503 } /* diff sha1: a192865ba41e1390addd295fe5c80509b76298b3 */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febe8-c0bf-7efb-8e62-37dd68167f08/from_2372_to_3570.update.rpf.hdiff", 676547682 } /* diff sha1: f15dd32f6a78ca2c2a1b8095c576de40bc4ae9a6 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febe9-373c-7163-949a-d3774aaa5fc5/from_2545_to_3570.update.rpf.hdiff", 538283177 } /* diff sha1: a4a97f765d7ad87e811deffaf9f2567b341f2824 */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febe9-8c0b-73d9-bab6-a980a568be4d/from_2612_to_3570.update.rpf.hdiff", 538281003 } /* diff sha1: 1a72dd90ba81d19850fcf8c028dd83d492f60225 */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febea-2c20-7d30-9d10-ea58ab107bee/from_2699_to_3570.update.rpf.hdiff", 493540630 } /* diff sha1: 82dc749a8f499e0aae9415b48072e3fdc9892b4f */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febea-ba06-7058-b11d-d5a231e9ff71/from_2802_to_3570.update.rpf.hdiff", 436956930 } /* diff sha1: 13a2c664aa130900f5639f29e980b703dbfd1ff3 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febeb-4157-7a8f-a966-88fb116d84d4/from_2944_to_3570.update.rpf.hdiff", 426904985 } /* diff sha1: d5f3c4ddd2852577229d1c84ffbab6dae9ac2eba */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febeb-bfbe-7ab5-9b0c-da339bd19df9/from_3095_to_3570.update.rpf.hdiff", 98581421 } /* diff sha1: 38d4df53a14c64cf080c29070d7acbcf723ca8c8 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febeb-bfbe-7ab5-9b0c-da339bd19df9/from_3095_to_3570.update.rpf.hdiff", 98581421 } /* diff sha1: 38d4df53a14c64cf080c29070d7acbcf723ca8c8 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febec-5db9-7657-a556-67a5cd5852a4/from_3258_to_3570.update.rpf.hdiff", 90104781 } /* diff sha1: 030408ceaef8f2b1a2629e6dcc681d990984052a */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febec-b502-7232-b60f-a7f46ca4e3ca/from_3323_to_3570.update.rpf.hdiff", 90083949 } /* diff sha1: f69bbdb6ae0d72e3907b039bdc2979c387249b54 */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febed-01a1-7d8b-951f-d8feedc61dbe/from_3407_to_3570.update.rpf.hdiff", 77717332 } /* diff sha1: eaf5dd480838f1340088df28e851bc5b89e4d52b */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febed-a3be-729d-b327-a6febefd4b59/from_3751_to_3570.update.rpf.hdiff", 52750371 } /* diff sha1: 9a5f76f976831a31db722f67a52db3e6e657b8df */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febed-a3be-729d-b327-a6febefd4b59/from_3751_to_3570.update.rpf.hdiff", 52750371 } /* diff sha1: 9a5f76f976831a31db722f67a52db3e6e657b8df */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "49ed7a6c3d035bcf764942dd58597211448941fd", "https://downloads.cfx-services.net/prod/019febee-2e46-7567-8459-cd0b783fdca5/from_3889_to_3570.update.rpf.hdiff", 53204940 } /* diff sha1: f06623c5740d351fd3c1cfb84260519f378f9c45 */
					},
				},
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "28095c30975ffaead56ff72dfb6418d19eef38dc", "https://downloads.cfx-services.net/prod/019febef-5d33-7cfa-9af8-78f63984a119/update2.rpf", 457410560,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "28095c30975ffaead56ff72dfb6418d19eef38dc", "https://downloads.cfx-services.net/prod/019febe9-e84e-79b5-97e3-300e7e15a4cf/from_2612_to_3570.update2.rpf.hdiff", 387180420 } /* diff sha1: fb60755ebfec5f0af4492b61c5e1315823f11544 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "28095c30975ffaead56ff72dfb6418d19eef38dc", "https://downloads.cfx-services.net/prod/019febea-7dad-78dd-bb59-36d61c5f0f9f/from_2699_to_3570.update2.rpf.hdiff", 385489322 } /* diff sha1: d791e6b9609b131a5969c0583b7b30fbc3ad4d90 */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "28095c30975ffaead56ff72dfb6418d19eef38dc", "https://downloads.cfx-services.net/prod/019febeb-03d3-72f2-a7c9-359466528bba/from_2802_to_3570.update2.rpf.hdiff", 385196841 } /* diff sha1: f5fd1edbf89477af059875613de84755858b24c1 */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "28095c30975ffaead56ff72dfb6418d19eef38dc", "https://downloads.cfx-services.net/prod/019febeb-7f42-716c-abc5-4d38050c561a/from_2944_to_3570.update2.rpf.hdiff", 384901901 } /* diff sha1: b262e45fe49bd0ce38cffda6184b32d917ec88c7 */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "28095c30975ffaead56ff72dfb6418d19eef38dc", "https://downloads.cfx-services.net/prod/019febeb-d875-704b-83c1-53b8118bfaa1/from_3095_to_3570.update2.rpf.hdiff", 383653754 } /* diff sha1: 3aaa632107d2ca98ee5c9c76fee0a7a748d615c0 */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "28095c30975ffaead56ff72dfb6418d19eef38dc", "https://downloads.cfx-services.net/prod/019febec-1b01-7c3a-86c5-9335fe238b79/from_3179_to_3570.update2.rpf.hdiff", 383654569 } /* diff sha1: 99aab3e05f6acda96ee484c81311990084d6e975 */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "28095c30975ffaead56ff72dfb6418d19eef38dc", "https://downloads.cfx-services.net/prod/019febec-7283-7ea0-9c96-9d1b8c7906a7/from_3258_to_3570.update2.rpf.hdiff", 383279408 } /* diff sha1: 9b297521ce42d917c0ee59a300827856363251e3 */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "28095c30975ffaead56ff72dfb6418d19eef38dc", "https://downloads.cfx-services.net/prod/019febec-ca82-78f4-9116-34a41d17b3a5/from_3323_to_3570.update2.rpf.hdiff", 383273840 } /* diff sha1: 0f5cf045ac35b863104baa8b69e0074603da45a6 */,
						{ "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "28095c30975ffaead56ff72dfb6418d19eef38dc", "https://downloads.cfx-services.net/prod/019febed-1323-70cb-ba2c-8fab5e73a011/from_3407_to_3570.update2.rpf.hdiff", 382901098 } /* diff sha1: a3705d9337423df47d944048d96e832b5828eb1a */,
						{ "e67aad3699a50988dd03d201110346e38ebf5225", "28095c30975ffaead56ff72dfb6418d19eef38dc", "https://downloads.cfx-services.net/prod/019febed-b294-7a75-ac72-d43ae3012002/from_3751_to_3570.update2.rpf.hdiff", 382001828 } /* diff sha1: 619176d571f1fa87c409224ae1e096404ff73f7d */,
						{ "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "28095c30975ffaead56ff72dfb6418d19eef38dc", "https://downloads.cfx-services.net/prod/019febed-f085-78b4-93c3-e9b6fb0e2711/from_3788_to_3570.update2.rpf.hdiff", 382005234 } /* diff sha1: 2958f3782922a34d17535d11894e7b0adaab9a72 */,
						{ "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "28095c30975ffaead56ff72dfb6418d19eef38dc", "https://downloads.cfx-services.net/prod/019febee-3d2e-7de1-9065-544f00a851f1/from_3889_to_3570.update2.rpf.hdiff", 382024588 } /* diff sha1: 36bf88796b4d497827ad91e0025c160dbc624138 */
					},
				},
			},
		}
	},
	{
		3407,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "360198195282249ed7a4f8504d3b509457b06c6c", "https://downloads.cfx-services.net/prod/019febdf-085d-71cc-9110-9bdf03e51cb0/GTA5.exe", 54341608 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febe5-fa13-738b-97ab-b8e39e340b71/update.rpf", 1432735744,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febdf-1d98-7ab5-a8e0-1d2d2fc79ef6/from_1604_to_3407.update.rpf.hdiff", 942349219 } /* diff sha1: b262bbf0ae4f964e77884cac069e2d2321e0839f */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febdf-a7fd-740e-8cfe-362b21ca14e1/from_2060_to_3407.update.rpf.hdiff", 714530091 } /* diff sha1: bc60c0b07b53ae933aed9ae40b5be2330f5f2a8a */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febe0-1b3f-79b9-8346-837bdae77a91/from_2189_to_3407.update.rpf.hdiff", 673138604 } /* diff sha1: b2d37b9ee1e83385e46f3ef72884b7877dd6b258 */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febe0-7fcd-7734-9776-cb1e84296822/from_2372_to_3407.update.rpf.hdiff", 656589671 } /* diff sha1: 1e9a2ce28c9177ba03e790ed40d28bba87d612d9 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febe0-f885-7433-953b-d71afc895291/from_2545_to_3407.update.rpf.hdiff", 518336416 } /* diff sha1: 9d70f91b4de33b71b616d0e6ad7cffebdd02569a */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febe1-4534-7e67-8240-4234687dbe79/from_2612_to_3407.update.rpf.hdiff", 518335629 } /* diff sha1: 69fb649c27f02aa0f9a24a3640b56b1203c2407f */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febe1-ceb9-7a29-89f9-8e79853a0b4b/from_2699_to_3407.update.rpf.hdiff", 473523013 } /* diff sha1: 797bf09eec34c5f22da35843fa1fa02379853377 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febe2-55a8-7d88-9239-58710723ebfe/from_2802_to_3407.update.rpf.hdiff", 416908267 } /* diff sha1: ee6003ae3e341db66ce1f96a045fbac76762eb0e */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febe2-d427-7860-b9f5-2db618df48ae/from_2944_to_3407.update.rpf.hdiff", 403895326 } /* diff sha1: d0a70ae28d789102cd15117fc54ec69dde8470af */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febe3-6217-7afe-85ab-20c3f8fdae91/from_3095_to_3407.update.rpf.hdiff", 75132778 } /* diff sha1: 50970d2ba367e464b4814048323f8ce9da36c002 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febe3-6217-7afe-85ab-20c3f8fdae91/from_3095_to_3407.update.rpf.hdiff", 75132778 } /* diff sha1: 50970d2ba367e464b4814048323f8ce9da36c002 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febe3-ec5b-7c28-8b5d-78073c047790/from_3258_to_3407.update.rpf.hdiff", 61660313 } /* diff sha1: ff4277c4f4720d360fef08808ff47dd8e5aaf30e */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febe4-3631-7b79-8cb4-708d39239652/from_3323_to_3407.update.rpf.hdiff", 61640641 } /* diff sha1: 8d74bafc7b4674cdc5cc4abe6d34607283519ef5 */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febe4-8584-753d-8359-9f6e1e46648f/from_3570_to_3407.update.rpf.hdiff", 57944658 } /* diff sha1: 82114f25bbd4fc836586e6b47e88bea416c2d6ed */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febe5-1321-768e-800a-2059efe70432/from_3751_to_3407.update.rpf.hdiff", 59329318 } /* diff sha1: ad6a9fbc7e6680f713d32d5491f3a9f90680a2f3 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febe5-1321-768e-800a-2059efe70432/from_3751_to_3407.update.rpf.hdiff", 59329318 } /* diff sha1: ad6a9fbc7e6680f713d32d5491f3a9f90680a2f3 */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "f6cdcdec5e3e993a31f45acc96b638283c474f53", "https://downloads.cfx-services.net/prod/019febe5-a304-7d0a-b462-4b3be12a33a8/from_3889_to_3407.update.rpf.hdiff", 59624127 } /* diff sha1: 08df2a3c27de5f8ff4f9fa94829211dda05f20e7 */
					},
				},
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "https://downloads.cfx-services.net/prod/019febe6-d89d-724d-95c3-12791d0092a6/update2.rpf", 440010752,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "https://downloads.cfx-services.net/prod/019febe1-96dd-7278-9b66-86da5bb2ed3c/from_2612_to_3407.update2.rpf.hdiff", 369334200 } /* diff sha1: eb45dd50cce0c58391437cf631c1c6294005d897 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "https://downloads.cfx-services.net/prod/019febe2-1d27-7d19-812e-bc9b7cea535a/from_2699_to_3407.update2.rpf.hdiff", 367631685 } /* diff sha1: 524200a1f7824137c6c4afa5827caae63b6418fa */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "https://downloads.cfx-services.net/prod/019febe2-9bfe-7ded-ae77-a3790a86be43/from_2802_to_3407.update2.rpf.hdiff", 367337281 } /* diff sha1: c3185fcdcf5926fa20977be423b647abac5b6e3d */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "https://downloads.cfx-services.net/prod/019febe3-1834-7977-b985-8929f3bbe9cd/from_2944_to_3407.update2.rpf.hdiff", 367059221 } /* diff sha1: a83a6e7c67c179d178999b62935f1dfc482adea9 */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "https://downloads.cfx-services.net/prod/019febe3-72ed-799a-bc34-3ac4a3a2e820/from_3095_to_3407.update2.rpf.hdiff", 364900867 } /* diff sha1: c62e0e3d4c9dfc14fbf39f4276684e3a1839c5a1 */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "https://downloads.cfx-services.net/prod/019febe3-b21b-796a-b570-15c2500da7b0/from_3179_to_3407.update2.rpf.hdiff", 364900652 } /* diff sha1: 84cd9d7dfb58be3979111f44c78da56e4f82fae2 */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "https://downloads.cfx-services.net/prod/019febe3-fc5b-711f-a3ac-6230db62d313/from_3258_to_3407.update2.rpf.hdiff", 364379838 } /* diff sha1: e18b16193287aeb8734455f31177ae384d2fbbad */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "https://downloads.cfx-services.net/prod/019febe4-48b7-7c4d-8e5d-510fc538b729/from_3323_to_3407.update2.rpf.hdiff", 364385726 } /* diff sha1: f897b850658b5f1009809bd47faf80049094a513 */,
						{ "28095c30975ffaead56ff72dfb6418d19eef38dc", "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "https://downloads.cfx-services.net/prod/019febe4-944d-7eb7-bb9e-9ce1d2ab12eb/from_3570_to_3407.update2.rpf.hdiff", 365504169 } /* diff sha1: afbafc03a8500c36313221f5c88139c6da2301b8 */,
						{ "e67aad3699a50988dd03d201110346e38ebf5225", "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "https://downloads.cfx-services.net/prod/019febe5-22b0-74a9-b256-bda29db3ca19/from_3751_to_3407.update2.rpf.hdiff", 365167827 } /* diff sha1: 8a6994b85b98a51b5a4cad770315addf85c75cf6 */,
						{ "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "https://downloads.cfx-services.net/prod/019febe5-5ac1-723c-9ddf-1c53f061ffbb/from_3788_to_3407.update2.rpf.hdiff", 365167825 } /* diff sha1: 18c4b1846c2befe03b4db7a39629329ee80092d1 */,
						{ "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "https://downloads.cfx-services.net/prod/019febe5-b33e-73a8-b0eb-c226296b68a7/from_3889_to_3407.update2.rpf.hdiff", 365186147 } /* diff sha1: 947e5f6aa48993959f045c24a6046b9772561cc5 */
					},
				},
			},
		}
	},
	{
		3323,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "059bcf06de5a683ad39f8d24543cae80a988b4cb", "https://downloads.cfx-services.net/prod/019febd7-5c35-79f8-8bd5-ab7177daf0db/GTA5.exe", 57496560 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febdd-f91a-773a-9446-c1a2434a86b0/update.rpf", 1423288320,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febd7-7010-7d07-bca4-a68e5bcadd50/from_1604_to_3323.update.rpf.hdiff", 932235268 } /* diff sha1: ea4f44780ed788a6c0acf0c09e1d1f96c3cc84ff */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febd7-fa9c-7604-8643-65a4543603ab/from_2060_to_3323.update.rpf.hdiff", 704310918 } /* diff sha1: 2bd7c551d7d9ded134c7b88556c83b0e1a9437e9 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febd8-6918-7c2a-9c5b-65b545ee5c82/from_2189_to_3323.update.rpf.hdiff", 663014659 } /* diff sha1: bef7ecca334d5dc3887749b80f653f6f7071017d */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febd8-ef27-760c-852a-654b8f03b2bb/from_2372_to_3323.update.rpf.hdiff", 646437484 } /* diff sha1: cb0ef5e15d897d42d843974135af5afcf13a6912 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febd9-5ad2-72ac-9c34-22c1708514ea/from_2545_to_3323.update.rpf.hdiff", 507689034 } /* diff sha1: d821a26c0e436f024850ee621d810ee3c6da41b0 */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febd9-a9ce-7bf2-a321-4b25bd988c28/from_2612_to_3323.update.rpf.hdiff", 507688645 } /* diff sha1: 8b1648690f13fb96e42a27cf55e808b35eb4527f */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febda-2aed-7bf4-bca7-88fa6ae578b8/from_2699_to_3323.update.rpf.hdiff", 462961012 } /* diff sha1: cd2bbded7f26f63cbf30d8607bb5a28034b7c46b */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febda-a746-7d85-a70a-61abfcb8fb1f/from_2802_to_3323.update.rpf.hdiff", 406279747 } /* diff sha1: ec590139ddddd6f4c4df2754ce6e96119b022ce4 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febdb-2992-76b0-aa53-4da32990f026/from_2944_to_3323.update.rpf.hdiff", 393108441 } /* diff sha1: be7846a48dd84f2fe0c75a7693091d69e0674006 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febdb-9d76-7823-9e70-d5480e62f1b4/from_3095_to_3323.update.rpf.hdiff", 63784896 } /* diff sha1: 46d8201479456cf560b74ed95f12557516e75e0d */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febdb-9d76-7823-9e70-d5480e62f1b4/from_3095_to_3323.update.rpf.hdiff", 63784896 } /* diff sha1: 46d8201479456cf560b74ed95f12557516e75e0d */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febdc-1f8f-7a0c-8522-dba7bbaeced5/from_3258_to_3323.update.rpf.hdiff", 232538 } /* diff sha1: 62ba41cf38e55fad1de2e4679787cae91a0dcdd2 */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febdc-5c34-7e66-8fb7-6a1d3120395b/from_3407_to_3323.update.rpf.hdiff", 52320836 } /* diff sha1: d93db1a27c4fd90f33505e97bc1f3f0a3ff9b19a */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febdc-a37d-7ea9-b419-103e48246deb/from_3570_to_3323.update.rpf.hdiff", 60991977 } /* diff sha1: 48b0cef83104fa9bd55103890d836a56af2d22a4 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febdd-291e-7764-a094-fc7e8c7fdae6/from_3751_to_3323.update.rpf.hdiff", 62355396 } /* diff sha1: d88fbaa4d01962a60855a5232d90fbf3dbc6a5b6 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febdd-291e-7764-a094-fc7e8c7fdae6/from_3751_to_3323.update.rpf.hdiff", 62355396 } /* diff sha1: d88fbaa4d01962a60855a5232d90fbf3dbc6a5b6 */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "3633a58585791b30281cec14f90f5ac1e2bfdc57", "https://downloads.cfx-services.net/prod/019febdd-a5bc-7f29-a2c4-2ce785f74b46/from_3889_to_3323.update.rpf.hdiff", 62355278 } /* diff sha1: b3754c5f2ccd2c74de49960273f0cb78b35b5745 */
					},
				},
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://downloads.cfx-services.net/prod/019febde-cd87-74f0-a5f0-113f674ae206/update2.rpf", 416063488,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://downloads.cfx-services.net/prod/019febd9-f7b8-75c6-b3c4-cebc468ee0bd/from_2612_to_3323.update2.rpf.hdiff", 345380507 } /* diff sha1: b1c69025dbd7bdf2233613f74def7d094b342d79 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://downloads.cfx-services.net/prod/019febda-7428-733f-b1a7-d9abbe65fcf6/from_2699_to_3323.update2.rpf.hdiff", 343671960 } /* diff sha1: 7cdbeb852cb14b00c3d13da7c0c61b73a9cd7a04 */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://downloads.cfx-services.net/prod/019febda-e91e-7dbd-8740-4c78ae6749d3/from_2802_to_3323.update2.rpf.hdiff", 343371839 } /* diff sha1: d663b68258da5662dcbd1cd2a281cbc13cc4b357 */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://downloads.cfx-services.net/prod/019febdb-68ea-7370-a04a-401713fbb0c1/from_2944_to_3323.update2.rpf.hdiff", 343112220 } /* diff sha1: 196689f39a02ddfd8a89c599c0c2a0e3aa9dd64d */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://downloads.cfx-services.net/prod/019febdb-ad41-7b8a-8daa-742e5172fe4f/from_3095_to_3323.update2.rpf.hdiff", 340833331 } /* diff sha1: af00ec37780727a25e02b555c4904b525241a2c4 */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://downloads.cfx-services.net/prod/019febdb-edf4-7ace-bf98-886d47c14cdd/from_3179_to_3323.update2.rpf.hdiff", 340835870 } /* diff sha1: d2006d94b6edeb0b495baf1f3c841340d25086e7 */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://downloads.cfx-services.net/prod/019febdc-24ff-7bad-b282-4798c1c7eff2/from_3258_to_3323.update2.rpf.hdiff", 327858356 } /* diff sha1: 82bd2cf214f88f25c01ab31b7a0237203ddfbcda */,
						{ "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://downloads.cfx-services.net/prod/019febdc-6995-707e-8542-34cf6f5d68f7/from_3407_to_3323.update2.rpf.hdiff", 340440056 } /* diff sha1: 1f88689b5f532721bb0048b541d0cf8a184134bc */,
						{ "28095c30975ffaead56ff72dfb6418d19eef38dc", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://downloads.cfx-services.net/prod/019febdc-b229-76de-b984-8bd7f03a2a15/from_3570_to_3323.update2.rpf.hdiff", 341928123 } /* diff sha1: 2be5d12b696a533cd96dc46536245debaed72d3b */,
						{ "e67aad3699a50988dd03d201110346e38ebf5225", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://downloads.cfx-services.net/prod/019febdd-3aab-7882-a140-1e76345bf43e/from_3751_to_3323.update2.rpf.hdiff", 341581134 } /* diff sha1: 36fde5a36e1454f450ada8cd878117eed8d6e7ae */,
						{ "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://downloads.cfx-services.net/prod/019febdd-7286-792e-848b-75ce271d4ba0/from_3788_to_3323.update2.rpf.hdiff", 341581140 } /* diff sha1: 6507e366962b85f419cacbcd0ba2745c7656990d */,
						{ "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "https://downloads.cfx-services.net/prod/019febdd-b7de-78eb-98df-2e76837484d8/from_3889_to_3323.update2.rpf.hdiff", 341581510 } /* diff sha1: 6babc0b352e0e9636913ee78062e31c4bdc2e191 */
					},
				},
			},
		}
	},
	{
		3258,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "17183412df26a019386ffd5097df697d9041bb3d", "https://downloads.cfx-services.net/prod/019febcf-c89e-77fc-8023-0650f664de4d/GTA5.exe", 56066032 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd6-45b3-748d-a84c-baa43c75160a/update.rpf", 1423288320,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febcf-dd02-7f31-a14e-b12d4d021670/from_1604_to_3258.update.rpf.hdiff", 932235213 } /* diff sha1: 0c56b47728ae7f3b7b3ed19fcd940a9de9e0336d */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd0-6b8c-7095-bafc-ed2d493315c2/from_2060_to_3258.update.rpf.hdiff", 704311288 } /* diff sha1: 104f4370c10d294e6aad8d7a11d0ac740945d050 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd0-da2b-73c4-884a-141f3802e529/from_2189_to_3258.update.rpf.hdiff", 663014595 } /* diff sha1: a86a8943032ef74edf65ef6eaa68148b16ab1c7e */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd1-3f57-786b-b14f-ccc1c4752b5e/from_2372_to_3258.update.rpf.hdiff", 646437117 } /* diff sha1: 7e4b9115d91c33b396b75ee897595e87f5571f90 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd1-9ed1-7ca6-9fed-836aa663fbe4/from_2545_to_3258.update.rpf.hdiff", 507688763 } /* diff sha1: 92ad3f81f9888bdf5fb3390dc1cfcfa33016bdbc */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd1-f1cd-75ac-b21c-776270187ee1/from_2612_to_3258.update.rpf.hdiff", 507688310 } /* diff sha1: ddc065ad6845b0f0eca85d124016dbdd32599c62 */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd2-76b4-7203-aba9-6d47e0aa8db1/from_2699_to_3258.update.rpf.hdiff", 462960589 } /* diff sha1: eef31a26c45de0f63a6a35b0d9d3215d59e4c4a3 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd2-f8bb-7d4e-a22c-fdb8c3e618d2/from_2802_to_3258.update.rpf.hdiff", 406279730 } /* diff sha1: 7a8216f59fe7fd73409768f273050b1797316087 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd3-7213-7dce-bccf-4926e09fc196/from_2944_to_3258.update.rpf.hdiff", 393108418 } /* diff sha1: d5a358eb14bb6ceb0f0cd93a318418b61b225580 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd3-e145-734d-bb37-724a9b01d4c1/from_3095_to_3258.update.rpf.hdiff", 63780343 } /* diff sha1: 7390c645e320807503b25aa86642daa6835c8a09 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd3-e145-734d-bb37-724a9b01d4c1/from_3095_to_3258.update.rpf.hdiff", 63780343 } /* diff sha1: 7390c645e320807503b25aa86642daa6835c8a09 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd4-6091-7079-9e99-ee59c58f6d55/from_3323_to_3258.update.rpf.hdiff", 232538 } /* diff sha1: 0a8f2f2cedf997b849c571bf9fd7ab0165e81faf */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd4-9755-7759-b204-ec4c47fc49c4/from_3407_to_3258.update.rpf.hdiff", 52340312 } /* diff sha1: 5dc9e22f2ec8517a085db8084021ebfef6bab3d7 */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd4-ded0-7562-8a5f-714d6be01b59/from_3570_to_3258.update.rpf.hdiff", 61011471 } /* diff sha1: 4c1419e5403e211e43348c278d58d6a78fd75795 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd5-6954-7bbe-a870-9e9044bb5e38/from_3751_to_3258.update.rpf.hdiff", 62377432 } /* diff sha1: 7e7d6181e459c7ef3c3f3b5681ac9a6e738ddf09 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd5-6954-7bbe-a870-9e9044bb5e38/from_3751_to_3258.update.rpf.hdiff", 62377432 } /* diff sha1: 7e7d6181e459c7ef3c3f3b5681ac9a6e738ddf09 */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "https://downloads.cfx-services.net/prod/019febd5-ec23-7a4b-a196-cdcafdeada03/from_3889_to_3258.update.rpf.hdiff", 62377290 } /* diff sha1: 046264b9c2291dd8644995b71c2ad49c5450f4ed */
					},
				},
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://downloads.cfx-services.net/prod/019febd7-1ec2-7259-99df-4f79f5351461/update2.rpf", 416053248,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://downloads.cfx-services.net/prod/019febd2-3db5-70b2-946a-8da94150ce5a/from_2612_to_3258.update2.rpf.hdiff", 345380170 } /* diff sha1: 2556e1bbfc0685e721e872d5c7f26e6b07fcfaf1 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://downloads.cfx-services.net/prod/019febd2-c092-7778-9be7-6dbfa2d33208/from_2699_to_3258.update2.rpf.hdiff", 343680669 } /* diff sha1: e0ff2108c7201f6fa3a4a52a55ac5bd5209fe1f3 */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://downloads.cfx-services.net/prod/019febd3-3a91-79ff-82fa-0cd28e68300b/from_2802_to_3258.update2.rpf.hdiff", 343376273 } /* diff sha1: 4fe9189dcbedbfda93ad7926139ea7e85fee3bcf */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://downloads.cfx-services.net/prod/019febd3-ac31-7498-88c2-5a25816e28d2/from_2944_to_3258.update2.rpf.hdiff", 343101974 } /* diff sha1: adf4ddaa6eb0e12fded154e8a89e028f8e4ed1a5 */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://downloads.cfx-services.net/prod/019febd3-efb4-71cf-8771-a1d4e9663121/from_3095_to_3258.update2.rpf.hdiff", 340810631 } /* diff sha1: 93af9830292c7380b2ebce4b399f077203089d65 */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://downloads.cfx-services.net/prod/019febd4-27f1-7609-ad13-db3331fb2baa/from_3179_to_3258.update2.rpf.hdiff", 340801650 } /* diff sha1: 27c126578d8860ae1fa3329df776083b67d39517 */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://downloads.cfx-services.net/prod/019febd4-65db-7bff-9336-910fa8682e81/from_3323_to_3258.update2.rpf.hdiff", 327846154 } /* diff sha1: 24e881657fec709df192f3ed4832cfa377eb3d44 */,
						{ "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://downloads.cfx-services.net/prod/019febd4-a4fd-732a-b397-d6727361cfd4/from_3407_to_3258.update2.rpf.hdiff", 340422150 } /* diff sha1: 85134cecbd7c954d8da937650f18dd6c7fc37776 */,
						{ "28095c30975ffaead56ff72dfb6418d19eef38dc", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://downloads.cfx-services.net/prod/019febd4-ed68-70e2-90ba-7d30505d940a/from_3570_to_3258.update2.rpf.hdiff", 341925886 } /* diff sha1: dc4c1593a4520bd39cad1e9f725b6cae5d1ea4e4 */,
						{ "e67aad3699a50988dd03d201110346e38ebf5225", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://downloads.cfx-services.net/prod/019febd5-7932-7120-8a0e-51e2e4d07781/from_3751_to_3258.update2.rpf.hdiff", 341570891 } /* diff sha1: 5d68a3a81dfc1e7ec09854d9d28efcd5dc1f4d2c */,
						{ "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://downloads.cfx-services.net/prod/019febd5-b059-7b74-9e38-29aa9f1a8093/from_3788_to_3258.update2.rpf.hdiff", 341570882 } /* diff sha1: 0a94a28b8cf5a61474a97b8d1a0b3fdff428e7e3 */,
						{ "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "https://downloads.cfx-services.net/prod/019febd5-ff87-7ca2-b498-885cec1f4a31/from_3889_to_3258.update2.rpf.hdiff", 341571264 } /* diff sha1: ca04ea11031e9eb9ed24894cfb8b8926bc347e2d */
					},
				},
			},
		}
	},
	{
		3179,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "cf2b853ae2125a26e636daa99f6377b05baaad8a", "https://downloads.cfx-services.net/prod/019febcc-665f-7cf9-b13f-940a3a3c2d7c/GTA5.exe", 55367152 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febcb-4516-79e2-b2f8-42bcb97ec0d0/update.rpf", 1416300544,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc4-f209-749f-95db-f8d0a1cb7ad3/from_1604_to_3095.update.rpf.hdiff", 923117905 } /* diff sha1: 1b22013721291d5bd65872bd9d179fc01a695fa3 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc5-813d-7629-bb79-c551d03bf65a/from_2060_to_3095.update.rpf.hdiff", 694772098 } /* diff sha1: e3e7071607195ebda498e945b07bc174caba90a5 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc5-e935-7b82-9f57-7782ef98bfd1/from_2189_to_3095.update.rpf.hdiff", 653422887 } /* diff sha1: 7b546f88560c86405cf43c7b4c5bc5c9d0bb4b50 */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc6-4b9a-7b6d-8faa-8a7f632b2a6e/from_2372_to_3095.update.rpf.hdiff", 636414267 } /* diff sha1: c04b08faa8eabbf161f4b7b27d44c114738d668c */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc6-ae87-7906-bece-bb91369638e5/from_2545_to_3095.update.rpf.hdiff", 496735908 } /* diff sha1: 4123b2c63d466ee53fb3b8f8e9306bdf44d2d08d */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc6-f802-7696-aef3-8a74528e8828/from_2612_to_3095.update.rpf.hdiff", 496733750 } /* diff sha1: 53ee1f7b28816eed1f9899b7d09a6547ee9fbbb9 */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc7-86f7-74e6-bb9d-50799233e868/from_2699_to_3095.update.rpf.hdiff", 452905479 } /* diff sha1: 0932832b87a22c963165da8d9cf16b8faede1cd2 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc8-023d-780d-acd9-ebc5c6b3ea78/from_2802_to_3095.update.rpf.hdiff", 386841388 } /* diff sha1: c11d4796c020236b173cb01eb99cade83d049319 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc8-7ffd-738d-b7b8-d0f9de1baef1/from_2944_to_3095.update.rpf.hdiff", 341954506 } /* diff sha1: 62e7153e824e585739105e3610f16b2518356e64 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc9-159b-750d-9f27-8c7347c19436/from_3258_to_3095.update.rpf.hdiff", 56823148 } /* diff sha1: 5b581fa05751c05ec24fd92da7c3644d4a234b74 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc9-66c7-74b1-b9e0-506762d9c303/from_3323_to_3095.update.rpf.hdiff", 56828311 } /* diff sha1: a3979a12f02688bb65b68a9d86c4d3c5e1990372 */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc9-a749-7aad-acdc-d818f0b71422/from_3407_to_3095.update.rpf.hdiff", 58856138 } /* diff sha1: 888c43f4588a63880f8a405c86dfff74f356ca18 */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc9-e5fa-7280-8d00-d6047a44b9d0/from_3570_to_3095.update.rpf.hdiff", 62532668 } /* diff sha1: 5b29bd24dfa6c2d2edbeb28b79f0d37c44f8e2f9 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febca-6f31-7e13-9fea-d3cb08c7ca28/from_3751_to_3095.update.rpf.hdiff", 63322610 } /* diff sha1: 45bef07abe6e6f4e10dab59d255a7cd4b2f3df4b */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febca-6f31-7e13-9fea-d3cb08c7ca28/from_3751_to_3095.update.rpf.hdiff", 63322610 } /* diff sha1: 45bef07abe6e6f4e10dab59d255a7cd4b2f3df4b */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febca-f277-7106-810a-05c130f2384a/from_3889_to_3095.update.rpf.hdiff", 63370104 } /* diff sha1: 1caa1ae407e0e2d058cb2bb56191af3c8620585b */
					},
				},
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://downloads.cfx-services.net/prod/019febcf-8cbf-72b1-bdc5-912ca67fc97a/update2.rpf", 403941376,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://downloads.cfx-services.net/prod/019febcc-9b82-7335-b79c-1d56bc2eda00/from_2612_to_3179.update2.rpf.hdiff", 333197023 } /* diff sha1: b3954312af1c0a580f547364a4dd73514cd15828 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://downloads.cfx-services.net/prod/019febcc-d5a0-73c8-8d9e-f7f0aca12a20/from_2699_to_3179.update2.rpf.hdiff", 331500185 } /* diff sha1: 63f69d3d5f75784f13d4b7cac1544385f111dc1e */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://downloads.cfx-services.net/prod/019febcd-0c4d-7b72-be3a-5abcb9318045/from_2802_to_3179.update2.rpf.hdiff", 331176908 } /* diff sha1: f7e02f4b87d5ccbfc577817c004d5e82b28f6884 */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://downloads.cfx-services.net/prod/019febcd-476d-75a8-8012-d05338a44a92/from_2944_to_3179.update2.rpf.hdiff", 330898163 } /* diff sha1: 451e7b646e9f0b7a62e23d6fae193cc09cdc564c */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://downloads.cfx-services.net/prod/019febcd-7c27-71ca-a79f-c906c12e196e/from_3095_to_3179.update2.rpf.hdiff", 313464391 } /* diff sha1: 191fe3dde2ab0d31118769a65488c6561cfd6cf0 */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://downloads.cfx-services.net/prod/019febcd-ae74-7e9b-81a0-e5334ec21443/from_3258_to_3179.update2.rpf.hdiff", 328689523 } /* diff sha1: fba401a95e81cbda8f88a861fb747f2774fad663 */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://downloads.cfx-services.net/prod/019febcd-ea5d-7404-b00b-627c44338e4f/from_3323_to_3179.update2.rpf.hdiff", 328713490 } /* diff sha1: 0d9ef440d1f9991f0ade93fe7f9b0b1d1eeb12b7 */,
						{ "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://downloads.cfx-services.net/prod/019febce-2104-78d9-a82f-40cf54274402/from_3407_to_3179.update2.rpf.hdiff", 328830163 } /* diff sha1: 9beb88d01a54b6639b16150a635f50fc8019f056 */,
						{ "28095c30975ffaead56ff72dfb6418d19eef38dc", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://downloads.cfx-services.net/prod/019febce-64a8-73f8-9e13-c9099962ce6f/from_3570_to_3179.update2.rpf.hdiff", 330186263 } /* diff sha1: 15bf0af7ff3bea712df361763fdef18e3bb834e3 */,
						{ "e67aad3699a50988dd03d201110346e38ebf5225", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://downloads.cfx-services.net/prod/019febce-d662-7bec-8d26-08f597b5a9e4/from_3751_to_3179.update2.rpf.hdiff", 329846475 } /* diff sha1: 2fdfb2807e3c3c555fe1e50326bec581edf293d6 */,
						{ "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://downloads.cfx-services.net/prod/019febcf-0b53-7043-8d87-77c2cdb21b6f/from_3788_to_3179.update2.rpf.hdiff", 329844981 } /* diff sha1: 5eb0980d35ce88055f56ca2a230aefe636533933 */,
						{ "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "cfea3456309664bf8762e3bce5957211f3ee8b39", "https://downloads.cfx-services.net/prod/019febcf-4233-746f-8b18-9b7e32981697/from_3889_to_3179.update2.rpf.hdiff", 329846859 } /* diff sha1: c362175f56ec437c6b56d018e6070484ed36e8e4 */
					},
				},
			},
		}
	},
	{
		3095,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "17a074bb8eaca5bd8df863de84869a4ab023e1eb", "https://downloads.cfx-services.net/prod/019febc4-de6b-717f-916a-542abc053db5/GTA5.exe", 49634800 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febcb-4516-79e2-b2f8-42bcb97ec0d0/update.rpf", 1416300544,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc4-f209-749f-95db-f8d0a1cb7ad3/from_1604_to_3095.update.rpf.hdiff", 923117905 } /* diff sha1: 1b22013721291d5bd65872bd9d179fc01a695fa3 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc5-813d-7629-bb79-c551d03bf65a/from_2060_to_3095.update.rpf.hdiff", 694772098 } /* diff sha1: e3e7071607195ebda498e945b07bc174caba90a5 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc5-e935-7b82-9f57-7782ef98bfd1/from_2189_to_3095.update.rpf.hdiff", 653422887 } /* diff sha1: 7b546f88560c86405cf43c7b4c5bc5c9d0bb4b50 */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc6-4b9a-7b6d-8faa-8a7f632b2a6e/from_2372_to_3095.update.rpf.hdiff", 636414267 } /* diff sha1: c04b08faa8eabbf161f4b7b27d44c114738d668c */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc6-ae87-7906-bece-bb91369638e5/from_2545_to_3095.update.rpf.hdiff", 496735908 } /* diff sha1: 4123b2c63d466ee53fb3b8f8e9306bdf44d2d08d */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc6-f802-7696-aef3-8a74528e8828/from_2612_to_3095.update.rpf.hdiff", 496733750 } /* diff sha1: 53ee1f7b28816eed1f9899b7d09a6547ee9fbbb9 */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc7-86f7-74e6-bb9d-50799233e868/from_2699_to_3095.update.rpf.hdiff", 452905479 } /* diff sha1: 0932832b87a22c963165da8d9cf16b8faede1cd2 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc8-023d-780d-acd9-ebc5c6b3ea78/from_2802_to_3095.update.rpf.hdiff", 386841388 } /* diff sha1: c11d4796c020236b173cb01eb99cade83d049319 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc8-7ffd-738d-b7b8-d0f9de1baef1/from_2944_to_3095.update.rpf.hdiff", 341954506 } /* diff sha1: 62e7153e824e585739105e3610f16b2518356e64 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc9-159b-750d-9f27-8c7347c19436/from_3258_to_3095.update.rpf.hdiff", 56823148 } /* diff sha1: 5b581fa05751c05ec24fd92da7c3644d4a234b74 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc9-66c7-74b1-b9e0-506762d9c303/from_3323_to_3095.update.rpf.hdiff", 56828311 } /* diff sha1: a3979a12f02688bb65b68a9d86c4d3c5e1990372 */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc9-a749-7aad-acdc-d818f0b71422/from_3407_to_3095.update.rpf.hdiff", 58856138 } /* diff sha1: 888c43f4588a63880f8a405c86dfff74f356ca18 */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febc9-e5fa-7280-8d00-d6047a44b9d0/from_3570_to_3095.update.rpf.hdiff", 62532668 } /* diff sha1: 5b29bd24dfa6c2d2edbeb28b79f0d37c44f8e2f9 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febca-6f31-7e13-9fea-d3cb08c7ca28/from_3751_to_3095.update.rpf.hdiff", 63322610 } /* diff sha1: 45bef07abe6e6f4e10dab59d255a7cd4b2f3df4b */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febca-6f31-7e13-9fea-d3cb08c7ca28/from_3751_to_3095.update.rpf.hdiff", 63322610 } /* diff sha1: 45bef07abe6e6f4e10dab59d255a7cd4b2f3df4b */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "fd46de4495d32f0533b8b3ae72507b829e8650f3", "https://downloads.cfx-services.net/prod/019febca-f277-7106-810a-05c130f2384a/from_3889_to_3095.update.rpf.hdiff", 63370104 } /* diff sha1: 1caa1ae407e0e2d058cb2bb56191af3c8620585b */
					},
				},
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://downloads.cfx-services.net/prod/019febcc-2988-7560-a301-2e6c5584f296/update2.rpf", 403945472,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://downloads.cfx-services.net/prod/019febc7-4efa-7d12-98a0-0c311cddfb92/from_2612_to_3095.update2.rpf.hdiff", 333194048 } /* diff sha1: 7e3c826eb759abaa35b68c1be25fa6e673ad9802 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://downloads.cfx-services.net/prod/019febc7-cbba-7307-8acd-ea67497461dc/from_2699_to_3095.update2.rpf.hdiff", 331503702 } /* diff sha1: fc111e310931d769d8dfce514d1b567f261ddf40 */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://downloads.cfx-services.net/prod/019febc8-4453-7bdf-8bdb-98b42e254e9e/from_2802_to_3095.update2.rpf.hdiff", 331190538 } /* diff sha1: 11683ea488d00b1e3e9bfb733fa9f032cee9b4b0 */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://downloads.cfx-services.net/prod/019febc8-b572-731d-a692-5819adee04bc/from_2944_to_3095.update2.rpf.hdiff", 330911852 } /* diff sha1: b018bd07b4d4f3ff3670cd0523cdf2d9ef9466fb */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://downloads.cfx-services.net/prod/019febc8-e7bd-7bc7-8867-1269d62cc52d/from_3179_to_3095.update2.rpf.hdiff", 313461829 } /* diff sha1: c0beaa52ddda5bd084ad1512cdbef79f4749ec31 */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://downloads.cfx-services.net/prod/019febc9-268b-7c2f-a5ad-a2356d48a0b0/from_3258_to_3095.update2.rpf.hdiff", 328700911 } /* diff sha1: 31048725810d4bf433da58c984577c51a03c9412 */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://downloads.cfx-services.net/prod/019febc9-76e7-7567-8193-49c13cac7c8b/from_3323_to_3095.update2.rpf.hdiff", 328715458 } /* diff sha1: a9a916d7ae704a2d1b162c5cbe88273ae64d272f */,
						{ "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://downloads.cfx-services.net/prod/019febc9-b775-79c2-9739-cf58cfbc047e/from_3407_to_3095.update2.rpf.hdiff", 328834461 } /* diff sha1: e80010fffe99cdbc8a6f772fb5df3f31d9b024e3 */,
						{ "28095c30975ffaead56ff72dfb6418d19eef38dc", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://downloads.cfx-services.net/prod/019febc9-f7c9-702c-af42-2bef4f06a259/from_3570_to_3095.update2.rpf.hdiff", 330190382 } /* diff sha1: aa87ea4c9730a2405f4ec0c5ae9409301dfe726c */,
						{ "e67aad3699a50988dd03d201110346e38ebf5225", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://downloads.cfx-services.net/prod/019febca-8303-7998-8d1b-459871bffbcd/from_3751_to_3095.update2.rpf.hdiff", 329850583 } /* diff sha1: 57a58e681842ef5d6da39a0f944c0e748833f3b8 */,
						{ "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://downloads.cfx-services.net/prod/019febca-c1fe-735f-bf09-e8d26eff2109/from_3788_to_3095.update2.rpf.hdiff", 329850582 } /* diff sha1: 555339cca02594ae6e9fb007ff5d8b35298c0a5d */,
						{ "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "https://downloads.cfx-services.net/prod/019febcb-00c6-7e35-8c99-e358e93e8d68/from_3889_to_3095.update2.rpf.hdiff", 329850939 } /* diff sha1: fce53d085d9bd305d708c8ac8dd73bc8111ea243 */
					},
				},
			},
		}
	},
	{
		2944,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "4d968a0754d59d30b29cd7b01a06e4685a5fa49c", "https://downloads.cfx-services.net/prod/019febbf-c741-7e45-9106-d7b547d73b13/GTA5.exe", 49828848 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc4-13e1-71ea-9568-9a3bb25d861a/update.rpf", 1087019008,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febbf-d841-7409-a1a9-d9ea37b15f62/from_1604_to_2944.update.rpf.hdiff", 596587088 } /* diff sha1: 3de54ae4dc5a3d27a4e5621c7690d19f0f6fc432 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc0-30d2-7bef-a181-a67ebecefe17/from_2060_to_2944.update.rpf.hdiff", 368280541 } /* diff sha1: 258f4ee6773872199f664e9c55f8243b0298c48d */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc0-6e3a-715a-ad00-c14452b0ce42/from_2189_to_2944.update.rpf.hdiff", 326614727 } /* diff sha1: 1bea466fcccfc2b5275995b85786afc3a952a6e3 */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc0-a97d-70c3-8b07-85d95cc87f5a/from_2372_to_2944.update.rpf.hdiff", 307384637 } /* diff sha1: 92d41ca77ab60a2de950da2f206a8469415f829b */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc0-d956-7db5-a67a-95b4315194c3/from_2545_to_2944.update.rpf.hdiff", 166552358 } /* diff sha1: a20129a7d79d56dc57e477adcf69583f30188d88 */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc0-f934-77b7-a692-9454fbf4c54f/from_2612_to_2944.update.rpf.hdiff", 166551505 } /* diff sha1: c65fad96304ee0205dd9efee508e04b53d50d895 */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc1-4f8d-7bf4-bb5d-d250d8970ec8/from_2699_to_2944.update.rpf.hdiff", 121830364 } /* diff sha1: 3e3b0549f13e0debbc37cfab480a2b902316c199 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc1-927b-7348-a258-2d4d1db007d3/from_2802_to_2944.update.rpf.hdiff", 56081281 } /* diff sha1: eb75888e835e3ef58aa736cbb36747194b0eeea6 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc1-cccb-7e92-9979-e1b6bbee15fe/from_3095_to_2944.update.rpf.hdiff", 15617655 } /* diff sha1: 5b98a4a7ff869b724d477d03b5b4bd7037468b74 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc1-cccb-7e92-9979-e1b6bbee15fe/from_3095_to_2944.update.rpf.hdiff", 15617655 } /* diff sha1: 5b98a4a7ff869b724d477d03b5b4bd7037468b74 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc2-2fb5-78f7-ae84-2c81fe184dc5/from_3258_to_2944.update.rpf.hdiff", 59814458 } /* diff sha1: 6877cf0cac02ee1ce085e500cb9a3530617aa63d */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc2-69e3-73f3-b8e8-a6d205d9db2c/from_3323_to_2944.update.rpf.hdiff", 59814456 } /* diff sha1: e6767ca11d311070115de3ffb64c0841a1a7770e */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc2-a9f2-73d2-9077-479d4b8f69f4/from_3407_to_2944.update.rpf.hdiff", 61283961 } /* diff sha1: 45c360e0da0f8bf9bc782a30480405f30c36f4b9 */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc2-e3a3-768b-8d1b-db45b7fafa95/from_3570_to_2944.update.rpf.hdiff", 64517668 } /* diff sha1: 687efe95a79426d04e9fefee5f3de4341070cd4c */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc3-6079-74e2-b8a4-b6af67728c33/from_3751_to_2944.update.rpf.hdiff", 64847231 } /* diff sha1: 13a9aa0d7aaafe897153d063c4bff1abc28c4450 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc3-6079-74e2-b8a4-b6af67728c33/from_3751_to_2944.update.rpf.hdiff", 64847231 } /* diff sha1: 13a9aa0d7aaafe897153d063c4bff1abc28c4450 */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "https://downloads.cfx-services.net/prod/019febc3-cae8-77d7-b00f-5e8fee458abe/from_3889_to_2944.update.rpf.hdiff", 64885368 } /* diff sha1: cf6a3a486592fc174c5762dcf7f30aaf365142c6 */
					},
				},
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "a3181d68a532950da5c584100b35f79eaca7c884", "https://downloads.cfx-services.net/prod/019febc4-aaf3-78ca-81d0-ec17a2809413/update2.rpf", 352088064,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "a3181d68a532950da5c584100b35f79eaca7c884", "https://downloads.cfx-services.net/prod/019febc1-1b7c-7198-bc20-b3dfc573d6e9/from_2612_to_2944.update2.rpf.hdiff", 280860829 } /* diff sha1: 1ec47c39f3c5a504e7ca7d89221eea17335fa555 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "a3181d68a532950da5c584100b35f79eaca7c884", "https://downloads.cfx-services.net/prod/019febc1-674d-785c-9018-5b8c84bdde8c/from_2699_to_2944.update2.rpf.hdiff", 277845689 } /* diff sha1: 00c967b55a0d8c299bc265f0979541b3eb49bb8f */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "a3181d68a532950da5c584100b35f79eaca7c884", "https://downloads.cfx-services.net/prod/019febc1-a20f-7c19-bd3c-b6fbf8520e99/from_2802_to_2944.update2.rpf.hdiff", 277310322 } /* diff sha1: 677514fc14030cdb8645b2b14bae383ba3a841df */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "a3181d68a532950da5c584100b35f79eaca7c884", "https://downloads.cfx-services.net/prod/019febc1-d582-738b-90a8-0deda033d5e6/from_3095_to_2944.update2.rpf.hdiff", 279052198 } /* diff sha1: a9155d4a892c21f9e1589c1b04ea7c53618e4d3e */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "a3181d68a532950da5c584100b35f79eaca7c884", "https://downloads.cfx-services.net/prod/019febc2-0202-7eef-9ae2-76e6d7e9522d/from_3179_to_2944.update2.rpf.hdiff", 279043729 } /* diff sha1: b445466b21226c260c76c18b43000e922f763148 */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "a3181d68a532950da5c584100b35f79eaca7c884", "https://downloads.cfx-services.net/prod/019febc2-3e93-74bf-96ac-46ade222d347/from_3258_to_2944.update2.rpf.hdiff", 279135120 } /* diff sha1: 4f5cc49e5c8a2bb825efcee77c5090bf842987ed */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "a3181d68a532950da5c584100b35f79eaca7c884", "https://downloads.cfx-services.net/prod/019febc2-7ac6-7d2d-8c98-4375fd05a4bd/from_3323_to_2944.update2.rpf.hdiff", 279135010 } /* diff sha1: 063bbc27788d848246ec183f46e93e0a9d89af30 */,
						{ "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "a3181d68a532950da5c584100b35f79eaca7c884", "https://downloads.cfx-services.net/prod/019febc2-ba13-7124-88a6-2641ef3182be/from_3407_to_2944.update2.rpf.hdiff", 279136123 } /* diff sha1: da11233b54ec1924a73504e9763bf1a272a50b5d */,
						{ "28095c30975ffaead56ff72dfb6418d19eef38dc", "a3181d68a532950da5c584100b35f79eaca7c884", "https://downloads.cfx-services.net/prod/019febc2-f4aa-7f2b-90ff-6166251be4ad/from_3570_to_2944.update2.rpf.hdiff", 279581616 } /* diff sha1: b916d5abfede87b084aac0db14ae7f7b2672b5e9 */,
						{ "e67aad3699a50988dd03d201110346e38ebf5225", "a3181d68a532950da5c584100b35f79eaca7c884", "https://downloads.cfx-services.net/prod/019febc3-6e7e-73b3-888c-fe9191cdb0ce/from_3751_to_2944.update2.rpf.hdiff", 279190411 } /* diff sha1: 55a76eea71fc935d802ff53c245984dd1758b4a4 */,
						{ "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "a3181d68a532950da5c584100b35f79eaca7c884", "https://downloads.cfx-services.net/prod/019febc3-9fea-77cd-ade8-de65f8ba4d85/from_3788_to_2944.update2.rpf.hdiff", 279190414 } /* diff sha1: b07bd6d9e0f563dd8ed3b3fc3b0ea557222dc65b */,
						{ "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "a3181d68a532950da5c584100b35f79eaca7c884", "https://downloads.cfx-services.net/prod/019febc3-dd12-7652-89b7-9a73ea03d28c/from_3889_to_2944.update2.rpf.hdiff", 279190748 } /* diff sha1: b1aa242fb76b6e3be0b7843cfc3439e7ad5d8346 */
					},
				},
			},
		}
	},
	{
		2802,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "ebb6c144c5befe3529235deccbd8f59d6ce1a76c", "https://downloads.cfx-services.net/prod/019febba-62f7-713d-894f-1b675f5741c3/GTA5.exe", 46709592 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febbe-e9b8-7966-97b1-2d5a8bc653d6/update.rpf", 1079308288,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febba-737f-7bf1-9da5-854a9ba5360d/from_1604_to_2802.update.rpf.hdiff", 586455637 } /* diff sha1: 5acad3e7e9c44e87f886c8b554395b24a749fa21 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febba-d8fe-778f-9490-bde1be333cc2/from_2060_to_2802.update.rpf.hdiff", 356615804 } /* diff sha1: cdd476afd0846a50821c5afde019f4e69cec4f9c */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febbb-1bab-78cb-a24b-33cf4f1454d1/from_2189_to_2802.update.rpf.hdiff", 314177287 } /* diff sha1: be29acd02d02a7b18f8d1829e00a9d9e225b24ea */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febbb-5386-7c3b-b0d9-0c69ba8d669f/from_2372_to_2802.update.rpf.hdiff", 294843255 } /* diff sha1: be53e3444cec1fe69b8b2d8afe8e509397d47ff3 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febbb-8dfa-7728-a2d2-e8c467bb395d/from_2545_to_2802.update.rpf.hdiff", 153796192 } /* diff sha1: d6e8425fab4b55118bcc3fbd94dfa6c70b17bf82 */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febbb-b019-7bff-9d1c-f66b9a147972/from_2612_to_2802.update.rpf.hdiff", 153795013 } /* diff sha1: 823f2df6a209a6a1dd5d06edcf74402868baedfc */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febbb-feea-7b25-9e0a-152f15217b2a/from_2699_to_2802.update.rpf.hdiff", 107750632 } /* diff sha1: ef20902617372028b4caac304b42d38a5d3eec8d */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febbc-4536-70bf-aa2c-6f6dd36e2fde/from_2944_to_2802.update.rpf.hdiff", 48422944 } /* diff sha1: 8baed7a1e37c6cc37be64e6ba5ebae246c2087dc */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febbc-81b6-7b60-a0fa-9141260cb5e5/from_3095_to_2802.update.rpf.hdiff", 52843554 } /* diff sha1: 19a5a38d4cc02ec5b086dcd8999cd83bed81e145 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febbc-81b6-7b60-a0fa-9141260cb5e5/from_3095_to_2802.update.rpf.hdiff", 52843554 } /* diff sha1: 19a5a38d4cc02ec5b086dcd8999cd83bed81e145 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febbc-f3dd-7830-a8d6-c8dbd8d29984/from_3258_to_2802.update.rpf.hdiff", 65327489 } /* diff sha1: 9dcfaf73965e139e6420b3049205fdc792654167 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febbd-36da-7d7f-8d5b-d34a05bd505b/from_3323_to_2802.update.rpf.hdiff", 65327479 } /* diff sha1: 36b634accceba01c1f42e227ded30ed40d9bfd23 */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febbd-77b3-70d9-9194-1693184a72c4/from_3407_to_2802.update.rpf.hdiff", 66636258 } /* diff sha1: b15cb1871bbd077d01559603d1b1e29aba3be90f */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febbd-b751-7ea0-b0a8-23c2a22f1cd1/from_3570_to_2802.update.rpf.hdiff", 66909639 } /* diff sha1: 076df60c0ecf01f7bf5e9bbc4fd1d032ad6a78b7 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febbe-37c1-745c-be6c-a39ab818749b/from_3751_to_2802.update.rpf.hdiff", 67298012 } /* diff sha1: a0e2f861f0d598ec34ca373485b3ae1ca95e5db5 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febbe-37c1-745c-be6c-a39ab818749b/from_3751_to_2802.update.rpf.hdiff", 67298012 } /* diff sha1: a0e2f861f0d598ec34ca373485b3ae1ca95e5db5 */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "66388a381347511b7b28aaf91741615e45008e8b", "https://downloads.cfx-services.net/prod/019febbe-a617-7a44-a03c-be507c8ae3d6/from_3889_to_2802.update.rpf.hdiff", 67297568 } /* diff sha1: cbee0566959e9130ea0c45bb53dc2d664f62b927 */
					},
				},
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://downloads.cfx-services.net/prod/019febbf-8dc0-7683-92dd-cb8ce200e8fd/update2.rpf", 344610816,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://downloads.cfx-services.net/prod/019febbb-cfcf-7c81-bf07-1438f37019aa/from_2612_to_2802.update2.rpf.hdiff", 273248338 } /* diff sha1: 0f392e33b0270e1cdabbff1a30e21e172db2e592 */,
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://downloads.cfx-services.net/prod/019febbc-180b-78af-85f2-d266c88af45b/from_2699_to_2802.update2.rpf.hdiff", 270218937 } /* diff sha1: 54bb6ba1f80437fd8a9742c25d1a1b5075d93f9f */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://downloads.cfx-services.net/prod/019febbc-544f-748f-887a-47f1e5e77f5d/from_2944_to_2802.update2.rpf.hdiff", 269824187 } /* diff sha1: fc8d8fc5d8eac3cd48d6c7b03c4b6356816d2324 */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://downloads.cfx-services.net/prod/019febbc-927f-792b-a2a4-06c5f5c6facc/from_3095_to_2802.update2.rpf.hdiff", 271850052 } /* diff sha1: e4115d62fdd9f23df93d5daa718431a24883976a */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://downloads.cfx-services.net/prod/019febbc-c2e7-7a23-8096-2bcb852f1abe/from_3179_to_2802.update2.rpf.hdiff", 271841345 } /* diff sha1: 08bca2abc165d9f44f4ab969b0d5118eef8882f2 */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://downloads.cfx-services.net/prod/019febbd-06cc-78c8-a5b7-91fc392eb1ef/from_3258_to_2802.update2.rpf.hdiff", 271934438 } /* diff sha1: ab1440c9acd248d1219b1fee619ba496c9db8635 */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://downloads.cfx-services.net/prod/019febbd-4974-7f28-ad0d-4e048cc46e48/from_3323_to_2802.update2.rpf.hdiff", 271926188 } /* diff sha1: 82177a7cf2f395ba599d81bac3023e78a9a6c830 */,
						{ "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://downloads.cfx-services.net/prod/019febbd-8a01-7bbe-9662-3ef015c88f25/from_3407_to_2802.update2.rpf.hdiff", 271939214 } /* diff sha1: cfec1f3ea06183161491b210670efba2206d5fa5 */,
						{ "28095c30975ffaead56ff72dfb6418d19eef38dc", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://downloads.cfx-services.net/prod/019febbd-c676-7741-a713-8aae19bb84fa/from_3570_to_2802.update2.rpf.hdiff", 272401850 } /* diff sha1: fda8061b6d044e590ca9d90d66737206f7d2eddf */,
						{ "e67aad3699a50988dd03d201110346e38ebf5225", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://downloads.cfx-services.net/prod/019febbe-4913-752e-a862-dc2046c5593a/from_3751_to_2802.update2.rpf.hdiff", 271977853 } /* diff sha1: 6d9086c2ec1bd6150c51e8a7a07ab20712186865 */,
						{ "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://downloads.cfx-services.net/prod/019febbe-7eb1-74ab-a65c-f25311eb17ea/from_3788_to_2802.update2.rpf.hdiff", 271977851 } /* diff sha1: a3b9c34047937bd4663d3b9cd5b2d75b8036eb7a */,
						{ "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://downloads.cfx-services.net/prod/019febbe-b7fe-70fb-bf4c-efb08ad5672d/from_3889_to_2802.update2.rpf.hdiff", 271978153 } /* diff sha1: f2cdf475b09530847acef907ac5356ee6777eee2 */
					},
				},
			},
		}
	},
	{
		2699,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "b9f3960ca0c7c05aab23d3b1d158309bc085fbbe", "https://downloads.cfx-services.net/prod/019febb4-a298-7d39-bc2d-e7acbf66e991/GTA5.exe", 61111680 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb9-6c70-702a-9ab4-eae1c1a64712/update.rpf", 1073854464,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb4-b767-7d6f-9496-82f67a8057f0/from_1604_to_2699.update.rpf.hdiff", 577779656 } /* diff sha1: 00cd98e09e02b24a3dbfb52c272e3e78e945fc4e */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb5-1fb9-72ea-9251-61052be26218/from_2060_to_2699.update.rpf.hdiff", 346805883 } /* diff sha1: 180cc0d3498dba3fb09118a670b2185ca8d324e6 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb5-60bd-7d6d-bd42-2dcd0f38f83c/from_2189_to_2699.update.rpf.hdiff", 304782654 } /* diff sha1: 1f33549326258481701e1c909e768d60cfdf6c9c */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb5-98e9-71f4-bf99-01f98b14f8a8/from_2372_to_2699.update.rpf.hdiff", 285467525 } /* diff sha1: e36f07ae02019e2f313d8f089a61a48b3545f556 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb5-c9b8-7535-88f6-8678c2526c96/from_2545_to_2699.update.rpf.hdiff", 144813901 } /* diff sha1: 997cba235f0925e686220200d7a6758b83784e73 */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb5-e720-7804-b079-244306e8379d/from_2612_to_2699.update.rpf.hdiff", 144812091 } /* diff sha1: e5636168e11f31204b6aa7bc26a2f29a15df2ea6 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb6-2fd5-7e69-8f9d-30aa2242405e/from_2802_to_2699.update.rpf.hdiff", 102324935 } /* diff sha1: 9e52c963dd498af0abe560241673e2e9963d917f */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb6-7beb-78ab-8bdb-51ef3eb6931b/from_2944_to_2699.update.rpf.hdiff", 108745603 } /* diff sha1: 590e601cbc82f332f7619b78dbb93a3d72d7c741 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb6-bf5f-7e5a-a93c-1a13f1c7cbfe/from_3095_to_2699.update.rpf.hdiff", 113480643 } /* diff sha1: f57949a6c14e71cfa119d7aacb128796c8fcd2e3 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb6-bf5f-7e5a-a93c-1a13f1c7cbfe/from_3095_to_2699.update.rpf.hdiff", 113480643 } /* diff sha1: f57949a6c14e71cfa119d7aacb128796c8fcd2e3 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb7-37ac-7f05-95fe-f899050f0636/from_3258_to_2699.update.rpf.hdiff", 116581930 } /* diff sha1: ade215b3b763433132d0636948ae0af442a5fdfc */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb7-7ee8-70f4-a577-7e7cbba6d7e2/from_3323_to_2699.update.rpf.hdiff", 116581929 } /* diff sha1: c6cd8ce18160bbc2f10a3bf8753f78772b740a9f */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb7-c77a-7ce0-9d85-f9e505e534ac/from_3407_to_2699.update.rpf.hdiff", 117825124 } /* diff sha1: e9c7233f506d7ada6fe127c4f538c246b2cd4181 */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb8-13b4-7561-bfea-478aa18c6ae0/from_3570_to_2699.update.rpf.hdiff", 118068195 } /* diff sha1: de55a1cc50e6b47b683c28322bff40cfce602aaa */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb8-a1b5-79a2-932f-921a42d25b6d/from_3751_to_2699.update.rpf.hdiff", 118291533 } /* diff sha1: c3b2c59022f00ac3e2d0de7dfefc3eca5934f8de */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb8-a1b5-79a2-932f-921a42d25b6d/from_3751_to_2699.update.rpf.hdiff", 118291533 } /* diff sha1: c3b2c59022f00ac3e2d0de7dfefc3eca5934f8de */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://downloads.cfx-services.net/prod/019febb9-1bff-7914-9215-657f63aafe27/from_3889_to_2699.update.rpf.hdiff", 118330125 } /* diff sha1: 5d798c4bad2659a2cc835445e2ca637ae7a414ef */
					},
				},
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://downloads.cfx-services.net/prod/019febba-23b3-70a4-a973-8eed357b3187/update2.rpf", 324530176,
					{
						{ "c993e2d14cce9462fa8ba056f3406d60050a1c92", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://downloads.cfx-services.net/prod/019febb6-045e-7c2b-bc60-9336d9f23ccd/from_2612_to_2699.update2.rpf.hdiff", 252956098 } /* diff sha1: 6a8df16ec61595298907a9204272567a4c9e1a2f */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://downloads.cfx-services.net/prod/019febb6-4bec-76ac-90d0-000aa97f2576/from_2802_to_2699.update2.rpf.hdiff", 250133852 } /* diff sha1: 547a19410dd0f7cb91816b028405cde27b4caa9a */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://downloads.cfx-services.net/prod/019febb6-9275-706e-8495-a9b272a7feee/from_2944_to_2699.update2.rpf.hdiff", 250283500 } /* diff sha1: 76f859c0f2bc7b7c38d1235ee70fc2baf50f6f98 */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://downloads.cfx-services.net/prod/019febb6-d940-78f2-ad53-8fb32b5d9950/from_3095_to_2699.update2.rpf.hdiff", 252084865 } /* diff sha1: b574885cb096adfbf15bb776eac029bc5423add9 */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://downloads.cfx-services.net/prod/019febb7-093f-7242-afa4-97c8a3d2e63e/from_3179_to_2699.update2.rpf.hdiff", 252085404 } /* diff sha1: 91ee0b5cbd819ae914fb61b2f47768593c2e5add */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://downloads.cfx-services.net/prod/019febb7-51d5-7671-b6ac-7cffd984f9bf/from_3258_to_2699.update2.rpf.hdiff", 252153440 } /* diff sha1: 1ab962c656573f190634897a3209b49fc35c9e05 */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://downloads.cfx-services.net/prod/019febb7-9989-7c4e-8a90-9a1f160e99dd/from_3323_to_2699.update2.rpf.hdiff", 252145327 } /* diff sha1: 17ec0c0f36846a6da8fec744a46379d73640c922 */,
						{ "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://downloads.cfx-services.net/prod/019febb7-dfbb-78b4-b2d5-7ceab1917bc8/from_3407_to_2699.update2.rpf.hdiff", 252158158 } /* diff sha1: 8c234441726c7c434907ad4b38fc7ac031379f40 */,
						{ "28095c30975ffaead56ff72dfb6418d19eef38dc", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://downloads.cfx-services.net/prod/019febb8-2e4b-771a-b8ba-c02458a4d15e/from_3570_to_2699.update2.rpf.hdiff", 252609115 } /* diff sha1: 281a29db0f8efd02c1c5ade6e16be12d024bbfbf */,
						{ "e67aad3699a50988dd03d201110346e38ebf5225", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://downloads.cfx-services.net/prod/019febb8-bb4b-74ee-b1c4-b417f7f4a148/from_3751_to_2699.update2.rpf.hdiff", 252195942 } /* diff sha1: ba863f41e6b1e55d3a05ab64d9b38a94b0b5353f */,
						{ "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://downloads.cfx-services.net/prod/019febb8-ecf3-7a0a-893e-9d0af7648949/from_3788_to_2699.update2.rpf.hdiff", 252195954 } /* diff sha1: 1ebbf147cb4116b2c5c19a3717bef6248c748b7c */,
						{ "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://downloads.cfx-services.net/prod/019febb9-38fd-7bf0-9a13-35856f104964/from_3889_to_2699.update2.rpf.hdiff", 252196380 } /* diff sha1: 2041ad9513e47a9101cf6a1638c6cc03fe2085f8 */
					},
				},
			},
		}
	},
	{
		2612,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "d423086fd7a7721b8be77cfb9a4f8826784b284b", "https://downloads.cfx-services.net/prod/019febaf-1bda-70fc-b257-9bbe6f2eb1f3/GTA5.exe", 60351952 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febb3-b3ee-7e33-b240-67e22b150b3d/update.rpf", 1056649216,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febaf-2d98-7a4b-bfc0-b54c420029d2/from_1604_to_2612.update.rpf.hdiff", 560353786 } /* diff sha1: 71780b7e6701c9efe313c8252345ab88df4f3734 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febaf-8c48-77d2-8780-41d5af85c010/from_2060_to_2612.update.rpf.hdiff", 330036916 } /* diff sha1: 9b67a7840d9c33a4354b168fb580876d1281d30a */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febaf-c7fa-7d16-83f7-759a11cd23a6/from_2189_to_2612.update.rpf.hdiff", 287033107 } /* diff sha1: 939717a2b3b3b4b25512b0eeb354cf557abc928a */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febaf-fb1a-7936-84aa-e259f4645f3e/from_2372_to_2612.update.rpf.hdiff", 266987849 } /* diff sha1: b1d38bb2ee75d5989201cfb58edcf2f841224c3d */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febb0-30a6-7bdb-9416-7551141b9d7d/from_2545_to_2612.update.rpf.hdiff", 1840945 } /* diff sha1: 35c6b1e1edb8a73e8156bf6d19f3d5aaee5c01a2 */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febb0-35cb-7c5e-b578-94dcacef904e/from_2699_to_2612.update.rpf.hdiff", 127701528 } /* diff sha1: fa76dab95d4b3a24c9670b1636bcb1fc38030364 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febb0-7cd8-733c-9293-b9d06a097094/from_2802_to_2612.update.rpf.hdiff", 131292485 } /* diff sha1: 586b0f78de285133a3234cea71ae87a29174b825 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febb0-c5a7-7e7d-82fa-91510074820f/from_2944_to_2612.update.rpf.hdiff", 136386970 } /* diff sha1: 69a7f2dfa365deee03f273cd45edfef63887d414 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febb1-0c9e-7caa-bedc-4e4acdfe9f7f/from_3095_to_2612.update.rpf.hdiff", 140238453 } /* diff sha1: ec4226c26fe0ab951a064dafc8953d2cf7cd9071 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febb1-0c9e-7caa-bedc-4e4acdfe9f7f/from_3095_to_2612.update.rpf.hdiff", 140238453 } /* diff sha1: ec4226c26fe0ab951a064dafc8953d2cf7cd9071 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febb1-8118-7960-8ef5-cc3e233c919e/from_3258_to_2612.update.rpf.hdiff", 144229149 } /* diff sha1: 13ff3b3e6c80627fed2039877f245e05574c1ffc */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febb1-c988-72e1-98b4-b6cff3a43e2a/from_3323_to_2612.update.rpf.hdiff", 144229138 } /* diff sha1: ccf193169dd665ef4105911fb370d96260a142ab */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febb2-1228-7a94-b2d6-06938ee5a4ff/from_3407_to_2612.update.rpf.hdiff", 145561398 } /* diff sha1: f1e075150bb219126533b6d1097af78543254802 */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febb2-5a56-70e7-bd91-91aba8794191/from_3570_to_2612.update.rpf.hdiff", 145733983 } /* diff sha1: 2f7d2241e291c51f1216e2c87328818604d7428a */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febb2-eda1-7dec-9ca9-230d2b3e7859/from_3751_to_2612.update.rpf.hdiff", 146033459 } /* diff sha1: 80308693f8f248a6d2b6aa4ad89f6dd4fc60318d */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febb2-eda1-7dec-9ca9-230d2b3e7859/from_3751_to_2612.update.rpf.hdiff", 146033459 } /* diff sha1: 80308693f8f248a6d2b6aa4ad89f6dd4fc60318d */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "80f9bd028e5bc781f641fe210a88579eff827989", "https://downloads.cfx-services.net/prod/019febb3-648b-75a5-80d6-dc84a55f1bee/from_3889_to_2612.update.rpf.hdiff", 146033047 } /* diff sha1: fc41080a0d7ef450de352794dc3b0659a7a23505 */
					},
				},
			},
			{
				"update2.rpf",
				{
					"update/update2.rpf", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://downloads.cfx-services.net/prod/019febb4-67bf-72c8-b80b-70898a88b36f/update2.rpf", 312209408,
					{
						{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://downloads.cfx-services.net/prod/019febb0-521f-76a7-bc0f-d016eb68cdcc/from_2699_to_2612.update2.rpf.hdiff", 240637812 } /* diff sha1: 847791743b223286abf5cee5ab7f773c895c95c6 */,
						{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://downloads.cfx-services.net/prod/019febb0-994a-7e5f-b16b-19437e3a236e/from_2802_to_2612.update2.rpf.hdiff", 240847147 } /* diff sha1: 34671a51615c1255de4095d47137d81da26a4adf */,
						{ "a3181d68a532950da5c584100b35f79eaca7c884", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://downloads.cfx-services.net/prod/019febb0-e13f-71c5-a053-e2768c2bfc82/from_2944_to_2612.update2.rpf.hdiff", 240982514 } /* diff sha1: 6bfe4bce142e697d83a039918f99eaf5d4ec3938 */,
						{ "1c785e7b5cfe8331aad335b3f78952bc66b9fcb6", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://downloads.cfx-services.net/prod/019febb1-2777-7a66-964d-ae74b8cebe21/from_3095_to_2612.update2.rpf.hdiff", 241447960 } /* diff sha1: 4ea67e0f9fadbe27c3a8b9c7aace9fdfd7f1fc4c */,
						{ "cfea3456309664bf8762e3bce5957211f3ee8b39", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://downloads.cfx-services.net/prod/019febb1-5680-7113-9701-31a6164c3044/from_3179_to_2612.update2.rpf.hdiff", 241446618 } /* diff sha1: 886224aa434a960e471de884fa884f12fc09c998 */,
						{ "6e7c1bdd4b0b8d47ac28bef19d2644f3240ad248", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://downloads.cfx-services.net/prod/019febb1-9efc-7eec-a115-e623a335b178/from_3258_to_2612.update2.rpf.hdiff", 241517010 } /* diff sha1: 44d6248a1b8865c763334db3ffc53a01ef29f5d1 */,
						{ "f2007adb84a8fc9e4481d58faf88ffca3af754c5", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://downloads.cfx-services.net/prod/019febb1-e8b5-71bc-a5f1-75d835daca51/from_3323_to_2612.update2.rpf.hdiff", 241507622 } /* diff sha1: e50895e7fec427a21102ba1aa4fe5175300e2652 */,
						{ "b379e1752718ea1d799194b8aa70b58b28cd8c5f", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://downloads.cfx-services.net/prod/019febb2-2ffa-7d54-8c55-134eb4b22b3f/from_3407_to_2612.update2.rpf.hdiff", 241528234 } /* diff sha1: 883af320813e26fe4b0139327cc16a197828597d */,
						{ "28095c30975ffaead56ff72dfb6418d19eef38dc", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://downloads.cfx-services.net/prod/019febb2-79a6-7493-80f6-07376ab5c262/from_3570_to_2612.update2.rpf.hdiff", 241971051 } /* diff sha1: 135778e8df16fd241bc1957e85020182f3a38884 */,
						{ "e67aad3699a50988dd03d201110346e38ebf5225", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://downloads.cfx-services.net/prod/019febb3-099a-700e-a390-170b3f49c0af/from_3751_to_2612.update2.rpf.hdiff", 241556042 } /* diff sha1: 763fd47174e0a190bca324adfab0254f5c030d6b */,
						{ "995aecbc44438ed75ae4048c4e365a937eb6c4c1", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://downloads.cfx-services.net/prod/019febb3-3650-72f7-a6ec-1160e826364f/from_3788_to_2612.update2.rpf.hdiff", 241556045 } /* diff sha1: 3ab445405457e4557fe65da4bc99429a14492fa7 */,
						{ "c58c1df3e3562ca4396c9eb2913f453dc6da36af", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://downloads.cfx-services.net/prod/019febb3-828c-7eb2-afe2-30cd0030dd58/from_3889_to_2612.update2.rpf.hdiff", 241556484 } /* diff sha1: 6d12cc2ff67c5736ea6bf800ac36c4879fb80e1c */
					},
				},
			},
		}
	},
	{
		2545,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "517556bb548880362c18d502361ce374070994c2", "https://downloads.cfx-services.net/prod/019feba8-9d5a-76a1-a1e2-5000237c89ff/GTA5.exe", 59988376 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019febae-3598-7c53-ba89-e140ca528a98/update.rpf", 1366638592,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019feba8-b0e0-744a-ae09-c5ec2091c997/from_1604_to_2545.update.rpf.hdiff", 804698938 } /* diff sha1: be441ec10c867d1a7e5af0bbdbfa5163ec4d620c */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019feba9-3fe5-7bc5-bd59-000a7fcfb901/from_2060_to_2545.update.rpf.hdiff", 573692039 } /* diff sha1: a5c4796b5373c2a9a2273d8a344e27d80f6dcc69 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019feba9-aa41-7569-8863-7fcfc04f9ec4/from_2189_to_2545.update.rpf.hdiff", 529825735 } /* diff sha1: 7436ea395e4de45d215a1c7098c48bd10d2364dd */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019febaa-074d-7e2d-a3bd-99e0c43a7fe4/from_2372_to_2545.update.rpf.hdiff", 509055770 } /* diff sha1: 84c9a2bfcd86d994f5c9a8fa5087c9f4dc0d7c6d */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019febaa-668f-7514-bbfb-593f54650f7b/from_2612_to_2545.update.rpf.hdiff", 311774185 } /* diff sha1: 621c16a1d9a3be46c2663dde02bd58f1716c5445 */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019febaa-a0ef-7a20-806b-6ca294bb0d69/from_2699_to_2545.update.rpf.hdiff", 437636032 } /* diff sha1: 1d52798a772db28de7020738ea6aef9b45112eb0 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019febaa-eeed-7d13-9986-7838bb092341/from_2802_to_2545.update.rpf.hdiff", 441227939 } /* diff sha1: 23df563de60020ec4edb6192ab4925ae67af2ae6 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019febab-3f53-7702-adec-0043120bb9fc/from_2944_to_2545.update.rpf.hdiff", 446324800 } /* diff sha1: 0eef4d6c08abd9834bf306ac4e172033bac0f694 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019febab-8d90-7400-bdd7-cdb1f0bac4c8/from_3095_to_2545.update.rpf.hdiff", 450172651 } /* diff sha1: 483f1b9d493818afe9fe32d01aa23d265b2f5cd7 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019febab-8d90-7400-bdd7-cdb1f0bac4c8/from_3095_to_2545.update.rpf.hdiff", 450172651 } /* diff sha1: 483f1b9d493818afe9fe32d01aa23d265b2f5cd7 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019febab-e1a0-7c88-9279-741fb9fd0045/from_3258_to_2545.update.rpf.hdiff", 454165325 } /* diff sha1: 63f68a67be0eb06b7ec00cdda882ef07e6d57b28 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019febac-36bf-7903-b55b-0dfb76c9794b/from_3323_to_2545.update.rpf.hdiff", 454165319 } /* diff sha1: 3f501ce62c7a3856406101a2a35ee2e133fa618a */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019febac-874d-7bc6-a875-cfef9a459b4f/from_3407_to_2545.update.rpf.hdiff", 455494428 } /* diff sha1: 182b01aea1eddf8277e2847e83fdeea79d4abb08 */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019febac-db14-71b1-91ab-9d63e83e5f55/from_3570_to_2545.update.rpf.hdiff", 455668062 } /* diff sha1: d2e00cbc299de083c72dd2e8ddcd3dd61772c3bd */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019febad-83fe-7d1a-9f95-99bec19de27c/from_3751_to_2545.update.rpf.hdiff", 455969203 } /* diff sha1: fd8cc84ba3ef52ce21d8b1d699b3f4d996aa4389 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019febad-83fe-7d1a-9f95-99bec19de27c/from_3751_to_2545.update.rpf.hdiff", 455969203 } /* diff sha1: fd8cc84ba3ef52ce21d8b1d699b3f4d996aa4389 */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://downloads.cfx-services.net/prod/019febad-dafc-78d7-8cb8-802aa5f6914e/from_3889_to_2545.update.rpf.hdiff", 455968681 } /* diff sha1: e7a087ed097da28759d4a3edc6c560f5f10201c9 */
					},
				},
			},
		}
	},
	{
		2372,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "470235e04299b02aa3aef834ef1ff834cac2327f", "https://downloads.cfx-services.net/prod/019feba3-acb7-7bd9-8745-8acaa64037ce/GTA5.exe", 59716912 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba7-e136-78ae-b343-6816f0bc3a4d/update.rpf", 1132066816,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba3-bf3a-74e9-bb47-a3eaf897a349/from_1604_to_2372.update.rpf.hdiff", 562253582 } /* diff sha1: 49c8a15a9314e71b3931a2c7602f8cd9688fcbd2 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba4-2237-785d-bc0d-45f91a2b48e8/from_2060_to_2372.update.rpf.hdiff", 329760934 } /* diff sha1: f3f6b96d648583e4427392ccf1df36cb1d435a3b */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba4-5e39-71f4-a206-fa0909033898/from_2189_to_2372.update.rpf.hdiff", 284412077 } /* diff sha1: 5870dc5116a7039476da7d714939ac1ec2e5f010 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba4-8feb-7020-8d70-1b2861e9b053/from_2545_to_2372.update.rpf.hdiff", 276106385 } /* diff sha1: 81b60c9be8f749c7bdd2b4dc6d80bda3913045c0 */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba4-c726-789b-99d2-f9cc3b6a4d66/from_2612_to_2372.update.rpf.hdiff", 343976392 } /* diff sha1: 6d00a65b77838cab4c2cc64cd4fd3eb69f75725d */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba5-028c-7b7f-bb29-4724d3161c93/from_2699_to_2372.update.rpf.hdiff", 345348050 } /* diff sha1: cd5650fc698852250ab39855ad43c3edb59b5c05 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba5-437d-7d57-b445-48e4b0fcff7c/from_2802_to_2372.update.rpf.hdiff", 349288020 } /* diff sha1: 2b83c01cbcfa112e62882159b7178527b3aaffe5 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba5-832c-7545-a046-39c40f6f9603/from_2944_to_2372.update.rpf.hdiff", 354164718 } /* diff sha1: 6699a31eb6be4b331d52383322079139aa2e8968 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba5-c4e2-77b3-b70a-02ecc35ffe16/from_3095_to_2372.update.rpf.hdiff", 356862631 } /* diff sha1: 6aa88c4943d6e36071b9ede4bd9b9652b5f76d14 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba5-c4e2-77b3-b70a-02ecc35ffe16/from_3095_to_2372.update.rpf.hdiff", 356862631 } /* diff sha1: 6aa88c4943d6e36071b9ede4bd9b9652b5f76d14 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba6-07b9-756e-825a-21aea26381be/from_3258_to_2372.update.rpf.hdiff", 359924636 } /* diff sha1: ecd71f547ba9d45088c61cb80c0981bf173db964 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba6-47ce-7770-9f61-965c7f47b01f/from_3323_to_2372.update.rpf.hdiff", 359924634 } /* diff sha1: b8f623b99c598fcaaf7f669741c29dee8c629346 */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba6-8d6f-75cd-a9f8-36bbbf891d24/from_3407_to_2372.update.rpf.hdiff", 360756556 } /* diff sha1: f86f62cb0123fa5e03dce616baa13496d55afdf3 */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba6-d1de-7335-83e1-7b75bc6a2e53/from_3570_to_2372.update.rpf.hdiff", 360941691 } /* diff sha1: 99db2ed3e7d40352e6d341c62ff03f558f8f0b14 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba7-57e2-7151-8821-db6eb09893c6/from_3751_to_2372.update.rpf.hdiff", 361097173 } /* diff sha1: a0712f7271e62f1bbf1a20d6e9c173b80956776b */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba7-57e2-7151-8821-db6eb09893c6/from_3751_to_2372.update.rpf.hdiff", 361097173 } /* diff sha1: a0712f7271e62f1bbf1a20d6e9c173b80956776b */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "1824cdbc27c3e0eaa86920a38751322727872831", "https://downloads.cfx-services.net/prod/019feba7-991c-7748-8a27-7aad77b6177b/from_3889_to_2372.update.rpf.hdiff", 361096506 } /* diff sha1: 1aeba6a347a1877297023b6676fdbf9023b742ae */
					},
				},
			},
		}
	},
	{
		2189,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "fcd5fd8a9f99f2e08b0cab5d500740f28a75b75a", "https://downloads.cfx-services.net/prod/019feb9d-812b-7586-be3f-1053202e2923/GTA5.exe", 63124096 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feba2-db5e-7c54-81f3-5ddff206b418/update.rpf", 1276805120,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feb9d-95b5-7a20-9af3-bafca6086427/from_1604_to_2189.update.rpf.hdiff", 562451691 } /* diff sha1: 16be09623e52097da007a166fba80921d7ce5863 */,
						{ "736f1cb26e59167f302c22385463d231cce302d3", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feb9d-f886-7884-a6ae-a5e1debe84bd/from_2060_to_2189.update.rpf.hdiff", 296656003 } /* diff sha1: d9e50da86b802e382b65bfa53a28648a92fe79d8 */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feb9e-2fc1-7bbd-9730-a71ee7e63a61/from_2372_to_2189.update.rpf.hdiff", 429153146 } /* diff sha1: 608466df488ba54851190d99d0bdc20160ff6815 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feb9e-79cf-77e7-9cc8-06e41038b41c/from_2545_to_2189.update.rpf.hdiff", 441617306 } /* diff sha1: 3428f7544fd574acbb95c46b2889400f21e260b8 */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feb9e-d27f-79bd-af83-aee5abb09fb6/from_2612_to_2189.update.rpf.hdiff", 508757790 } /* diff sha1: 9b640d294575a5ddcf680c8aab0a019d3b33229d */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feb9f-2ad5-70be-b86a-b24f78030d23/from_2699_to_2189.update.rpf.hdiff", 509396786 } /* diff sha1: bc5037ea0e7e80de9bae1d4d903d769b1bf0179b */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feb9f-84e2-716c-be73-98dbae19bbc3/from_2802_to_2189.update.rpf.hdiff", 513356064 } /* diff sha1: 0dbfc1422fe085d4654311358cf47204ae8360b9 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feb9f-e3df-76c8-9418-a8bdfa3ab8aa/from_2944_to_2189.update.rpf.hdiff", 518134363 } /* diff sha1: 23cb0ec2ad7aed1f58bd64ce37c1f7c1d7e590ad */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feba0-4372-76e8-9ebe-a159aa5e8dc9/from_3095_to_2189.update.rpf.hdiff", 518604133 } /* diff sha1: 73b7e835643658012708e6c280eb24c354fa2b0b */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feba0-4372-76e8-9ebe-a159aa5e8dc9/from_3095_to_2189.update.rpf.hdiff", 518604133 } /* diff sha1: 73b7e835643658012708e6c280eb24c354fa2b0b */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feb90-ea2f-7429-9d9d-82db556f65c7/from_3258_to_2189.update.rpf.hdiff", 521239366 } /* diff sha1: ab61a067d7afaab16e4769def84d6c0588bb7bd4 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feba0-a3cd-7ba5-aa8c-297ee229a4a7/from_3323_to_2189.update.rpf.hdiff", 521239331 } /* diff sha1: 2875855cfc9d2aebd7aaeac7536a2548c8136bca */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feba1-01fb-73b9-a56e-5821ece17ee9/from_3407_to_2189.update.rpf.hdiff", 522046015 } /* diff sha1: aafcccc2747d58e468e4b00b2fca2db9c4225411 */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feba1-5cb9-76b7-8331-b55b32032065/from_3570_to_2189.update.rpf.hdiff", 522170835 } /* diff sha1: 5c21932f630ad903d8e28f156af21d3e61b6769d */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feba2-16f6-71c0-ba44-171d5bb616c1/from_3751_to_2189.update.rpf.hdiff", 521487114 } /* diff sha1: adfaf3e2ab4ed35216611f4a64f23de435dd5ff4 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feba2-16f6-71c0-ba44-171d5bb616c1/from_3751_to_2189.update.rpf.hdiff", 521487114 } /* diff sha1: adfaf3e2ab4ed35216611f4a64f23de435dd5ff4 */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://downloads.cfx-services.net/prod/019feba2-768f-7585-b415-1f781f8df300/from_3889_to_2189.update.rpf.hdiff", 521492140 } /* diff sha1: 61e20c53fd7c7cf6592e9302692f5b346c9bcc9c */
					},
				},
			},
		}
	},
	{
		2060,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "741c8b91ef57140c023d8d29e38aab599759de76", "https://downloads.cfx-services.net/prod/019feb97-1d25-7e31-bb99-9466be1ddeca/GTA5.exe", 60589184 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb9c-b1a8-79b3-b808-6402241ee1f5/update.rpf", 1229002752,
					{
						{ "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb97-2fda-757d-98e9-73c8c3272bcb/from_1604_to_2060.update.rpf.hdiff", 512193764 } /* diff sha1: 67030f9da5c2d9be1e3187e3efcec1c02c8b2af4 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb97-8764-70e6-b452-6c86145beab5/from_2189_to_2060.update.rpf.hdiff", 249363428 } /* diff sha1: 7168f459a2f2a75e3fd253f6c1b5fe1cfa93e6db */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb97-bac0-7160-bfc7-af460685f0fe/from_2372_to_2060.update.rpf.hdiff", 427205591 } /* diff sha1: 49344c636938b690173344c2e8210d8b12bbc296 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb98-062c-7679-b00e-cb3a181f2ddd/from_2545_to_2060.update.rpf.hdiff", 438194552 } /* diff sha1: afe305848d46ac47c56a7c7d7e0f3089734bffe5 */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb98-57ee-7a2a-8155-e363871ad90f/from_2612_to_2060.update.rpf.hdiff", 504469096 } /* diff sha1: a00b8440d3135267f4097932d34d043cc970d54f */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb98-ad1b-7478-b010-5315de691eaf/from_2699_to_2060.update.rpf.hdiff", 504126319 } /* diff sha1: 4c86479be22672bc3497c6da2a4a6151a568d122 */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb99-0670-7b34-a482-51cdb2e40d40/from_2802_to_2060.update.rpf.hdiff", 508501688 } /* diff sha1: 02d1f7276d1f93fcc204d7b7e1e31db7058a33ca */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb99-6433-7810-b473-10da42fb91cf/from_2944_to_2060.update.rpf.hdiff", 512509365 } /* diff sha1: d69b4ba86899e0692217d4173ba1ba68f6816908 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb99-c01c-7ee2-a8f0-b6844d061d60/from_3095_to_2060.update.rpf.hdiff", 512663221 } /* diff sha1: 8fccc8eecc4e44533eac1b8be577dc4260224ec2 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb99-c01c-7ee2-a8f0-b6844d061d60/from_3095_to_2060.update.rpf.hdiff", 512663221 } /* diff sha1: 8fccc8eecc4e44533eac1b8be577dc4260224ec2 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb9a-20fa-7583-a141-7e18d078cdbd/from_3258_to_2060.update.rpf.hdiff", 515246812 } /* diff sha1: 535f49a489f7c3618a0174f514c017851bb4f7fd */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb9a-7c08-76a8-83f2-1b9770189e2b/from_3323_to_2060.update.rpf.hdiff", 515246794 } /* diff sha1: bfcf008b95313770edc03662a16d30b7bb245f26 */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb9a-d516-7226-9fe7-233eccf005dc/from_3407_to_2060.update.rpf.hdiff", 516145143 } /* diff sha1: 64b91a5a8d6cef3364f5d2d1d26036f351d403a9 */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb9b-34d6-726e-bb48-378dbb25cc64/from_3570_to_2060.update.rpf.hdiff", 516214365 } /* diff sha1: ce7630d66372d701af6dcc610f4e3b793b316495 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb9b-f175-79da-833e-ef9aff2447ce/from_3751_to_2060.update.rpf.hdiff", 516323686 } /* diff sha1: d2c274c424e36d17f30d02245c32673cead39a6a */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb9b-f175-79da-833e-ef9aff2447ce/from_3751_to_2060.update.rpf.hdiff", 516323686 } /* diff sha1: d2c274c424e36d17f30d02245c32673cead39a6a */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "736f1cb26e59167f302c22385463d231cce302d3", "https://downloads.cfx-services.net/prod/019feb9c-5125-74a2-bd70-ff6a08d0ba1b/from_3889_to_2060.update.rpf.hdiff", 516323421 } /* diff sha1: b286fb0b686393c818268c1b66a7907737bf1acb */
					},
				},
			},
		}
	},
	{
		1604,
		{
			{
				"GTA5.exe",
				{ "GTA5.exe", "8939c8c71aa98ad7ca6ac773fae1463763c420d8", "https://downloads.cfx-services.net/prod/019feb91-45f3-7e09-91a7-5b1241012bea/GTA5.exe", 72484280 },
			},
			{
				"update.rpf",
				{
					"update/update.rpf", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb96-7f71-7447-aaad-fba55eaa97b7/update.rpf", 966805504,
					{
						{ "736f1cb26e59167f302c22385463d231cce302d3", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb91-5a0d-7d45-bf1f-348d2ae70694/from_2060_to_1604.update.rpf.hdiff", 252578178 } /* diff sha1: a6b119525e18bb37cbf65b23d51317cd47177188 */,
						{ "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb91-89f9-7ce7-8cc7-2c3f5366ca7f/from_2189_to_1604.update.rpf.hdiff", 255544048 } /* diff sha1: 29d31e6a9d964662a490682b07ecd3ac7f776913 */,
						{ "1824cdbc27c3e0eaa86920a38751322727872831", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb91-b8af-784d-a70c-9ce7614cd6e9/from_2372_to_1604.update.rpf.hdiff", 400087270 } /* diff sha1: ca3064d9bb8108b626e5e1b7c747c29cca597587 */,
						{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb91-fffc-755c-aa77-cd7dd0c66f2f/from_2545_to_1604.update.rpf.hdiff", 409505316 } /* diff sha1: 92998e2e6d977815f0a73137275119f7dda2a68c */,
						{ "80f9bd028e5bc781f641fe210a88579eff827989", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb92-4f35-7c26-aaf0-84e5174dfca2/from_2612_to_1604.update.rpf.hdiff", 475094324 } /* diff sha1: 1982eb4530c60c9cdaef64c719d8383cd75954ee */,
						{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb92-a409-7d82-be5b-3fc74af87514/from_2699_to_1604.update.rpf.hdiff", 475411202 } /* diff sha1: 574d2f5f091ce6e59036fb74cf5ad785252e10af */,
						{ "66388a381347511b7b28aaf91741615e45008e8b", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb92-f7e2-7ab8-9e9e-cd76d2615f71/from_2802_to_1604.update.rpf.hdiff", 478650625 } /* diff sha1: 8f8ad96b2939ddf43c9c014af3592a396330f4d4 */,
						{ "abc628b0ae04e68f88e0581f3572d26dbaed84d2", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb93-4ff9-707e-8a95-c0cef947ae7c/from_2944_to_1604.update.rpf.hdiff", 481127813 } /* diff sha1: ed3677dc68d2c1db95a90b3fd184ef8ed1c82d0c */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb93-a8d4-72c4-9c10-dd49d6a95846/from_3095_to_1604.update.rpf.hdiff", 481316317 } /* diff sha1: 4cdcfc8e677169799861f5a97247bc7152d75768 */,
						{ "fd46de4495d32f0533b8b3ae72507b829e8650f3", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb93-a8d4-72c4-9c10-dd49d6a95846/from_3095_to_1604.update.rpf.hdiff", 481316317 } /* diff sha1: 4cdcfc8e677169799861f5a97247bc7152d75768 */,
						{ "abf3a580ddfc4cb372b5a4ce48ed7b2ea31e5270", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb94-00ca-7bf4-a2d0-eb3f07243f8e/from_3258_to_1604.update.rpf.hdiff", 483477046 } /* diff sha1: dc84f0bd06326be309bd4b409c354903690ac196 */,
						{ "3633a58585791b30281cec14f90f5ac1e2bfdc57", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb94-58f3-70fb-9250-caaca70ae236/from_3323_to_1604.update.rpf.hdiff", 483477045 } /* diff sha1: 4a826e8ab5034b13835c5e303cce9b2f9a718b77 */,
						{ "f6cdcdec5e3e993a31f45acc96b638283c474f53", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb94-ad39-772d-bf93-8a0f66cf634f/from_3407_to_1604.update.rpf.hdiff", 484276041 } /* diff sha1: fa5c1ba46ab12fb57aad511f20bc712518a20c0f */,
						{ "49ed7a6c3d035bcf764942dd58597211448941fd", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb95-09bd-7cdc-a6ac-b7be3b2ce198/from_3570_to_1604.update.rpf.hdiff", 484285949 } /* diff sha1: 2107e0cea4a3a40c1e95243b5c535684a12d7069 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb95-c447-7c99-bb3e-48526b79a627/from_3751_to_1604.update.rpf.hdiff", 484294097 } /* diff sha1: cb44e563041743f303b25066d17cef5740882bc1 */,
						{ "4d475df8caa95cb99aedcb0b555a3f83f8acf60c", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb95-c447-7c99-bb3e-48526b79a627/from_3751_to_1604.update.rpf.hdiff", 484294097 } /* diff sha1: cb44e563041743f303b25066d17cef5740882bc1 */,
						{ "31cbd81373475d5407c20058733ea910cadce13b", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://downloads.cfx-services.net/prod/019feb96-20c5-76c7-bd89-af7715a0d3d5/from_3889_to_1604.update.rpf.hdiff", 484297827 } /* diff sha1: 8f7d6a0880e675f8345c116da6302a8f7a03c917 */
					},
				},
			},
		}
	},
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
				}
			},
			{
				"shaders_x64.rpf",
				{
					"shaders_x64.rpf", "f8ecee595e74c66c5bd02fd87c2947cf475a2614", "https://content.cfx.re/mirrors/patches_redm/1491.50/shaders_x64.rpf", 233921358,
				}
			},
			{
				"update_1.rpf",
				{
					"update_1.rpf", "8c25d7345b7e69ebaee24ccfea97739ace59ba51", "https://content.cfx.re/mirrors/patches_redm/1491.50/update_1.rpf", 2833741450,
				}
			},
			{
				"update_2.rpf",
				{
					"update_2.rpf", "5a77f9b8cb24e1c3e78ee33c7ed218a32e3d2e32", "https://content.cfx.re/mirrors/patches_redm/1491.50/update_2.rpf", 152046254,
				}
			},
			{
				"update_3.rpf",
				{
					"update_3.rpf", "be15563d37c1ab0f655eeebb45f4d30527df950d", "https://content.cfx.re/mirrors/patches_redm/1491.50/update_3.rpf", 132374684,
				}
			},
			{
				"update_4.rpf",
				{
					"update_4.rpf", "853a63af1698a970dfb73295faa76a31e56fe4bd", "https://content.cfx.re/mirrors/patches_redm/1491.50/update_4.rpf", 2015028563,
				}
			}
		}
	},
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
	if (GetTargetGameBuild() >= GetDefaultBuild())
	{
		for (auto [_, entry]: g_entriesToLoadPerBuild[GetTargetGameBuild()])
		{
			g_requiredEntries.push_back(entry);
		}
	}
	else
	{
		// Download files for the default executable build because that's what we run regardless of the requested version.
		for (auto [_, entry]: g_entriesToLoadPerBuild[GetDefaultBuild()])
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

	if (IsTargetGameBuildOrGreater<3258>())
	{
		g_requiredEntries.push_back({ "update/x64/dlcpacks/mp2024_01/dlc.rpf", "bf9efb4348cc95ecd2ab0bfd956768148c7b48bf", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 1178773504 });
	}

	if (IsTargetGameBuildOrGreater<3407>())
	{
		g_requiredEntries.push_back({ "update/x64/dlcpacks/mp2024_02/dlc.rpf", "c15b71266137bddb93ad1197afeb35d54b6a21fe", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 1183961088 });
	}

	if (IsTargetGameBuildOrGreater<xbr::Build::Summer_2025>())
	{
		g_requiredEntries.push_back({ "update/x64/dlcpacks/mp2025_01/dlc.rpf", "30434278A73A9DD4EF7D07CC00E89757C7AA005B", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 1793306624 });
	}

	if (IsTargetGameBuildOrGreater<xbr::Build::Winter_2025>())
	{
		g_requiredEntries.push_back({ "update/x64/dlcpacks/mp2025_02/dlc.rpf", "EB3F51A0A99F9E2653A5C3F28208FF348CDD2942", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 1905895424 });
	}

	if (IsTargetGameBuildOrGreater<xbr::Build::Summer_2026>())
	{
		g_requiredEntries.push_back({ "update/x64/dlcpacks/mp2026_01/dlc.rpf", "252FD77A7E865EE061D52F720248D69C53BFE32A", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 938670080 });
	}
#elif IS_RDR3
	for (auto [_, entry]: g_entriesToLoadPerBuild[GetTargetGameBuild()])
	{
		g_requiredEntries.push_back(entry);
	}

	if (IsTargetGameBuild<1491>())
	{
		g_requiredEntries.push_back({ "x64/dlcpacks/mp009/dlc.rpf", "7ae2012968709d6d1079c88ee40369f4359778bf", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 494360763 });
	}
#endif

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
