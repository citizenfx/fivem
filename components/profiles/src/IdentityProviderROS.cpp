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
#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#define ROS_PLATFORM_KEY "C/9UmxenWfiN5LxXok/KWT4dX9MA+umtsmsIO3/RvegqJKPWhKne4VgNt+oq5de8Le+JLBsATQXtiKTVMk6CO24="

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
	const char* baseString = va("e=%d,t=%s,p=%s,v=%d", 1, "mp3", "pcros", 11);

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

	httpClient->DoPostRequest(L"prod.ros.rockstargames.com", 80, L"/mp3/11/gameservices/auth.asmx/CreateTicketSc3", queryString, [=] (bool success, const char* data, size_t size)
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
				std::string avatarUrl = tree.get<std::string>("Response.RockstarAccount.AvatarUrl");
				uint64_t rockstarId = tree.get<uint64_t>("Response.RockstarAccount.RockstarId");

				fwRefContainer<ProfileImpl> profileImpl(profile);
				profileImpl->SetTileURI("http://cdn.sc.rockstargames.com/images/avatars/128x128/" + avatarUrl.substr(avatarUrl.find(std::string("avatars/")) + 8));
				profileImpl->SetDisplayName(nickname);

				// save the parameter list to the profile
				std::map<std::string, std::string> storeParameters(parameters);

				if (storeParameters.find("_savePassword") == storeParameters.end() || storeParameters["_savePassword"] == "false")
				{
					storeParameters.erase("password");
				}

				profileImpl->SetParameters(storeParameters);

				resultEvent.set(ProfileIdentityResult(terminal::TokenType::ROS, va("%s&&%lld", ticket.c_str(), rockstarId)));
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
	Botan::StreamCipher* rc4 = Botan::retrieve_stream_cipher("RC4")->clone();
	rc4->set_key(rc4Key, sizeof(rc4Key));

	// encrypt the passed user data using the key
	std::vector<uint8_t> inData(input.size());
	memcpy(&inData[0], input.c_str(), inData.size());

	rc4->encipher(inData);

	// write the inData to the output stream
	output << std::string(reinterpret_cast<const char*>(&inData[0]), inData.size());

	// get a hash for the stream's content so far
	std::string tempContent = output.str();

	Botan::HashFunction* sha1 = Botan::retrieve_hash("SHA1")->clone();
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
	Botan::StreamCipher* rc4 = Botan::retrieve_stream_cipher("RC4")->clone();
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
	m_rc4 = Botan::retrieve_stream_cipher("RC4")->clone();

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