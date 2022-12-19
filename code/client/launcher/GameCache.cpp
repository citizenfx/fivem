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

	// methods
	bool IsPrimitiveFile() const
	{
		return std::string_view{ filename }.find("ros_") == 0 || std::string_view{ filename }.find("launcher/") == 0;
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

				if (_strnicmp(remotePath, "http", 4) != 0 && _strnicmp(remotePath, "ipfs", 4) != 0)
				{
					remotePath = va("rockstar:%s", entry.remotePath);
				}

				// if the file isn't of the original size
				auto download = CL_QueueDownload(remotePath, localFileName.c_str(), entry.remoteSize, ((entry.remoteSize != entry.localSize && !entry.archivedFile) ? compressionAlgo_e::XZ : compressionAlgo_e::None));
				BumpDownloadCount(download, entry.checksums[0]);

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

						fseek(f, 0, SEEK_END);
						entry->streamImport = (void*)f;
						entry->streamSize = ftell(f);

						entry->read = [](const hpatch_TStreamInput* entry, hpatch_StreamPos_t at, uint8_t* begin, uint8_t* end) -> hpatch_BOOL {
							auto size = end - begin;

							FILE* f = (FILE*)entry->streamImport;
							fseek(f, at, SEEK_SET);

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
	if (IsTargetGameBuild<2802>())
	{
		g_requiredEntries.push_back({ "GTA5.exe", "ebb6c144c5befe3529235deccbd8f59d6ce1a76c", "https://content.cfx.re/mirrors/patches/2802.0/GTA5.exe", 46709592 });
		g_requiredEntries.push_back({ "update/update.rpf", "66388a381347511b7b28aaf91741615e45008e8b", "https://content.cfx.re/mirrors/patches/2802.0/update.rpf", 1079308288 });
		g_requiredEntries.push_back({ "update/update2.rpf", "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "https://content.cfx.re/mirrors/patches/2802.0/update2.rpf", 344610816 });
	}
	else if (IsTargetGameBuild<2699>())
	{
		g_requiredEntries.push_back({ "GTA5.exe", "b9f3960ca0c7c05aab23d3b1d158309bc085fbbe", "https://content.cfx.re/mirrors/patches/2699.0/GTA5.exe", 61111680 });
		g_requiredEntries.push_back({ "update/update.rpf", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://content.cfx.re/mirrors/patches/2699.0/update.rpf", 1073854464,
		{
			{ "66388a381347511b7b28aaf91741615e45008e8b", "86d88c5ea36e67683a138c0e690c42fe288205fa", "https://content.cfx.re/mirrors/patches/2802_0_2699_update.hdiff", 102324935 },
		} });
		g_requiredEntries.push_back({ "update/update2.rpf", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://content.cfx.re/mirrors/patches/2699.0/update2.rpf", 324530176,
		{
			{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://content.cfx.re/mirrors/patches/2802_0_2699_update2.hdiff", 250133852 },
			{ "9f4805db10e5b3029215216befcbe2ce676a8269", "414a04256bf0b00b78324478508a6beaea1ef5a7", "https://content.cfx.re/mirrors/patches/2699_16_2699_update2.hdiff", 200996724 },
		} });
	}
	else if (IsTargetGameBuild<2612>())
	{
		g_requiredEntries.push_back({ "GTA5.exe", "d423086fd7a7721b8be77cfb9a4f8826784b284b", "https://content.cfx.re/mirrors/patches/2612.1/GTA5.exe", 60351952 });
		g_requiredEntries.push_back({ "update/update.rpf", "80f9bd028e5bc781f641fe210a88579eff827989", "https://content.cfx.re/mirrors/patches/2612.1/update.rpf", 1056649216,
		{
			{ "66388a381347511b7b28aaf91741615e45008e8b", "80f9bd028e5bc781f641fe210a88579eff827989", "https://content.cfx.re/mirrors/patches/2802_0_2612_update.hdiff", 131292485 },
			{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "80f9bd028e5bc781f641fe210a88579eff827989", "https://content.cfx.re/mirrors/patches/2699_2612_update.hdiff", 127701528 },
		} });
		g_requiredEntries.push_back({ "update/update2.rpf", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://content.cfx.re/mirrors/patches/2612.1/update2.rpf", 312209408,
		{
			{ "c7de68bdc56ec4577bd4fce5d85cca9a4d529839", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://content.cfx.re/mirrors/patches/2802_0_2612_update2.hdiff", 240847147 },
			{ "9f4805db10e5b3029215216befcbe2ce676a8269", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://content.cfx.re/mirrors/patches/2699_16_2612_update2.hdiff", 240637739 },
			{ "414a04256bf0b00b78324478508a6beaea1ef5a7", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://content.cfx.re/mirrors/patches/2699_2612_update2.hdiff", 240637812 },
			{ "e040bb68272c3518e411ada7fb43a2a45151b070", "c993e2d14cce9462fa8ba056f3406d60050a1c92", "https://content.cfx.re/mirrors/patches/2628_2612_update2.hdiff", 161966401 },
		} });
	}
	else if (IsTargetGameBuild<2545>())
	{
		g_requiredEntries.push_back({ "GTA5.exe", "517556bb548880362c18d502361ce374070994c2", "https://content.cfx.re/mirrors/patches/2545.0/GTA5.exe", 59988376 });
		g_requiredEntries.push_back({ "update/update.rpf", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches/2545.0/update.rpf", 1366638592,
		{
			{ "66388a381347511b7b28aaf91741615e45008e8b", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches/2802_0_2545_update.hdiff", 441227939 },
			{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches/2699_2545_update.hdiff", 437636032 },
			{ "80f9bd028e5bc781f641fe210a88579eff827989", "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "https://content.cfx.re/mirrors/patches/2612_2545_update.hdiff", 311774185 },
		} });
	}
	else if (IsTargetGameBuild<2372>())
	{
		g_requiredEntries.push_back({ "GTA5.exe", "470235e04299b02aa3aef834ef1ff834cac2327f", "https://content.cfx.re/mirrors/patches/2372.0/GTA5.exe", 59716912 });
		g_requiredEntries.push_back({ "update/update.rpf", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches/2372.0/update.rpf", 1132066816,
		{
			{ "66388a381347511b7b28aaf91741615e45008e8b", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches/2802_0_2372_update.hdiff", 349288020 },
			{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches/2699_2372_update.hdiff", 345348050 },
			{ "80f9bd028e5bc781f641fe210a88579eff827989", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches/2612_2372_update.hdiff", 343976392 },
			{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "1824cdbc27c3e0eaa86920a38751322727872831", "https://content.cfx.re/mirrors/patches/2545_2372_update.hdiff", 276106385 },
		} });
	}
	else if (IsTargetGameBuild<2189>())
	{
		g_requiredEntries.push_back({ "GTA5.exe", "fcd5fd8a9f99f2e08b0cab5d500740f28a75b75a", "https://content.cfx.re/mirrors/patches/2189.0/GTA5.exe", 63124096 });
		g_requiredEntries.push_back({ "update/update.rpf", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches/2189.0/update.rpf", 1276805120,
		{
			{ "66388a381347511b7b28aaf91741615e45008e8b", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches/2802_0_2189_update.hdiff", 513356064 },
			{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches/2699_2189_update.hdiff", 509396786 },
			{ "80f9bd028e5bc781f641fe210a88579eff827989", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches/2612_2189_update.hdiff", 508757790 },
			{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches/2545_2189_update.hdiff", 441617306 },
			{ "1824cdbc27c3e0eaa86920a38751322727872831", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches/2372_2189_update.hdiff", 429153146 },
			{ "748d16b81a34ad317b93cd85e2b088dabdce5cc7", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches/2245_2189_update.hdiff", 193145914 },
			{ "36c5c94274602527f946497553e118d72500c09f", "fe387dbc0f700d690b53d44ce1226c624c24b8fc", "https://content.cfx.re/mirrors/patches/2215_2189_update.hdiff", 21294688 },	
		} });
	}
	else if (IsTargetGameBuild<2060>())
	{
		g_requiredEntries.push_back({ "GTA5.exe", "741c8b91ef57140c023d8d29e38aab599759de76", "https://content.cfx.re/mirrors/patches/2060.2/GTA5.exe", 60589184 });
		g_requiredEntries.push_back({ "update/update.rpf", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches/2060.2/update.rpf", 1229002752,
		{
			{ "66388a381347511b7b28aaf91741615e45008e8b", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches/2802_0_2060_update.hdiff", 508501688 },
			{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches/2699_2060_update.hdiff", 504126319 },
			{ "80f9bd028e5bc781f641fe210a88579eff827989", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches/2612_2060_update.hdiff", 504469096 },
			{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches/2545_2060_update.hdiff", 438194552 },
			{ "1824cdbc27c3e0eaa86920a38751322727872831", "736f1cb26e59167f302c22385463d231cce302d3", "https://content.cfx.re/mirrors/patches/2372_2060_update.hdiff", 427205591 },
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
			{ "66388a381347511b7b28aaf91741615e45008e8b", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches/2802_0_1604_update.hdiff", 478650625 },
			{ "86d88c5ea36e67683a138c0e690c42fe288205fa", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches/2699_1604_update.hdiff", 475411202 },
			{ "80f9bd028e5bc781f641fe210a88579eff827989", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches/2612_1604_update.hdiff", 475094324 },
			{ "2993b3c30f61cbbb8dbce859604d7fb717ff8dae", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches/2545_1604_update.hdiff", 409505316 },
			{ "1824cdbc27c3e0eaa86920a38751322727872831", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://content.cfx.re/mirrors/patches/2372_1604_update.hdiff", 400087270 },
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
#elif IS_RDR3
	if (IsTargetGameBuild<1491>())
	{
		g_requiredEntries.push_back({ "RDR2.exe", "0e5fcd9ca85ca1556820afbd7082225c70344a48", "ipfs://bafybeidi57jjjtenbp7tyiyqkwbsqs7kkl5oipvonvygijaohg7lmaedqu", 89045344 });
		g_requiredEntries.push_back({ "appdata0_update.rpf", "0cf86c0249299c03b112da8c3be03890ecf37b2a", "ipfs://bafybeiegcuomoefqoknwgnopx4kucknabzbpceqkqpyiovnkygbk5qox6i", 3163615 });
		g_requiredEntries.push_back({ "shaders_x64.rpf", "c645a9feda39d2110c6224d82eb5a06d28f2f690", "ipfs://bafybeibw6nbijjs3y5jbt6s2xr5neo6c6u7ryqbnbftp3vbtnogwdorvoy", 233917870 });
		g_requiredEntries.push_back({ "update_1.rpf", "abbd8d5ba9f309e947df3d02f9d86901900f9279", "ipfs://bafybeicpau23p5xeiv24f3wknofzcbzcmzhggohazvnunh3wictpevrhni", 2833730538 });
		g_requiredEntries.push_back({ "update_2.rpf", "50f7f284ffba0429399450030634b8c7b632a69e", "ipfs://bafybeiaqwolwlyvn6lrs7y57xiwqx4fdqfxgrbo54ofrysecrs5cnc4nou", 152038510 });
		g_requiredEntries.push_back({ "update_3.rpf", "7495a5f780921e4e9c736531b1b7dc1a822e50e7", "ipfs://bafybeiewhm7gcx7zjfine3lx3mohnx764nwm2sfc6dqqgxtdhyewkesze4", 132370220 });
		g_requiredEntries.push_back({ "update_4.rpf", "69a13c91d9c09c1f4ce9a33cf5d40a0be4db920c", "ipfs://bafybeieclgpinpfipgqdgrc6dlqovexfdclq6js44ph4ttfn6ng444le5q", 2015025315 });
	}
	else if (IsTargetGameBuild<1436>())
	{
		g_requiredEntries.push_back({ "RDR2.exe", "f998b4863b11793547c09c226ab884e1e26931f2", "ipfs://bafybeignvzbrnkq35qa2jypvrcbx73qsiknqhhkv57lkwhkmnixsq5bn7e", 89104336 });
		g_requiredEntries.push_back({ "appdata0_update.rpf", "ba1d727a70fa1c204441c8e3768a1a40b02ef67f", "ipfs://bafybeihevef6cp25uczabosbvna4uc5idifctitzwuhped4l5ntk7ruqoa", 3163551 });
		g_requiredEntries.push_back({ "shaders_x64.rpf", "f4f06c18701d66958eb6f0ac243c8467033b864b", "ipfs://bafybeihfgavfbsihf65dtv6vyipphbcfrrytcj53hulzkoy7nonpwh4h74", 233898030 });
		g_requiredEntries.push_back({ "update_1.rpf", "9e62163f0383aa4eb30a02fa0f5628bbf4538543", "ipfs://bafybeifqlaf6vvjbp76ofdvju57oirz74pkm7f62srf36pljgjoo2b3hni", 2836982634 });
		g_requiredEntries.push_back({ "update_2.rpf", "87323b6d0e1c790972041a034a6f293eb774c84d", "ipfs://bafybeigewsce6w5mtwtywykb7eg2mhey4phhbvkw7cftdjrsowpnebzoxq", 152008542 });
		g_requiredEntries.push_back({ "update_3.rpf", "a2708ff55294d70bda198a1ff98c1f4b55b0c0df", "ipfs://bafybeibzrkqunak23ppqkk54lkkmmtojqke5pc73f2mfodhwkg7vwrh6pm", 132374108 });
		g_requiredEntries.push_back({ "update_4.rpf", "4ec55a211e7cb1d68c8fd471cfb049d7690fc9ee", "ipfs://bafybeic6e75dlx2ynxek5gb7qy3tl7ysmj4xqt4un3c6efrckxxtqtskke", 2014659811 });
	}
	else if (IsTargetGameBuild<1355>())
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
