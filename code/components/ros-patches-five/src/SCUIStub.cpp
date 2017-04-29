/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ros/EndpointMapper.h>

#include <fstream>

#include <cpr/cpr.h>

#include "base64.h"

#include <botan/botan.h>
#include <botan/hash.h>
#include <botan/stream_cipher.h>
#include <botan/base64.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#define ROS_PLATFORM_KEY "C4pWJwWIKGUxcHd69eGl2AOwH2zrmzZAoQeHfQFcMelybd32QFw9s10px6k0o75XZeB5YsI9Q9TdeuRgdbvKsxc="

class ROSCryptoState
{
private:
    Botan::StreamCipher* m_rc4;

    uint8_t m_rc4Key[32];
    uint8_t m_xorKey[16];
    uint8_t m_hashKey[16];

public:
    ROSCryptoState();

    inline const uint8_t* GetXorKey()
    {
        return m_xorKey;
    }

    inline const uint8_t* GetHashKey()
    {
        return m_hashKey;
    }
};

class SCUIHandler : public net::HttpHandler
{
private:
	std::string m_scuiData;

public:
	SCUIHandler()
	{
		std::ifstream scuiFile(MakeRelativeCitPath(L"citizen/ros/scui.html"));

		m_scuiData = std::string(std::istreambuf_iterator<char>(scuiFile), std::istreambuf_iterator<char>());
	}

	bool HandleRequest(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response) override
	{
		response->SetStatusCode(200);
		response->SetHeader("Content-Type", "text/html; charset=utf-8");

		response->End(m_scuiData);

		return true;
	}
};

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <tinyxml2.h>

namespace
{
	template<typename TValue>
	void GetJsonValue(TValue value, rapidjson::Document& document, rapidjson::Value& outValue)
	{
		outValue.CopyFrom(rapidjson::Value(value), document.GetAllocator());
	}

	template<>
	void GetJsonValue<const char*>(const char* value, rapidjson::Document& document, rapidjson::Value& outValue)
	{
		outValue.CopyFrom(rapidjson::Value(value, document.GetAllocator()), document.GetAllocator());
	}
}

const char* GetROSVersionString()
{
    const char* baseString = va("e=%d,t=%s,p=%s,v=%d", 1, "gta5", "pcros", 11);

    // create the XOR'd buffer
    std::vector<uint8_t> xorBuffer(strlen(baseString) + 4);

    // set the key for the XOR buffer
    *(uint32_t*)&xorBuffer[0] = 0xC5C5C5C5;

    for (int i = 4; i < xorBuffer.size(); i++)
    {
        xorBuffer[i] = baseString[i - 4] ^ 0xC5;
    }

    // base64 the string
    size_t base64len;
    char* base64str = base64_encode(&xorBuffer[0], xorBuffer.size(), &base64len);

    // create a wide string version
    std::string str(base64str, base64len);

    free(base64str);

    // return va() version of the base64 string
    return va("ros %s", str);
}

void SaveAccountData(const std::string& data);
bool LoadAccountData(boost::property_tree::ptree& tree);
bool LoadAccountData(std::string& str);

std::string BuildPOSTString(const std::map<std::string, std::string>& fields)
{
    std::stringstream retval;

    for (auto& field : fields)
    {
        retval << field.first << "=" << url_encode(field.second) << "&";
    }

    std::string str = std::string(retval.str().c_str());
    return str.substr(0, str.length() - 1);
}

