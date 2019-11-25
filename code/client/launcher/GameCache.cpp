/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"

#include <CfxState.h>
#include <HostSharedData.h>

#undef interface
#include "InstallerExtraction.h"
#include <array>

#include <Error.h>

#if defined(GTA_FIVE) || defined(IS_RDR3)
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

	// constructor
	GameCacheEntry(const char* filename, const char* checksum, const char* remotePath, size_t localSize)
		: filename(filename), checksums({ checksum }), remotePath(remotePath), localSize(localSize), remoteSize(localSize), archivedFile(nullptr)
	{

	}

	GameCacheEntry(const char* filename, const char* checksum, const char* remotePath, const char* archivedFile, size_t localSize, size_t remoteSize)
		: filename(filename), checksums({ checksum }), remotePath(remotePath), localSize(localSize), remoteSize(remoteSize), archivedFile(archivedFile)
	{

	}

	GameCacheEntry(const char* filename, std::initializer_list<const char*> checksums, const char* remotePath, size_t localSize)
		: filename(filename), checksums(checksums), remotePath(remotePath), localSize(localSize), remoteSize(localSize), archivedFile(nullptr)
	{

	}

	GameCacheEntry(const char* filename, std::initializer_list<const char*> checksums, const char* remotePath, const char* archivedFile, size_t localSize, size_t remoteSize)
		: filename(filename), checksums(checksums), remotePath(remotePath), localSize(localSize), remoteSize(remoteSize), archivedFile(archivedFile)
	{

	}

	// methods
	bool IsPrimitiveFile() const
	{
		return std::string(filename).find("ros_2034/") == 0 || std::string(filename).find("launcher/") == 0;
	}

	std::wstring GetCacheFileName() const
	{
		std::string filenameBase = filename;

		if (IsPrimitiveFile())
		{
			return MakeRelativeCitPath(ToWide(va("cache\\game\\%s", filenameBase.c_str())));
		}

		std::replace(filenameBase.begin(), filenameBase.end(), '/', '+');

		return MakeRelativeCitPath(ToWide(va("cache\\game\\%s_%s", filenameBase.c_str(), checksums[0])));
	}

	std::wstring GetRemoteBaseName() const
	{
		std::string remoteNameBase = remotePath;

		int slashIndex = remoteNameBase.find_last_of('/') + 1;

		return MakeRelativeCitPath(ToWide("cache\\game\\" + remoteNameBase.substr(slashIndex)));
	}

	std::wstring GetLocalFileName() const
	{
		return MakeRelativeGamePath(ToWide(filename));
	}
};

struct GameCacheStorageEntry
{
	// sha1-sized file checksum
	uint8_t checksum[20];

	// file modification time
	time_t fileTime;
};

