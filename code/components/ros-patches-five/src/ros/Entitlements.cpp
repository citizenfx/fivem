/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <Hooking.h>

#include <ros/EndpointMapper.h>

#include <LegitimacyAPI.h>

#include <base64.h>
#include <botan/base64.h>
#include <botan/exceptn.h>

#include <zlib.h>

#include <sstream>

#include <json.hpp>

#include <cpr/cpr.h>
#include <ICoreGameInit.h>

#include <CrossBuildRuntime.h>
#include "CnlEndpoint.h"
#include <ShlObj.h>
#include <KnownFolders.h>
#include <Error.h>
#include <cpr/cpr.h>

#include <regex>
#include <array>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include "FormData.h"

struct BuildData
{
	std::string version;
	size_t size;
	int build;
	std::string sha256;
};

std::unordered_map<int, BuildData> buildInfoRDR3 = {
	{ 1491, { "1.0.1491.50", 89562608, 89, "b56c9548f670654a9b73bf25def3cd73af12e269f6e47dba28a34079adaf465e" } },
	{ 1436, { "1.0.1436.31", 89104336, 84, "1ed4b36caad567acbed0cc3b3c917cb46843e82a88097cc8015efadeeb88b097" } },
	{ 1355, { "1.0.1355.18", 84664448, 80, "e80698b7f53395912d34bbbf15ac852b452126e7315b3a5115a6d9a30ad33d4c" } },
	{ 1311, { "1.0.1311.20", 91439232, 79, "2fa69ba127e8c43b06bbec8e65a5fb94359194d961e8067466ca9c7fb6ffba90" } },
};

std::unordered_map<int, BuildData> buildInfoGTA5 = {
	{ 3407, {"1.0.3407.0", 54341608, 107, "a947ba8627560835b4c28c9bb3c2ed2cbb5874cfec235e50e162ffc12e6455b2" } },
	{ 3323, { "1.0.3323.0", 57496560, 105, "ed2322439789e7e8f94c8a1b3c1b80d4bcc48c86ed277d42dbf40fc28d412051" } },
	{ 3258, { "1.0.3258.0", 56066032, 104, "9ec3afccec172a55712bede2dd41d2c330b6d803f9b61b723c8914616ef25aea" } },
	{ 3095, { "1.0.3095.0", 49634800, 103, "4c663c738e184ea60b3c3208147c3815605d98d1802ec08107b2c22ac5f2c46d" } },
	{ 2944, { "1.0.2944.0", 49828848, 101, "eb0125ab36004ccdba7bfa918dee37035fb9e23448b850256fd6141ffe194466" } },
	{ 2802, { "1.0.2802.0", 46709592, 98, "3af30164562e302f249c32b5cf4159793ee2c408749ee6cdea8adafbfc466c03" } },
	{ 2699, { "1.0.2699.0", 61111680, 96, "7dc4a3a7516522e4a4b5869f2b09053da6b6829ab333ca0aa14a43aa45fd1e17" } },
	{ 2612, { "1.0.2612.0", 60351952, 95, "06b59a02747c3d9f74c6c3621756387f4d85b148a882f4a6735a03383875c1f9" } },
	{ 2545, { "1.0.2545.0", 59988376, 94, "964a8c25af4f622aedcd0de13717d261a07ccfcc734f5885bcfd5f173330cd06" } },
	{ 2372, { "1.0.2372.0", 59716912, 92, "7e9009bce1aa47b4fc0e216c8d11ec8aaae2a83ba8140463f6f38b98a8a1492e" } },
	{ 2189, { "1.0.2189.0", 63124096, 88, "3f83e88b7ac80b4cf4a1f60a8ac8241b1c6c7235b897db315778c2f31a98cdeb" } },
	{ 2060, { "1.0.2060.0", 60589184, 83, "b21d443583b432ee4333bcd1179f4336f63071d90d55b6177c7588b21dbf61f0" } },
	{ 1604, { "1.0.1604.1", 72484280, 80, "f5912107843d200a91c7c59e8cf6f504acfbd0a527cc69558b3710a6ef8c9c33" } },
};

using json = nlohmann::json;

int StoreDecryptedBlob(void* a1, void* a2, uint32_t a3, void* inOutBlob, uint32_t a5, void* a6);

