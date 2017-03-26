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
	const char* checksum;

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
		: filename(filename), checksum(checksum), remotePath(remotePath), localSize(localSize), remoteSize(localSize), archivedFile(nullptr)
	{

	}

	GameCacheEntry(const char* filename, const char* checksum, const char* remotePath, const char* archivedFile, size_t localSize, size_t remoteSize)
		: filename(filename), checksum(checksum), remotePath(remotePath), localSize(localSize), remoteSize(remoteSize), archivedFile(archivedFile)
	{

	}

	// methods
	std::wstring GetCacheFileName() const
	{
		std::string filenameBase = filename;

		if (filenameBase.find("ros/") == 0)
		{
			return MakeRelativeCitPath(ToWide(va("cache\\game\\%s", filenameBase.c_str())));
		}

		std::replace(filenameBase.begin(), filenameBase.end(), '/', '+');

		return MakeRelativeCitPath(ToWide(va("cache\\game\\%s_%s", filenameBase.c_str(), checksum)));
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
	//{ L"GTA5.exe", "883d05ce147ec01f94f862453bb69fe96cc15539", "Game_EFIGS/GTA_V_Patch_1_0_393_4.exe", "$/GTA5.exe", 55839112, 422755424 },
	//{ L"update/update.rpf", "d9f84cd5b8b5bafaeee92dd43568887172849d01", "Game_EFIGS/GTA_V_Patch_1_0_393_4.exe", "$/update/update.rpf", 374835200, 422755424 },

	{ "GTA5.exe", "a6b255a2e2b8ea48ec5776f42770cf54522a85bf", "https://runtime.fivem.net/patches/GTA_V_Patch_1_0_505_2.exe", "$/GTA5.exe", 54920072, 505185312 },
	{ "update/update.rpf", "64ddef8f58ad064745fb18879b309a6d8b0f346b", "https://runtime.fivem.net/patches/GTA_V_Patch_1_0_505_2.exe", "$/update/update.rpf", 457312256, 505185312 },
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

	{ "ros/cef.pak", "EC38FF4278D4E13FD8681A205F29CDA000D05759", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/cef.pak", 2018390, 56061688 },
	{ "ros/cef_100_percent.pak", "6B96A6E9E418AE73B4EC7CB6CB7C10BAA2A98449", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/cef_100_percent.pak", 444515, 56061688 },
	{ "ros/cef_200_percent.pak", "273A18D4BBB2F2E7080A95BFC2EF2EF034AC5E2C", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/cef_200_percent.pak", 598403, 56061688 },
	{ "ros/d3dcompiler_43.dll", "AA4E953BFE720661855FFD0F9DB72EAF4A13C1AD", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/d3dcompiler_43.dll", 2106328, 56061688 },
	{ "ros/d3dcompiler_47.dll", "B4CF857E05BBD1F3C22BF36975C5CC7C1714719C", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/d3dcompiler_47.dll", 4164568, 56061688 },
	{ "ros/ffmpegsumo.dll", "A1E04C46AE4773064EBD7E6F8851E171755BABC5", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/ffmpegsumo.dll", 995288, 56061688 },
	{ "ros/icudtl.dat", "0249F28CA75B97FAFEF60A40A3A45620AB31EB52", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/icudtl.dat", 10457856, 56061688 },
	{ "ros/libcef.dll", "EF60987C3EEC40D1F3C61D62A587D2F6809A3259", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/libcef.dll", 54800856, 56061688 },
	{ "ros/libEGL.dll", "3FC73976A52D70E13393B2D52C73E041C9A5E77E", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/libEGL.dll", 92120, 56061688 },
	{ "ros/libGLESv2.dll", "C13F27A8B164243D09E8F2736BECABDE0F1B8425", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/libGLESv2.dll", 1896408, 56061688 },
	{ "ros/socialclub.dll", "8DCF23DB5B40DE6F41DC14D0B357816E90DA6C0B", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/socialclub.dll", 7685080, 56061688 },
	{ "ros/steam_api64.dll", "2C112BF98AA0379CACDEA69EF12CB2D97AE4AAED", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/steam_api64.dll", 211368, 56061688 },
	{ "ros/subprocess.exe", "D26B2900230859AAD73842EAAB1453F79DF8FD0B", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/subprocess.exe", 959960, 56061688 },
	{ "ros/uninstallRGSCRedistributable.exe", "B5DD082D2286A56540E989939AAFB112ACFD173C", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/uninstallRGSCRedistributable.exe", 196800, 56061688 },
	{ "ros/locales/am.pak", "E59863F606BD0290CD5F33AB7F5C15D359B81743", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/am.pak", 21323, 56061688 },
	{ "ros/locales/ar.pak", "4774DD4AD8DEE8DD559A9B91461AED093B616363", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ar.pak", 23400, 56061688 },
	{ "ros/locales/bg.pak", "4DE238F51B3A77D2E92E76DB28DE6888D151B022", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/bg.pak", 24877, 56061688 },
	{ "ros/locales/bn.pak", "E1053547B3BAAE5763415C2ACE123C2AC0F4B5FF", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/bn.pak", 30612, 56061688 },
	{ "ros/locales/ca.pak", "40ED169AED88D121C094EE82C097A82DACA93265", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ca.pak", 16550, 56061688 },
	{ "ros/locales/cs.pak", "02AFD88350004FEAE261E076B53B6EFA4C6A9FE0", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/cs.pak", 15663, 56061688 },
	{ "ros/locales/da.pak", "14AD54A6EEBEFEB8292887283050CF9E1F0CB633", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/da.pak", 14452, 56061688 },
	{ "ros/locales/de.pak", "FDDC7378EFF93712A1B162FB40A4460A2605B9EE", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/de.pak", 15875, 56061688 },
	{ "ros/locales/el.pak", "1640312BA5BF4F03B9ECFD80274464ABFA5B96C4", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/el.pak", 26710, 56061688 },
	{ "ros/locales/en-GB.pak", "FB7E73ABBBCD2942C1072C08C196AA87C9D4671B", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/en-GB.pak", 13508, 56061688 },
	{ "ros/locales/en-US.pak", "DA1E9AD60B741D51A6D6BDD4BA3091C274D3BA0A", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/en-US.pak", 13506, 56061688 },
	{ "ros/locales/es-419.pak", "FC7D24AC42A99BFE14EED3CF6208B62D86C4CAFE", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/es-419.pak", 16099, 56061688 },
	{ "ros/locales/es.pak", "16148B8AD9EFAC56A576E2B77BF86DB0C0A697AD", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/es.pak", 16638, 56061688 },
	{ "ros/locales/et.pak", "472B2A5E5424B9F3744AE317628E49B616DE3D6F", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/et.pak", 14624, 56061688 },
	{ "ros/locales/fa.pak", "9EDC860DD1EB4346CAD58FD32D7F44A27E39A232", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/fa.pak", 20712, 56061688 },
	{ "ros/locales/fi.pak", "DB313CA2D37133603ABB1C01B9CCF172EF180C80", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/fi.pak", 15530, 56061688 },
	{ "ros/locales/fil.pak", "C788271FF0D7027DD0C4CAF757F474BD2C25BB05", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/fil.pak", 16427, 56061688 },
	{ "ros/locales/fr.pak", "59009507FB3E9ADEDB3D2CB694025BF9D0AC3E59", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/fr.pak", 16906, 56061688 },
	{ "ros/locales/gu.pak", "B43AF7DC3690F3123F5D053AC91BF8C6CAF1D9CD", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/gu.pak", 28507, 56061688 },
	{ "ros/locales/he.pak", "C3F5A837D2C6E719679F9F8EDC4CBB6F5F0ED9D0", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/he.pak", 17806, 56061688 },
	{ "ros/locales/hi.pak", "8E3BB723F8BA1AF052077050F68923C725352EAF", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/hi.pak", 28822, 56061688 },
	{ "ros/locales/hr.pak", "AF9AD74C4C9B38AE556E9542698DF2EB72DC8AAA", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/hr.pak", 15635, 56061688 },
	{ "ros/locales/hu.pak", "5B3DA555EB6D9B85A58F2A3AC81177AE013F2C03", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/hu.pak", 16336, 56061688 },
	{ "ros/locales/id.pak", "047A76C6F02C2DCA9344196B512A2B1C3E280701", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/id.pak", 14475, 56061688 },
	{ "ros/locales/it.pak", "E85C7E6FAA52D8B4C8EAAD9B95288A148BB363A5", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/it.pak", 15426, 56061688 },
	{ "ros/locales/ja.pak", "3BF9BC51526FDA0D6DD509AA169F991E603DDD6A", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ja.pak", 18064, 56061688 },
	{ "ros/locales/kn.pak", "0C954833A03EF6527F11A661CB21F738820D98A8", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/kn.pak", 32623, 56061688 },
	{ "ros/locales/ko.pak", "9A4007BA729077C14412528F1C0F0B42F8E5FEFC", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ko.pak", 15627, 56061688 },
	{ "ros/locales/lt.pak", "C3769F6AB57A7C62FDDA92B82B2C02E1E5D7DA8B", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/lt.pak", 16185, 56061688 },
	{ "ros/locales/lv.pak", "74F7F315ACB05B10179DD23139FDC7FBAAC3EE67", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/lv.pak", 16509, 56061688 },
	{ "ros/locales/ml.pak", "5063DB743448A68A6D34D66409C67F79C016B20C", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ml.pak", 37175, 56061688 },
	{ "ros/locales/mr.pak", "C976EC48B327BECB3F062AC348A9F630A4B9E913", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/mr.pak", 28811, 56061688 },
	{ "ros/locales/ms.pak", "904D6EA2B41E3741B6A866D7E4882E12FD04075B", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ms.pak", 14607, 56061688 },
	{ "ros/locales/nb.pak", "3DC05131B591101343C804308266B5A3FD6EEA2E", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/nb.pak", 14775, 56061688 },
	{ "ros/locales/nl.pak", "446C47C96E7842938989CC872318320AB55F7339", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/nl.pak", 15168, 56061688 },
	{ "ros/locales/pl.pak", "61703810054BACDC6E95D8E44DA6A2BB8E8849D8", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/pl.pak", 15824, 56061688 },
	{ "ros/locales/pt-BR.pak", "2E5512784306CCA5B4190AC2FAAC71B1BEF29ADF", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/pt-BR.pak", 15534, 56061688 },
	{ "ros/locales/pt-PT.pak", "1DC37B83E3DFC1933FE6BBA6D3012BC8512ED565", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/pt-PT.pak", 15661, 56061688 },
	{ "ros/locales/ro.pak", "6C8F086BC406ADE972D742BAC4078B4714090D06", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ro.pak", 16632, 56061688 },
	{ "ros/locales/ru.pak", "38B37D68CDC3C526BF49620FA0A56F50D12AEF2F", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ru.pak", 23341, 56061688 },
	{ "ros/locales/sk.pak", "8F2F4C45541C8A5952E04EC69269DA8D9437CABD", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/sk.pak", 16312, 56061688 },
	{ "ros/locales/sl.pak", "AC2C05A0CD2344CB3C92EF6E8CB24C1A02D4351C", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/sl.pak", 15044, 56061688 },
	{ "ros/locales/sr.pak", "1549461D200F4A61F0CC9FA6026116E88FCB1AC2", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/sr.pak", 22919, 56061688 },
	{ "ros/locales/sv.pak", "FCFCE623E26D8D483DD0FCA5BE76421657591FE2", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/sv.pak", 14471, 56061688 },
	{ "ros/locales/sw.pak", "CEC7C53A41FB2B21ED94729AFD24936BFA186F55", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/sw.pak", 14981, 56061688 },
	{ "ros/locales/ta.pak", "D342E36D960E4C0FBF0EA8BADA170F86BB81A73C", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ta.pak", 35140, 56061688 },
	{ "ros/locales/te.pak", "9959A7767C8BB28DA2EC9AA3CF035C5F5471FB8B", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/te.pak", 33022, 56061688 },
	{ "ros/locales/th.pak", "AD6B58CB76D407806A0A5F890AFA977419E0E676", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/th.pak", 28995, 56061688 },
	{ "ros/locales/tr.pak", "6137D2EA86BFD42A3F677FBAB872CEC671EAD714", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/tr.pak", 15033, 56061688 },
	{ "ros/locales/uk.pak", "C8CA4AA09A22FE45CA668BB3DEC6130382D8CB85", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/uk.pak", 24616, 56061688 },
	{ "ros/locales/vi.pak", "61F9F0B37667CFE96E33C2C196B7B4120D7906DB", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/vi.pak", 17227, 56061688 },
	{ "ros/locales/zh-CN.pak", "B6C40BF1052C2C340A8EE24E92A8A8F7AAEA2359", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/zh-CN.pak", 13239, 56061688 },
	{ "ros/locales/zh-TW.pak", "3D7CA4D02AF0B084996F5F94637E8299BF501366", "https://runtime.fivem.net/patches/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/zh-TW.pak", 13450, 56061688 },

	{ "GTAVLauncher.exe", "4f8b3691b70deadb8b23e0916207556f0c59d0ea", "https://runtime.fivem.net/patches/GTA_V_Launcher_1_0_505_2.exe", "$/GTAVLauncher.exe", 21048712, 19489360 }
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
		auto requiredHash = ParseHexString<20>(entry.checksum);
		bool found = false;

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
		taskDialogConfig.pszContent = va(L"The local \x039B game cache is outdated, and needs to be updated. This will copy %.2f MB of data from the local disk, and download %.2f MB of data from the internet.\nDo you wish to continue?", (localSize / 1024.0 / 1024.0), (remoteSize / 1024.0 / 1024.0));
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
		auto hash = ParseHexString<20>(entry.checksum);

		bool fileOutdated = CheckFileOutdatedWithUI(entry.GetLocalFileName().c_str(), hash.data());

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

		// if not, copy it from the local filesystem (we're abusing the download code here a lot)
		if (!fileOutdated)
		{
			// should we 'nope' this file?
			if (_strnicmp(entry.remotePath, "nope:", 5) == 0)
			{
				if (FILE* f = _wfopen(MakeRelativeCitPath(L"cache\\game\\cache.dat").c_str(), L"ab"))
				{
					auto hash = ParseHexString<20>(entry.checksum);

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
									if (foundHashes.find(dlEntry.checksum) == foundHashes.end())
									{
										std::wstring cacheName = dlEntry.GetCacheFileName();

										if (cacheName.find(L'/') != std::string::npos)
										{
											std::wstring cachePath = cacheName.substr(0, cacheName.find_last_of(L'/'));

											CreateDirectory(cachePath.c_str(), nullptr);
										}

										interface.addFile(entry, cacheName);

										foundHashes.insert(dlEntry.checksum);
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
								auto hash = ParseHexString<20>(entry.checksum);

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

		if (origFileName.find("ros/") == 0)
		{
			origFileName = "Social Club/" + origFileName.substr(4);
		}

		if (GetFileAttributes(entry.GetCacheFileName().c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			retval.insert({ origFileName, converter.to_bytes(entry.GetCacheFileName()) });
		}
	}

	return retval;
}
#endif