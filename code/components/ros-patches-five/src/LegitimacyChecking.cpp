#include <StdInc.h>

#include <KnownFolders.h>
#include <ShlObj.h>

#include <dpapi.h>

#include <cpr/cpr.h>

#include <sstream>

#include <LegitimacyAPI.h>

#include <botan/cbc.h>

#include <Error.h>

#include "CnlEndpoint.h"

__declspec(dllexport) void IDidntDoNothing()
{

}

extern std::string GetROSEmail();

// {E091E21C-C61F-49F6-8560-CEF64DC42002}
#define INITGUID
#include <guiddef.h>

// {38D8F400-AA8A-4784-A9F0-26A08628577E}
DEFINE_GUID(CfxStorageGuid,
	0x38d8f400, 0xaa8a, 0x4784, 0xa9, 0xf0, 0x26, 0xa0, 0x86, 0x28, 0x57, 0x7e);

// {45ACDD04-ECA8-4C35-9622-4FAB4CA16E14}
DEFINE_GUID(CfxStorageGuidRDR,
	0x45acdd04, 0xeca8, 0x4c35, 0x96, 0x22, 0x4f, 0xab, 0x4c, 0xa1, 0x6e, 0x14);

// {86219F24-F0E4-47C3-9D1F-4C7B156087EE}
DEFINE_GUID(CfxStorageGuidNY,
	0x86219f24, 0xf0e4, 0x47c3, 0x9d, 0x1f, 0x4c, 0x7b, 0x15, 0x60, 0x87, 0xee);


#pragma comment(lib, "rpcrt4.lib")

std::string GetOwnershipPath()
{
    PWSTR appDataPath;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &appDataPath);

    if (SUCCEEDED(hr))
	{
        std::string cfxPath = ToNarrow(appDataPath) + "\\DigitalEntitlements";
		if (!CreateDirectory(ToWide(cfxPath).c_str(), nullptr))
		{
			auto error = GetLastError();

			if (error != ERROR_ALREADY_EXISTS)
			{
				FatalError("CreateDirectory for %s failed: GetLastError = 0x%x\n\nMake sure your AppData folder is not write-protected.", cfxPath, error);
			}
		}

        CoTaskMemFree(appDataPath);

        RPC_CSTR str;

#ifdef GTA_FIVE
        UuidToStringA(&CfxStorageGuid, &str);
#elif defined(IS_RDR3)
		UuidToStringA(&CfxStorageGuidRDR, &str);
#elif defined(GTA_NY)
		UuidToStringA(&CfxStorageGuidNY, &str);
#else
#error No entitlement GUID?
#endif

        cfxPath += "\\";
        cfxPath += (char*)str;

        RpcStringFreeA(&str);

        return cfxPath;
    }

	FatalError("SHGetKnownFolderPath for FOLDERID_LocalAppData failed: HRESULT = 0x%08x\n\nMake sure your AppData folder is not write-protected.", hr);
    return "";
}

