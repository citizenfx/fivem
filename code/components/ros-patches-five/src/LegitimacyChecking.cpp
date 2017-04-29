#include <StdInc.h>

#include <KnownFolders.h>
#include <ShlObj.h>

#include <dpapi.h>

#include <cpr/cpr.h>

#include <sstream>

#include <botan/cbc.h>

#include <Error.h>

__declspec(dllexport) void IDidntDoNothing()
{

}

// {E091E21C-C61F-49F6-8560-CEF64DC42002}
#define INITGUID
#include <guiddef.h>

// {38D8F400-AA8A-4784-A9F0-26A08628577E}
DEFINE_GUID(CfxStorageGuid,
	0x38d8f400, 0xaa8a, 0x4784, 0xa9, 0xf0, 0x26, 0xa0, 0x86, 0x28, 0x57, 0x7e);

#pragma comment(lib, "rpcrt4.lib")

std::string GetOwnershipPath()
{
    PWSTR appDataPath;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &appDataPath))) {
        std::string cfxPath = ToNarrow(appDataPath) + "\\DigitalEntitlements";
        CreateDirectory(ToWide(cfxPath).c_str(), nullptr);

        CoTaskMemFree(appDataPath);

        RPC_CSTR str;
        UuidToStringA(&CfxStorageGuid, &str);

        cfxPath += "\\";
        cfxPath += (char*)str;

        RpcStringFreeA(&str);

        return cfxPath;
    }

    return "";
}