template<typename TAlloc>
auto HeadersHmac(const std::vector<uint8_t, TAlloc>& challenge, const char* method, const char* path, const std::string& sessionKey, const std::string& sessionTicket)
{
    auto hmac = std::unique_ptr<Botan::MessageAuthenticationCode>(Botan::get_mac("HMAC(SHA1)")->clone());

    ROSCryptoState cryptoState;

    // set the key
    uint8_t hmacKey[16];

    // xor the RC4 key with the platform key (and optionally the session key)
    auto rc4Xor = Botan::base64_decode(sessionKey);

    for (int i = 0; i < sizeof(hmacKey); i++)
    {
        hmacKey[i] = rc4Xor[i] ^ cryptoState.GetXorKey()[i];
    }

    hmac->set_key(hmacKey, sizeof(hmacKey));

    // method
    hmac->update(method);
    hmac->update(0);

    // path
    hmac->update(path);
    hmac->update(0);

    // ros-SecurityFlags
    hmac->update("239");
    hmac->update(0);

    // ros-SessionTicket
    hmac->update(sessionTicket);
    hmac->update(0);

    // ros-Challenge
    hmac->update(Botan::base64_encode(challenge));
    hmac->update(0);

    // platform hash key
    hmac->update(cryptoState.GetHashKey(), 16);

    // set the request header
    auto hmacValue = hmac->final();

    return hmacValue;
}

class LoginHandler2 : public net::HttpHandler
{
private:
    std::future<void> m_future;

public:
    std::string DecryptROSData(const char* data, size_t size, const std::string& sessionKey = "");

    std::string EncryptROSData(const std::string& input, const std::string& sessionKey = "");

    void ProcessLogin(const std::string& username, const std::string& password, const std::function<void(const std::string&, const std::string&)>& cb)
    {
        m_future = cpr::PostCallback([=](cpr::Response r)
        {
            if (r.error)
            {
                cb("Error contacting Rockstar Online Services.", "");
            }
            else
            {
                std::string returnedXml = DecryptROSData(r.text.data(), r.text.size());
                std::istringstream stream(returnedXml);

                boost::property_tree::ptree tree;
                boost::property_tree::read_xml(stream, tree);

                if (tree.get("Response.Status", 0) == 0)
                {
					auto code = tree.get<std::string>("Response.Error.<xmlattr>.Code");
					auto codeEx = tree.get<std::string>("Response.Error.<xmlattr>.CodeEx");

					if (code == "AuthenticationFailed" && codeEx == "LoginAttempts")
					{
						cb(va(
							"Login attempts exceeded. Please log in on https://socialclub.rockstargames.com/ and fill out the CAPTCHA, then try again."
						), "");
					}
					else
					{
						cb(va(
							"Could not sign on to the Social Club. Error code: %s/%s",
							code,
							codeEx
						), "");
					}

                    return;
                }
                else
                {
                    cb("", returnedXml);
                }
            }
        }, cpr::Url{ "http://ros.citizenfx.internal/gta5/11/gameservices/auth.asmx/CreateTicketSc3" }, cpr::Body{ EncryptROSData(BuildPOSTString({
            {"ticket", ""},
            {"email", (username.find('@') != std::string::npos) ? username : "" },
            {"nickname", (username.find('@') == std::string::npos) ? username : "" },
            {"password", password},
            {"platformName", "pcros"}}))
        }, cpr::Header{
            {"User-Agent", GetROSVersionString()},
            {"Host", "prod.ros.rockstargames.com"}
        });
    }