std::string GetMachineGuid()
{
    std::vector<wchar_t> data(128);
    DWORD size = data.size();

    if (RegGetValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", L"MachineGuid", RRF_RT_REG_SZ, nullptr, &data[0], &size) == 0)
    {
        return ToNarrow(std::wstring(data.data(), fwMax((int)size - 1, 128) / sizeof(wchar_t)));
    }

    throw std::exception();
}

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

std::string g_entitlementSource;

__declspec(noinline) static void SetEntitlementSource(const std::string& entitlementSource)
{
	g_entitlementSource = entitlementSource;
}

__declspec(noinline) static bool HasEntitlementSource()
{
	return !g_entitlementSource.empty();
}

bool LoadOwnershipTicket()
{
    std::string filePath = GetOwnershipPath();

    FILE* f = _wfopen(ToWide(filePath).c_str(), L"rb");

    if (!f)
    {
        return false;
    }

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

    // decrypt the stored data - setup blob
    DATA_BLOB cryptBlob;
    cryptBlob.pbData = &fileData[0];
    cryptBlob.cbData = fileData.size();

    DATA_BLOB outBlob;

    // call DPAPI
    if (CryptUnprotectData(&cryptBlob, nullptr, nullptr, nullptr, nullptr, 0, &outBlob))
    {
        // parse the file
        std::string data(reinterpret_cast<char*>(outBlob.pbData), outBlob.cbData);
        
        // free the out data
        LocalFree(outBlob.pbData);

		rapidjson::Document doc;
		doc.Parse(data.c_str(), data.size());

		if (!doc.HasParseError())
		{
			if (doc.IsObject())
			{
				SetEntitlementSource(doc["guid"].GetString());

				if (HasEntitlementSource())
				{
					return true;
				}
			}
		}
    }

    return false;
}

bool SaveOwnershipTicket(const std::string& guid)
{
	rapidjson::Document doc;
	doc.SetObject();
	doc.AddMember("guid", rapidjson::Value(guid.c_str(), guid.size(), doc.GetAllocator()), doc.GetAllocator());

	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> w(sb);

	if (!doc.Accept(w))
	{
		return false;
	}

    // encrypt the actual string
    DATA_BLOB cryptBlob;
    cryptBlob.pbData = reinterpret_cast<uint8_t*>(const_cast<char*>(sb.GetString()));
    cryptBlob.cbData = sb.GetLength();

    DATA_BLOB outBlob;

    std::string filePath = GetOwnershipPath();

    FILE* f = _wfopen(ToWide(filePath).c_str(), L"wb");

    if (!f)
    {
        return false;
    }

    if (CryptProtectData(&cryptBlob, nullptr, nullptr, nullptr, nullptr, 0, &outBlob))
    {
        fwrite(outBlob.pbData, 1, outBlob.cbData, f);
        fclose(f);

        LocalFree(outBlob.pbData);
    }

    return true;
}

#include <steam/steam_api.h>

void tohex(unsigned char* in, size_t insz, char* out, size_t outsz)
{
    unsigned char* pin = in;
    const char* hex = "0123456789abcdef";
    char* pout = out;
    for (; pin < in + insz; pout += 2, pin++)
    {
        pout[0] = hex[(*pin >> 4) & 0xF];
        pout[1] = hex[*pin & 0xF];
        if (pout + 3 - out > outsz)
        {
            break;
        }
    }
    pout[0] = 0;
}

std::string GetAuthSessionTicket(uint32_t appID)
{
	static uint32_t lastAppID;

	if (appID != 0)
	{
		lastAppID = appID;
	}
	else
	{
		appID = lastAppID;
	}

	// init Steam
	SetEnvironmentVariable(L"SteamAppId", fmt::sprintf(L"%d", appID).c_str());

	{
		struct deleter
		{
			~deleter()
			{
				_unlink("steam_appid.txt");
			}
		} deleter;

		FILE* f = fopen("steam_appid.txt", "w");

		if (f)
		{
			fprintf(f, "%d", appID);
			fclose(f);
		}

		if (!SteamAPI_Init())
		{
			return "";
		}
	}

	struct shutdown
	{
		~shutdown()
		{
			SteamAPI_Shutdown();
		}
	} shutdown;

	// get local ownership
	if (!SteamApps()->BIsSubscribed() ||
		SteamApps()->GetAppOwner() != SteamUser()->GetSteamID())
	{
		return "";
	}

	// verify remote ownership
	static uint8_t ticket[16384] = { 0 };
	uint32_t ticketLength;
	SteamUser()->GetAuthSessionTicket(ticket, sizeof(ticket), &ticketLength);

	static char outHex[16384];
	tohex(ticket, ticketLength, outHex, sizeof(outHex));

	return outHex;
}

bool VerifySteamOwnership()
{
	// unsupported
	return false;
}

#include <botan/base64.h>
#include <botan/auto_rng.h>
#include <botan/key_filt.h>
#include <botan/pipe.h>

#include <EntitlementTables_1.h>
#include <EntitlementTables_2.h>

uint8_t* entitlement_tables_dec;
uint8_t* entitlement_key_dec;

void SetupEntitlementTables()
{
    if (!entitlement_tables_dec)
    {
        Botan::Pipe pipe(Botan::get_cipher("AES-256/CBC", Botan::SymmetricKey("7af5ce4e0224189eb1a0f7870a6a3c76bb10141dc26801ea37d142d96ff62675"), Botan::InitializationVector("58e742124051d7434ae7e0dbbfc89c23"), Botan::DECRYPTION));
        pipe.process_msg(entitlement_tables, entitlement_tables_len);

        auto decryptedTables = pipe.read_all(0);

        Botan::Pipe pipe2(Botan::get_cipher("AES-256/CBC", Botan::SymmetricKey("7af5ce4e0224189eb1a0f7870a6a3c76bb10141dc26801ea37d142d96ff62675"), Botan::InitializationVector("58e742124051d7434ae7e0dbbfc89c23"), Botan::DECRYPTION));
        pipe2.process_msg(entitlement_key, entitlement_key_len);

        auto decryptedKey = pipe2.read_all(0);

        entitlement_tables_dec = new uint8_t[decryptedTables.size()];
        memcpy(entitlement_tables_dec, decryptedTables.data(), decryptedTables.size());

        entitlement_key_dec = new uint8_t[decryptedKey.size()];
        memcpy(entitlement_key_dec, decryptedKey.data(), decryptedKey.size());
    }
}

class EntitlementBlockCipher : public Botan::BlockCipher
{
public:
    virtual Botan::BlockCipher* clone() const override
    {
        return new EntitlementBlockCipher();
    }

    virtual void clear() override
    {

    }

    virtual size_t block_size() const override
    {
        return 16;
    }

    virtual void encrypt_n(const byte in[], byte out[],
        size_t blocks) const override
    {
        throw std::exception();
    }

private:
    inline const uint32_t* GetSubkey(int idx) const
    {
        uint32_t* keyUints = (uint32_t*)entitlement_key_dec;
        return &keyUints[4 * idx];
    }

    inline const uint32_t* GetDecryptTable(int idx) const
    {
        uint32_t* dtUints = (uint32_t*)entitlement_tables_dec;
        return &dtUints[16 * 256 * idx];
    }

    inline const uint32_t* GetDecryptBytes(const uint32_t* tables, int idx) const
    {
        return &tables[256 * idx];
    }

public:
    virtual void decrypt_n(const byte in[], byte out[],
        size_t blocks) const override
    {
        SetupEntitlementTables();

        for (size_t i = 0; i < blocks; i++)
        {
            const byte* inBuf = &in[16 * i];
            byte* outBuf = &out[16 * i];

            DecryptRoundA(inBuf, outBuf, GetSubkey(0), GetDecryptTable(0));
            DecryptRoundA(outBuf, outBuf, GetSubkey(1), GetDecryptTable(1));

            for (int j = 2; j <= 15; j++)
            {
                DecryptRoundB(outBuf, outBuf, GetSubkey(j), GetDecryptTable(j));
            }

            DecryptRoundA(outBuf, outBuf, GetSubkey(16), GetDecryptTable(16));
        }
    }

private:
    void DecryptRoundA(const byte in[], byte out[], const uint32_t* key, const uint32_t* table) const
    {
        uint32_t x1 = GetDecryptBytes(table, 0)[in[0]] ^
            GetDecryptBytes(table, 1)[in[1]] ^
            GetDecryptBytes(table, 2)[in[2]] ^
            GetDecryptBytes(table, 3)[in[3]] ^
            key[0];

        uint32_t x2 = GetDecryptBytes(table, 4)[in[4]] ^
            GetDecryptBytes(table, 5)[in[5]] ^
            GetDecryptBytes(table, 6)[in[6]] ^
            GetDecryptBytes(table, 7)[in[7]] ^
            key[1];

        uint32_t x3 = GetDecryptBytes(table, 8)[in[8]] ^
            GetDecryptBytes(table, 9)[in[9]] ^
            GetDecryptBytes(table, 10)[in[10]] ^
            GetDecryptBytes(table, 11)[in[11]] ^
            key[2];

        uint32_t x4 = GetDecryptBytes(table, 12)[in[12]] ^
            GetDecryptBytes(table, 13)[in[13]] ^
            GetDecryptBytes(table, 14)[in[14]] ^
            GetDecryptBytes(table, 15)[in[15]] ^
            key[3];

        *(uint32_t*)&out[0] = x1;
        *(uint32_t*)&out[4] = x2;
        *(uint32_t*)&out[8] = x3;
        *(uint32_t*)&out[12] = x4;
    }

    void DecryptRoundB(const byte in[], byte out[], const uint32_t* key, const uint32_t* table) const
    {
        uint32_t x1 = GetDecryptBytes(table, 0)[in[0]] ^
            GetDecryptBytes(table, 7)[in[7]] ^
            GetDecryptBytes(table, 10)[in[10]] ^
            GetDecryptBytes(table, 13)[in[13]] ^
            key[0];

        uint32_t x2 = GetDecryptBytes(table, 1)[in[1]] ^
            GetDecryptBytes(table, 4)[in[4]] ^
            GetDecryptBytes(table, 11)[in[11]] ^
            GetDecryptBytes(table, 14)[in[14]] ^
            key[1];

        uint32_t x3 = GetDecryptBytes(table, 2)[in[2]] ^
            GetDecryptBytes(table, 5)[in[5]] ^
            GetDecryptBytes(table, 8)[in[8]] ^
            GetDecryptBytes(table, 15)[in[15]] ^
            key[2];

        uint32_t x4 = GetDecryptBytes(table, 3)[in[3]] ^
            GetDecryptBytes(table, 6)[in[6]] ^
            GetDecryptBytes(table, 9)[in[9]] ^
            GetDecryptBytes(table, 12)[in[12]] ^
            key[3];

        out[0] = (byte)((x1 >> 0) & 0xFF);
        out[1] = (byte)((x1 >> 8) & 0xFF);
        out[2] = (byte)((x1 >> 16) & 0xFF);
        out[3] = (byte)((x1 >> 24) & 0xFF);
        out[4] = (byte)((x2 >> 0) & 0xFF);
        out[5] = (byte)((x2 >> 8) & 0xFF);
        out[6] = (byte)((x2 >> 16) & 0xFF);
        out[7] = (byte)((x2 >> 24) & 0xFF);
        out[8] = (byte)((x3 >> 0) & 0xFF);
        out[9] = (byte)((x3 >> 8) & 0xFF);
        out[10] = (byte)((x3 >> 16) & 0xFF);
        out[11] = (byte)((x3 >> 24) & 0xFF);
        out[12] = (byte)((x4 >> 0) & 0xFF);
        out[13] = (byte)((x4 >> 8) & 0xFF);
        out[14] = (byte)((x4 >> 16) & 0xFF);
        out[15] = (byte)((x4 >> 24) & 0xFF);
    }

public:
    virtual Botan::Key_Length_Specification key_spec() const override
    {
        return Botan::Key_Length_Specification(272);
    }

    virtual void key_schedule(const byte key[], size_t length) override
    {
        // yeah you wish
    }

    virtual std::string name() const override
    {
        return "RAEA(tm) (c) OpenIV-Putin-Team";
    }
};

static uint32_t BigLong(uint32_t val)
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (val << 16) | (val >> 16);
}

