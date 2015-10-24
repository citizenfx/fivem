/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include "InstallerExtraction.h"
#include <array>

#ifdef GTA_FIVE
// entry for a cached-intent file
struct GameCacheEntry
{
	// local filename to map from
	const wchar_t* filename;

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
	GameCacheEntry(const wchar_t* filename, const char* checksum, const char* remotePath, size_t localSize)
		: filename(filename), checksum(checksum), remotePath(remotePath), localSize(localSize), remoteSize(localSize), archivedFile(nullptr)
	{

	}

	GameCacheEntry(const wchar_t* filename, const char* checksum, const char* remotePath, const char* archivedFile, size_t localSize, size_t remoteSize)
		: filename(filename), checksum(checksum), remotePath(remotePath), localSize(localSize), remoteSize(remoteSize), archivedFile(archivedFile)
	{

	}

	// methods
	std::wstring GetCacheFileName() const
	{
		std::wstring filenameBase = filename;

		if (filenameBase.find(L"ros/") == 0)
		{
			return MakeRelativeCitPath(va(L"cache\\game\\%s", filenameBase.c_str()));
		}

		std::replace(filenameBase.begin(), filenameBase.end(), '/', '+');

		wchar_t checksumWide[42];
		mbstowcs(checksumWide, checksum, _countof(checksumWide));

		return MakeRelativeCitPath(va(L"cache\\game\\%s_%s", filenameBase.c_str(), checksumWide));
	}

	std::wstring GetRemoteBaseName() const
	{
		std::string remoteNameBase = remotePath;
		std::wstring remoteNameBaseWide(remoteNameBase.begin(), remoteNameBase.end());

		int slashIndex = remoteNameBaseWide.find_last_of(L'/') + 1;

		return MakeRelativeCitPath(va(L"cache\\game\\%s", remoteNameBaseWide.substr(slashIndex).c_str()));
	}