bool LoadOwnershipTicket();
std::string GetRockstarTicketXml();

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
			cpr::Url{ CNL_ENDPOINT "api/validate/entitlement" },
			cpr::Payload{
				{ "entitlementId", ros::GetEntitlementSource() },
				{ "machineHash", machineHash },
				{ "rosId", fmt::sprintf("%lld", accountId) }
			});

		if (r.error || r.status_code >= 400)
		{
			if (r.status_code == 401)
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

class LambdaHttpHandler : public net::HttpHandler
{
public:
	template<typename TFn>
	LambdaHttpHandler(TFn&& fn)
	{
		m_function = fn;
	}

	virtual bool HandleRequest(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response) override
	{
		if (m_function)
		{
			m_function(request, response);
			return true;
		}

		return false;
	}

private:
	std::function<void(const fwRefContainer<net::HttpRequest> & request, const fwRefContainer<net::HttpResponse> & response)> m_function;
};

#if defined(IS_RDR3)
bool GetMTLSessionInfo(std::string& ticket, std::string& sessionTicket, std::array<uint8_t, 16>& sessionKey, uint64_t& accountId);

static std::string GetRosTicket(const std::string& body)
{
	auto postData = net::DecodeFormData(body);

	std::string ticket;
	std::string sessionTicket;
	std::array<uint8_t, 16> sessionKeyArray;
	uint64_t accountId = 0;
	
	assert(GetMTLSessionInfo(ticket, sessionTicket, sessionKeyArray, accountId));

	std::string sessionKey = Botan::base64_encode(sessionKeyArray.data(), 16);

	rapidjson::Document doc2;
	doc2.SetObject();

	doc2.AddMember("ticket", rapidjson::Value(ticket.c_str(), doc2.GetAllocator()), doc2.GetAllocator());
	doc2.AddMember("sessionKey", rapidjson::Value(sessionKey.c_str(), doc2.GetAllocator()), doc2.GetAllocator());
	doc2.AddMember("sessionTicket", rapidjson::Value(sessionTicket.c_str(), doc2.GetAllocator()), doc2.GetAllocator());

	if (postData.find("payload") != postData.end())
	{
		doc2.AddMember("payload", rapidjson::Value(postData["payload"].c_str(), doc2.GetAllocator()), doc2.GetAllocator());
	}
	else
	{
		doc2.AddMember("machineHash", rapidjson::Value(postData["machineHash"].c_str(), doc2.GetAllocator()), doc2.GetAllocator());
		doc2.AddMember("locale", rapidjson::Value(postData["locale"].c_str(), doc2.GetAllocator()), doc2.GetAllocator());
	}

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
		auto postData = net::DecodeFormData(body);

		auto accountId = ROS_DUMMY_ACCOUNT_ID;
		auto machineHash = postData["machineHash"];

#ifdef GTA_FIVE
		auto outStr = GetEntitlementBlock(accountId, machineHash);
#elif IS_RDR3
		auto outStr = GetRosTicket(body);
#else
		auto outStr = "AAAA";
#endif

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


	mapper->AddGameService("matchmaking.asmx/Find", [] (const std::string& body)
	{
		{

			return std::string(R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="281" xmlns="FindResponse">
    <Status>1</Status>
    <Results Count="1">
        <R MatchId="feedbabe-feed-feed-1212-91533740d43e" Owner="40404">
            <Data>AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=</Data>
            <Attributes>0,0,1107077218,0,14,0,3,0,0,0</Attributes>
        </R>
    </Results>
</Response>)");
		}
	});

	mapper->AddGameService("Presence.asmx/Query", [](const std::string& body)
	{
		trace("query: %s\n", body);

		return R"(<?xml version="1.0" encoding="utf-8"?><Response><Status>0</Status></Response>)";
	});

	mapper->AddGameService("matchmaking.asmx/Advertise", [] (const std::string& body)
	{
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

	static int photoIdx = 0;
	static std::map<int, nlohmann::json> photoData;

	mapper->AddPrefix("/uploadPhoto", new LambdaHttpHandler([](fwRefContainer<net::HttpRequest> req, fwRefContainer<net::HttpResponse> res)
	{
		req->SetDataHandler([req, res](const std::vector<uint8_t>& postData)
		{
			auto idx = req->GetHeader("X-Agile-Authorization", "-1");
			auto dataIt = photoData.find(std::stoi(idx));

			if (dataIt == photoData.end())
			{
				res->WriteHead(400);
				res->End("Bad request.");

				return;
			}

			auto& data = dataIt->second;

			std::string token;
			std::string clientId;

			if (Instance<ICoreGameInit>::Get()->GetData("discourseUserToken", &token) &&
				Instance<ICoreGameInit>::Get()->GetData("discourseClientId", &clientId))
			{
				// pass Multipart as lvalue to work around bug in cpr
				// https://github.com/whoshuu/cpr/issues/216#issuecomment-450474117
				auto mp = cpr::Multipart{
						{ "type", "image" },
						{ "files[]", cpr::Buffer{postData.begin(), postData.end(), "image.jpg"} }
				};

				auto h = cpr::Header{
					{ "User-Agent", "CitizenFX/Five" },
					{ "User-Api-Client-Id", clientId },
					{ "User-Api-Key", token },
				};

				auto upload = cpr::Post(cpr::Url{ "https://forum.cfx.re/uploads.json" },
					h,
					mp);

				if (!upload.error && upload.status_code < 400)
				{
					auto resp = upload.text;
					auto dataRef = nlohmann::json::parse(data["DataJson"].get<std::string>());

					auto json = nlohmann::json::parse(resp);

					if (json.value("id", 0) != 0)
					{
						auto url = json.value("short_url", "");
						auto tw = json.value("thumbnail_width", 0);
						auto th = json.value("thumbnail_height", 0);

						std::string name;
						net::UrlDecode(data.value("ContentName", ""), name);

						auto b = nlohmann::json::object({
							{ "category", 68 },
							{ "title", name },
							{ "raw", fmt::sprintf("![image|%dx%d](%s) \n\n[details=\"Metadata\"]\n```json\n%s\n```\n[/details]", tw, th, url, dataRef.dump(4)) },
							{ "tags", nlohmann::json::array({"gta5photo", dataRef.value("area", "somewhere")}) }
						});

						h["Content-Type"] = "application/json; charset=utf-8";

						auto post = cpr::Post(cpr::Url{ "https://forum.cfx.re/posts.json" },
							h,
							cpr::Body{ b.dump() });

						if (!post.error && post.status_code < 400)
						{
							trace("posted: %s\n", post.text);

							res->WriteHead(200);
							res->End("OK!");
							return;
						}
					}
				}
			}

			res->WriteHead(400);
			res->End("Failed to upload.");
		});
	}));

	mapper->AddGameService("ugc.asmx/CreateContent", [](const std::string& body)
	{
		auto postData = net::DecodeFormData(body);
		auto jsonData = nlohmann::json::parse(postData["paramsJson"]);

		if (postData["contentType"] == "gta5photo")
		{
			std::string token;
			std::string clientId;

			if (Instance<ICoreGameInit>::Get()->GetData("discourseUserToken", &token) &&
				Instance<ICoreGameInit>::Get()->GetData("discourseClientId", &clientId))
			{
				auto jsonDataNested = nlohmann::json::parse(jsonData["DataJson"].get<std::string>());
				jsonDataNested["llat"] = fmt::sprintf("%d", photoIdx);
				jsonDataNested["lldir"] = "ugc/gta5photo";
				jsonDataNested["llfn"] = "1.jpg";
				jsonDataNested["llurl"] = "http://localhost:32891/uploadPhoto";

				photoData[photoIdx] = jsonData;
				photoIdx++;

				return fmt::sprintf(R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="640" xmlns="CreateContent">
    <Status>1</Status>
    <Result ci="1234567890123456789012" cd="1" n="hey that's me" rci="1234567890123456789012" ud="1" u="1" v="1">
        <da>
            <![CDATA[%s]]>
        </da>
    </Result>
</Response>)", jsonDataNested.dump());
			}
		}

		return std::string(R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="640" xmlns="CreateContent">
<Status>0</Status>
</Response>)");
	});

mapper->AddGameService("ugc.asmx/Publish", [](const std::string& body)
{
		return R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="640" xmlns="Publish">
    <Status>1</Status>
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
		auto postData = net::DecodeFormData(body);

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

	mapper->AddGameService("legalpolicies.asmx/GetAcceptedVersion", [](const std::string& body)
	{
		auto postData = net::DecodeFormData(body);

		return fmt::sprintf(R"(<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="0" xmlns="http://services.ros.rockstargames.com/">
  <Status>1</Status>
  <Version>4</Version>
  <PolicyTag>%s</PolicyTag>
</Response>)",
		postData["policyTag"]);
	});

	mapper->AddGameService("App.asmx/GetBuildManifestFull", [](const std::string& body)
	{
		auto postData = net::DecodeFormData(body);

		std::unordered_map<int, BuildData> buildInfo{};
		std::string gameExe;

		if (postData["branchAccessToken"].find("RDR2") != std::string::npos)
		{
			buildInfo = buildInfoRDR3;
			gameExe = "RDR2.exe";
		}
		else if (postData["branchAccessToken"].find("GTA5") != std::string::npos)
		{
			buildInfo = buildInfoGTA5;
			gameExe = "GTA5.exe";
		}

		if (const auto& info = buildInfo.find(xbr::GetGameBuild()); info != buildInfo.end())
		{
			return fmt::sprintf(R"(
<?xml version="1.0" encoding="utf-8"?>
<Response xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" ms="0" xmlns="GetBuildManifestFull">
  <Status>1</Status>
  <Result BuildId="%d" VersionNumber="%s" BuildDateUtc="2019-11-05T11:39:37.0266667">
    <FileManifest>
		<FileDetails FileEntryId="9178" FileEntryVersionId="9648" FileSize="%d" TimestampUtc="2019-11-05T11:39:34.8800000">
			<RelativePath>%s</RelativePath>
			<SHA256Hash>%s</SHA256Hash>
			<FileChunks>
				<Chunk FileChunkId="13046" SHA256Hash="%s" StartByteOffset="0" Size="%d" />
			</FileChunks>
		</FileDetails>
    </FileManifest>
    <IsPreload>false</IsPreload>
  </Result>
</Response>)", info->second.build, info->second.version, info->second.size, gameExe, info->second.sha256, info->second.sha256, info->second.size);
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
	<App Id="12" Name="rdr2_rdo" TitleId="13" IsReleased="false">
		<Branches/>
	</App>
	<App Id="13" Name="rdr2_sp" TitleId="13" IsReleased="false">
		<Branches/>
	</App>
	<App Id="14" Name="rdr2_sp_rgl" TitleId="13" IsReleased="false">
		<Branches/>
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
		return fmt::sprintf(R"(<?xml version="1.0" encoding="utf-8"?>
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
        <Branch Id="13" Name="default" BuildId="%d" IsDefault="true" AppId="3">
          <AccessToken>BRANCHACCESS token="GTA5",signature="GTA5"</AccessToken>
        </Branch>
      </Branches>
    </App>
    <App Id="10" Name="rdr2" TitleId="13" IsReleased="true">
		<!--<Branches />-->
      <Branches>
        <Branch Id="12" Name="default" BuildId="%d" IsDefault="true" AppId="10">
          <AccessToken>BRANCHACCESS token="RDR2",signature="RDR2"</AccessToken>
        </Branch>
      </Branches>
    </App>
    <App Id="12" Name="rdr2_rdo" TitleId="13" IsReleased="false">
		<!--<Branches />-->
      <Branches>
        <Branch Id="14" Name="default" BuildId="-1" IsDefault="true" AppId="12">
          <AccessToken>BRANCHACCESS token="RDR2",signature="RDR2"</AccessToken>
        </Branch>
      </Branches>
    </App>
    <App Id="13" Name="rdr2_sp" TitleId="13" IsReleased="false">
		<Branches />
      <!--<Branches>
        <Branch Id="16" Name="default" BuildId="-1" IsDefault="true" AppId="13">
          <AccessToken>BRANCHACCESS token="RDR2",signature="RDR2"</AccessToken>
        </Branch>
      </Branches>-->
    </App>
    <App Id="14" Name="rdr2_sp_rgl" TitleId="13" IsReleased="false">
		<Branches />
      <!--<Branches>
        <Branch Id="15" Name="default" BuildId="-1" IsDefault="true" AppId="14">
          <AccessToken>BRANCHACCESS token="RDR2",signature="RDR2"</AccessToken>
        </Branch>
      </Branches>-->
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
</Response>)",
		buildInfoGTA5[xbr::GetGameBuild()].build,
		buildInfoRDR3[xbr::GetGameBuild()].build);
	});
});