static uint64_t BigLongLong(uint64_t val)
{
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
    return (val << 32) | (val >> 32);
}

class EntitlementBlock
{
public:
    static std::shared_ptr<EntitlementBlock> Read(const uint8_t* buffer, const uint8_t* rsaPubKey = nullptr);

public:
    inline uint64_t GetRockstarId() const
    {
        return m_rockstarId;
    }

    inline const std::string& GetMachineHash() const
    {
        return m_machineHash;
    }

    inline const std::string& GetXml() const
    {
        return m_xml;
    }

    inline time_t GetExpirationDate() const
    {
        return m_date;
    }

    inline bool IsValid() const
    {
        return m_valid;
    }

private:
    uint64_t m_rockstarId;
    std::string m_machineHash;
    std::string m_xml;
    time_t m_date;
    bool m_valid;
};

#include <botan/sha160.h>
#include <botan/pubkey.h>
#include <botan/rsa.h>
#include <botan/ber_dec.h>

#include <RSAKey.h>

#include <iomanip>

std::shared_ptr<EntitlementBlock> EntitlementBlock::Read(const uint8_t* buffer, const uint8_t* rsaPubKey)
{
	if (!rsaPubKey)
	{
		rsaPubKey = rsa_pub;
	}

	const uint8_t* start = buffer;

	uint32_t length = BigLong(*(uint32_t*)buffer);
	buffer += 4;

	uint64_t rockstarId = BigLongLong(*(uint64_t*)buffer);
	buffer += 8;

	uint32_t machineHashSize = BigLong(*(uint32_t*)buffer);
	buffer += 4;

	std::string machineHash((const char*)buffer, machineHashSize);
	buffer += machineHashSize;

	uint32_t dateSize = BigLong(*(uint32_t*)buffer);
	buffer += 4;

	std::string date((const char*)buffer, dateSize);
	buffer += dateSize;

	uint32_t xmlSize = BigLong(*(uint32_t*)buffer);
	buffer += 4;

	std::string xml((const char*)buffer, xmlSize);
	buffer += xmlSize;

	uint32_t sigSize = BigLong(*(uint32_t*)buffer);
	buffer += 4;

	std::vector<uint8_t> sig(sigSize);
	memcpy(sig.data(), buffer, sigSize);

	// verify sig
	Botan::SHA_160 hashFunction;
	auto result = hashFunction.process(&start[4], length);

	std::vector<uint8_t> msg(result.size() + 1);
	msg[0] = 2;
	memcpy(&msg[1], &result[0], result.size());

	Botan::BigInt n, e;

	Botan::BER_Decoder(rsaPubKey, rsa_pub_len)
	.start_cons(Botan::SEQUENCE)
	.decode(n)
	.decode(e)
	.end_cons();

	Botan::AutoSeeded_RNG rng;
	auto pk = Botan::RSA_PublicKey(n, e);

	auto signer = std::make_unique<Botan::PK_Verifier>(pk, "EMSA_PKCS1(SHA-1)");

	bool valid = signer->verify_message(msg, sig);

	auto rv = std::make_shared<EntitlementBlock>();
	rv->m_machineHash = std::move(machineHash);
	rv->m_rockstarId = rockstarId;
	rv->m_xml = std::move(xml);

	std::istringstream ss(date);
	tm time;
	ss >> std::get_time(&time, "%Y-%m-%dT%H:%M:%S");

	rv->m_date = mktime(&time);

	rv->m_valid = valid;

	return rv;
}

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