    void HandleValidateRequest(const rapidjson::Document& document, fwRefContainer<net::HttpResponse> response)
    {
        std::string ticket = document["ticket"].GetString();
        std::string sessionKey = document["sessionKey"].GetString();
        std::string sessionTicket = document["sessionTicket"].GetString();
        std::string machineHash = document["machineHash"].GetString();

        auto cb = [=](const std::string& error, const std::string& data)
        {
            if (!error.empty())
            {
                response->SetStatusCode(403);
                response->SetHeader("Content-Type", "text/plain; charset=utf-8");
                response->End(error);
            }
            else
            {
                std::istringstream stream(data);

                boost::property_tree::ptree tree;
                boost::property_tree::read_xml(stream, tree);

                response->SetStatusCode(200);
                response->SetHeader("Content-Type", "text/plain; charset=utf-8");
                response->End(tree.get<std::string>("Response.Result.Data"));
            }
        };

        Botan::AutoSeeded_RNG rng;
        auto challenge = rng.random_vec(8);

        m_future = cpr::PostCallback([=](cpr::Response r)
        {
            if (r.error || r.status_code != 200)
            {
                cb("Error contacting Rockstar Online Services.", "");
            }
            else
            {
                std::string returnedXml = DecryptROSData(r.text.data(), r.text.size(), sessionKey);
                std::istringstream stream(returnedXml);

                boost::property_tree::ptree tree;
                boost::property_tree::read_xml(stream, tree);

                if (tree.get("Response.Status", 0) == 0)
                {
                    cb(va(
                        "Could not get entitlement block from the Social Club. Error code: %s/%s",
                        tree.get<std::string>("Response.Error.<xmlattr>.Code").c_str(),
                        tree.get<std::string>("Response.Error.<xmlattr>.CodeEx").c_str()
                    ), "");

                    return;
                }
                else
                {
                    cb("", returnedXml);
                }
            }
        }, cpr::Url{ "http://ros.citizenfx.internal/gta5/11/gameservices/entitlements.asmx/GetEntitlementBlock" }, cpr::Body{ EncryptROSData(BuildPOSTString({
            { "ticket", ticket },
            { "locale", "en-US" },
            { "machineHash", machineHash }
            }), sessionKey)
        }, cpr::Header{
            { "User-Agent", GetROSVersionString() },
            { "Host", "prod.ros.rockstargames.com" },
            { "ros-SecurityFlags", "239" },
            { "ros-SessionTicket", sessionTicket },
            { "ros-Challenge", Botan::base64_encode(challenge) },
            { "ros-HeadersHmac", Botan::base64_encode(HeadersHmac(challenge, "POST", "/gta5/11/gameservices/entitlements.asmx/GetEntitlementBlock", sessionKey, sessionTicket )) }
        });
    }

