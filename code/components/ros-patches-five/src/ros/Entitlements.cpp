/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ros/EndpointMapper.h>

#include <base64.h>
#include <botan/base64.h>
#include <botan/exceptn.h>

#include <zlib.h>

#include <sstream>

#include <json.hpp>

using json = nlohmann::json;

int StoreDecryptedBlob(void* a1, void* a2, uint32_t a3, void* inOutBlob, uint32_t a5, void* a6);

// TODO: turn into a generic utility
static std::map<std::string, std::string> ParsePOSTString(const std::string& postDataString)
{
	std::map<std::string, std::string> postMap;

	// split the string by the usual post map characters
	int curPos = 0;

	while (true)
	{
		int endPos = postDataString.find_first_of('&', curPos);

		int equalsPos = postDataString.find_first_of('=', curPos);

		std::string key;
		std::string value;

		UrlDecode(postDataString.substr(curPos, equalsPos - curPos), key);
		UrlDecode(postDataString.substr(equalsPos + 1, endPos - equalsPos - 1), value);

		postMap[key] = value;

		// save and continue
		curPos = endPos;

		if (curPos == std::string::npos)
		{
			break;
		}

		curPos++;
	}

	return postMap;
}

extern std::string g_entitlementSource;

bool LoadOwnershipTicket();
std::string GetRockstarTicketXml();

#include <ShlObj.h>
#include <KnownFolders.h>

std::string GetFilePath(const std::string& str)
{
	PWSTR appDataPath;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &appDataPath))) {
		std::string cfxPath = ToNarrow(appDataPath) + "\\DigitalEntitlements";
		CreateDirectory(ToWide(cfxPath).c_str(), nullptr);

		CoTaskMemFree(appDataPath);

		cfxPath += "\\" + str;

		return cfxPath;
	}

	return "";
}

#include <Error.h>

#include <cpr/cpr.h>

std::string GetOwnershipPath();

std::string GetEntitlementBlock(uint64_t accountId, const std::string& machineHash)
{
	if (!LoadOwnershipTicket())
	{
		FatalError("RS10");
	}

	std::string filePath = GetFilePath(fmt::sprintf("%08x_%lld", HashString(machineHash.c_str()), accountId));

	FILE* f = _wfopen(ToWide(filePath).c_str(), L"rb");

	if (f)
	{
		struct _stat64i32 stat;
		_wstat(ToWide(filePath).c_str(), &stat);

		if ((_time64(nullptr) - stat.st_mtime) > 259200)
		{
			fclose(f);
			f = nullptr;
		}
	}

	std::string outStr;

	bool success = false;

	if (f)
	{
		std::vector<uint8_t> fileData;
		int pos;

		// get the file length
		fseek(f, 0, SEEK_END);
		pos = ftell(f);
		fseek(f, 0, SEEK_SET);

		// resize the buffer
		fileData.resize(pos);

		// read the file and close it
		fread(&fileData[0], 1, pos, f);

		fclose(f);

		outStr = std::string(fileData.begin(), fileData.end());

		try
		{
			Botan::base64_decode(outStr);

			success = true;
		}
		catch (Botan::Exception&)
		{
			trace("Couldn't decode base64 entitlement data (%s) - refetching...\n", outStr);

			success = false;
		}
	}
	
	if (!success)
	{
		auto r = cpr::Post(
			cpr::Url{ "https://lambda.fivem.net/api/validate/entitlement" },
			cpr::Payload{
				{ "entitlementId", g_entitlementSource },
				{ "machineHash", machineHash },
				{ "rosId", fmt::sprintf("%lld", accountId) }
			});

		if (r.status_code != 200)
		{
			if (r.status_code < 500)
			{
				DeleteFileW(ToWide(GetOwnershipPath()).c_str());
			}

			FatalError("Could not contact entitlement service. Status code: %d, error message: %d/%s, response body: %s", r.status_code, (int)r.error.code, r.error.message, r.text);
		}

		f = _wfopen(ToWide(filePath).c_str(), L"wb");

		if (f)
		{
			fwrite(r.text.c_str(), 1, r.text.size(), f);
			fclose(f);
		}

		outStr = r.text;
	}

	return outStr;
}