HRESULT RunCor(PCWSTR pszVersion, PCWSTR pszAssemblyName,
	PCWSTR pszClassName, std::string* outArg);

void RunLegitimacyNui();

std::string g_rosData;
bool g_oldAge;

#include <array>
#if defined(IS_RDR3) || defined(GTA_FIVE) || defined(GTA_NY)
bool GetMTLSessionInfo(std::string& ticket, std::string& sessionTicket, std::array<uint8_t, 16>& sessionKey, uint64_t& accountId);
#endif

extern std::string GetExternalSteamTicket();

bool VerifyRetailOwnershipInternal(int pass)
{
	std::string ticket;
	std::string sessionKey;
	Botan::secure_vector<uint8_t> machineHash;
	std::array<uint8_t, 16> sessionKeyArray;
	std::string sessionTicket;
	uint64_t accountId = 0;

#ifndef IS_RDR3
	if (pass == 2)
	{
		trace(__FUNCTION__ ": Running legitimacy NUI.\n");

		RunLegitimacyNui();

		trace(__FUNCTION__ ": Returned from legitimacy NUI.\n");

		if (g_rosData.empty())
		{
			trace(__FUNCTION__ ": No ROS data received, exiting.\n");
			return false;
		}

		rapidjson::Document doc;
		doc.Parse(g_rosData.c_str());

		ticket = doc["Ticket"].GetString();
		sessionKey = doc["SessionKey"].GetString();
		sessionTicket = doc["SessionTicket"].GetString();

		Botan::AutoSeeded_RNG rng;
		machineHash = rng.random_vec(32);
		*(uint64_t*)&machineHash[4] = atoi(doc["RockstarId"].GetString()) ^ 0xDEADCAFEBABEFEED;
		accountId = atoi(doc["RockstarId"].GetString());

		trace(__FUNCTION__ ": Caught machine hash details from NUI.\n");
	}
#endif

	if (pass == 1)
	{
		if (!GetMTLSessionInfo(ticket, sessionTicket, sessionKeyArray, accountId))
		{
			return false;
		}

		Botan::AutoSeeded_RNG rng;
		machineHash = rng.random_vec(32);
		*(uint64_t*)&machineHash[4] = accountId ^ 0xDEADCAFEBABEFEED;

		sessionKey = Botan::base64_encode(sessionKeyArray.data(), 16);

		trace(__FUNCTION__ ": Caught machine hash details from MTL.\n");
	}

	rapidjson::Document doc2;
	doc2.SetObject();

	doc2.AddMember("ticket", rapidjson::Value(ticket.c_str(), doc2.GetAllocator()), doc2.GetAllocator());
	doc2.AddMember("sessionKey", rapidjson::Value(sessionKey.c_str(), doc2.GetAllocator()), doc2.GetAllocator());
	doc2.AddMember("sessionTicket", rapidjson::Value(sessionTicket.c_str(), doc2.GetAllocator()), doc2.GetAllocator());
	doc2.AddMember("machineHash", rapidjson::Value(Botan::base64_encode(machineHash).c_str(), doc2.GetAllocator()), doc2.GetAllocator());

	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> w(sb);

	doc2.Accept(w);

#ifdef GTA_FIVE
	trace(__FUNCTION__ ": Going to call /ros/validate.\n");

	auto b = cpr::Post(cpr::Url{ "http://localhost:32891/ros/validate" },
		cpr::Body{ std::string(sb.GetString(), sb.GetLength()) });

	if (!b.error && b.status_code == 200)
	{
		auto blob = Botan::base64_decode(b.text);

		trace(__FUNCTION__ ": Decrypting result.\n");

		auto cbc = new Botan::CBC_Decryption(new EntitlementBlockCipher(), new Botan::Null_Padding());
		cbc->start(&blob[4], 16);

		cbc->finish(blob, 20);

		auto entitlementBlock = EntitlementBlock::Read(&blob[20]);

		trace(__FUNCTION__ ": Fetched and decrypted result from CPR.\n");

		if (entitlementBlock->IsValid())
		{
			trace(__FUNCTION__ ": Valid entitlement block.\n");

			if (time(nullptr) < entitlementBlock->GetExpirationDate())
			{
				trace(__FUNCTION__ ": Valid expiration date.\n");

				if (entitlementBlock->GetMachineHash() == Botan::base64_encode(machineHash))
				{
					trace(__FUNCTION__ ": Valid account ID.\n");

					// if (entitlementBlock->GetRockstarId() == someID)
					{
						std::istringstream stream(entitlementBlock->GetXml());

						boost::property_tree::ptree tree;
						boost::property_tree::read_xml(stream, tree);

						for (auto& p : tree.get_child("EntitlementsListXml"))
						{
							if (p.first == "Entitlement")
							{
								try
								{
									bool valid = false;
									std::string match;

#ifdef GTA_FIVE
									std::string entitlementCode = p.second.get<std::string>("<xmlattr>.EntitlementCode");

									// epic
									if (entitlementCode == "04541FB36991EF2C43B38659B204EC3B941649666404498FB2CAD04B29E64A33" || entitlementCode == "F17551269FFD860D9749C30B21BDD96BF2613D26FEB3CCAFDC1D5AC9A1DE65F4")
									{
										if (!g_oldAge)
										{
											valid = true;
											match = entitlementCode;
										}
									}
#endif

									if (!valid)
									{
										std::string friendlyName = p.second.get<std::string>("<xmlattr>.FriendlyName");

#ifdef GTA_FIVE
										if (friendlyName == "Access to Grand Theft Auto V for PC" || friendlyName == "Access to Grand Theft Auto V for PC Steam")
#else
										if (friendlyName.find("Red Dead Redemption 2") == 0)
#endif
										{
											valid = true;
											match = friendlyName;
										}
									}

									if (valid)
									{
										trace(__FUNCTION__ ": Found matching entitlement for %s - creating token.\n", match);

										auto r = cpr::Post(cpr::Url{ CNL_ENDPOINT "api/validate/entitlement/rosfive" },
											cpr::Payload{
												{ "rosData", b.text },
												{
													"gameName",
													"gta5"
												},
												{ "steamSource", GetExternalSteamTicket() }
											});

										if (r.error)
										{
											FatalError("Error generating ROS entitlement token: %d (%s)", (int)r.error.code, r.error.message);
											return false;
										}

										if (r.status_code >= 500)
										{
											FatalError("Error generating ROS entitlement token: %d (%s)", (int)r.status_code, r.text);
											return false;
										}

										if (r.status_code == 200)
										{
											trace(__FUNCTION__ ": Got a token and saved it.\n");

											SetEntitlementSource(r.text);

											return true;
										}
									}
								}
								catch (const std::exception & e)
								{

								}
							}
						}
					}
				}
			}
		}

#ifdef GTA_FIVE
		// create a thread as CEF does something odd to the current thread's win32k functionality leading to a crash as it's already shut down
		// we pass using & since we join
		if (pass == 2)
		{
			std::thread([&]()
			{
				MessageBox(nullptr, va(L"The Social Club account specified (%s) does not own a valid license to Grand Theft Auto V.", ToWide(GetROSEmail())), L"Authentication error", MB_OK | MB_ICONWARNING);
			})
			.join();
		}
#endif
	}
	else if (!b.error)
	{
		trace(__FUNCTION__ ": Obtained error from CPR: %s - %d - %s.\n", b.error.message, b.status_code, b.text);

		std::thread([&b]()
		{
			MessageBox(nullptr, ToWide(b.text).c_str(), L"Authentication error", MB_OK | MB_ICONWARNING);
		}).join();
	}
#else
	auto r = cpr::Post(cpr::Url{ CNL_ENDPOINT "api/validate/entitlement/ros2" },
		cpr::Payload{
			{ "ticket", ticket },
			{ "gameName",
#ifdef GTA_NY
				"gta4"
#else
				"rdr3"
#endif
			},
			{ "sessionKey", sessionKey },
			{ "sessionTicket", sessionTicket },
			{ "rosId", fmt::sprintf("%d", accountId) },
		});

	if (r.error)
	{
		FatalError("Error generating ROS entitlement token: %d (%s)", (int)r.error.code, r.error.message);
		return false;
	}

	if (r.status_code >= 500)
	{
		FatalError("Error generating ROS entitlement token: %d (%s)", (int)r.status_code, r.text);
		return false;
	}

	if (r.status_code == 200)
	{
		trace(__FUNCTION__ ": Got a token and saved it.\n");

		SetEntitlementSource(r.text);

		return true;
	}

	if (r.status_code == 403)
	{
		FatalError(
			"Unable to verify ownership of Red Dead Redemption 2/Red Dead Online\n"
			"- Try launching the game through Steam/Epic Games Launcher first.\n"
			"- Wait until the game is fully running and close it.\n"
			"- Try launching RedM again."
		);
		return false;
	}
#endif

	return false;
}

