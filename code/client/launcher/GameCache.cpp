/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include "InstallerExtraction.h"
#include <array>

#include <Error.h>

#ifdef GTA_FIVE
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
	std::wstring GetCacheFileName() const
	{
		std::string filenameBase = filename;

		if (filenameBase.find("ros_1238/") == 0)
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
	{ "GTA5.exe", "a50ebaa7bf8dcf4a29ff6630e313f1e73770c855", "https://runtime.fivem.net/patches/GTA_V_Patch_1_0_1365_1.exe", "$/GTA5.exe", 68521904, 942456720 },
	{ "update/update.rpf", "8b5f4c3f76820fe63f6ba909c1898c9fc505a4aa", "https://runtime.fivem.net/patches/GTA_V_Patch_1_0_1365_1.exe", "$/update/update.rpf", 882171904, 942456720 },
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
	{ "update/x64/dlcpacks/mpxmas_604490/dlc.rpf", "929e5b79c9915f40f212f1ed9f9783f558242c3d", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpxmas_604490/dlc.rpf", 46063616 },
	{ "update/x64/dlcpacks/mpapartment/dlc.rpf", "e1bed90e750848407f6afbe1db21aa3691bf9d82", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpapartment/dlc.rpf", 636985344 },
	{ "update/x64/dlcpacks/patchday8ng/dlc.rpf", "2f9840c20c9a93b48cfcf61e07cf17c684858e36", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday8ng/dlc.rpf", 365330432 },

	//617
	{ "update/x64/dlcpacks/mpjanuary2016/dlc.rpf", "4f0d5fa835254eb918716857a47e8ce63e158c22", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpjanuary2016/dlc.rpf", 149417984 },
	{ "update/x64/dlcpacks/mpvalentines2/dlc.rpf", "b1ef3b0e4741978b5b04c54c6eca8b475681469a", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpvalentines2/dlc.rpf", 25075712 },

	//678
	{ "update/x64/dlcpacks/mplowrider2/dlc.rpf", "6b9ac7b7b35b56208541692cf544788d35a84c82", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmplowrider2/dlc.rpf", 334028800 },
	{ "update/x64/dlcpacks/patchday9ng/dlc.rpf", "e29c191561d8fa4988a71be7be5ca9c6e1335537", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday9ng/dlc.rpf", 160526336 },

	//757
	{ "update/x64/dlcpacks/mpexecutive/dlc.rpf", "3fa67dd4005993c9a7a66879d9f244a55fea95e9", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpexecutive/dlc.rpf", 801570816 },
	{ "update/x64/dlcpacks/patchday10ng/dlc.rpf", "4140c1f56fd29b0364be42a11fcbccd9e345d6ff", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday10ng/dlc.rpf", 94134272 },

	//791 Cunning Stunts
	{ "update/x64/dlcpacks/mpstunt/dlc.rpf", "c5d338068f72685523a49fddfd431a18c4628f61", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpstunt/dlc.rpf", 348049408 },
	{ "update/x64/dlcpacks/patchday11ng/dlc.rpf", "7941a22c6238c065f06ff667664c368b6dc10711", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday11ng/dlc.rpf", 9957376 },

	//877 Bikers
	{ "update/x64/dlcpacks/mpbiker/dlc.rpf", "52c48252eeed97e9a30efeabbc6623c67566c237", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfmpbiker/dlc.rpf", 1794048000 },
	{ "update/x64/dlcpacks/patchday12ng/dlc.rpf", "4f3f3e88d4f01760648057c56fb109e1fbeb116a", "nope:https://runtime.fivem.net/patches/dlcpacks/patchday4ng/dlc.rpfpatchday12ng/dlc.rpf", 155365376 },

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

	{ "ros_1238/cef.pak", "C308AC7D34F80FE9486716FBF4C713B55924B661", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/cef.pak", 3866710, 85694480 },
	{ "ros_1238/cef_100_percent.pak", "66EF9A8559E1A8F7976F95FDD2355A0D0E101532", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/cef_100_percent.pak", 647967, 85694480 },
	{ "ros_1238/cef_200_percent.pak", "0E65DCF353901A8F5D4309585337F6465C3DFC78", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/cef_200_percent.pak", 753127, 85694480 },
	{ "ros_1238/chrome_elf.dll", "0B4217D47BBC9BD0384BFBC53B5EA327C66435E4", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/chrome_elf.dll", 547304, 85694480 },
	{ "ros_1238/d3dcompiler_43.dll", "A9BC4C3DD06E852A1CC616C850D3F7960A15680E", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/d3dcompiler_43.dll", 2106344, 85694480 },
	{ "ros_1238/d3dcompiler_47.dll", "74F12BAF1075693685501B2F7651B2782588E7F2", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/d3dcompiler_47.dll", 4300776, 85694480 },
	{ "ros_1238/icudtl.dat", "C35D6E4DB540518263214697F589C54FAAC87533", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/icudtl.dat", 10171360, 85694480 },
	{ "ros_1238/libcef.dll", "DEB6C0393ED3FF34E6ED3828826F4C7825DDDA16", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/libcef.dll", 94052328, 85694480 },
	{ "ros_1238/libEGL.dll", "1AF9912BAB68710CE062652AA44EA5E7F2A150E9", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/libEGL.dll", 98792, 85694480 },
	{ "ros_1238/libGLESv2.dll", "6AF4BE0CA9FB99A88E798EAC3C72F04C117751F0", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/libGLESv2.dll", 4434920, 85694480 },
	{ "ros_1238/natives_blob.bin", "7BCA7FCF54CC81CAFBAEEF44EF23DAE4A7C97619", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/natives_blob.bin", 218121, 85694480 },
	{ "ros_1238/scui.pak", "D9F9DA62C0EBF0BA35AF083B78608236332787AC", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/scui.pak", 6171598, 85694480 },
	{ "ros_1238/snapshot_blob.bin", "0EC391456619FC5BCA046A23BADCF151D72D1B35", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/snapshot_blob.bin", 1362716, 85694480 },
	{ "ros_1238/socialclub.dll", "742732F9237E088DF92B24272942AB366F4A561E", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/socialclub.dll", 8512488, 85694480 },
	{ "ros_1238/subprocess.exe", "9BA0627D1CFA1595A796FBBA41D0E55B9E38FDF9", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/subprocess.exe", 1090024, 85694480 },
	{ "ros_1238/v8_context_snapshot.bin", "9A66D10886CC99BFD7E32C2178894717F33664EE", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/v8_context_snapshot.bin", 1772488, 85694480 },
	{ "ros_1238/widevinecdmadapter.dll", "6252B95BF3B078645EFCB1DC54C2743B87C666FF", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/widevinecdmadapter.dll", 286696, 85694480 },
	{ "ros_1238/locales/am.pak", "780E74DA7D6299BA8D8278CB42685287BFE9D1DE", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/am.pak", 288192, 85694480 },
	{ "ros_1238/locales/ar.pak", "0E34AE587EAC54F8535C80EAC6F8D0C73CF8ADD3", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/ar.pak", 283698, 85694480 },
	{ "ros_1238/locales/bg.pak", "8EBA4746AF8927CB75390CD6EBEE59E027B4636E", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/bg.pak", 329621, 85694480 },
	{ "ros_1238/locales/bn.pak", "AD5A2E603C97D9B48DE47DCA188315D0790765DE", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/bn.pak", 430606, 85694480 },
	{ "ros_1238/locales/ca.pak", "97EF23AF64A67C8A444E1983D600B4D610F03D34", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/ca.pak", 204954, 85694480 },
	{ "ros_1238/locales/cs.pak", "9ADAE19B1982968EE0650AD5DCDED1E625632B14", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/cs.pak", 208304, 85694480 },
	{ "ros_1238/locales/da.pak", "463ECE138DACBD6835F72A84EABAA57E20E99DDF", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/da.pak", 187208, 85694480 },
	{ "ros_1238/locales/de.pak", "60FE88E7AA2E52A57C6F679F235943DF943E3263", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/de.pak", 203794, 85694480 },
	{ "ros_1238/locales/el.pak", "E3E89A5C4729F20916F40DCCC13D6B8C4AF4C6DA", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/el.pak", 362601, 85694480 },
	{ "ros_1238/locales/en-GB.pak", "249E3084DD6D148F2487CE9E1A48B7FAE97FDF0E", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/en-GB.pak", 168518, 85694480 },
	{ "ros_1238/locales/en-US.pak", "5DE10AAD0EFA91043E80C2397375DF52AD825545", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/en-US.pak", 169455, 85694480 },
	{ "ros_1238/locales/es-419.pak", "5323B07879FCEFE5A0C4924A83DC811F7B6DF6D7", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/es-419.pak", 201890, 85694480 },
	{ "ros_1238/locales/es.pak", "17D3C5F66B050D4A7B8D798647DB60D64E7E25FB", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/es.pak", 205495, 85694480 },
	{ "ros_1238/locales/et.pak", "58514249041B33360A8DD0D21A7DABE921E7DC3C", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/et.pak", 182828, 85694480 },
	{ "ros_1238/locales/fa.pak", "ADAD87081BD3208868B98B5BEE32CB097EBCEBFB", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/fa.pak", 289551, 85694480 },
	{ "ros_1238/locales/fi.pak", "AC64415B900EDC0F1FECE2638E19E1B1E104F5CB", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/fi.pak", 189663, 85694480 },
	{ "ros_1238/locales/fil.pak", "D619F2F3ED06DCA54AC02BC6410A291B78A3CD42", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/fil.pak", 208302, 85694480 },
	{ "ros_1238/locales/fr.pak", "B44BDC854769EFF7755055470D4B743968281F04", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/fr.pak", 219692, 85694480 },
	{ "ros_1238/locales/gu.pak", "22D4631FF1D32F332F1133E809E774E1A8D29EB3", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/gu.pak", 408074, 85694480 },
	{ "ros_1238/locales/he.pak", "581690EBCAF2137839212411FBA5B44207153BD2", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/he.pak", 241853, 85694480 },
	{ "ros_1238/locales/hi.pak", "BD9998EC61BD9F9FD48E890DCF323D9409772AC5", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/hi.pak", 415545, 85694480 },
	{ "ros_1238/locales/hr.pak", "C41F1E90E9313B13D1BCE7055354596992935CD5", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/hr.pak", 196271, 85694480 },
	{ "ros_1238/locales/hu.pak", "272AD9767C2DC35938C8104D7AB5D40BCEACF933", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/hu.pak", 215127, 85694480 },
	{ "ros_1238/locales/id.pak", "32EF210DAAEE35E675525CE50A91F0EC303EB8FF", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/id.pak", 181124, 85694480 },
	{ "ros_1238/locales/it.pak", "C26951918D1A44CC0F518EEF3C2E7C91BC4AFB23", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/it.pak", 198516, 85694480 },
	{ "ros_1238/locales/ja.pak", "74D460ACEC0C1791824D0CF9470B76D2771F1CF8", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/ja.pak", 246094, 85694480 },
	{ "ros_1238/locales/kn.pak", "5E8AE3EEBA26548897D10551DB651F753911B179", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/kn.pak", 471779, 85694480 },
	{ "ros_1238/locales/ko.pak", "FD5072F271E90A9DBF4637C0C56B3B0F60678C41", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/ko.pak", 207438, 85694480 },
	{ "ros_1238/locales/lt.pak", "61779BFDEDE897F89C2618E489C3E57600046FB2", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/lt.pak", 211038, 85694480 },
	{ "ros_1238/locales/lv.pak", "7383BDF2105F822923483721BFAD83CF721D9236", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/lv.pak", 210459, 85694480 },
	{ "ros_1238/locales/ml.pak", "6177F6C89BF61827217323EF2056B6A5B9772A5A", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/ml.pak", 512370, 85694480 },
	{ "ros_1238/locales/mr.pak", "A6592A51E543527BD68CD4770260EDF22E8828A0", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/mr.pak", 409172, 85694480 },
	{ "ros_1238/locales/ms.pak", "06E8900F5BB63FBF8044091CAAA95AE8ED5FC15C", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/ms.pak", 187796, 85694480 },
	{ "ros_1238/locales/nb.pak", "56258B92C4B1E4032F7C45DAAEFC64DFD300D426", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/nb.pak", 184630, 85694480 },
	{ "ros_1238/locales/nl.pak", "1C64BE844AEF9059234F201B1FF4401ABC4C18AA", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/nl.pak", 193431, 85694480 },
	{ "ros_1238/locales/pl.pak", "887A8304EAFDEA125E45D83F624C16EBBF772F97", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/pl.pak", 203748, 85694480 },
	{ "ros_1238/locales/pt-BR.pak", "2CB78986FD9DD8F8A9D50C90E860F28B9F052F2F", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/pt-BR.pak", 199224, 85694480 },
	{ "ros_1238/locales/pt-PT.pak", "13E7F9FCC875ECA59F49B9B87CDF1FD07A649D29", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/pt-PT.pak", 200771, 85694480 },
	{ "ros_1238/locales/ro.pak", "B896211961B3F0F53ED64C0D4DB7E7F00E5D3BFF", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/ro.pak", 206682, 85694480 },
	{ "ros_1238/locales/ru.pak", "9B9E82643E7669D3F50C43FB8C4BD85AFD7833DB", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/ru.pak", 313752, 85694480 },
	{ "ros_1238/locales/sk.pak", "8BF7EE156BCFFBDAB6A10285F518441E3E2533A6", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/sk.pak", 212466, 85694480 },
	{ "ros_1238/locales/sl.pak", "4C578336BAF8A662CB2951C8274201C9DCBE498E", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/sl.pak", 196260, 85694480 },
	{ "ros_1238/locales/sr.pak", "460513B76BA9845F3E580EFD71922E07EADC9DD3", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/sr.pak", 307758, 85694480 },
	{ "ros_1238/locales/sv.pak", "05125215C5451A7471BA7972BF18F99D3925C1CA", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/sv.pak", 185484, 85694480 },
	{ "ros_1238/locales/sw.pak", "1EA9F306A47DB04637D501A0FDBD0EDA5F84C80E", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/sw.pak", 189595, 85694480 },
	{ "ros_1238/locales/ta.pak", "1C08DAA3F94E6DC72B9B275218F3DC8D496760B6", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/ta.pak", 478276, 85694480 },
	{ "ros_1238/locales/te.pak", "BEE2FDC9533FA8552A73AB760132714338E9F84F", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/te.pak", 453971, 85694480 },
	{ "ros_1238/locales/th.pak", "0BB6902FEA12BB6A5496AB0CD2D71B70DD26D5A2", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/th.pak", 389366, 85694480 },
	{ "ros_1238/locales/tr.pak", "6CCC0C8B430D14335707947B897835D0529C436B", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/tr.pak", 200055, 85694480 },
	{ "ros_1238/locales/uk.pak", "43092BD4966E2C7F2A0A3425456C61704A1BD398", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/uk.pak", 322593, 85694480 },
	{ "ros_1238/locales/vi.pak", "CAB3084DB7531EAB27DD8A804780E7EF83D16B6C", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/vi.pak", 228436, 85694480 },
	{ "ros_1238/locales/zh-CN.pak", "BC70FAC761065BED92E317946F6A4AF5879ECDA8", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/zh-CN.pak", 171281, 85694480 },
	{ "ros_1238/locales/zh-TW.pak", "490DEE3F6B8B04BAF930803A8C34F51A508B5F71", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/locales/zh-TW.pak", 171913, 85694480 },
	{ "ros_1238/swiftshader/libEGL.dll", "4BD9A9AC4EE6573DE983BE470BD97A7F8A40BEEC", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/swiftshader/libEGL.dll", 139240, 85694480 },
	{ "ros_1238/swiftshader/libGLESv2.dll", "DFCA1EA0642EE0AF2ED205CD2BA4088907809225", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.8-Setup.exe", "$/swiftshader/libGLESv2.dll", 3019240, 85694480 },

	{ "GTAVLauncher.exe", "91a10672836802c9e3ffbb8e9348b11292a31a04", "http://patches.rockstargames.com/prod/gtav/Launcher_EFIGS/GTA_V_Launcher_1_0_1365_1.exe", "$/GTAVLauncher.exe", 21271984, 19787536 }
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
	const char* underscorePos = strchr(inString, '_');

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
			retval.push_back(entry);
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
	taskDialogConfig.pszWindowTitle = L"FiveM: Game cache outdated";
	taskDialogConfig.pszMainIcon = TD_INFORMATION_ICON;
	taskDialogConfig.pszMainInstruction = L"FiveM needs to update the game cache";

	if (shouldAllow)
	{
		taskDialogConfig.pszContent = va(L"The local FiveM game cache is outdated, and needs to be updated. This will copy %.2f MB of data from the local disk, and download %.2f MB of data from the internet.\nDo you wish to continue?", (localSize / 1024.0 / 1024.0), (remoteSize / 1024.0 / 1024.0));
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

static void PerformUpdate(const std::vector<GameCacheEntry>& entries)
{
	// create UI
	UI_DoCreation();

	// hash local files for those that *do* exist, add those that don't match to the download queue and add those that do match to be copied locally
	std::set<std::string> referencedFiles; // remote URLs that we already requested
	std::vector<GameCacheEntry> extractedEntries; // entries to extract from an archive

	// entries for notification purposes
	std::vector<std::pair<GameCacheEntry, bool>> notificationEntries;

	for (auto& entry : entries)
	{
		// check if the file is outdated
		std::vector<std::array<uint8_t, 20>> hashes;

		for (auto& checksum : entry.checksums)
		{
			hashes.push_back(ParseHexString<20>(checksum));
		}

		std::array<uint8_t, 20> outHash;
		bool fileOutdated = CheckFileOutdatedWithUI(entry.GetLocalFileName().c_str(), hashes, &outHash);

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

				if (_strnicmp(remotePath, "http", 4) != 0)
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

			ExitProcess(0);
		}
	}
	else
	{
		return;
	}

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
				if (lastArchive != entry.remotePath)
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
		FatalError("Fetching game cache failed.");
	}
}

std::map<std::string, std::string> UpdateGameCache()
{
	// perform a game update
	auto differences = CompareCacheDifferences();

	if (!differences.empty())
	{
		PerformUpdate(differences);
	}

	// get a list of cache files that should be mapped given an updated cache
	std::map<std::string, std::string> retval;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

	for (auto& entry : g_requiredEntries)
	{
		std::string origFileName = entry.filename;

		if (origFileName.find("ros_1238/") == 0)
		{
			origFileName = "Social Club/" + origFileName.substr(9);
		}

		if (GetFileAttributes(entry.GetCacheFileName().c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			retval.insert({ origFileName, converter.to_bytes(entry.GetCacheFileName()) });
		}
	}

	return retval;
}
#endif