    bool HandleRequest(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response) override
    {
        request->SetDataHandler([=](const std::vector<uint8_t>& data)
        {
            // get the string
            std::string str(data.begin(), data.end());

            // parse the data
            rapidjson::Document document;
            document.Parse(str.c_str());

            if (document.HasParseError())
            {
                response->SetStatusCode(200);
                response->End(va("{ \"error\": \"pe %d\" }", document.GetParseError()));

                return;
            }

            if (request->GetPath() == "/ros/validate")
            {
                HandleValidateRequest(document, response);

                return;
            }

            auto handleResponse = [=](const std::string& error, const std::string& loginData)
            {
                if (!error.empty())
                {
                    // and write HTTP response
                    response->SetStatusCode(403);
                    response->SetHeader("Content-Type", "text/plain; charset=utf-8");
                    response->End(error);

                    return;
                }

                std::istringstream stream(loginData);

                boost::property_tree::ptree tree;
                boost::property_tree::read_xml(stream, tree);

                // generate initial XML to be contained by JSON
                tinyxml2::XMLDocument document;

                auto rootElement = document.NewElement("Response");
                document.InsertFirstChild(rootElement);

                // set root attributes
                rootElement->SetAttribute("ms", 30.0);
                rootElement->SetAttribute("xmlns", "CreateTicketResponse");

                // elements
                auto appendChildElement = [&](tinyxml2::XMLNode* node, const char* key, auto value)
                {
                    auto element = document.NewElement(key);
                    element->SetText(value);

                    node->InsertEndChild(element);

                    return element;
                };

                auto appendElement = [&](const char* key, auto value)
                {
                    return appendChildElement(rootElement, key, value);
                };

                // computer name as placeholder nick (f- yeah!)
                char nickname[16];
                DWORD size = 16;

                GetComputerNameA(nickname, &size);

                // create the document
                appendElement("Status", 1);
                appendElement("Ticket", tree.get<std::string>("Response.Ticket").c_str()); // 'a' repeated
                appendElement("PosixTime", static_cast<unsigned int>(time(nullptr)));
                appendElement("SecsUntilExpiration", 86399);
                appendElement("PlayerAccountId", tree.get<int>("Response.PlayerAccountId"));
                appendElement("PublicIp", "127.0.0.1");
                appendElement("SessionId", tree.get<std::string>("Response.SessionId").c_str());
                appendElement("SessionKey", tree.get<std::string>("Response.SessionKey").c_str()); // '0123456789abcdef'
                appendElement("SessionTicket", tree.get<std::string>("Response.SessionTicket").c_str());
                appendElement("CloudKey", "8G8S9JuEPa3kp74FNQWxnJ5BXJXZN1NFCiaRRNWaAUR=");

                // services
                auto servicesElement = appendElement("Services", "");
                servicesElement->SetAttribute("Count", 0);

                // Rockstar account
                tinyxml2::XMLNode* rockstarElement = appendElement("RockstarAccount", "");
                appendChildElement(rockstarElement, "RockstarId", tree.get<std::string>("Response.RockstarAccount.RockstarId").c_str());
                appendChildElement(rockstarElement, "Age", 18);
                appendChildElement(rockstarElement, "AvatarUrl", "Bully/b20.png");
                appendChildElement(rockstarElement, "CountryCode", "CA");
                appendChildElement(rockstarElement, "Email", tree.get<std::string>("Response.RockstarAccount.Email").c_str());
                appendChildElement(rockstarElement, "LanguageCode", "en");
                appendChildElement(rockstarElement, "Nickname", nickname);

                appendElement("Privileges", "1,2,3,4,5,6,8,9,10,11,14,15,16,17,18,19,21,22");

                // format as string
                tinyxml2::XMLPrinter printer;
                document.Print(&printer);

                // JSON document
                rapidjson::Document json;

                // this is an object
                json.SetObject();

                // append data
                auto appendJson = [&](const char* key, auto value)
                {
                    rapidjson::Value jsonKey(key, json.GetAllocator());

                    rapidjson::Value jsonValue;
                    GetJsonValue(value, json, jsonValue);

                    json.AddMember(jsonKey, jsonValue, json.GetAllocator());
                };

                appendJson("SessionKey", tree.get<std::string>("Response.SessionKey").c_str());
                appendJson("SessionTicket", tree.get<std::string>("Response.SessionTicket").c_str());
                appendJson("Ticket", tree.get<std::string>("Response.Ticket").c_str());
                appendJson("Email", tree.get<std::string>("Response.RockstarAccount.Email").c_str());
                appendJson("SaveEmail", true);
                appendJson("SavePassword", true);
                appendJson("Password", "DetCon1");
                appendJson("Nickname", const_cast<const char*>(nickname));
                appendJson("RockstarId", tree.get<std::string>("Response.RockstarAccount.RockstarId").c_str());
                appendJson("CallbackData", 2);
                appendJson("Local", false);
                appendJson("SignedIn", true);
                appendJson("SignedOnline", true);
                appendJson("AutoSignIn", false);
                appendJson("Expiration", 86399);
                appendJson("AccountId", tree.get<std::string>("Response.PlayerAccountId").c_str());
                appendJson("Age", 18);
                appendJson("AvatarUrl", "Bully/b20.png");
                appendJson("XMLResponse", printer.CStr());
				appendJson("OrigNickname", tree.get<std::string>("Response.RockstarAccount.Nickname").c_str());

                // serialize json
                rapidjson::StringBuffer buffer;

                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                json.Accept(writer);

                // and write HTTP response
                response->SetStatusCode(200);
                response->SetHeader("Content-Type", "application/json; charset=utf-8");
                response->End(std::string(buffer.GetString(), buffer.GetSize()));
            };

            bool local = (document.HasMember("local") && document["local"].GetBool());

            if (!local)
            {
                std::string username = document["username"].GetString();
                std::string password = document["password"].GetString();

                ProcessLogin(username, password, [=](const std::string& error, const std::string& loginData)
                {
                    if (error.empty())
                    {
                        SaveAccountData(loginData);
                    }

                    handleResponse(error, loginData);
                });
            }
            else
            {
                std::string str;
                bool hasData = LoadAccountData(str);

                if (!hasData)
                {
                    handleResponse("No login data.", "");
                }
                else
                {
                    handleResponse("", str);
                }
            }
        });

        return true;
    }
};