bool VerifyRetailOwnership()
{
	if (!VerifyRetailOwnershipInternal(1))
	{
#ifndef IS_RDR3
		return VerifyRetailOwnershipInternal(2);
#else
		return false;
#endif
	}

	return true;
}

#include <coreconsole.h>

static ConVar<std::string>* tokenVar;

bool LegitimateCopy()
{
    return LoadOwnershipTicket() || (VerifySteamOwnership() && SaveOwnershipTicket(ros::GetEntitlementSource())) || (VerifyRetailOwnership() && SaveOwnershipTicket(ros::GetEntitlementSource()));
}

void VerifyOwnership(int parentPid)
{
    if (!LegitimateCopy())
    {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, parentPid);
        TerminateProcess(hProcess, 0x8000DEAD);
        TerminateProcess(GetCurrentProcess(), 0x8000DEAD);
    }
}

namespace ros
{
	__declspec(noinline) std::string GetEntitlementSource()
	{
		return g_entitlementSource;
	}
}

void LoadOwnershipEarly()
{
	// ConVar_ScriptRestricted because ownership ticket is sensitive information. Should never be exposed to 3rd party
	tokenVar = new ConVar<std::string>("cl_ownershipTicket", ConVar_ScriptRestricted, "");

	if (!tokenVar->GetValue().empty())
	{
		SaveOwnershipTicket(tokenVar->GetValue());
	}
}

static InitFunction initFunction([]()
{
	LoadOwnershipTicket();
});

static HookFunction hookFunction([]()
{
	LoadOwnershipTicket();

	tokenVar->GetHelper()->SetValue(ros::GetEntitlementSource());
});