// global cache mapping of ROS files to disk files
static GameCacheEntry g_requiredEntries[] =
{
#if defined(GTA_FIVE)
	{ "GTA5.exe", "8939c8c71aa98ad7ca6ac773fae1463763c420d8", "https://runtime.fivem.net/patches/GTA_V_Patch_1_0_1604_0.exe", "$/GTA5.exe", 72484280, 1031302600 },
	{ "update/update.rpf", "fc941d698834e30e40a06a40f6a35b1b18e1c50c", "https://runtime.fivem.net/patches/GTA_V_Patch_1_0_1604_0.exe", "$/update/update.rpf", 966805504, 1031302600 },
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
	{ "update/x64/dlcpacks/patchday17ng/dlc.rpf", { "7dc8639f1ffa25b3237d01aea1e9975238628952", "c7163e1d8105c87b867b09928ea8346e26b27565" }, "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday12ng/dlc.rpf", 59975680 },

	//DLCPacks17
	{ "update/x64/dlcpacks/mpassault/dlc.rpf", "7c65b096261dd88bd1f952fc6046626f1ca56215", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 314443776 },
	{ "update/x64/dlcpacks/patchday18ng/dlc.rpf", "9e16b7af4a1e58878f0dd16dd86cbd772a8ce9ef", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday12ng/dlc.rpf", 4405248 },

	//DLCPacks18
	{ "update/x64/dlcpacks/mpbattle/dlc.rpf", "80018257a637417b911bd4540938866ae95d0cf5", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 3981039616 },
	{ "update/x64/dlcpacks/mpbattle/dlc1.rpf", "b16fb76065132f5f9af4b2a92431b9f91b670542", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 992296960 },
	{ "update/x64/dlcpacks/patchday19ng/dlc.rpf", "3373311add1eb5ff850e1f3fbb7d15512cbc5b8b", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday12ng/dlc.rpf", 765630464 },

	//DLCPacks19
	{ "update/x64/dlcpacks/mpchristmas2018/dlc.rpf", "c4cda116420f14a28e5a999740cc53cf53a950ec", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 3247781888 },
	{ "update/x64/dlcpacks/patchday20ng/dlc.rpf", "fbba396a0ede622e08f76c5ced8ac1d6839c0227", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday12ng/dlc.rpf", 457129984 },

	{ "ros_2034/SocialClubD3D12Renderer.dll", "C308AC7D34F80FE9486716FBF4C713B55924B661", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/cef.pak", 3866710, 85063288 },
	{ "ros_2034/SocialClubVulkanLayer.dll", "C308AC7D34F80FE9486716FBF4C713B55924B661", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/cef.pak", 3866710, 85063288 },
	{ "ros_2034/SocialClubVulkanLayer.json", "C308AC7D34F80FE9486716FBF4C713B55924B661", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/cef.pak", 3866710, 85063288 },
	{ "ros_2034/cef.pak", "C308AC7D34F80FE9486716FBF4C713B55924B661", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/cef.pak", 3866710, 85063288 },
	{ "ros_2034/cef_100_percent.pak", "66EF9A8559E1A8F7976F95FDD2355A0D0E101532", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/cef_100_percent.pak", 647967, 85063288 },
	{ "ros_2034/cef_200_percent.pak", "0E65DCF353901A8F5D4309585337F6465C3DFC78", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/cef_200_percent.pak", 753127, 85063288 },
	{ "ros_2034/chrome_elf.dll", "D616A5BB2670476ACC87A8FDF5211F9A46F03688", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/chrome_elf.dll", 547304, 85063288 },
	{ "ros_2034/d3dcompiler_43.dll", "80FE574E9EB795C8B148C218E4CE5EE52E9C6AB9", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/d3dcompiler_43.dll", 2106344, 85063288 },
	{ "ros_2034/d3dcompiler_47.dll", "5E5D103067802BB20ACB660CF6FCD74DADC4F6ED", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/d3dcompiler_47.dll", 4300776, 85063288 },
	{ "ros_2034/icudtl.dat", "C35D6E4DB540518263214697F589C54FAAC87533", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/icudtl.dat", 10171360, 85063288 },
	{ "ros_2034/libcef.dll", "C30E1DD3CA864BD46A7F316B4663472CF3601423", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/libcef.dll", 94052328, 85063288 },
	{ "ros_2034/libEGL.dll", "6399CE3EC756586380DE8DDE4734CD8C98255654", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/libEGL.dll", 98792, 85063288 },
	{ "ros_2034/libGLESv2.dll", "7FBD036872AEADCAAEC84501E255F73DFB70CDDD", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/libGLESv2.dll", 4434920, 85063288 },
	{ "ros_2034/natives_blob.bin", "7BCA7FCF54CC81CAFBAEEF44EF23DAE4A7C97619", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/natives_blob.bin", 218121, 85063288 },
	{ "ros_2034/scui.pak", "97DB3B1AAAB9D410A33B3B280069886708535F7A", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/scui.pak", 6035130, 85063288 },
	{ "ros_2034/snapshot_blob.bin", "0EC391456619FC5BCA046A23BADCF151D72D1B35", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/snapshot_blob.bin", 1362716, 85063288 },
	{ "ros_2034/socialclub.dll", "EAEA6D9A4564E5C58A401CEFED378AB947F1A5C3", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/socialclub.dll", 7962088, 85063288 },
	{ "ros_2034/SocialClubHelper.exe", "8D6C6BC68548951358ACC4F51265CAD8177E35D6", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/SocialClubHelper.exe", 1092072, 85063288 },
	{ "ros_2034/v8_context_snapshot.bin", "9A66D10886CC99BFD7E32C2178894717F33664EE", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/v8_context_snapshot.bin", 1772488, 85063288 },
	{ "ros_2034/widevinecdmadapter.dll", "9D58F5009C73D5FB9BE1089A46CD9576D2F5654B", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/widevinecdmadapter.dll", 286696, 85063288 },
	{ "ros_2034/locales/am.pak", "780E74DA7D6299BA8D8278CB42685287BFE9D1DE", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/am.pak", 288192, 85063288 },
	{ "ros_2034/locales/ar.pak", "0E34AE587EAC54F8535C80EAC6F8D0C73CF8ADD3", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/ar.pak", 283698, 85063288 },
	{ "ros_2034/locales/bg.pak", "8EBA4746AF8927CB75390CD6EBEE59E027B4636E", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/bg.pak", 329621, 85063288 },
	{ "ros_2034/locales/bn.pak", "AD5A2E603C97D9B48DE47DCA188315D0790765DE", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/bn.pak", 430606, 85063288 },
	{ "ros_2034/locales/ca.pak", "97EF23AF64A67C8A444E1983D600B4D610F03D34", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/ca.pak", 204954, 85063288 },
	{ "ros_2034/locales/cs.pak", "9ADAE19B1982968EE0650AD5DCDED1E625632B14", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/cs.pak", 208304, 85063288 },
	{ "ros_2034/locales/da.pak", "463ECE138DACBD6835F72A84EABAA57E20E99DDF", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/da.pak", 187208, 85063288 },
	{ "ros_2034/locales/de.pak", "60FE88E7AA2E52A57C6F679F235943DF943E3263", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/de.pak", 203794, 85063288 },
	{ "ros_2034/locales/el.pak", "E3E89A5C4729F20916F40DCCC13D6B8C4AF4C6DA", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/el.pak", 362601, 85063288 },
	{ "ros_2034/locales/en-GB.pak", "249E3084DD6D148F2487CE9E1A48B7FAE97FDF0E", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/en-GB.pak", 168518, 85063288 },
	{ "ros_2034/locales/en-US.pak", "5DE10AAD0EFA91043E80C2397375DF52AD825545", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/en-US.pak", 169455, 85063288 },
	{ "ros_2034/locales/es-419.pak", "5323B07879FCEFE5A0C4924A83DC811F7B6DF6D7", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/es-419.pak", 201890, 85063288 },
	{ "ros_2034/locales/es.pak", "17D3C5F66B050D4A7B8D798647DB60D64E7E25FB", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/es.pak", 205495, 85063288 },
	{ "ros_2034/locales/et.pak", "58514249041B33360A8DD0D21A7DABE921E7DC3C", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/et.pak", 182828, 85063288 },
	{ "ros_2034/locales/fa.pak", "ADAD87081BD3208868B98B5BEE32CB097EBCEBFB", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/fa.pak", 289551, 85063288 },
	{ "ros_2034/locales/fi.pak", "AC64415B900EDC0F1FECE2638E19E1B1E104F5CB", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/fi.pak", 189663, 85063288 },
	{ "ros_2034/locales/fil.pak", "D619F2F3ED06DCA54AC02BC6410A291B78A3CD42", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/fil.pak", 208302, 85063288 },
	{ "ros_2034/locales/fr.pak", "B44BDC854769EFF7755055470D4B743968281F04", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/fr.pak", 219692, 85063288 },
	{ "ros_2034/locales/gu.pak", "22D4631FF1D32F332F1133E809E774E1A8D29EB3", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/gu.pak", 408074, 85063288 },
	{ "ros_2034/locales/he.pak", "581690EBCAF2137839220341FBA5B44207153BD2", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/he.pak", 241853, 85063288 },
	{ "ros_2034/locales/hi.pak", "BD9998EC61BD9F9FD48E890DCF323D9409772AC5", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/hi.pak", 415545, 85063288 },
	{ "ros_2034/locales/hr.pak", "C41F1E90E9313B13D1BCE7055354596992935CD5", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/hr.pak", 196271, 85063288 },
	{ "ros_2034/locales/hu.pak", "272AD9767C2DC35938C8104D7AB5D40BCEACF933", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/hu.pak", 215127, 85063288 },
	{ "ros_2034/locales/id.pak", "32EF210DAAEE35E675525CE50A91F0EC303EB8FF", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/id.pak", 181124, 85063288 },
	{ "ros_2034/locales/it.pak", "C26951918D1A44CC0F518EEF3C2E7C91BC4AFB23", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/it.pak", 198516, 85063288 },
	{ "ros_2034/locales/ja.pak", "74D460ACEC0C1791824D0CF9470B76D2771F1CF8", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/ja.pak", 246094, 85063288 },
	{ "ros_2034/locales/kn.pak", "5E8AE3EEBA26548897D10551DB651F753911B179", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/kn.pak", 471779, 85063288 },
	{ "ros_2034/locales/ko.pak", "FD5072F271E90A9DBF4637C0C56B3B0F60678C41", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/ko.pak", 207438, 85063288 },
	{ "ros_2034/locales/lt.pak", "61779BFDEDE897F89C2618E489C3E57600046FB2", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/lt.pak", 211038, 85063288 },
	{ "ros_2034/locales/lv.pak", "7383BDF2105F822923483721BFAD83CF721D9236", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/lv.pak", 210459, 85063288 },
	{ "ros_2034/locales/ml.pak", "6177F6C89BF61827217323EF2056B6A5B9772A5A", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/ml.pak", 512370, 85063288 },
	{ "ros_2034/locales/mr.pak", "A6592A51E543527BD68CD4770260EDF22E8828A0", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/mr.pak", 409172, 85063288 },
	{ "ros_2034/locales/ms.pak", "06E8900F5BB63FBF8044091CAAA95AE8ED5FC15C", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/ms.pak", 187796, 85063288 },
	{ "ros_2034/locales/nb.pak", "56258B92C4B1E4032F7C45DAAEFC64DFD300D426", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/nb.pak", 184630, 85063288 },
	{ "ros_2034/locales/nl.pak", "1C64BE844AEF9059234F201B1FF4401ABC4C18AA", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/nl.pak", 193431, 85063288 },
	{ "ros_2034/locales/pl.pak", "887A8304EAFDEA125E45D83F624C16EBBF772F97", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/pl.pak", 203748, 85063288 },
	{ "ros_2034/locales/pt-BR.pak", "2CB78986FD9DD8F8A9D50C90E860F28B9F052F2F", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/pt-BR.pak", 199224, 85063288 },
	{ "ros_2034/locales/pt-PT.pak", "13E7F9FCC875ECA59F49B9B87CDF1FD07A649D29", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/pt-PT.pak", 200771, 85063288 },
	{ "ros_2034/locales/ro.pak", "B896211961B3F0F53ED64C0D4DB7E7F00E5D3BFF", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/ro.pak", 206682, 85063288 },
	{ "ros_2034/locales/ru.pak", "9B9E82643E7669D3F50C43FB8C4BD85AFD7833DB", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/ru.pak", 313752, 85063288 },
	{ "ros_2034/locales/sk.pak", "8BF7EE156BCFFBDAB6A10285F518441E3E2533A6", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/sk.pak", 212466, 85063288 },
	{ "ros_2034/locales/sl.pak", "4C578336BAF8A662CB2951C8274201C9DCBE498E", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/sl.pak", 196260, 85063288 },
	{ "ros_2034/locales/sr.pak", "460513B76BA9845F3E580EFD71922E07EADC9DD3", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/sr.pak", 307758, 85063288 },
	{ "ros_2034/locales/sv.pak", "05125215C5451A7471BA7972BF18F99D3925C1CA", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/sv.pak", 185484, 85063288 },
	{ "ros_2034/locales/sw.pak", "1EA9F306A47DB04637D501A0FDBD0EDA5F84C80E", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/sw.pak", 189595, 85063288 },
	{ "ros_2034/locales/ta.pak", "1C08DAA3F94E6DC72B9B275218F3DC8D496760B6", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/ta.pak", 478276, 85063288 },
	{ "ros_2034/locales/te.pak", "BEE2FDC9533FA8552A73AB760132714338E9F84F", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/te.pak", 453971, 85063288 },
	{ "ros_2034/locales/th.pak", "0BB6902FEA12BB6A5496AB0CD2D71B70DD26D5A2", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/th.pak", 389366, 85063288 },
	{ "ros_2034/locales/tr.pak", "6CCC0C8B430D14335707947B897835D0529C436B", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/tr.pak", 200055, 85063288 },
	{ "ros_2034/locales/uk.pak", "43092BD4966E2C7F2A0A3425456C61704A1BD398", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/uk.pak", 322593, 85063288 },
	{ "ros_2034/locales/vi.pak", "CAB3084DB7531EAB27DD8A804780E7EF83D16B6C", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/vi.pak", 228436, 85063288 },
	{ "ros_2034/locales/zh-CN.pak", "BC70FAC761065BED92E317946F6A4AF5879ECDA8", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/zh-CN.pak", 171281, 85063288 },
	{ "ros_2034/locales/zh-TW.pak", "490DEE3F6B8B04BAF930803A8C34F51A508B5F71", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/locales/zh-TW.pak", 171913, 85063288 },
	{ "ros_2034/swiftshader/libEGL.dll", "1C6BCE6CDB4B2E1766A67F931A72519CEFF6AEB1", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/swiftshader/libEGL.dll", 139240, 85063288 },
	{ "ros_2034/swiftshader/libGLESv2.dll", "484A9699A06A7D87EE5D2BACDF21B284F53328C5", "https://fivem.net/404-intentional-please-follow-instructions-to-prime-game-cache", "$/swiftshader/libGLESv2.dll", 3019240, 85063288 },
#elif defined(IS_RDR3)
	{ "RDR2.exe", "e3548977700a80de6eb3f241cc091810cdfc04ea", "ipfs://bafybeidfsyzzijulnnxoskegcmkjcrqgszos4u2hwjqtjql3orlw7uum7q", 89376912 },
	{ "appdata0_update.rpf", "e312b3d8b213a46fbbbf138fe7ed3b7bac20dca6", "ipfs://bafybeigc6pfglf3bhiki6lna6oxbqzxkarge5gi757lzy4lswghvbanude", 2860767 },
	{ "update.rpf", "86a19a74de8a37e8f926968d27b35f0f9889bd54", "ipfs://bafybeig6df4wibiuy4dv2yoo5tttbyllrt3wuli7snivojaoma5vxfbpne", 2801186198 },
	{ "shaders_x64.rpf", "cc018f7a7316bcc33107c3bbc8640d55afa84a80", "ipfs://bafybeicncagqngk3brfu4m5xgvj4mll6ae2dwbz3gfkhkkvpkkffkaqam4", 233006430 },
#endif

	{ "launcher/Launcher.exe", "54456C7E24724BF76CBA02210CF2737339663744", "ipfs://bafybeicam5f3ptgq6oazcswjcksb5xwnxwed2jhcelbcnpj43zcvg62dyi", 37970576 },
	{ "launcher/Launcher.rpf", "619AFA8203CF1DACEAE115898DE3EE30F40B678D", "ipfs://bafybeiacp4ynb56vyrme3cjt2e4xke45kipbqqcggnv7ggai2o22iknuji", 784384 },
	{ "launcher/Redistributables/SocialClub/Social-Club-Setup.exe", "8260133F1A5BF9131BF01172D2A181881BB98BD0", "ipfs://bafybeigowqcjmgwxqqwxc5z2uvrt54wukb3n36uzux6y47kc425a477px4", 99013544 },
	{ "launcher/Redistributables/VCRed/vc_redist.x64.exe", "BE4F7AD520349D9D0683AB9DD171B3A1E4DB426B", "ipfs://bafybeiese5x5trw2o6bb3o7oqj75465unj23e4siwy6jn5yj6injqeadeq", 15080792 },
	{ "launcher/Redistributables/VCRed/vc_redist.x86.exe", "13439C916FAAF5E08CAFCE790381578685B0A552", "ipfs://bafybeict7f4zkke24j2usbwjskdju7n3zxxatxsw3kwmcvey262zs67f3e", 14381032 },
	{ "launcher/RockstarService.exe", "724C0E73F5E59DF9302AE734D918D364E3ACEF54", "ipfs://bafybeihjgnkp2exfzkv6ebolk3ps2mpfklbr7hwvnjqhvngeykvzxtxd2y", 474256 },
	{ "launcher/RockstarSteamHelper.exe", "FD742FB740A99B4705CA9AD977B036323C2094C7", "ipfs://bafybeie5nd36d5b7xoeocv7van42sl22pk6c5dwn4s5xh3xdx6mcg2e7va", 444560 },
	{ "launcher/ThirdParty/Epic/EOSSDK-Win64-Shipping.dll", "AF01787DDB7DE00239EDC62D33E0B20C0BE80037", "ipfs://bafybeicmrpc564y5w2uyfvhfgcea47xejs52rpld5z62hq4ppnvh6kik4m", 9971968 },
	{ "launcher/api-ms-win-core-console-l1-1-0.dll", "724F4F91041AD595E365B724A0348C83ACF12BBB", "ipfs://bafkreie7gyemcxc54l2xpircbtqsjnjqqjlrpv3y6hrzihstni5lnepxgm", 19208 },
	{ "launcher/api-ms-win-core-datetime-l1-1-0.dll", "4940D5B92B6B80A40371F8DF073BF3EB406F5658", "ipfs://bafkreib3ndl2wbsb3zvt5aosbg34bu4jnzh7u5tbpo5n2aplkqbwzxi3a4", 18696 },
	{ "launcher/api-ms-win-core-debug-l1-1-0.dll", "E7C8A6C29C3158F8B332EEA5C33C3B1E044B5F73", "ipfs://bafkreicvk5hz5agtcmciyjc2z36sdaa5bvwjbcukkbe3jrdckpx26qqpre", 18696 },
	{ "launcher/api-ms-win-core-errorhandling-l1-1-0.dll", "51CBB7BA47802DC630C2507750432C55F5979C27", "ipfs://bafkreiftofirfj6kjrwmb37oarf5qjce2mthun46gor6yemnq7tvmbbaju", 18696 },
	{ "launcher/api-ms-win-core-file-l1-1-0.dll", "9ACBEEF0AC510C179B319CA69CD5378D0E70504D", "ipfs://bafkreicw3yer57sgp7rdzsmjyhxcd4zetin5wilywukrdy55kfg7clc4zm", 22280 },
	{ "launcher/api-ms-win-core-file-l1-2-0.dll", "04669214375B25E2DC8A3635484E6EEB206BC4EB", "ipfs://bafkreibsfwld2krk57lyj2mws7cz2skikpljx3mo7vfuix2zfeutbjvrmu", 18696 },
	{ "launcher/api-ms-win-core-file-l2-1-0.dll", "402B7B8F8DCFD321B1D12FC85A1EE5137A5569B2", "ipfs://bafkreia6ujt2fzrij4l52vemn4rilym7p3nrlvxhg6svheiubts4xfjcly", 18696 },
	{ "launcher/api-ms-win-core-handle-l1-1-0.dll", "A2E2A40CEA25EA4FD64B8DEAF4FBE4A2DB94107A", "ipfs://bafkreieo4lpdo6qelrjlxmcqq6xdyl4vk5xn7mgcoz7ubmjuktznt53z3y", 18696 },
	{ "launcher/api-ms-win-core-heap-l1-1-0.dll", "B4310929CCB82DD3C3A779CAB68F1F9F368076F2", "ipfs://bafkreiehuxcuoxu4e35l72wwqaw2zctc4kah4uha2ggex6w4wfpl6w6lya", 19208 },
	{ "launcher/api-ms-win-core-interlocked-l1-1-0.dll", "F779CDEF9DED19402AA72958085213D6671CA572", "ipfs://bafkreic42ah7i4ywsh4b75jiys22fzaiksaqp36cftdfoyciwd644pp3ze", 18696 },
	{ "launcher/api-ms-win-core-libraryloader-l1-1-0.dll", "47143A66B4A2E2BA019BF1FD07BCCA9CFB8BB117", "ipfs://bafkreieoahvzep6ekp4spj7muhekuvsd4q5tmddwwzeard2rwmkirfykua", 19720 },
	{ "launcher/api-ms-win-core-localization-l1-2-0.dll", "9874398548891F6A08FC06437996F84EB7495783", "ipfs://bafkreifaptehrk2vswvm2svsfgthsrit7cl326wrjphmgu3zgn4ri2zasq", 21256 },
	{ "launcher/api-ms-win-core-memory-l1-1-0.dll", "9C03356CF48112563BB845479F40BF27B293E95E", "ipfs://bafkreigc5cd2c6dv2oijtvtcuqxvrqjaxhgiu6m27wd2tze234723xjlna", 19208 },
	{ "launcher/api-ms-win-core-namedpipe-l1-1-0.dll", "CB59F1FE73C17446EB196FC0DD7D944A0CD9D81F", "ipfs://bafkreiczh6rkujdukcfnsqv3vih5zgq3vxmbzbnq37y4io4qur6chlk7w4", 18696 },
	{ "launcher/api-ms-win-core-processenvironment-l1-1-0.dll", "2745259F4DBBEFBF6B570EE36D224ABDB18719BC", "ipfs://bafkreihkffzp5qjdawbfcyvohynofnwbiduebpqop25vdj5hpn765wqthi", 19720 },
	{ "launcher/api-ms-win-core-processthreads-l1-1-0.dll", "50699041060D14576ED7BACBD44BE9AF80EB902A", "ipfs://bafkreifbeimdm4y4pzjmilkybhgafml4l3ewiyawghwblkccah2chwskyq", 20744 },
	{ "launcher/api-ms-win-core-processthreads-l1-1-1.dll", "0BFFB9ED366853E7019452644D26E8E8F236241B", "ipfs://bafkreih6oydbjeb6nvdkdpsqrxglmxhwskoxskq5wlbwl7etp4viujafaq", 19208 },
	{ "launcher/api-ms-win-core-profile-l1-1-0.dll", "E7E0B18A40A35BD8B0766AC72253DE827432E148", "ipfs://bafkreie6tcydupfb5ov5z2363ham5nerfo3i5nuphyg7c7zzy6sv7lnddu", 18184 },
	{ "launcher/api-ms-win-core-rtlsupport-l1-1-0.dll", "24F37D46DFC0EF303EF04ABF9956241AF55D25C9", "ipfs://bafkreignyth6x6olvbnq2olzx363ewgb6lh4w6pn2ag2fx57hcoqqdsdpe", 19208 },
	{ "launcher/api-ms-win-core-string-l1-1-0.dll", "637E4A9946691F76E6DEB69BDC21C210921D6F07", "ipfs://bafkreiefjw35ecc4vlhyhvtbm5q5rpolvs2uubwjvgyxdmobufph3qijba", 18696 },
	{ "launcher/api-ms-win-core-synch-l1-1-0.dll", "5584C189216A17228CCA6CD07037AAA9A8603241", "ipfs://bafkreicqufkc2fvuf3ft5xa63ueic5cbohvff5yvlzjgtljzenhq5jur3y", 20744 },
	{ "launcher/api-ms-win-core-synch-l1-2-0.dll", "A9AEBBBB73B7B846B051325D7572F2398F5986EE", "ipfs://bafkreiczyfcuced7l4vzjo7ynbxp52dc2iarjpgjqkgsphphx5te24qrgi", 19208 },
	{ "launcher/api-ms-win-core-sysinfo-l1-1-0.dll", "F20AE25484A1C1B43748A1F0C422F48F092AD2C1", "ipfs://bafkreiedcdmfkompqpfvxhfdvxvtldnbgvcvplwfzawi56i2fh3zur7wea", 19720 },
	{ "launcher/api-ms-win-core-timezone-l1-1-0.dll", "4BF13DB65943E708690D6256D7DDD421CC1CC72B", "ipfs://bafkreici5nnveit3n62344glgqajzdngqnllmlz6ob63k2xzk4zyxkbhea", 19208 },
	{ "launcher/api-ms-win-core-util-l1-1-0.dll", "1E1A5AB47E4C2B3C32C81690B94954B7612BB493", "ipfs://bafkreifedpw6da72dryddazs226fj3ytqf5o5zwvfm5lichzl6stfoaj6e", 18696 },
	{ "launcher/api-ms-win-crt-conio-l1-1-0.dll", "49002B58CB0DF2EE8D868DEC335133CF225657DF", "ipfs://bafkreigvsyghgvxivol5bllxoohbrsaegpnco5tru3ujvfb4p5zfp7zwmm", 19720 },
	{ "launcher/api-ms-win-crt-convert-l1-1-0.dll", "C84E41FDCC4CA89A76AE683CB390A9B86500D3CA", "ipfs://bafkreiedm6hrqh2g7z37rl7arp6erlv3bnavjlkfwlx6tp5nzedtcp3aq4", 22792 },
	{ "launcher/api-ms-win-crt-environment-l1-1-0.dll", "9A4818897251CACB7FE1C6FE1BE3E854985186AD", "ipfs://bafkreihxy3d6uixn2l4l2b5klmz4xtugf3y5zxbce3vrgdqaddqc76i5ye", 19208 },
	{ "launcher/api-ms-win-crt-filesystem-l1-1-0.dll", "78FA03C89EA12FF93FA499C38673039CC2D55D40", "ipfs://bafkreidbamzcapjjvmqygwpcsfabx4erxmo3djwx5wmkxgt2tfbdqs4oe4", 20744 },
	{ "launcher/api-ms-win-crt-heap-l1-1-0.dll", "60B4CF246C5F414FC1CD12F506C41A1043D473EE", "ipfs://bafkreifpi6xl4bs26lyelim7edwh4vfg446aypu2keekmmevu4rsw5jydi", 19720 },
	{ "launcher/api-ms-win-crt-locale-l1-1-0.dll", "9C1DF49A8DBDC8496AC6057F886F5C17B2C39E3E", "ipfs://bafkreiftw3xjrlfbjt23zhz3y6exxqrzgs7yl7clyjnxkbx6jtm2ozyepi", 19208 },
	{ "launcher/api-ms-win-crt-math-l1-1-0.dll", "8B35EC4676BD96C2C4508DC5F98CA471B22DEED7", "ipfs://bafkreifcludfjxvqz2q254mjxiqxjuhrhppvf4ey2ou6ynwrlzf7wmgete", 27912 },
	{ "launcher/api-ms-win-crt-multibyte-l1-1-0.dll", "91EEF52C557AEFD0FDE27E8DF4E3C3B7F99862F2", "ipfs://bafkreig76jcedx4juaw54hgzqtsnhaqiiw5p37yqkrmo2egvcajgcfyrlm", 26888 },
	{ "launcher/api-ms-win-crt-private-l1-1-0.dll", "0C33CFE40EDD278A692C2E73E941184FD24286D9", "ipfs://bafkreifnxnsy3uol5swkptatek2rs5xtbm3mz4fhkhz3vupstu2qwgjmtq", 71432 },
	{ "launcher/api-ms-win-crt-process-l1-1-0.dll", "EC96F7BEEAEC14D3B6C437B97B4A18A365534B9B", "ipfs://bafkreih4kcrxvtbvgrojsncaillscksk5cfkkkujjtnd3s47nw2g3bjfla", 19720 },
	{ "launcher/api-ms-win-crt-runtime-l1-1-0.dll", "A19ACEFA3F95D1B565650FDBC40EF98C793358E9", "ipfs://bafkreibxdjckxelbjkgcnukzx24hfj5uh5ljznp2zcw2tgwotdzgji5vam", 23304 },
	{ "launcher/api-ms-win-crt-stdio-l1-1-0.dll", "982B5DA1C1F5B9D74AF6243885BCBA605D54DF8C", "ipfs://bafkreiabrhf5qtpkanlwhj7feis6b4nh3tweajzurbkbhlotes7742efo4", 24840 },
	{ "launcher/api-ms-win-crt-string-l1-1-0.dll", "7F389E6F2D6E5BEB2A3BAF622A0C0EA24BC4DE60", "ipfs://bafkreieikme6xbw4zwhcgs5alyj74c7vtkz5woeox67wwt6wcywy4kd6qi", 24840 },
	{ "launcher/api-ms-win-crt-time-l1-1-0.dll", "EE815A158BAACB357D9E074C0755B6F6C286B625", "ipfs://bafkreiaavqbnhg33czagquhafsskmea7ixlppnbzpte6a2psz2aaxbiaxe", 21256 },
	{ "launcher/api-ms-win-crt-utility-l1-1-0.dll", "EAA07829D012206AC55FB1AF5CC6A35F341D22BE", "ipfs://bafkreieyimdkgvd34l2ija6wrucgnmq53ko3jprqjpw4t763su6cnswfue", 19208 },
	{ "launcher/mtl_libovr.dll", "3AADE10DBF3C51233AA701AD1E12CD17A9DCB722", "ipfs://bafkreibl2mipn422ovvhqk7wxzbk6kl5kd63zkgops3lydrdkltqcnggpm", 190952 },
	{ "launcher/offline.pak", "C1EA3EF8587EC1BA896398912EA3BCA9D5C934B3", "ipfs://bafybeifmzqvwg45wjiwbxlxhzfkv52a4vtchiyeq6vau6bo56nusq7dtjq", 1726729 },
	{ "launcher/steam_api64.dll", "BD014660F7978A07BA2F99B6CF0621D678602663", "ipfs://bafkreiayablal4hj6yh2i44q475rosr254nlac2qrd4nhm2bt5l3hs26n4", 121256 },
	{ "launcher/ucrtbase.dll", "4189F4459C54E69C6D3155A82524BDA7549A75A6", "ipfs://bafybeibmbkp6yqvjdlejxl6sles7pemzemnah4cxyef2wf2e2ogul3w6je", 1016584 },
	{ "ros_2034/SocialClubD3D12Renderer.dll", "F72BA64FDA3AED8FBED7EF47A2E52112D811A3A2", "ipfs://bafybeibh6mvelnf4v2quqo7amt3tznedax3z4dajtbdaxct7e66vvtztvi", 413672 },
	{ "ros_2034/SocialClubHelper.exe", "18C57DF6CD51B0F583A026DA4A23035C6858353E", "ipfs://bafybeig6o5ez6hhkwcimldum7nh6a24allrehxe53fbxp7qinz6pouugnu", 2614760 },
	{ "ros_2034/SocialClubVulkanLayer.dll", "59796C13F22CF1A436DB3A878EFE1807B4ECC850", "ipfs://bafybeifzlg5znzyiziskh3ugcl7gusbr4ee7n5tywwyqlvoek44mrpq37m", 461800 },
	{ "ros_2034/SocialClubVulkanLayer.json", "5DA071BDE81BF96C8939978343C6B5B93730CB39", "ipfs://bafkreia462gkxii2ivjxi6ybhoyko24xn7tiv2yfzibv6qquoa6jq4fmsy", 339 },
	{ "ros_2034/cef.pak", "09F37AFFF84B2445F0AFA8CBB803D53BADA62080", "ipfs://bafybeiekeql7lci4l24or3il3cwphdj3vn7iwsvnd3ns3wlt6tfh7uy7gu", 3659767 },
	{ "ros_2034/cef_100_percent.pak", "22A96140286FDB004540A2051B93432AA133843D", "ipfs://bafybeiadr3qvz7ikdyt3dy4m6jdddl4lfcnbajysisrl53psiexffu3pbe", 737157 },
	{ "ros_2034/cef_200_percent.pak", "4BB734F61C04BFC68F7E15F128A2853A5F7649EA", "ipfs://bafybeia7xsiqzbkkso5yr3ske2lcqazda3kpsyxbxgibljxrbqneilbdqe", 864626 },
	{ "ros_2034/chrome_elf.dll", "1279FE4A98EBDAE23DF608119272A40C51BB3A9D", "ipfs://bafybeiex64e3al7zxifpptcx22oo7gsmu7expuk5lbsngtocvbkacnnlye", 805864 },
	{ "ros_2034/d3dcompiler_43.dll", "745AADA73AA1AB448C55FF681E875D592C593870", "ipfs://bafybeiav2xdcp4ttckhzawfghjd2e2wqqfi6f4wpktejkhtqyfvvambjxi", 2106344 },
	{ "ros_2034/d3dcompiler_47.dll", "5226E8C8A559B7D25CB2C2516498B413B01FAAD8", "ipfs://bafybeih2jyxdq67j5z2plkl6uf4nn3d2lrvnmux4p3mws56ukc5ho6ryx4", 4336616 },
	{ "ros_2034/icudtl.dat", "5CC62646E6C73B4BE276D08719BC5E257AF972BB", "ipfs://bafybeibrjnvo2pe3arhxymsln5gkda7uqcuoqovkwrgaft7okaeagurgxi", 10326688 },
	{ "ros_2034/libEGL.dll", "DAF2E612B503771FB07366749F65C46BA50E2561", "ipfs://bafkreicljoe7dgu6gm7bcg3ybi67nnbogpayz4yqbiluuzrows3cyt2yyu", 146408 },
	{ "ros_2034/libGLESv2.dll", "23ED37EC635B37A236EB70C4833114DD4AA18AD8", "ipfs://bafybeicr3wbpm7swlyvhvx6eqbzvyjvxkx6masiirteueocobr6cb4aw3a", 5431272 },
	{ "ros_2034/libcef.dll", "C7788FE47331F6E63C806734EB52A73F462CA33B", "ipfs://bafybeig3u3ghrakl2cnvz5ktza5dnf3vzja4udozr56y6v4yprygjpwzka", 109920232 },
	{ "ros_2034/locales/am.pak", "C38470C74EC5FA6C39C557A9AE1C62EA8C5949E7", "ipfs://bafybeicvbbw47hwdkzqxebciauo7btygv5sfp7rrciutf47fd4b3mhgk2a", 308569 },
	{ "ros_2034/locales/ar.pak", "488117B83394AC599F68D89EE6CB4AFDC101617D", "ipfs://bafybeih3dfdwfxjqkuyauqtnhbwf5yko2tgbq365skupcvvnrd5idv7hne", 309144 },
	{ "ros_2034/locales/bg.pak", "82BB3358F6404168A22618C666736D45B9652C15", "ipfs://bafybeia7ptzt2h4vc7wnado63pyepri3rmazimzhi4j7rjvukz3szmdx5q", 354995 },
	{ "ros_2034/locales/bn.pak", "CAEE7C690B81EE2A39D806598891BBE419FED0F0", "ipfs://bafybeigagugytussgw4ehtezffbui4vg2gi3fh4slhvhb6c7mvu3tgyh2q", 457090 },
	{ "ros_2034/locales/ca.pak", "4E5A93371D8E69306FF7104EC9E4BA9F1E658DE0", "ipfs://bafkreihv24fz2xpx6yx4virunpleaxuhfgoohve2sx5rwnxzi5ryt27bz4", 220145 },
	{ "ros_2034/locales/cs.pak", "8E28D1B3E2DCEE254DB0906470FE9EF061F6F301", "ipfs://bafkreidfknycg5zee23czhf7hhzcjzdq3h5c52bvmj5d5fral4q5x3xxc4", 223622 },
	{ "ros_2034/locales/da.pak", "C69E36ABB056EE8F3F79348ABAC3ACB4394A915D", "ipfs://bafkreiegk4lrvvszu7m3t4w6xsgtrmq5nitlfrrkl6uya2wv6hscfap6mm", 200457 },
	{ "ros_2034/locales/de.pak", "BE21B950998704AC0F6A4F8FE2DFE7831E4DB188", "ipfs://bafkreieup2lvkqdfw4at7cwndhfrad5ls6klb7a5a7saubkmqogql3whca", 218494 },
	{ "ros_2034/locales/el.pak", "1C27383402BF6E116DFE35275C10A258BFB2C1DC", "ipfs://bafybeifzzqrcmzt4wxrsgsxow6gljvpdawdqhsdax6zyhadd3vnpsepmhy", 388334 },
	{ "ros_2034/locales/en-GB.pak", "1FDFEE5FDC8E4DC1BF797773A4C7B381640E94DB", "ipfs://bafkreicvdhwrpog654ypiqocpb5j6ux4wpnwrlhc2ymvmlrw6yukulualq", 179898 },
	{ "ros_2034/locales/en-US.pak", "C250B53402E3CA81A5B15B4AE9EFBE374D0B40DC", "ipfs://bafkreifjvvs2fpabftbc57hkit7ufxqgkayeh56oo3gkxdw2um2fnuzz44", 181262 },
	{ "ros_2034/locales/es-419.pak", "AC9C6DDCAFDED36C1B7341D715D4D5AEC80EF6C4", "ipfs://bafkreietse7eymklz6uctlemaeb7rsi7piham4cv3aq6hcf7gnciy3tcwq", 216250 },
	{ "ros_2034/locales/es.pak", "7D68745DF63153295BA95362298A6C807E22FB8F", "ipfs://bafkreibaubhpb5bz6s3qyfydzi3lnw4opea4neunx65kumr3ulixluc5zm", 219649 },
	{ "ros_2034/locales/et.pak", "0D699B20763B1617466C767422AB165B59FFA44D", "ipfs://bafkreibihtzeajfucqefkeoyvdtl3xpklhxgey7ncf2rrzvfg7mvl2rngq", 196109 },
	{ "ros_2034/locales/fa.pak", "C3252469CB0704118DC87EFF963730786EDE9CF8", "ipfs://bafybeic3ae44eclkbnp3q5hdpbnjwp75qmbphsmlbq7uq24nn73ur47fzu", 311518 },
	{ "ros_2034/locales/fi.pak", "A8E758C20BE09FB539F70A0808F16088D75F9412", "ipfs://bafkreifxr6lxroilver3zcdgkcflbtjgjfs3c5kon6q2tmarlmtnrqb4le", 202474 },
	{ "ros_2034/locales/fil.pak", "849467DF01B5B214DFD5BDE4E6571DD68D368164", "ipfs://bafkreifgbha5he7eabruz2qjbpwhh2ksydjeqgbonululbeai35nz3l5tu", 222492 },
	{ "ros_2034/locales/fr.pak", "117E0FA329FD8C6CF2C50683112AA0AA9FB78E92", "ipfs://bafkreihfs75si47yhxse4rarhq5rwk5k6e2b5aluauuf6p7jmu74xexgbi", 234784 },
	{ "ros_2034/locales/gu.pak", "C0CC8D54E5E10011AEE01445C84FB9F0D5886976", "ipfs://bafybeihinkmjhchgublm4o73zigd3pys6h2ax22gumquleckuydnizu23y", 433963 },
	{ "ros_2034/locales/he.pak", "A3DE5BDE7F03F3CE7A49D8C9C3D222D4F1B92E33", "ipfs://bafybeih3sbhclegaroi5yy7wir5d6wkovce7hnerz2pix5fa7rbq6gkbje", 263008 },
	{ "ros_2034/locales/hi.pak", "C8E80B209AFF6C0672846E45A5CAD70E294372C4", "ipfs://bafybeiads6vsbdue5bg4eehdsjx6zyjhnfo5pehv3hnkagsuhvfvpi5a64", 444860 },
	{ "ros_2034/locales/hr.pak", "340B0FE0BFAD72027B5C4851AD0CDD1E88DB8F2E", "ipfs://bafkreiaguvvemvizbms3ihj5khooqdiqmd7ghxpfy7b4bbmln77la354gu", 212148 },
	{ "ros_2034/locales/hu.pak", "AF775FBF31FE24FF03D7F20E54A13EBB704B1243", "ipfs://bafkreick6coqn3md2okliyli7j6yckwhmxd43ruuqp2bxjnznes4y2dqeu", 230497 },
	{ "ros_2034/locales/id.pak", "48EB5000362567230AECE13533A20201BC7E9DA5", "ipfs://bafkreigqrj2rs5rlismb23mmz276wlltn355yjfnh7655uk5p35eipjmry", 193889 },
	{ "ros_2034/locales/it.pak", "1AA31C84E57DAFC025C7101FF67686F5CA4C1427", "ipfs://bafkreiadvsgqnaz64hk754il3zpyrzncxtazc3urnek2wj4ifnzrnt3vn4", 212034 },
	{ "ros_2034/locales/ja.pak", "F5E8088157758222A5F12D92468E3278379ACC9D", "ipfs://bafybeibs2m2dvc3tk72foc3jyjkkxy2fswvsif33jb6owusqtgkjnmurlu", 263635 },
	{ "ros_2034/locales/kn.pak", "A404D241DC5C878EE984B9A020868C3B3A657D48", "ipfs://bafybeibnlk3kpj3fntoszsnkiwdv3owhu7tkd3ol2r2uxso2nlhk3bj65u", 506652 },
	{ "ros_2034/locales/ko.pak", "CCF76DE8F9B8123534FDBC295EB611084827AE44", "ipfs://bafkreiadwbggxo5q44fjy7hpno3x5zvxm2crpagfwguxaniwev63j4anuq", 221281 },
	{ "ros_2034/locales/lt.pak", "5BF289E0B7FBF3BA76C458CDEE3DA37AE8916FC6", "ipfs://bafkreigiqxeqb7mey3mj3azgcg33et2nroezpjpg2vqyhzftbtlddmrjyu", 227745 },
	{ "ros_2034/locales/lv.pak", "C99AE49A35A91CB65DEC70C7D047317E7A4990B1", "ipfs://bafkreid3e5w2wfpx5wmk2qbhxied6d2yyq3d2jjgyp3syta7osb6unrad4", 226576 },
	{ "ros_2034/locales/ml.pak", "AF5EE2F04A369FF547A302FC25D8A83625B4834A", "ipfs://bafybeiaqvw4y5uf36wwkl2d5sftmnm7w4mwflgtsihepsnlogtzgpgrcvm", 545408 },
	{ "ros_2034/locales/mr.pak", "F5B0DAA6B06102EB7CB8E0D3A2B8A7CC8E4EB3B5", "ipfs://bafybeififhgqvz5clguryne5ulgb6xckpoi6d64r4d5icgg2yng6sqjvcy", 440783 },
	{ "ros_2034/locales/ms.pak", "F321C7D05192A568BFA9A3F1F2E1E4F990CCB0FD", "ipfs://bafkreiczar27fjtqbp3rv7drzl6p5d4krnw23oluph2fdtnzumo4idog3y", 200631 },
	{ "ros_2034/locales/nb.pak", "11447469B1E2EA31E5E41175EACBF2688CFDCBFC", "ipfs://bafkreibxmbhvvg7kuekldnzkstmgrfd3htgaoxhry3gpx5jhdhfzmy6g24", 197834 },
	{ "ros_2034/locales/nl.pak", "3CF49D1A6C4EAD4DDDE1BDB7B47F27D396DEA174", "ipfs://bafkreics2pbxo4umlmpaj33wbz7xhynjjdzz4v2xddnkdmwm5qdluzvc6e", 207112 },
	{ "ros_2034/locales/pl.pak", "3B458BE93A629DD21412C90261C6E0FC75884A01", "ipfs://bafkreiagsll5ccgh5eb3j7d2p4tz4ido7ugabv5yq3aisnqkurfevc5dkq", 219794 },
	{ "ros_2034/locales/pt-BR.pak", "6D800996B7AED2CCFD61F862F5972ECD863F301D", "ipfs://bafkreiepipvufq4citasp7zx56l6enhanyhnc3qedn7kpf2tel3mx7iyqq", 213275 },
	{ "ros_2034/locales/pt-PT.pak", "F9510A1E9EB748A1DA710568CFA7E8E7572C8F9D", "ipfs://bafkreig66wvytboyg73eso5enk2xuld5cbjzn5ta4c3md5dixptlmbmp2y", 215681 },
	{ "ros_2034/locales/ro.pak", "3715F4B689B36371BE57F8B3B428463B04E3D589", "ipfs://bafkreie77wdxmymbx4jqf4f4rsccnyshvj5my5ypfxt7slulesfl47cwge", 220783 },
	{ "ros_2034/locales/ru.pak", "B5E70099AB9221C7B48172615E90EEC4651BA962", "ipfs://bafybeiefjh3m6xvtywvrwrc5l7ojxd5oz6vez2cvwltkcr3udcv6tsgzuy", 342198 },
	{ "ros_2034/locales/sk.pak", "5B1145F2AFF82A48ADFC8649468C738729BE11F5", "ipfs://bafkreihxx4wie5rqb2u3urh24zcpmktg6wjgpk3zzanurwxv7airvh4iju", 227524 },
	{ "ros_2034/locales/sl.pak", "2BF6609859614FFFF59835CC646F312616CA3221", "ipfs://bafkreifseja5c7ymrerzn4zgh4v63vcembqevhko5g5jlxj24u74hvb5ga", 213458 },
	{ "ros_2034/locales/sr.pak", "DE8301191E06A92FDA4B6AA0173D13144BC9D201", "ipfs://bafybeibnn4vtpbo33ixd3ts4g23dque22pvlonailtgfqrobbxwatairbu", 331863 },
	{ "ros_2034/locales/sv.pak", "59BCD61F90A572EE04B653E380F7FB037558EDF7", "ipfs://bafkreidosobnmnulvjfzogvxnryz4lmcgwbxeuqy4xzjd45ox5w2umyazi", 198993 },
	{ "ros_2034/locales/sw.pak", "E227032C65FD15F95134D5737D6A82153D64F88E", "ipfs://bafkreib6lco4zla4em2lqxj7c53sf7lprweizhsepeizrgvt2b57evs7um", 203901 },
	{ "ros_2034/locales/ta.pak", "C55A45300374620C318931CB57AD40A765C310E3", "ipfs://bafybeickcdlzuhkf3ku7a7wsxo54qqhgggfdhfufjd7qtzfzun5otuiin4", 513725 },
	{ "ros_2034/locales/te.pak", "C5F840C16BBB8D881CBBE6DC5ECE0224A2B233FA", "ipfs://bafybeicviijj6stuh47fkwfkhqrulvvqiu73kfqtkippfldoqqbplockiu", 488193 },
	{ "ros_2034/locales/th.pak", "77115AFC2AACCEAF7E06BFFA204636F5D43896DE", "ipfs://bafybeievyx7hsppxdncdbdmppgm2or4et5cxpu2hy7cspl6av66jqs4ad4", 414114 },
	{ "ros_2034/locales/tr.pak", "263531664B606FE910BA60E34D2FA588EEA940C6", "ipfs://bafkreibmjiczy3gick3eaijentsuutkz2x2oqm6jsilpyy23m56jccwene", 212733 },
	{ "ros_2034/locales/uk.pak", "D4E839D8839A4E974D0AD0481516CDADB20CC22F", "ipfs://bafybeiekqgklueejjqde2idrqcgomz3etldcczkmhdtudpdknqanfah5g4", 346713 },
	{ "ros_2034/locales/vi.pak", "816C6A6AAB8F0FD3262E55EACB26EA7F02886103", "ipfs://bafkreicju7crd34dkr5t5hhxzguyk4rzxtxlhtq62fg4rktbo7457vejve", 245342 },
	{ "ros_2034/locales/zh-CN.pak", "68887DDD4DCB4AB75A1A4FA7148CE7EECB3B1524", "ipfs://bafkreia2keaomodaj2v4hnr4xlor2m7svhd3vkkwsgklk32z6a7aoebbom", 184050 },
	{ "ros_2034/locales/zh-TW.pak", "7B77CAA9C113E815950835EA543F1C46638CB62A", "ipfs://bafkreifarz2moorsoymbat6xxzc34y2z6dxific6uhon6i67hasushldqi", 184212 },
	{ "ros_2034/natives_blob.bin", "9D60E499A907811A3155E9A07F8645D6C83CB909", "ipfs://bafkreia4vyqcvwqbnt2flk7wtvmdkjfb2n5bg4nnj367vrf25ud4mqbl3u", 83328 },
	{ "ros_2034/scui.pak", "BB2E5201A8CD16EDDE31D4E39257640A956F35E4", "ipfs://bafybeicmyipjgtb6njbkir6dftzts6e536b76y3b5mouklrwyq5z7rhcgu", 6028093 },
	{ "ros_2034/snapshot_blob.bin", "49038CDCB26B4DCC37F017787265180478E894B5", "ipfs://bafybeifkqj3yt6gpu73bnqes5gzmtptgnm7tksywjnjuzodhplfc7y45vi", 287384 },
	{ "ros_2034/socialclub.dll", "8CB5C91391FECBF87EA4B0D1E8000CCFC046831D", "ipfs://bafybeibddmfwo6h6syl3yd7ryp4se2mjjob2beiecsbn6rm2zhmjzstcae", 5179880 },
	{ "ros_2034/swiftshader/libEGL.dll", "B20AD83B8D44E5A0AE98A4A430C9179D6D04CB27", "ipfs://bafkreigmtgbepae5cbkdvdourqdw4iwtgz3bvsj5m6d66ph6veimbevosq", 155112 },
	{ "ros_2034/swiftshader/libGLESv2.dll", "DDB924538652AAED98AA9F4CE8268658C2D5BC15", "ipfs://bafybeic2cbgzh4snob6bueuezexwqzyo3ymnbydos7kcx3dt5k7v5ejav4", 2686440 },
	{ "ros_2034/uninstallRGSCRedistributable.exe", "5C7B092135788F36930C4FFE445830D55573E3E2", "ipfs://bafybeietxbzwoybysynm5uv7ofpahuyhxdvlneqrjcwctzv2nuusmdw2ii", 396664 },
	{ "ros_2034/v8_context_snapshot.bin", "1834A6FF2B69121D01DA29EB1CB82ED29F493AE2", "ipfs://bafybeigttpnua6jtrksibuzt27xgbix6kvcdsjf6as2pfqkc5icrsbfywq", 688952 },

	{ "launcher/LauncherPatcher.exe", "1C6BCE6CDB4B2E1766A67F931A72519CEFF6AEB1", "", "", 0, 0 },
	{ "launcher/index.bin", "85e2cc75d6d07518883ce5d377d3425b74636667", "", "", 0, 0 },

#if defined(GTA_FIVE)
	{ "GTAVLauncher.exe", "0962fb5a39d9ec40d4fce2ac636ef11dcd0457cf", "http://patches.rockstargames.com/prod/gtav/Launcher_EFIGS/GTA_V_Launcher_1_0_1604_0.exe", "$/GTAVLauncher.exe", 21606840, 20081224 },
#endif
};

static bool ParseCacheFileName(const char* inString, std::string& fileNameOut, std::string& hashOut)
{
	// check if the file name meets the minimum length for there to be a hash
	int length = strlen(inString);

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

template<int Size>
static std::array<uint8_t, Size> ParseHexString(const char* string)
{
	std::array<uint8_t, Size> retval;

	assert(strlen(string) == Size * 2);

	for (int i = 0; i < Size; i++)
	{
		const char* startIndex = &string[i * 2];
		char byte[3] = { startIndex[0], startIndex[1], 0 };

		retval[i] = strtoul(byte, nullptr, 16);
	}

	return retval;
}

static std::vector<GameCacheStorageEntry> LoadCacheStorage()
{
	// create the cache directory if needed
	CreateDirectory(MakeRelativeCitPath(L"cache").c_str(), nullptr);
	CreateDirectory(MakeRelativeCitPath(L"cache\\game").c_str(), nullptr);

	// output buffer
	std::vector<GameCacheStorageEntry> cacheStorage;

	// iterate over files in cache
	WIN32_FIND_DATA findData;

	HANDLE hFind = FindFirstFile(MakeRelativeCitPath(L"cache\\game\\*.*").c_str(), &findData);

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
		if (FILE* f = _wfopen(MakeRelativeCitPath(L"cache\\game\\cache.dat").c_str(), L"rb"))
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
						// as it doesn't add to the list
						retval.push_back(entry);
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

static bool ShowDownloadNotification(const std::vector<std::pair<GameCacheEntry, bool>>& entries)
{
	// iterate over the entries
	std::wstringstream detailStr;
	size_t localSize = 0;
	size_t remoteSize = 0;

	bool shouldAllow = true;

	for (auto& entry : entries)
	{
		// is the file allowed?
		if (_strnicmp(entry.first.remotePath, "nope:", 5) == 0)
		{
			shouldAllow = false;
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
	taskDialogConfig.pszWindowTitle = PRODUCT_NAME L": Game cache outdated";
	taskDialogConfig.pszMainIcon = TD_INFORMATION_ICON;
	taskDialogConfig.pszMainInstruction = PRODUCT_NAME L" needs to update the game cache";

	if (shouldAllow)
	{
		taskDialogConfig.pszContent = va(L"The local " PRODUCT_NAME L" game cache is outdated, and needs to be updated. This will copy %.2f MB of data from the local disk, and download %.2f MB of data from the internet.\nDo you wish to continue?", (localSize / 1024.0 / 1024.0), (remoteSize / 1024.0 / 1024.0));
	}
	else
	{
		const TASKDIALOG_BUTTON buttons[] = {
			{ 42, L"Close" }
		};

		taskDialogConfig.pszContent = va(L"DLC files are missing (or corrupted) in your game installation. Please update or verify the game using Steam or the Social Club launcher and try again. See http://rsg.ms/verify step 4 for more info.");

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

	TaskDialogIndirect(&taskDialogConfig, &outButton, nullptr, nullptr);

	return (outButton != IDNO && outButton != 42);
}

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

	for (auto& entry : entries)
	{
		// check if the file is outdated
		std::vector<std::array<uint8_t, 20>> hashes;

		for (auto& checksum : entry.checksums)
		{
			hashes.push_back(ParseHexString<20>(checksum));
		}
		
		std::array<uint8_t, 20> outHash;
		bool fileOutdated = false;
		
		if (_strnicmp(entry.remotePath, "nope:", 5) != 0)
		{
			UI_UpdateText(0, L"Verifying game content...");

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
				if (FILE* f = _wfopen(MakeRelativeCitPath(L"cache\\game\\cache.dat").c_str(), L"ab"))
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
				CL_QueueDownload(va("file:///%s", converter.to_bytes(entry.GetLocalFileName()).c_str()), converter.to_bytes(entry.GetCacheFileName()).c_str(), entry.localSize, false);

				notificationEntries.push_back({ entry, true });
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
				CL_QueueDownload(remotePath, localFileName.c_str(), entry.remoteSize, false);

				referencedFiles.insert(entry.remotePath);

				notificationEntries.push_back({ entry, false });
			}

			// if we want an archived file from here, we should *likely* note its existence
			extractedEntries.push_back(entry);
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

	UI_UpdateText(0, L"Updating game cache...");

	bool retval = DL_RunLoop();

	// if succeeded, try extracting any entries
	if (retval)
	{
		// sort extracted entries by 'archive' they belong to
		std::sort(extractedEntries.begin(), extractedEntries.end(), [] (const auto& left, const auto& right)
		{
			return strcmp(left.remotePath, right.remotePath) < 0;
		});

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
				if (strlen(entry.remotePath) > 0 && entry.archivedFile && strlen(entry.archivedFile) > 0 && lastArchive != entry.remotePath)
				{
					// process each entry
					//retval = retval && ExtractInstallerFile(entry.GetRemoteBaseName(), entry.archivedFile, entry.GetCacheFileName());
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
						std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

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
								if (_wcsicmp(converter.from_bytes(dlEntry.archivedFile).c_str(), fileName.c_str()) == 0)
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
						if (FILE* f = _wfopen(MakeRelativeCitPath(L"cache\\game\\cache.dat").c_str(), L"ab"))
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

extern void StartIPFS();

std::map<std::string, std::string> UpdateGameCache()
{
	std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");

	bool ipfsPeer = true;

	if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		ipfsPeer = (GetPrivateProfileInt(L"Game", L"DisableIPFSPeer", 0, fpath.c_str()) != 1);
	}

	static HostSharedData<CfxState> initState("CfxInitState");
	
	if (ipfsPeer && initState->IsMasterProcess())
	{
		StartIPFS();
	}

	// delete bad migration on 2019-01-10 (incorrect update.rpf download URL caused Steam users to fetch 1493.1 instead of 1604.0)
	{
		auto dataPath = MakeRelativeCitPath(L"cache\\game\\cache.dat");
		auto failPath = MakeRelativeCitPath(L"cache\\game\\update+update.rpf_fc941d698834e30e40a06a40f6a35b1b18e1c50c");

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

	// perform a game update
	auto differences = CompareCacheDifferences();

	if (!differences.empty())
	{
		if (!PerformUpdate(differences))
		{
			return {};
		}
	}

	// get a list of cache files that should be mapped given an updated cache
	std::map<std::string, std::string> retval;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

	for (auto& entry : g_requiredEntries)
	{
		std::string origFileName = entry.filename;

		if (origFileName.find("ros_2034/") == 0)
		{
			origFileName = "Social Club/" + origFileName.substr(9);
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