std::string LoginHandler2::EncryptROSData(const std::string& input, const std::string& sessionKey /* = "" */)
{
    // initialize state
    ROSCryptoState state;
    std::stringstream output;

    // decode session key, if needed
    bool hasSecurity = (!sessionKey.empty());

    uint8_t sessKey[16];

    if (hasSecurity)
    {
        auto keyData = Botan::base64_decode(sessionKey);
        memcpy(sessKey, keyData.data(), sizeof(sessKey));
    }

    // get a random RC4 key
    uint8_t rc4Key[16];

    Botan::AutoSeeded_RNG rng;
    rng.randomize(rc4Key, sizeof(rc4Key));

    // XOR the key with the global XOR key and write it to the output
    for (int i = 0; i < sizeof(rc4Key); i++)
    {
        char thisChar = rc4Key[i] ^ state.GetXorKey()[i];

        output << std::string(&thisChar, 1);

        if (hasSecurity)
        {
            rc4Key[i] ^= sessKey[i];
        }
    }

    // create a RC4 cipher for the data
    Botan::StreamCipher* rc4 = Botan::get_stream_cipher("RC4")->clone();
    rc4->set_key(rc4Key, sizeof(rc4Key));

    // encrypt the passed user data using the key
    std::vector<uint8_t> inData(input.size());
    memcpy(&inData[0], input.c_str(), inData.size());

    rc4->encipher(inData);

    // write the inData to the output stream
    output << std::string(reinterpret_cast<const char*>(&inData[0]), inData.size());

    // get a hash for the stream's content so far
    std::string tempContent = output.str();

    Botan::Buffered_Computation* sha1;

    if (!hasSecurity)
    {
        sha1 = Botan::get_hash("SHA1")->clone();
    }
    else
    {
        auto hmac = Botan::get_mac("HMAC(SHA1)")->clone();
        hmac->set_key(rc4Key, sizeof(rc4Key));

        sha1 = hmac;
    }

    sha1->update(reinterpret_cast<const uint8_t*>(tempContent.c_str()), tempContent.size());
    sha1->update(state.GetHashKey(), 16);

    auto hashData = sha1->final();

    // free the algorithms
    delete rc4;
    delete sha1;

    // and return the appended output
    return tempContent + std::string(reinterpret_cast<const char*>(&hashData[0]), hashData.size());
}