#include <regex>
#include <array>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#if defined(IS_RDR3)
bool GetMTLSessionInfo(std::string& ticket, std::string& sessionTicket, std::array<uint8_t, 16>& sessionKey);

static std::string GetRosTicket(const std::string& body)
{
	auto postData = ParsePOSTString(body);

	std::string ticket;
	std::string sessionTicket;
	std::array<uint8_t, 16> sessionKeyArray;
	
	assert(GetMTLSessionInfo(ticket, sessionTicket, sessionKeyArray));

	std::string sessionKey = Botan::base64_encode(sessionKeyArray.data(), 16);

	rapidjson::Document doc2;
	doc2.SetObject();

	doc2.AddMember("ticket", rapidjson::Value(ticket.c_str(), doc2.GetAllocator()), doc2.GetAllocator());
	doc2.AddMember("sessionKey", rapidjson::Value(sessionKey.c_str(), doc2.GetAllocator()), doc2.GetAllocator());
	doc2.AddMember("sessionTicket", rapidjson::Value(sessionTicket.c_str(), doc2.GetAllocator()), doc2.GetAllocator());
	doc2.AddMember("payload", rapidjson::Value(postData["payload"].c_str(), doc2.GetAllocator()), doc2.GetAllocator());

	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> w(sb);

	doc2.Accept(w);

	auto r = cpr::Post(cpr::Url{ "http://localhost:32891/ros/validate" },
		cpr::Body{ std::string(sb.GetString(), sb.GetLength()) });

	trace("%s\n", r.text);

	return r.text;
}
#endif

