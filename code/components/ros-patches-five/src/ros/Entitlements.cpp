/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ros/EndpointMapper.h>

#include <base64.h>

#include <zlib.h>

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

static InitFunction initFunction([] ()
{
	EndpointMapper* mapper = Instance<EndpointMapper>::Get();

	mapper->AddGameService("entitlements.asmx/GetEntitlementBlock", [] (const std::string& body)
	{
		auto postData = ParsePOSTString(body);

		//return "<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"0\" xmlns=\"GetEntitlementBlockResponse\"><Status>1</Status><Result Version=\"1\"><Data>%s</Data></Result></Response>";
		auto accountId = ROS_DUMMY_ACCOUNT_ID;
		auto machineHash = postData["machineHash"];

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
		}
		else
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
				FatalError("RS20");
			}

			f = _wfopen(ToWide(filePath).c_str(), L"wb");

			if (f)
			{
				fwrite(r.text.c_str(), 1, r.text.size(), f);
				fclose(f);
			}

			outStr = r.text;
		}

		return fmt::sprintf(
			"<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"0.574\" xmlns=\"GetEntitlementBlockResponse\"><Status>1</Status><Result Version=\"1\"><Data>%s</Data></Result></Response>",
			outStr
		);
	});

	mapper->AddGameService("entitlements.asmx/GetEntitlements", [] (const std::string& body)
	{
		return "<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"0\" xmlns=\"EntitlementsResponse\">\r\n\r\n  <Status>1</Status>\r\n\r\n  <Entitlements xsi:type=\"EntitlementsListXmlMD5\">\r\n\r\n    <Entitlement InstanceId=\"1\" EntitlementCode=\"1972D87D58D9790D41A19FCDC1C3600A\" FriendlyName=\"$500,000 for Grand Theft Auto V Story Mode\" Count=\"1\" Visible=\"true\" Type=\"Durable\">\r\n\r\n      <CreatedDate>2015-04-14T00:00:00.000Z</CreatedDate>\r\n\r\n    </Entitlement>\r\n\r\n    <Entitlement InstanceId=\"2\" EntitlementCode=\"27BF767F361818E864967CBF808DC6C2\" FriendlyName=\"Access to Grand Theft Auto V for PC\" Count=\"1\" Visible=\"false\" Type=\"Durable\">\r\n\r\n      <CreatedDate>2015-04-14T00:00:00.000Z</CreatedDate>\r\n\r\n    </Entitlement>\r\n\r\n<Entitlement InstanceId=\"3\" EntitlementCode=\"4D754F8EF1B135DBD3DDDE760A9352DA\" FriendlyName=\"Access to Grand Theft Auto V for PC\" Count=\"1\" Visible=\"true\" Type=\"Durable\"><CreatedDate>2015-04-14T00:00:00.000Z</CreatedDate></Entitlement><Entitlement InstanceId=\"4\" EntitlementCode=\"4748A48AFB22BAE2FD6A4506655B2D95\" FriendlyName=\"Access to Grand Theft Auto V for PC Steam\" Count=\"1\" Visible=\"true\" Type=\"Durable\">\r\n\r\n      <CreatedDate>2015-04-14T00:00:000Z</CreatedDate>\r\n\r\n    </Entitlement>\r\n\r\n  </Entitlements>\r\n\r\n</Response>";
	});

	mapper->AddGameService("GeoLocation.asmx/GetRelayServers", [] (const std::string& body)
	{
		return "<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"15.6263\" xmlns=\"RegionBucketLookUpResponse\"><Status>1</Status><LocInfo RegionCode=\"3\" Longitude=\"0.0\" Latitude=\"0.0\" CountryCode=\"US\" /><RelaysList Count=\"1\" IsSecure=\"false\"><Server Host=\"185.56.65.153:61456\" IsXblSg=\"false\" /></RelaysList></Response>";
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

	mapper->AddGameService("socialclub.asmx/CheckText", [] (const std::string& body)
	{
		return "<?xml version=\"1.0\" encoding=\"utf-8\"?><Response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ms=\"0\" xmlns=\"CheckText\"><Status>1</Status></Response>";
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