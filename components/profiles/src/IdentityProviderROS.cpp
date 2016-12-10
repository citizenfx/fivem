/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ProfileManagerImpl.h"
#include "HttpClient.h"
#include "base64.h"

#include <botan/botan.h>
#include <botan/hash.h>
#include <botan/stream_cipher.h>
#include <sstream>

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

class ROSIdentityProvider : public ProfileIdentityProvider
{
private:
	const wchar_t* GetROSVersionString();

	std::string DecryptROSData(const char* data, size_t size);

	std::string EncryptROSData(const std::string& input);

public:
	virtual const char* GetIdentifierKey() override;

	virtual bool RequiresCredentials() override;

	virtual concurrency::task<ProfileIdentityResult> ProcessIdentity(fwRefContainer<Profile> profile, const std::map<std::string, std::string>& parameters) override;
};

const char* ROSIdentityProvider::GetIdentifierKey()
{
	// Rockstar Online Services.
	return "ros";
}

bool ROSIdentityProvider::RequiresCredentials()
{
	return true;
}

const wchar_t* ROSIdentityProvider::GetROSVersionString()
{
	const char* baseString = va("e=%d,t=%s,p=%s,v=%d", 1, "gta5", "pcros", 11);

	// create the XOR'd buffer
	std::vector<uint8_t> xorBuffer(strlen(baseString) + 4);

	// set the key for the XOR buffer
	*(uint32_t*)&xorBuffer[0] = 0xCDCDCDCD;

	for (int i = 4; i < xorBuffer.size(); i++)
	{
		xorBuffer[i] = baseString[i - 4] ^ 0xCD;
	}

	// base64 the string
	size_t base64len;
	char* base64str = base64_encode(&xorBuffer[0], xorBuffer.size(), &base64len);

	// create a wide string version
	std::string str(base64str, base64len);
	std::wstring wideStr(str.begin(), str.end());

	free(base64str);

	// return va() version of the base64 string
	return va(L"ros %s", wideStr.c_str());
}

concurrency::task<ProfileIdentityResult> ROSIdentityProvider::ProcessIdentity(fwRefContainer<Profile> profile, const std::map<std::string, std::string>& parameters)
{
	// task completion source
	concurrency::task_completion_event<ProfileIdentityResult> resultEvent;

	// build a request for the parameters passed

	// get a HTTP client with the right user agent
	std::shared_ptr<HttpClient> httpClient = std::make_shared<HttpClient>(GetROSVersionString());

	// get the id/password
	auto& usernameIt = parameters.find("username");
	auto& passwordIt = parameters.find("password");

	auto& username = usernameIt->second;
	auto& password = passwordIt->second;

	fwMap<fwString, fwString> postMap;
	postMap["ticket"]		= "";
	postMap["email"]		= (username.find('@') != std::string::npos) ? username : "";
	postMap["nickname"]		= (username.find('@') == std::string::npos) ? username : "";
	postMap["password"]		= password;
	postMap["platformName"] = "pcros";

	// encrypt the query string
	fwString queryString = EncryptROSData(httpClient->BuildPostString(postMap));

	httpClient->DoPostRequest(L"ros.citizenfx.internal", 80, L"/gta5/11/gameservices/auth.asmx/CreateTicketSc3", queryString, [=] (bool success, const char* data, size_t size)
	{
		std::shared_ptr<HttpClient> httpClientRef = httpClient;

		if (!success)
		{
			resultEvent.set(ProfileIdentityResult("Error contacting Rockstar Online Services."));
		}
		else
		{
			std::string returnedXml = DecryptROSData(data, size);
			std::istringstream stream(returnedXml);

			boost::property_tree::ptree tree;
			boost::property_tree::read_xml(stream, tree);

			if (tree.get("Response.Status", 0) == 0)
			{
				resultEvent.set(ProfileIdentityResult(va(
					"Could not sign on to the Social Club. Error code: %s/%s",
					tree.get<std::string>("Response.Error.<xmlattr>.Code").c_str(),
					tree.get<std::string>("Response.Error.<xmlattr>.CodeEx").c_str()
				)));
			}
			else
			{
				std::string ticket = tree.get<std::string>("Response.Ticket");
				std::string nickname = tree.get<std::string>("Response.RockstarAccount.Nickname");
				boost::optional<std::string> avatarUrl = tree.get_optional<std::string>("Response.RockstarAccount.AvatarUrl");
				uint64_t rockstarId = tree.get<uint64_t>("Response.RockstarAccount.RockstarId");

				// new ROS security requirements
				std::string sessionKey = tree.get<std::string>("Response.SessionKey");
				std::string sessionTicket = tree.get<std::string>("Response.SessionTicket");

				fwRefContainer<ProfileImpl> profileImpl(profile);

				if (avatarUrl)
				{
					std::string avatarUrlEntry = avatarUrl.get();

					profileImpl->SetTileURI("http://cdn.sc.rockstargames.com/images/avatars/128x128/" + avatarUrlEntry.substr(avatarUrlEntry.find(std::string("avatars/")) + 8));
				}
				else
				{
					// placeholder...
					profileImpl->SetTileURI("data:image/png;base64,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");
				}

				profileImpl->SetDisplayName(nickname);

				// save the parameter list to the profile
				std::map<std::string, std::string> storeParameters(parameters);

				if (storeParameters.find("_savePassword") == storeParameters.end() || storeParameters["_savePassword"] == "false")
				{
					storeParameters.erase("password");
				}

				profileImpl->SetParameters(storeParameters);

				resultEvent.set(ProfileIdentityResult(1, va("%s&&%lld&&gta5&&%s&&%s", ticket.c_str(), rockstarId, sessionKey.c_str(), sessionTicket.c_str())));
			}
		}
	});

	return concurrency::task<ProfileIdentityResult>(resultEvent);
}

std::string ROSIdentityProvider::EncryptROSData(const std::string& input)
{
	// initialize state
	ROSCryptoState state;
	std::stringstream output;

	// get a random RC4 key
	uint8_t rc4Key[16];

	Botan::AutoSeeded_RNG rng;
	rng.randomize(rc4Key, sizeof(rc4Key));

	// XOR the key with the global XOR key and write it to the output
	for (int i = 0; i < sizeof(rc4Key); i++)
	{
		char thisChar = rc4Key[i] ^ state.GetXorKey()[i];

		output << std::string(&thisChar, 1);
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

	Botan::HashFunction* sha1 = Botan::get_hash("SHA1")->clone();
	sha1->update(reinterpret_cast<const uint8_t*>(tempContent.c_str()), tempContent.size());
	sha1->update(state.GetHashKey(), 16);

	auto hashData = sha1->final();

	// free the algorithms
	delete rc4;
	delete sha1;
	
	// and return the appended output
	return tempContent + std::string(reinterpret_cast<const char*>(&hashData[0]), hashData.size());
}

std::string ROSIdentityProvider::DecryptROSData(const char* data, size_t size)
{
	// initialize state
	ROSCryptoState state;

	// read the packet RC4 key from the packet
	uint8_t rc4Key[16];

	for (int i = 0; i < sizeof(rc4Key); i++)
	{
		rc4Key[i] = data[i] ^ state.GetXorKey()[i];
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
	int start = 20;

	while (start < size)
	{
		// calculate the end of this block
		int end = min(size, (size_t)start + blockSize);

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

static InitFunction initFunction([] ()
{
	ProfileManagerImpl* ourProfileManager = static_cast<ProfileManagerImpl*>(Instance<ProfileManager>::Get());

	ourProfileManager->AddIdentityProvider(new ROSIdentityProvider());
});