std::string LoginHandler2::DecryptROSData(const char* data, size_t size, const std::string& sessionKey)
{
    // initialize state
    ROSCryptoState state;

    // read the packet RC4 key from the packet
    uint8_t rc4Key[16];

    bool hasSecurity = (!sessionKey.empty());

    uint8_t sessKey[16];

    if (hasSecurity)
    {
        auto keyData = Botan::base64_decode(sessionKey);
        memcpy(sessKey, keyData.data(), sizeof(sessKey));
    }

    for (int i = 0; i < sizeof(rc4Key); i++)
    {
        rc4Key[i] = data[i] ^ state.GetXorKey()[i];

        if (hasSecurity)
        {
            rc4Key[i] ^= sessKey[i];
        }
    }

    // initialize RC4 with the packet key
    Botan::StreamCipher* rc4 = Botan::get_stream_cipher("RC4")->clone();
    rc4->set_key(rc4Key, sizeof(rc4Key));

    // read the block size from the data
    uint8_t blockSizeData[4];
    uint8_t blockSizeDataLE[4];
    rc4->cipher(reinterpret_cast<const uint8_t*>(&data[16]), blockSizeData, 4);

    // swap endianness
    blockSizeDataLE[3] = blockSizeData[0];
    blockSizeDataLE[2] = blockSizeData[1];
    blockSizeDataLE[1] = blockSizeData[2];
    blockSizeDataLE[0] = blockSizeData[3];

    uint32_t blockSize = (*(uint32_t*)&blockSizeDataLE) + 20;

    // create a buffer for the block
    std::vector<uint8_t> blockData(blockSize);

    // a result stringstream as well
    std::stringstream result;

    // loop through packet blocks
    size_t start = 20;

    while (start < size)
    {
        // calculate the end of this block
        int end = min(size, start + blockSize);

        // remove the size of the SHA1 hash from the end
        end -= 20;

        int thisLen = end - start;

        // decrypt the block
        rc4->cipher(reinterpret_cast<const uint8_t*>(&data[start]), &blockData[0], thisLen);

        // TODO: compare the resulting hash

        // append to the result buffer
        result << std::string(reinterpret_cast<const char*>(&blockData[0]), thisLen);

        // increment the counter
        start += blockSize;
    }

    delete rc4;

    return result.str();
}

ROSCryptoState::ROSCryptoState()
{
    // initialize the key inputs
    size_t outLength;
    uint8_t* platformStr = base64_decode(ROS_PLATFORM_KEY, strlen(ROS_PLATFORM_KEY), &outLength);

    memcpy(m_rc4Key, &platformStr[1], sizeof(m_rc4Key));
    memcpy(m_xorKey, &platformStr[33], sizeof(m_xorKey));
    memcpy(m_hashKey, &platformStr[49], sizeof(m_hashKey));

    free(platformStr);

    // create the RC4 cipher and decode the keys
    m_rc4 = Botan::get_stream_cipher("RC4")->clone();

    // set the key
    m_rc4->set_key(m_rc4Key, sizeof(m_rc4Key));

    // decode the xor key
    m_rc4->cipher1(m_xorKey, sizeof(m_xorKey));

    // reset the key
    m_rc4->set_key(m_rc4Key, sizeof(m_rc4Key));

    // decode the hash key
    m_rc4->cipher1(m_hashKey, sizeof(m_hashKey));

    // and we're done
    delete m_rc4;
}

