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

		if (filenameBase.find("ros_1231/") == 0)
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
	{ "GTA5.exe", "9242ac4e38b626f2c8784451aa02976111d5265f", "https://runtime.fivem.net/patches/GTA_V_Patch_1_0_1290_1.exe", "$/GTA5.exe", 63111600, 934129320 },
	{ "update/update.rpf", "d6e32cbcfc40575e1e3e5f70bdd9052e10b67b27", "https://runtime.fivem.net/patches/GTA_V_Patch_1_0_1290_1.exe", "$/update/update.rpf", 879337472, 934129320 },
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

	{ "ros_1231/cef.pak", "229DD3682DDA8258497F342319CDBEC9FF35BC33", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/cef.pak", 2749972, 72795808 },
	{ "ros_1231/cef_100_percent.pak", "E14361DB195ACF3A6676DE3C25A2F81AC3AB67B3", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/cef_100_percent.pak", 146067, 72795808 },
	{ "ros_1231/cef_200_percent.pak", "3EF18057982B409DB394046F7C5D37F7985DEAF7", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/cef_200_percent.pak", 235262, 72795808 },
	{ "ros_1231/d3dcompiler_43.dll", "8AEA40EE6891138F0295E2F65D448A0A6488EDFE", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/d3dcompiler_43.dll", 2106344, 72795808 },
	{ "ros_1231/d3dcompiler_47.dll", "BFE9F0775D2ED0C86C37E38F5A70EA323F0CA977", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/d3dcompiler_47.dll", 4461544, 72795808 },
	{ "ros_1231/icudtl.dat", "7C401AFE79968C4E8BA632E8B3E8D7927D9143BF", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/icudtl.dat", 10127152, 72795808 },
	{ "ros_1231/libcef.dll", "57C3C61F81E94892E8DDC66CB174A81C4D4B2481", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/libcef.dll", 69763560, 72795808 },
	{ "ros_1231/libEGL.dll", "44978C74E597ECB8971EB21BB3F36D0F2F46192F", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/libEGL.dll", 100328, 72795808 },
	{ "ros_1231/libGLESv2.dll", "EE1C438A7B98F43D6984B1865C58DFF205F1997D", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/libGLESv2.dll", 2329576, 72795808 },
	{ "ros_1231/natives_blob.bin", "E9A4573D81D20B6E19A9747499863883172179B9", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/natives_blob.bin", 415490, 72795808 },
	{ "ros_1231/scui.pak", "022AD5DB7674E0CFB32D7DF0A0F66FC6D7D7B2C6", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/scui.pak", 6212172, 72795808 },
	{ "ros_1231/snapshot_blob.bin", "C9A5515E595EC964B77F0E0702793FA5CC2AE79D", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/snapshot_blob.bin", 662124, 72795808 },
	{ "ros_1231/socialclub.dll", "0565ACF18CADDDE9262B8948C7E7555C84CC625C", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/socialclub.dll", 8485352, 72795808 },
	{ "ros_1231/subprocess.exe", "2D8AB603A083767A2E249535C396E64498242DB5", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/subprocess.exe", 1071080, 72795808 },
	{ "ros_1231/widevinecdmadapter.dll", "A50A6505A8CD940F74D8B7F64BC9CFF5AC4C9774", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/widevinecdmadapter.dll", 285672, 72795808 },
	{ "ros_1231/locales/am.pak", "AFDE424D27C81C1425B87B2AD904B8FB2808703F", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/am.pak", 77527, 72795808 },
	{ "ros_1231/locales/ar.pak", "30485F99DBA6320FC4AEDF9B344601BA911CFCBA", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/ar.pak", 76375, 72795808 },
	{ "ros_1231/locales/bg.pak", "2E4F998E69A0CC3E2622036CBEDA7DD4EDC7F96F", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/bg.pak", 86110, 72795808 },
	{ "ros_1231/locales/bn.pak", "1D669A93F832DDF27D238300C3EB70F4CA1D8AC7", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/bn.pak", 114883, 72795808 },
	{ "ros_1231/locales/ca.pak", "5394CC7505F2637F0053526AEE7D57162E521CA5", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/ca.pak", 55675, 72795808 },
	{ "ros_1231/locales/cs.pak", "39585F7CC783C9356CC9866D5C696563F7300524", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/cs.pak", 55671, 72795808 },
	{ "ros_1231/locales/da.pak", "5AC7EE4213F60F8B358E42ECC38C1D5E9B7DA916", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/da.pak", 51068, 72795808 },
	{ "ros_1231/locales/de.pak", "90FDABD738A832315E8262E4F46FF341AD71DB66", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/de.pak", 55396, 72795808 },
	{ "ros_1231/locales/el.pak", "8DE2CE464C7C4D4879B95601278E008FF0127908", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/el.pak", 96384, 72795808 },
	{ "ros_1231/locales/en-GB.pak", "67FEE528BDF8D0FE2E4BE6185A0A864741780B82", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/en-GB.pak", 46091, 72795808 },
	{ "ros_1231/locales/en-US.pak", "15C8070EA65FF847CE09C79434CDBC394CC73100", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/en-US.pak", 46171, 72795808 },
	{ "ros_1231/locales/es-419.pak", "DE66F383476C9742639F88A8A28672B5CE063206", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/es-419.pak", 53969, 72795808 },
	{ "ros_1231/locales/es.pak", "25AC0369FDC993CB6D7E5F02D3EB431E3E0648C5", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/es.pak", 55938, 72795808 },
	{ "ros_1231/locales/et.pak", "C6BFFE782997634A8CD75969E408C5C3F981E168", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/et.pak", 48380, 72795808 },
	{ "ros_1231/locales/fa.pak", "EFF71B01205BEEAD7976862D0A5E1046B48B2038", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/fa.pak", 75548, 72795808 },
	{ "ros_1231/locales/fi.pak", "B8F7F2637FE84FDB16DDC425DE23CB6206B73C7C", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/fi.pak", 50631, 72795808 },
	{ "ros_1231/locales/fil.pak", "837CB2AB7E3CC1E9A4CE353A4D4B5AE94E0EE677", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/fil.pak", 56390, 72795808 },
	{ "ros_1231/locales/fr.pak", "621C3511A2D76C61D27E976EED942B8A5E007D85", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/fr.pak", 58923, 72795808 },
	{ "ros_1231/locales/gu.pak", "E897ED691C4C815AD7AE551376FB4E1BBD23AF9B", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/gu.pak", 107875, 72795808 },
	{ "ros_1231/locales/he.pak", "3A260C1A6C2F0A9C6F6D9F38F42848714FFA6829", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/he.pak", 63544, 72795808 },
	{ "ros_1231/locales/hi.pak", "4F04A569D31D2AFE2F9B07F2775228DD6017EAA2", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/hi.pak", 111058, 72795808 },
	{ "ros_1231/locales/hr.pak", "92693CB796F1BAEABCD6879826BE6561F95FE925", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/hr.pak", 52100, 72795808 },
	{ "ros_1231/locales/hu.pak", "136EE789B6B33412275FCFCB9C394C9BDDEDE05B", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/hu.pak", 56162, 72795808 },
	{ "ros_1231/locales/id.pak", "27EE2B4F010AA5794C3CF4E1702E81BF8415D1BC", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/id.pak", 48968, 72795808 },
	{ "ros_1231/locales/it.pak", "BEBBBF56EBF136E70D415C6010539F03FD689983", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/it.pak", 53890, 72795808 },
	{ "ros_1231/locales/ja.pak", "3FFC69BDA30A0F13252C160169573B4D921397A2", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/ja.pak", 65231, 72795808 },
	{ "ros_1231/locales/kn.pak", "7F91C7BCC2D6561587A2DB7B6C27E62285FE2C43", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/kn.pak", 123630, 72795808 },
	{ "ros_1231/locales/ko.pak", "556D087541D11D06D9BAFB386945B6194C1393F1", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/ko.pak", 55660, 72795808 },
	{ "ros_1231/locales/lt.pak", "773A65D7A035CDC3B95EE74039F52FAED853C893", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/lt.pak", 55901, 72795808 },
	{ "ros_1231/locales/lv.pak", "33B7C71AE290CD38D94F005B26E565F3F640881C", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/lv.pak", 56886, 72795808 },
	{ "ros_1231/locales/ml.pak", "5E1C5A2763FA755642B3B027B939CDD22337B962", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/ml.pak", 134264, 72795808 },
	{ "ros_1231/locales/mr.pak", "1C87AB0C5B8D3B56CDF32FE3F2A78180C852918E", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/mr.pak", 110576, 72795808 },
	{ "ros_1231/locales/ms.pak", "F471A6BFE9EFB8848C6E1D80A4300A03E612ABD4", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/ms.pak", 49935, 72795808 },
	{ "ros_1231/locales/nb.pak", "CA40B417C309130FA1FB11B6525E546ADA344453", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/nb.pak", 50043, 72795808 },
	{ "ros_1231/locales/nl.pak", "EEB48C750AEDB8CFE38AF7513703731347A7CE1A", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/nl.pak", 52308, 72795808 },
	{ "ros_1231/locales/pl.pak", "82FC930C37678083F8AF478817A67687B7687D41", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/pl.pak", 54721, 72795808 },
	{ "ros_1231/locales/pt-BR.pak", "F35D32C1F476EED043986FD075CE8D1FA93D67D5", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/pt-BR.pak", 54142, 72795808 },
	{ "ros_1231/locales/pt-PT.pak", "F7F6C445D966C7E12D08F0018B88EE94859B8306", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/pt-PT.pak", 53929, 72795808 },
	{ "ros_1231/locales/ro.pak", "D74D0609C8BEAAF0A9B39792A5BCC2C28E4B7F43", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/ro.pak", 55629, 72795808 },
	{ "ros_1231/locales/ru.pak", "7D8F87C14D872B0CEB5AD23D1C4449AAEAFD06E9", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/ru.pak", 84306, 72795808 },
	{ "ros_1231/locales/sk.pak", "106878860F1F0459C2198A66A1E894BAE2D1470A", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/sk.pak", 56091, 72795808 },
	{ "ros_1231/locales/sl.pak", "A867AAB5F74C83C383A253917B988788A1DE565E", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/sl.pak", 52427, 72795808 },
	{ "ros_1231/locales/sr.pak", "9AFFEA5067DB5AC8BE0FC2E581844A5C06EEDC37", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/sr.pak", 82361, 72795808 },
	{ "ros_1231/locales/sv.pak", "16A3656EDE64B6C658A6799B13B1C622757BCB2B", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/sv.pak", 49750, 72795808 },
	{ "ros_1231/locales/sw.pak", "4FEFFD55300DF8EC80DD6683BA35074F199A3E28", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/sw.pak", 49884, 72795808 },
	{ "ros_1231/locales/ta.pak", "C3B5493866F9221998EF7F0413EA42EF9BC30720", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/ta.pak", 127484, 72795808 },
	{ "ros_1231/locales/te.pak", "16EEC51F1B9CD72276A62AFDE2DA0FB771E386E1", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/te.pak", 117722, 72795808 },
	{ "ros_1231/locales/th.pak", "B9321567D972634C8ADE577FE1ED6C218939F2B8", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/th.pak", 104890, 72795808 },
	{ "ros_1231/locales/tr.pak", "D4F9A4C442EC30276244299318D823FD0EB1ECD4", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/tr.pak", 52673, 72795808 },
	{ "ros_1231/locales/uk.pak", "45CC3E0E29089395EAD9115E664A716A3CECB9C8", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/uk.pak", 87212, 72795808 },
	{ "ros_1231/locales/vi.pak", "2A3C7EA5385230E834A92E5B2EB453CD30482A8A", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/vi.pak", 60615, 72795808 },
	{ "ros_1231/locales/zh-CN.pak", "605387D0737CBE29E417C4909667C432608B7275", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/zh-CN.pak", 45082, 72795808 },
	{ "ros_1231/locales/zh-TW.pak", "652C109482CC03E2A04BCF2021EC27C6CFA4D596", "http://patches.rockstargames.com/prod/socialclub/Social-Club-v1.2.3.1-Setup.exe", "$/locales/zh-TW.pak", 45661, 72795808 },

	{ "GTAVLauncher.exe", "28916b79d44ced3d22fa8ad087e3949749779850", "https://runtime.fivem.net/patches/GTA_V_Launcher_1_0_1290_2.exe", "$/GTAVLauncher.exe", 21434808, 19981008 }
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

		if (origFileName.find("ros_1231/") == 0)
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