static InitFunction initFunction([] ()
{
	EndpointMapper* mapper = Instance<EndpointMapper>::Get();

#if defined(IS_RDR3)
	mapper->AddGameService("entitlements.asmx/GetTitleAccessTokenEntitlementBlock", [](const std::string& body)
	{
		return GetRosTicket(body);
	});
#endif

	mapper->AddGameService("entitlements.asmx/GetEntitlementBlock", [] (const std::string& body)
	{
		auto postData = ParsePOSTString(body);

		auto accountId = ROS_DUMMY_ACCOUNT_ID;
		auto machineHash = postData["machineHash"];

		auto outStr = GetEntitlementBlock(accountId, machineHash);

		return fmt::sprintf(
			"<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"0.574\" xmlns=\"GetEntitlementBlockResponse\"><Status>1</Status><Result Version=\"1\"><Data>%s</Data></Result></Response>",
			outStr
		);
	});

#if defined(IS_RDR3)
	mapper->AddGameService("entitlements.asmx/GetEntitlementBlock2", [](const std::string& body)
	{
		return GetRosTicket(body);
	});
#endif

	mapper->AddGameService("entitlements.asmx/GetEntitlements", [] (const std::string& body)
	{
		return "<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"0\" xmlns=\"EntitlementsResponse\">\r\n\r\n  <Status>1</Status>\r\n\r\n  <Entitlements xsi:type=\"EntitlementsListXmlMD5\">\r\n\r\n    <Entitlement InstanceId=\"1\" EntitlementCode=\"1972D87D58D9790D41A19FCDC1C3600A\" FriendlyName=\"$500,000 for Grand Theft Auto V Story Mode\" Count=\"1\" Visible=\"true\" Type=\"Durable\">\r\n\r\n      <CreatedDate>2015-04-14T00:00:00.000Z</CreatedDate>\r\n\r\n    </Entitlement>\r\n\r\n    <Entitlement InstanceId=\"2\" EntitlementCode=\"27BF767F361818E864967CBF808DC6C2\" FriendlyName=\"Access to Grand Theft Auto V for PC\" Count=\"1\" Visible=\"false\" Type=\"Durable\">\r\n\r\n      <CreatedDate>2015-04-14T00:00:00.000Z</CreatedDate>\r\n\r\n    </Entitlement>\r\n\r\n<Entitlement InstanceId=\"3\" EntitlementCode=\"4D754F8EF1B135DBD3DDDE760A9352DA\" FriendlyName=\"Access to Grand Theft Auto V for PC\" Count=\"1\" Visible=\"true\" Type=\"Durable\"><CreatedDate>2015-04-14T00:00:00.000Z</CreatedDate></Entitlement><Entitlement InstanceId=\"4\" EntitlementCode=\"4748A48AFB22BAE2FD6A4506655B2D95\" FriendlyName=\"Access to Grand Theft Auto V for PC Steam\" Count=\"1\" Visible=\"true\" Type=\"Durable\">\r\n\r\n      <CreatedDate>2015-04-14T00:00:000Z</CreatedDate>\r\n\r\n    </Entitlement>\r\n\r\n  </Entitlements>\r\n\r\n</Response>";
	});

	mapper->AddGameService("GeoLocation.asmx/GetRelayServers", [] (const std::string& body)
	{
		return "<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"15.6263\" xmlns=\"RegionBucketLookUpResponse\"><Status>1</Status><LocInfo RegionCode=\"3\" Longitude=\"0.0\" Latitude=\"0.0\" CountryCode=\"US\" /><RelaysList Count=\"1\" IsSecure=\"false\"><Server Host=\"185.56.65.153:61456\" IsXblSg=\"false\" /></RelaysList></Response>";
	});

	mapper->AddGameService("matchmaking.asmx/Find", [](const std::string& body)
	{
		return R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="281" xmlns="FindResponse">
    <Status>1</Status>
    <Results Count="1">
        <R MatchId="feedbabe-feed-feed-1212-91533740d43e" Owner="40404">
            <Data>AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=</Data>
            <Attributes>0,0,1107077218,0,14,0,3,0,0,0</Attributes>
        </R>
    </Results>
</Response>)";
	});

	mapper->AddGameService("matchmaking.asmx/Advertise", [] (const std::string& body)
	{
		trace("advertise: %s\n", body);

		return "<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"15.6263\" xmlns=\"AdvertiseResponse\"><Status>1</Status><MatchId>875fd057-fe8d-4145-a4e1-76b57a81817d</MatchId></Response>";
	});

	mapper->AddGameService("matchmaking.asmx/Unadvertise", [] (const std::string& body)
	{
		return "<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"15.6263\" xmlns=\"UnadvertiseResponse\"><Status>1</Status></Response>";
	});

	mapper->AddGameService("matchmaking.asmx/Update", [] (const std::string& body)
	{
		return "<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"15.6263\" xmlns=\"UpdateResponse\"><Status>1</Status></Response>";
	});

	mapper->AddGameService("socialclub.asmx/CreateScAuthToken", [] (const std::string& body)
	{
		return "<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"0\" xmlns=\"CreateScAuthToken\"><Status>1</Status><Result>AAAAArgQdyps/xBHKUumlIADBO75R0gAekcl3m2pCg3poDsXy9n7Vv4DmyEmHDEtv49b5BaUWBiRR/lVOYrhQpaf3FJCp4+22ETI8H0NhuTTijxjbkvDEViW9x6bOEAWApixmQue2CNN3r7X8vQ/wcXteChEHUHi</Result></Response>";
	});

	mapper->AddGameService("socialclub.asmx/CreateScAuthToken2", [](const std::string& body)
	{
		return "<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"0\" ScAuthToken=\"AAAAArgQdyps/xBHKUumlIADBO75R0gAekcl3m2pCg3poDsXy9n7Vv4DmyEmHDEtv49b5BaUWBiRR/lVOYrhQpaf3FJCp4+22ETI8H0NhuTTijxjbkvDEViW9x6bOEAWApixmQue2CNN3r7X8vQ/wcXteChEHUHi\" xmlns=\"CreateScAuthToken2\"><Status xmlns=\"CreateScAuthTokenResponse\">1</Status></Response>";
	});

	mapper->AddGameService("auth.asmx/ExchangeTicket", [](const std::string& body)
	{
		return GetRockstarTicketXml();
	});

	mapper->AddGameService("auth.asmx/CreateScAuthToken2", [](const std::string& body)
	{
		return "<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"0\" ScAuthToken=\"AAAAArgQdyps/xBHKUumlIADBO75R0gAekcl3m2pCg3poDsXy9n7Vv4DmyEmHDEtv49b5BaUWBiRR/lVOYrhQpaf3FJCp4+22ETI8H0NhuTTijxjbkvDEViW9x6bOEAWApixmQue2CNN3r7X8vQ/wcXteChEHUHi\" xmlns=\"CreateScAuthToken2\"><Status xmlns=\"CreateScAuthTokenResponse\">1</Status></Response>";
	});

	mapper->AddGameService("socialclub.asmx/CheckText", [] (const std::string& body)
	{
		return "<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"0\" xmlns=\"CheckText\"><Status>1</Status></Response>";
	});

	mapper->AddGameService("ugc.asmx/CheckText", [](const std::string& body)
	{
		return "<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"0\" xmlns=\"CheckText\"><Status>1</Status></Response>";
	});

	mapper->AddGameService("ProfileStatGroups.asmx/ReadByGroup", [](const std::string& body)
	{
		return R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="0" xmlns="ReadUnrankedStatsResponse">
    <Status>1</Status>
    <Results count="0" total="0" />
</Response>)";
	});

	mapper->AddGameService("Friends.asmx/GetFriends", [](const std::string& body)
	{
		return R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="0" xmlns="GetFriends">
    <Status>1</Status>
    <Result Count="0" Total="0" />
</Response>)";
	});

	mapper->AddGameService("Presence.asmx/GetPresenceServers", [](const std::string& body)
	{
		return R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="0" xmlns="GetFriends">
    <Status>1</Status>
    <Result Count="0" Total="0" />
</Response>)";
	});

	mapper->AddGameService("Conductor.svc/Route", [](const std::string& body)
	{
		json v = json::parse(body);
		int viewIdx = v["request"]["views"][0].value("view", 0);

		return fmt::sprintf(R"({"Result":{"tokenInfo":{"token":"GSTOKEN token=\"RDR2\",signature=\"RDR2\"","ttlSeconds":300},"views":[{"view":%d,"uri":"ws:\/\/cfx-web-rdr2-prod.ros.rockstargames.com:80","sandboxViewChannel":%d,"gatewayChannel":1337}]},"ms":15,"Status":1})", viewIdx, viewIdx);
	});

	mapper->AddGameService("Conductor.svc/ImpersonateRoute", [](const std::string& body)
	{
		json v = json::parse(body);
		int viewIdx = v["request"]["views"][0].value("view", 0);

		return fmt::sprintf(R"({"Result":{"tokenInfo":{"token":"GSTOKEN token=\"RDR2\",signature=\"RDR2\"","ttlSeconds":300},"views":[{"view":%d,"uri":"ws:\/\/cfx-web-rdr2-prod.ros.rockstargames.com:80","sandboxViewChannel":%d,"gatewayChannel":1337}]},"ms":15,"Status":1})", viewIdx, viewIdx);
	});

	mapper->AddGameService("Rdr2Title.asmx/TitlePromotion", [](const std::string& body)
	{
		trace("returning title promotions for %s\n", body);

		return R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="15" xmlns="TitlePromotion">
    <Status>1</Status>
    <Result>
        <Promotions count="4" lut="1574334907">
            <P n="SP_GAME_CONTENT_CROSS_PROMOTION_REVOLVER" v="0" gm="0" />
            <P n="SP_GAME_CONTENT_CROSS_PROMOTION_HATCHET" v="0" gm="0" />
            <P n="AWARD_MP_GAME_CONTENT_CROSS_PROMOTION_MASK" v="0" gm="1" />
            <P n="AWARD_MP_GAME_CONTENT_CROSS_PROMOTION_CARDS" v="0" gm="1" />
        </Promotions>
    </Result>
</Response>)";
	});

	mapper->AddGameService("ugc.asmx/QueryContent", [](const std::string& body)
	{
		trace("ugc body: %s\n", body);

		/*auto r = cpr::Post(
			cpr::Url{ "http://localhost:8902/ugc.asmx/QueryContent" },
			cpr::Body{ body },
			cpr::Header{ { "content-type", "application/x-www-form-urlencoded" } });

		return r.text;*/

		return R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="93" xmlns="QueryContent">
	<Status>1</Status>
	<Result Count="0" Total="0" Hash="0">
	</Result>
</Response>)";
	});

	mapper->AddGameService("achievements.asmx/GetPlayerAchievements", [](const std::string& body)
	{
		return R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="0" xmlns="GetPlayerAchievements">
    <Status>1</Status>
    <Result Count="0" Total="0" />
</Response>)";
	});

	mapper->AddGameService("Friends.asmx/GetBlocked", [](const std::string& body)
	{
		return R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="0" xmlns="GetFriends">
    <Status>1</Status>
    <Result Count="0" Total="0" />
</Response>)";
	});

	mapper->AddGameService("Friends.asmx/CountAll", [](const std::string& body)
	{
		return R"(<?xml version="1.0" encoding="utf-8"?>
<TResponseOfCountAllResult xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="0" xmlns="http://services.ros.rockstargames.com/">
    <Status>1</Status>
    <Result b="0" f="0" ir="0" is="0" />
</TResponseOfCountAllResult>)";
	});

	mapper->AddGameService("App.asmx/GetBuildManifestFullNoAuth", [](const std::string& body)
	{
		auto postData = ParsePOSTString(body);

		if (postData["branchAccessToken"].find("YAFA") != std::string::npos)
		{
			return R"(
<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="0" xmlns="GetBuildManifestFullNoAuth">
  <Status>1</Status>
  <Result BuildId="59" VersionNumber="1.0.8.161" BuildDateUtc="2019-11-05T10:11:09.2666667">
    <FileManifest>
      <!--<FileDetails FileEntryId="973" FileEntryVersionId="9369" FileSize="36705936" TimestampUtc="2019-10-29T13:11:52.1166667">
        <RelativePath>Launcher.exe</RelativePath>
        <SHA256Hash>1f88c8fe80c9a7776dcbf99202a292d368d17e471c47768ce89de23aede389c3</SHA256Hash>
        <FileChunks>
          <Chunk FileChunkId="11625" SHA256Hash="1f88c8fe80c9a7776dcbf99202a292d368d17e471c47768ce89de23aede389c3" StartByteOffset="0" Size="36705936" />
        </FileChunks>
      </FileDetails>-->
    </FileManifest>
    <IsPreload>false</IsPreload>
  </Result>
</Response>)";
		}

		return R"(<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="RetrieveFileChunkNoAuth" ms="0">
<Status>0</Status>
<Error Code="Expired" CodeEx="BranchAccessToken"/>
</Response>)";
	});

	mapper->AddGameService("app.asmx/RetrieveFileChunkMulti", [](const std::string& body)
	{
		// fileEntryAndChunkIdCsv=1%3a1

		return R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="0" xmlns="RetrieveFileChunkNoAuthMulti">
  <Status>1</Status>
  <Result>
    <Chunk FileEntryId="1" FileChunkId="1">
      <RedirectUrl>http://fivem.net/a.dll</RedirectUrl>
    </Chunk>
  </Result>
</Response>)";
	});

	mapper->AddGameService("app.asmx/GetTitleAccessToken", [](const std::string& body)
	{
		return R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="15" xmlns="GetTitleAccessToken">
    <Status>1</Status>
    <Result>TITLEACCESS token="GAME",signature="GAME"</Result>
</Response>)";

#if 0
		auto r = cpr::Post(
			cpr::Url{ "http://localhost:8902/app.asmx/GetTitleAccessToken" },
			cpr::Payload{
				{ "titleId", "13" }
			});

		auto t = r.text;

		auto a = t.find("<Result>") + 8;
		auto b = t.find("</Result>");

		auto tkn = t.substr(a, b - a);

		trace("%s\n", tkn);

		return fmt::sprintf(R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="15" xmlns="GetTitleAccessToken">
    <Status>1</Status>
    <Result>%s</Result>
</Response>)", tkn);
#endif
	});

	mapper->AddGameService("App.asmx/GetBuildManifestFull", [](const std::string& body)
	{
		auto postData = ParsePOSTString(body);

		if (postData["branchAccessToken"].find("RDR2") != std::string::npos)
		{
			std::stringstream rss;
			rss << "<!--";

			for (int i = 0; i < 131072 / 26; i++)
			{
				rss << "abcdefghijklmnopqrstuvqxyz";
			}

			rss << "-->";

			auto rs = rss.str();
			rs = "";

			return fmt::sprintf(R"(
<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="0" xmlns="GetBuildManifestFull">
  <Status>1</Status>
  <Result BuildId="63" VersionNumber="1.0.1207.77" BuildDateUtc="2019-11-05T11:39:37.0266667">
    <FileManifest>
		<FileDetails FileEntryId="9178" FileEntryVersionId="9648" FileSize="89394320" TimestampUtc="2019-11-05T11:39:34.8800000">
			<RelativePath>RDR2.exe</RelativePath>
			<SHA256Hash>c526511cb96324c43c706ebdd36a33689fa2fe41f63e0f728b3b70fd7e7ec025</SHA256Hash>
			<FileChunks>
				<Chunk FileChunkId="13046" SHA256Hash="c526511cb96324c43c706ebdd36a33689fa2fe41f63e0f728b3b70fd7e7ec025" StartByteOffset="0" Size="89394320" />
			</FileChunks>
		</FileDetails>
%s
    </FileManifest>
    <IsPreload>false</IsPreload>
  </Result>
</Response>)", rs);
		}
		else if (postData["branchAccessToken"].find("GTA5") != std::string::npos)
		{
			return fmt::sprintf(R"(
<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="0" xmlns="GetBuildManifestFull">
  <Status>1</Status>
  <Result BuildId="80" VersionNumber="1.0.1604.1" BuildDateUtc="2019-11-05T11:39:37.0266667">
    <FileManifest>
		<FileDetails FileEntryId="9178" FileEntryVersionId="9648" FileSize="72484280" TimestampUtc="2019-11-05T11:39:34.8800000">
			<RelativePath>GTA5.exe</RelativePath>
			<SHA256Hash>f5912107843d200a91c7c59e8cf6f504acfbd0a527cc69558b3710a6ef8c9c33</SHA256Hash>
			<FileChunks>
				<Chunk FileChunkId="13046" SHA256Hash="f5912107843d200a91c7c59e8cf6f504acfbd0a527cc69558b3710a6ef8c9c33" StartByteOffset="0" Size="72484280" />
			</FileChunks>
		</FileDetails>
    </FileManifest>
    <IsPreload>false</IsPreload>
  </Result>
</Response>)");
		}

		return std::string{ R"(<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="RetrieveFileChunkNoAuth" ms="0">
<Status>0</Status>
<Error Code="Expired" CodeEx="BranchAccessToken"/>
</Response>)" };
	});

	mapper->AddGameService("app.asmx/GetDefaultApps", [](const std::string& body)
	{
		return R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="0" xmlns="GetDefaultApps">
  <Status>1</Status>
  <Result>
    <App Id="8" Name="lanoire" TitleId="9" IsReleased="true">
      <Branches />
    </App>
    <App Id="9" Name="mp3" TitleId="10" IsReleased="true">
      <Branches />
    </App>
    <App Id="3" Name="gta5" TitleId="11" IsReleased="true">
      <Branches />
    </App>
    <App Id="10" Name="rdr2" TitleId="13" IsReleased="true">
      <Branches />
    </App>
    <App Id="6" Name="gtasa" TitleId="18" IsReleased="true">
      <Branches />
    </App>
    <App Id="2" Name="launcher" TitleId="21" IsReleased="true">
      <Branches>
        <Branch Id="4" Name="default" BuildId="59" IsDefault="true" AppId="2">
          <AccessToken>BRANCHACCESS token="YAFA",signature="YAFA"</AccessToken>
        </Branch>
      </Branches>
    </App>
    <App Id="4" Name="bully" TitleId="23" IsReleased="true">
      <Branches />
    </App>
    <App Id="1" Name="lanoirevr" TitleId="24" IsReleased="true">
      <Branches />
    </App>
    <App Id="5" Name="gta3" TitleId="26" IsReleased="true">
      <Branches />
    </App>
    <App Id="7" Name="gtavc" TitleId="27" IsReleased="true">
      <Branches />
    </App>
  </Result>
</Response>)";
	});

	mapper->AddGameService("app.asmx/GetApps", [](const std::string& body)
	{
		return R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="0" xmlns="GetApps">
  <Status>1</Status>
  <Result>
    <App Id="8" Name="lanoire" TitleId="9" IsReleased="true">
      <Branches />
    </App>
    <App Id="9" Name="mp3" TitleId="10" IsReleased="true">
      <Branches />
    </App>
    <App Id="3" Name="gta5" TitleId="11" IsReleased="true">
      <Branches>
        <Branch Id="13" Name="default" BuildId="80" IsDefault="true" AppId="3">
          <AccessToken>BRANCHACCESS token="GTA5",signature="GTA5"</AccessToken>
        </Branch>
      </Branches>
    </App>
    <App Id="10" Name="rdr2" TitleId="13" IsReleased="true">
		<!--<Branches />-->
      <Branches>
        <Branch Id="12" Name="default" BuildId="63" IsDefault="true" AppId="10">
          <AccessToken>BRANCHACCESS token="RDR2",signature="RDR2"</AccessToken>
        </Branch>
      </Branches>
    </App>
    <App Id="6" Name="gtasa" TitleId="18" IsReleased="true">
      <Branches />
    </App>
    <App Id="2" Name="launcher" TitleId="21" IsReleased="true">
      <Branches>
        <Branch Id="4" Name="default" BuildId="59" IsDefault="true" AppId="2">
          <AccessToken>BRANCHACCESS token="YAFA",signature="YAFA"</AccessToken>
        </Branch>
      </Branches>
    </App>
    <App Id="4" Name="bully" TitleId="23" IsReleased="true">
      <Branches />
    </App>
    <App Id="1" Name="lanoirevr" TitleId="24" IsReleased="true">
      <Branches />
    </App>
    <App Id="5" Name="gta3" TitleId="26" IsReleased="true">
      <Branches />
    </App>
    <App Id="7" Name="gtavc" TitleId="27" IsReleased="true">
      <Branches />
    </App>
  </Result>
</Response>)";
	});


	/*mapper->AddGameService("Telemetry.asmx/SubmitCompressed", [](const std::string& body)
	{
		size_t dataStart = body.find("data=");

		// decompress the json buffer
		std::vector<uint8_t> tempBytes(65535);

		size_t destLength = tempBytes.size();

		{
			z_stream stream;
			int err;

			stream.next_in = (z_const Bytef *)body.c_str() + dataStart;
			stream.avail_in = (uInt)body.length() - dataStart;

			stream.next_out = &tempBytes[0];
			stream.avail_out = (uInt)destLength;

			stream.zalloc = (alloc_func)0;
			stream.zfree = (free_func)0;

			err = inflateInit2(&stream, -15);
			if (err = Z_OK)
			{
				err = inflate(&stream, Z_FINISH);
				destLength = stream.total_out;

				err = inflateEnd(&stream);
			}
		}

		return "<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"0\" xmlns=\"CheckText\"><Status>1</Status></Response>";
	});*/
});