class LoginHandler : public net::HttpHandler
{
public:
	bool HandleRequest(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response) override
	{
		// generate initial XML to be contained by JSON
		tinyxml2::XMLDocument document;

		auto rootElement = document.NewElement("Response");
		document.InsertFirstChild(rootElement);

		// set root attributes
		rootElement->SetAttribute("ms", 30.0);
		rootElement->SetAttribute("xmlns", "CreateTicketResponse");

		// elements
		auto appendChildElement = [&] (tinyxml2::XMLNode* node, const char* key, auto value)
		{
			auto element = document.NewElement(key);
			element->SetText(value);

			node->InsertEndChild(element);

			return element;
		};

		auto appendElement = [&] (const char* key, auto value)
		{
			return appendChildElement(rootElement, key, value);
		};

		// computer name as placeholder nick (f- yeah!)
		char nickname[16];
		DWORD size = 16;

		GetComputerNameA(nickname, &size);

		// create the document
		appendElement("Status", 1);
		appendElement("Ticket", "YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh"); // 'a' repeated
		appendElement("PosixTime", static_cast<unsigned int>(time(nullptr)));
		appendElement("SecsUntilExpiration", 86399);
		appendElement("PlayerAccountId", va("%lld", ROS_DUMMY_ACCOUNT_ID));
		appendElement("PublicIp", "127.0.0.1");
		appendElement("SessionId", 5);
		appendElement("SessionKey", "MDEyMzQ1Njc4OWFiY2RlZg=="); // '0123456789abcdef'
		appendElement("SessionTicket", "vhASmPR0NnA7MZsdVCTCV/3XFABWGa9duCEscmAM0kcCDVEa7YR/rQ4kfHs2HIPIttq08TcxIzuwyPWbaEllvQ==");
		appendElement("CloudKey", "8G8S9JuEPa3kp74FNQWxnJ5BXJXZN1NFCiaRRNWaAUR=");
		
		// services
		auto servicesElement = appendElement("Services", "");
		servicesElement->SetAttribute("Count", 0);

		// Rockstar account
		tinyxml2::XMLNode* rockstarElement = appendElement("RockstarAccount", "");
		appendChildElement(rockstarElement, "RockstarId", va("%lld", ROS_DUMMY_ACCOUNT_ID));
		appendChildElement(rockstarElement, "Age", 18);
		appendChildElement(rockstarElement, "AvatarUrl", "Bully/b20.png");
		appendChildElement(rockstarElement, "CountryCode", "CA");
		appendChildElement(rockstarElement, "Email", "onlineservices@citizen.re");
		appendChildElement(rockstarElement, "LanguageCode", "en");
		appendChildElement(rockstarElement, "Nickname", nickname);
		
		appendElement("Privileges", "1,2,3,4,5,6,8,9,10,11,14,15,16,17,18,19,21,22");

		// format as string
		tinyxml2::XMLPrinter printer;
		document.Print(&printer);

		// JSON document
		rapidjson::Document json;

		// this is an object
		json.SetObject();

		// append data
		auto appendJson = [&] (const char* key, auto value)
		{
			rapidjson::Value jsonKey(key, json.GetAllocator());
			
			rapidjson::Value jsonValue;
			GetJsonValue(value, json, jsonValue);

			json.AddMember(jsonKey, jsonValue, json.GetAllocator());
		};

		appendJson("SessionKey", "MDEyMzQ1Njc4OWFiY2RlZg==");
		appendJson("Ticket", "YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh");
		appendJson("Email", "onlineservices@citizen.re");
		appendJson("SaveEmail", true);
		appendJson("SavePassword", true);
		appendJson("Password", "DetCon1");
		appendJson("Nickname", const_cast<const char*>(nickname));
		appendJson("RockstarId", va("%lld", ROS_DUMMY_ACCOUNT_ID));
		appendJson("CallbackData", 2);
		appendJson("Local", false);
		appendJson("SignedIn", true);
		appendJson("SignedOnline", true);
		appendJson("AutoSignIn", false);
		appendJson("Expiration", 86399);
		appendJson("AccountId", va("%lld", ROS_DUMMY_ACCOUNT_ID));
		appendJson("Age", 18);
		appendJson("AvatarUrl", "Bully/b20.png");
		appendJson("XMLResponse", printer.CStr());

		// serialize json
		rapidjson::StringBuffer buffer;

		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		json.Accept(writer);

		// and write HTTP response
		response->SetStatusCode(200);
		response->SetHeader("Content-Type", "application/json; charset=utf-8");
		response->End(std::string(buffer.GetString(), buffer.GetSize()));

		return true;
	}
};

static InitFunction initFunction([] ()
{
	EndpointMapper* endpointMapper = Instance<EndpointMapper>::Get();
	endpointMapper->AddPrefix("/scui/v2/desktop", new SCUIHandler()); // TODO: have a generic static HTTP handler someplace?
	endpointMapper->AddPrefix("/cfx/login", new LoginHandler());
    endpointMapper->AddPrefix("/ros/login", new LoginHandler2());
    endpointMapper->AddPrefix("/ros/validate", new LoginHandler2());

	// somehow launcher likes using two slashes - this should be handled better tbh
	endpointMapper->AddPrefix("//scui/v2/desktop", new SCUIHandler());
});