	std::wstring GetLocalFileName() const
	{
		return MakeRelativeGamePath(filename);
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

	// note to R*: please don't sue me for me temporarily hosting these myself :-)
	//{ L"GTA5.exe", "883d05ce147ec01f94f862453bb69fe96cc15539", "http://refint.org/files/GTA_V_Patch_1_0_393_4.exe", "$/GTA5.exe", 55839112, 422755424 },
	//{ L"update/update.rpf", "d9f84cd5b8b5bafaeee92dd43568887172849d01", "http://refint.org/files/GTA_V_Patch_1_0_393_4.exe", "$/update/update.rpf", 374835200, 422755424 },

	{ L"GTA5.exe", "a6b255a2e2b8ea48ec5776f42770cf54522a85bf", "Game_EFIGS/GTA_V_Patch_1_0_505_2.exe", "$/GTA5.exe", 54920072, 505185312 },
	{ L"update/update.rpf", "64ddef8f58ad064745fb18879b309a6d8b0f346b", "Game_EFIGS/GTA_V_Patch_1_0_505_2.exe", "$/update/update.rpf", 457312256, 505185312 },

	{ L"update/x64/dlcpacks/patchday4ng/dlc.rpf", "124c908d82724258a5721535c87f1b8e5c6d8e57", "DigitalData/DLCPacks2/update/x64/dlcpacks/patchday4ng/dlc.rpf", 312438784 },
	{ L"update/x64/dlcpacks/mpluxe/dlc.rpf", "78f7777b49f4b4d77e3da6db728cb3f7ec51e2fc", "DigitalData/DLCPacks2/update/x64/dlcpacks/mpluxe/dlc.rpf", 226260992 },

	{ L"update/x64/dlcpacks/patchday5ng/dlc.rpf", "af3b2a59b4e1e5fd220c308d85753bdbffd8063c", "DigitalData/DLCPacks3/update/x64/dlcpacks/patchday5ng/dlc.rpf", 7827456 },
	{ L"update/x64/dlcpacks/mpluxe2/dlc.rpf", "1e59e1f05be5dba5650a1166eadfcb5aeaf7737b", "DigitalData/DLCPacks3/update/x64/dlcpacks/mpluxe2/dlc.rpf", 105105408 },

	{ L"update/x64/dlcpacks/mpreplay/dlc.rpf", "f5375beef591178d8aaf334431a7b6596d0d793a", "DigitalData/DLCPacks4/update/x64/dlcpacks/mpreplay/dlc.rpf", 429932544 },
	{ L"update/x64/dlcpacks/patchday6ng/dlc.rpf", "5d38b40ad963a6cf39d24bb5e008e9692838b33b", "DigitalData/DLCPacks4/update/x64/dlcpacks/patchday6ng/dlc.rpf", 31907840 },

	{ L"update/x64/dlcpacks/mphalloween/dlc.rpf", "3f960c014e83be00cf8e6b520bbf22f7da6160a4", "DigitalData/DLCPacks5/update/x64/dlcpacks/mphalloween/dlc.rpf", 104658944 },
	{ L"update/x64/dlcpacks/mplowrider/dlc.rpf", "eab744fe959ca29a2e5f36843d259ffc9d04a7f6", "DigitalData/DLCPacks5/update/x64/dlcpacks/mplowrider/dlc.rpf", 1088813056 },
	{ L"update/x64/dlcpacks/patchday7ng/dlc.rpf", "29df23f3539907a4e15f1cdb9426d462c1ad0337", "DigitalData/DLCPacks5/update/x64/dlcpacks/patchday7ng/dlc.rpf", 43843584 },

	{ L"ros/cef.pak", "EC38FF4278D4E13FD8681A205F29CDA000D05759", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/cef.pak", 2018390, 56061688 },
	{ L"ros/cef_100_percent.pak", "6B96A6E9E418AE73B4EC7CB6CB7C10BAA2A98449", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/cef_100_percent.pak", 444515, 56061688 },
	{ L"ros/cef_200_percent.pak", "273A18D4BBB2F2E7080A95BFC2EF2EF034AC5E2C", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/cef_200_percent.pak", 598403, 56061688 },
	{ L"ros/d3dcompiler_43.dll", "AA4E953BFE720661855FFD0F9DB72EAF4A13C1AD", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/d3dcompiler_43.dll", 2106328, 56061688 },
	{ L"ros/d3dcompiler_47.dll", "B4CF857E05BBD1F3C22BF36975C5CC7C1714719C", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/d3dcompiler_47.dll", 4164568, 56061688 },
	{ L"ros/ffmpegsumo.dll", "A1E04C46AE4773064EBD7E6F8851E171755BABC5", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/ffmpegsumo.dll", 995288, 56061688 },
	{ L"ros/icudtl.dat", "0249F28CA75B97FAFEF60A40A3A45620AB31EB52", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/icudtl.dat", 10457856, 56061688 },
	{ L"ros/libcef.dll", "EF60987C3EEC40D1F3C61D62A587D2F6809A3259", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/libcef.dll", 54800856, 56061688 },
	{ L"ros/libEGL.dll", "3FC73976A52D70E13393B2D52C73E041C9A5E77E", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/libEGL.dll", 92120, 56061688 },
	{ L"ros/libGLESv2.dll", "C13F27A8B164243D09E8F2736BECABDE0F1B8425", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/libGLESv2.dll", 1896408, 56061688 },
	{ L"ros/socialclub.dll", "8DCF23DB5B40DE6F41DC14D0B357816E90DA6C0B", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/socialclub.dll", 7685080, 56061688 },
	{ L"ros/steam_api64.dll", "2C112BF98AA0379CACDEA69EF12CB2D97AE4AAED", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/steam_api64.dll", 211368, 56061688 },
	{ L"ros/subprocess.exe", "D26B2900230859AAD73842EAAB1453F79DF8FD0B", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/subprocess.exe", 959960, 56061688 },
	{ L"ros/uninstallRGSCRedistributable.exe", "B5DD082D2286A56540E989939AAFB112ACFD173C", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/uninstallRGSCRedistributable.exe", 196800, 56061688 },
	{ L"ros/locales/am.pak", "E59863F606BD0290CD5F33AB7F5C15D359B81743", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/am.pak", 21323, 56061688 },
	{ L"ros/locales/ar.pak", "4774DD4AD8DEE8DD559A9B91461AED093B616363", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ar.pak", 23400, 56061688 },
	{ L"ros/locales/bg.pak", "4DE238F51B3A77D2E92E76DB28DE6888D151B022", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/bg.pak", 24877, 56061688 },
	{ L"ros/locales/bn.pak", "E1053547B3BAAE5763415C2ACE123C2AC0F4B5FF", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/bn.pak", 30612, 56061688 },
	{ L"ros/locales/ca.pak", "40ED169AED88D121C094EE82C097A82DACA93265", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ca.pak", 16550, 56061688 },
	{ L"ros/locales/cs.pak", "02AFD88350004FEAE261E076B53B6EFA4C6A9FE0", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/cs.pak", 15663, 56061688 },
	{ L"ros/locales/da.pak", "14AD54A6EEBEFEB8292887283050CF9E1F0CB633", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/da.pak", 14452, 56061688 },
	{ L"ros/locales/de.pak", "FDDC7378EFF93712A1B162FB40A4460A2605B9EE", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/de.pak", 15875, 56061688 },
	{ L"ros/locales/el.pak", "1640312BA5BF4F03B9ECFD80274464ABFA5B96C4", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/el.pak", 26710, 56061688 },
	{ L"ros/locales/en-GB.pak", "FB7E73ABBBCD2942C1072C08C196AA87C9D4671B", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/en-GB.pak", 13508, 56061688 },
	{ L"ros/locales/en-US.pak", "DA1E9AD60B741D51A6D6BDD4BA3091C274D3BA0A", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/en-US.pak", 13506, 56061688 },
	{ L"ros/locales/es-419.pak", "FC7D24AC42A99BFE14EED3CF6208B62D86C4CAFE", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/es-419.pak", 16099, 56061688 },
	{ L"ros/locales/es.pak", "16148B8AD9EFAC56A576E2B77BF86DB0C0A697AD", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/es.pak", 16638, 56061688 },
	{ L"ros/locales/et.pak", "472B2A5E5424B9F3744AE317628E49B616DE3D6F", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/et.pak", 14624, 56061688 },
	{ L"ros/locales/fa.pak", "9EDC860DD1EB4346CAD58FD32D7F44A27E39A232", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/fa.pak", 20712, 56061688 },
	{ L"ros/locales/fi.pak", "DB313CA2D37133603ABB1C01B9CCF172EF180C80", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/fi.pak", 15530, 56061688 },
	{ L"ros/locales/fil.pak", "C788271FF0D7027DD0C4CAF757F474BD2C25BB05", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/fil.pak", 16427, 56061688 },
	{ L"ros/locales/fr.pak", "59009507FB3E9ADEDB3D2CB694025BF9D0AC3E59", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/fr.pak", 16906, 56061688 },
	{ L"ros/locales/gu.pak", "B43AF7DC3690F3123F5D053AC91BF8C6CAF1D9CD", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/gu.pak", 28507, 56061688 },
	{ L"ros/locales/he.pak", "C3F5A837D2C6E719679F9F8EDC4CBB6F5F0ED9D0", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/he.pak", 17806, 56061688 },
	{ L"ros/locales/hi.pak", "8E3BB723F8BA1AF052077050F68923C725352EAF", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/hi.pak", 28822, 56061688 },
	{ L"ros/locales/hr.pak", "AF9AD74C4C9B38AE556E9542698DF2EB72DC8AAA", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/hr.pak", 15635, 56061688 },
	{ L"ros/locales/hu.pak", "5B3DA555EB6D9B85A58F2A3AC81177AE013F2C03", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/hu.pak", 16336, 56061688 },
	{ L"ros/locales/id.pak", "047A76C6F02C2DCA9344196B512A2B1C3E280701", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/id.pak", 14475, 56061688 },
	{ L"ros/locales/it.pak", "E85C7E6FAA52D8B4C8EAAD9B95288A148BB363A5", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/it.pak", 15426, 56061688 },
	{ L"ros/locales/ja.pak", "3BF9BC51526FDA0D6DD509AA169F991E603DDD6A", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ja.pak", 18064, 56061688 },
	{ L"ros/locales/kn.pak", "0C954833A03EF6527F11A661CB21F738820D98A8", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/kn.pak", 32623, 56061688 },
	{ L"ros/locales/ko.pak", "9A4007BA729077C14412528F1C0F0B42F8E5FEFC", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ko.pak", 15627, 56061688 },
	{ L"ros/locales/lt.pak", "C3769F6AB57A7C62FDDA92B82B2C02E1E5D7DA8B", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/lt.pak", 16185, 56061688 },
	{ L"ros/locales/lv.pak", "74F7F315ACB05B10179DD23139FDC7FBAAC3EE67", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/lv.pak", 16509, 56061688 },
	{ L"ros/locales/ml.pak", "5063DB743448A68A6D34D66409C67F79C016B20C", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ml.pak", 37175, 56061688 },
	{ L"ros/locales/mr.pak", "C976EC48B327BECB3F062AC348A9F630A4B9E913", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/mr.pak", 28811, 56061688 },
	{ L"ros/locales/ms.pak", "904D6EA2B41E3741B6A866D7E4882E12FD04075B", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ms.pak", 14607, 56061688 },
	{ L"ros/locales/nb.pak", "3DC05131B591101343C804308266B5A3FD6EEA2E", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/nb.pak", 14775, 56061688 },
	{ L"ros/locales/nl.pak", "446C47C96E7842938989CC872318320AB55F7339", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/nl.pak", 15168, 56061688 },
	{ L"ros/locales/pl.pak", "61703810054BACDC6E95D8E44DA6A2BB8E8849D8", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/pl.pak", 15824, 56061688 },
	{ L"ros/locales/pt-BR.pak", "2E5512784306CCA5B4190AC2FAAC71B1BEF29ADF", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/pt-BR.pak", 15534, 56061688 },
	{ L"ros/locales/pt-PT.pak", "1DC37B83E3DFC1933FE6BBA6D3012BC8512ED565", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/pt-PT.pak", 15661, 56061688 },
	{ L"ros/locales/ro.pak", "6C8F086BC406ADE972D742BAC4078B4714090D06", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ro.pak", 16632, 56061688 },
	{ L"ros/locales/ru.pak", "38B37D68CDC3C526BF49620FA0A56F50D12AEF2F", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ru.pak", 23341, 56061688 },
	{ L"ros/locales/sk.pak", "8F2F4C45541C8A5952E04EC69269DA8D9437CABD", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/sk.pak", 16312, 56061688 },
	{ L"ros/locales/sl.pak", "AC2C05A0CD2344CB3C92EF6E8CB24C1A02D4351C", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/sl.pak", 15044, 56061688 },
	{ L"ros/locales/sr.pak", "1549461D200F4A61F0CC9FA6026116E88FCB1AC2", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/sr.pak", 22919, 56061688 },
	{ L"ros/locales/sv.pak", "FCFCE623E26D8D483DD0FCA5BE76421657591FE2", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/sv.pak", 14471, 56061688 },
	{ L"ros/locales/sw.pak", "CEC7C53A41FB2B21ED94729AFD24936BFA186F55", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/sw.pak", 14981, 56061688 },
	{ L"ros/locales/ta.pak", "D342E36D960E4C0FBF0EA8BADA170F86BB81A73C", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/ta.pak", 35140, 56061688 },
	{ L"ros/locales/te.pak", "9959A7767C8BB28DA2EC9AA3CF035C5F5471FB8B", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/te.pak", 33022, 56061688 },
	{ L"ros/locales/th.pak", "AD6B58CB76D407806A0A5F890AFA977419E0E676", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/th.pak", 28995, 56061688 },
	{ L"ros/locales/tr.pak", "6137D2EA86BFD42A3F677FBAB872CEC671EAD714", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/tr.pak", 15033, 56061688 },
	{ L"ros/locales/uk.pak", "C8CA4AA09A22FE45CA668BB3DEC6130382D8CB85", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/uk.pak", 24616, 56061688 },
	{ L"ros/locales/vi.pak", "61F9F0B37667CFE96E33C2C196B7B4120D7906DB", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/vi.pak", 17227, 56061688 },
	{ L"ros/locales/zh-CN.pak", "B6C40BF1052C2C340A8EE24E92A8A8F7AAEA2359", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/zh-CN.pak", 13239, 56061688 },
	{ L"ros/locales/zh-TW.pak", "3D7CA4D02AF0B084996F5F94637E8299BF501366", "http://patches.rockstargames.com/prod/socialclub/Social%20Club%20v1.1.6.8%20Setup.exe", "$/locales/zh-TW.pak", 13450, 56061688 },

	{ L"GTAVLauncher.exe", "4f8b3691b70deadb8b23e0916207556f0c59d0ea", "http://patches.rockstargames.com/prod/gtav/Launcher_EFIGS/GTA_V_Launcher_1_0_505_2.exe", "$/GTAVLauncher.exe", 21048712, 19489360 }
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

				if (GetFileAttributes(cacheFileName.c_str()) == INVALID_FILE_ATTRIBUTES)
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

	for (auto& entry : entries)
	{
		// if it's a local file...
		if (entry.second)
		{
			localSize += entry.first.localSize;

			detailStr << entry.first.filename << L" (local, " << va(L"%.2f", entry.first.localSize / 1024.0 / 1024.0) << L" MB)\n";
		}
		else
		{
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
	taskDialogConfig.pszContent = va(L"The local FiveM game cache is outdated, and needs to be updated. This will copy %.2f MB of data from the local disk, and download %.2f MB of data from the internet.\nDo you wish to continue?", (localSize / 1024.0 / 1024.0), (remoteSize / 1024.0 / 1024.0));
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

	return (outButton != IDNO);
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
			CL_QueueDownload(va("file:///%s", converter.to_bytes(entry.GetLocalFileName()).c_str()), converter.to_bytes(entry.GetCacheFileName()).c_str(), entry.localSize, false);

			notificationEntries.push_back({ entry, true });
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
			extractedEntries.push_back(GameCacheEntry{ L"", "", "", 0 });

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
		FatalError("are you still the one i knoooooooooow?");
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
		std::string origFileName = converter.to_bytes(entry.filename);

		if (origFileName.find("ros/") == 0)
		{
			origFileName = "Social Club/" + origFileName.substr(4);
		}

		retval.insert({ origFileName, converter.to_bytes(entry.GetCacheFileName()) });
	}

	return retval;
}
#endif