std::string GetMachineGuid()
{
    std::vector<wchar_t> data(128);
    DWORD size = data.size();

    if (RegGetValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", L"MachineGuid", RRF_RT_REG_SZ, nullptr, &data[0], &size) == 0)
    {
        return ToNarrow(std::wstring(data.data(), max((int)size - 1, 128) / sizeof(wchar_t)));
    }

    throw std::exception();
}

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

std::string g_entitlementSource;

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
				g_entitlementSource = doc["guid"].GetString();
				return true;
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

bool VerifySteamOwnership()
{
    // init Steam
    SetEnvironmentVariable(L"SteamAppId", L"271590");

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
            fprintf(f, "%d", 271590);
            fclose(f);
        }

        if (!SteamAPI_Init())
        {
            return false;
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
    if (!SteamApps()->BIsSubscribedApp(271590))
    {
        return false;
    }

    // verify remote ownership
    uint8_t ticket[16384] = { 0 };
    uint32_t ticketLength;
    SteamUser()->GetAuthSessionTicket(ticket, sizeof(ticket), &ticketLength);

    char outHex[16384];
    tohex(ticket, ticketLength, outHex, sizeof(outHex));

    std::string s = outHex;

    // call into remote validation API
	auto r = cpr::Post(cpr::Url{ "https://lambda.fivem.net/api/validate/entitlement/steam" },
                       cpr::Payload{ {"ticket", s } });

	if (r.status_code == 200)
	{
		g_entitlementSource = r.text;
		return true;
	}

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

bool VerifyRetailOwnership()
{
    std::string outArg;
    if (FAILED(RunCor(L"v4.0.30319", MakeRelativeCitPath(L"OwnershipUI.dll").c_str(), L"OwnershipUI.InitUX", &outArg)))
    {
		std::string exceptionStr;

		std::wstring tempPath = _wgetenv(L"TEMP");
		tempPath += L"\\FiveM_OwnershipUICrash.log";

		FILE* ef = _wfopen(tempPath.c_str(), L"rb");

		if (ef)
		{
			char buffer[65536];
			int len = fread(buffer, 1, sizeof(buffer), ef);

			buffer[len] = '\0';

			fclose(ef);

			exceptionStr = buffer;

			_wunlink(tempPath.c_str());
		}

        FatalError("Could not initialize the Common Language Runtime for validation. Make sure you've installed the .NET Framework 4.0 or higher.\n%s", exceptionStr);
    }

    if (outArg.empty())
    {
        return false;
    }

    std::string body;

    {
        std::stringstream ss(outArg);
        std::string username;
        std::string password;

        std::getline(ss, username, '\n');
        std::getline(ss, password, '\n');
        
        rapidjson::Document doc2;
        doc2.SetObject();

        doc2.AddMember("username", rapidjson::Value(username.c_str(), doc2.GetAllocator()), doc2.GetAllocator());
        doc2.AddMember("password", rapidjson::Value(password.c_str(), doc2.GetAllocator()), doc2.GetAllocator());

        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> w(sb);

        doc2.Accept(w);
        
        body = std::string(sb.GetString(), sb.GetSize());
    }

    auto r = cpr::Post(cpr::Url{ "http://localhost:32891/ros/login" },
        cpr::Body{ body });

    if (!r.error && r.status_code == 200)
    {
        rapidjson::Document doc;
        doc.Parse(r.text.c_str());

        std::string ticket = doc["Ticket"].GetString();
        std::string sessionKey = doc["SessionKey"].GetString();
        std::string sessionTicket = doc["SessionTicket"].GetString();

        Botan::AutoSeeded_RNG rng;
        auto machineHash = rng.random_vec(32);
		*(uint64_t*)&machineHash[4] = atoi(doc["RockstarId"].GetString()) ^ 0xDEADCAFEBABEFEED;

        rapidjson::Document doc2;
        doc2.SetObject();

        doc2.AddMember("ticket", rapidjson::Value(ticket.c_str(), doc2.GetAllocator()), doc2.GetAllocator());
        doc2.AddMember("sessionKey", rapidjson::Value(sessionKey.c_str(), doc2.GetAllocator()), doc2.GetAllocator());
        doc2.AddMember("sessionTicket", rapidjson::Value(sessionTicket.c_str(), doc2.GetAllocator()), doc2.GetAllocator());
        doc2.AddMember("machineHash", rapidjson::Value(Botan::base64_encode(machineHash).c_str(), doc2.GetAllocator()), doc2.GetAllocator());

        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> w(sb);

        doc2.Accept(w);

        auto b = cpr::Post(cpr::Url{ "http://localhost:32891/ros/validate" },
            cpr::Body{ std::string(sb.GetString(), sb.GetLength()) });

        if (!b.error && b.status_code == 200)
        {
            auto blob = Botan::base64_decode(b.text);
            
            auto cbc = new Botan::CBC_Decryption(new EntitlementBlockCipher(), new Botan::Null_Padding());
            cbc->start(&blob[4], 16);

            cbc->finish(blob, 20);

            auto entitlementBlock = EntitlementBlock::Read(&blob[20]);

            if (entitlementBlock->IsValid())
            {
                if (time(nullptr) < entitlementBlock->GetExpirationDate())
                {
                    if (entitlementBlock->GetMachineHash() == Botan::base64_encode(machineHash))
                    {
                        // if (entitlementBlock->GetRockstarId() == someID)
                        {
                            std::istringstream stream(entitlementBlock->GetXml());

                            boost::property_tree::ptree tree;
                            boost::property_tree::read_xml(stream, tree);

                            for (auto& p : tree.get_child("EntitlementsListXml"))
                            {
                                if (p.first == "Entitlement")
                                {
                                    std::string friendlyName = p.second.get<std::string>("<xmlattr>.FriendlyName");

                                    if (friendlyName == "Access to Grand Theft Auto V for PC" || friendlyName == "Access to Grand Theft Auto V for PC Steam")
                                    {
										auto r = cpr::Post(cpr::Url{ "https://lambda.fivem.net/api/validate/entitlement/ros" },
											cpr::Payload{ { "rosData", b.text } });

										if (r.error)
										{
											FatalError("Error generating ROS entitlement token: %d (%s)", (int)r.error.code, r.error.message);
											return false;
										}

										if (r.status_code == 200)
										{
											g_entitlementSource = r.text;
											return true;
										}
                                    }
                                }
                            }
                        }
                    }
                }
            }

			MessageBox(nullptr, va(L"The Social Club account specified (%s) does not own a valid license to Grand Theft Auto V.", ToWide(doc["OrigNickname"].GetString())), L"Authentication error", MB_OK | MB_ICONWARNING);
        }
        else if (!b.error)
        {
            MessageBox(nullptr, ToWide(b.text).c_str(), L"Authentication error", MB_OK | MB_ICONWARNING);
        }
    }
    else if (!r.error)
    {
        MessageBox(nullptr, ToWide(r.text).c_str(), L"Authentication error", MB_OK | MB_ICONWARNING);
    }

    return false;
}

bool LegitimateCopy()
{
    return LoadOwnershipTicket() || (VerifySteamOwnership() && SaveOwnershipTicket(g_entitlementSource)) || (VerifyRetailOwnership() && SaveOwnershipTicket(g_entitlementSource));
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