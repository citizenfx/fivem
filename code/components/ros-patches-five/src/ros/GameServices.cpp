/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ros/EndpointMapper.h>

#include <base64.h>

#include <botan/botan.h>
#include <botan/base64.h>
#include <botan/hash.h>
#include <botan/hmac.h>
#include <botan/stream_cipher.h>

#include <sstream>

class GameServicesHandler : public net::HttpHandler
{
private:
	std::string m_gameName;

public:
	GameServicesHandler(const std::string& gameName);

	bool HandleRequest(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response) override;

private:
	void SendResponse(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response, const std::string& outputData);

private:
	std::string m_securityKey;
};

class ROSCryptoState
{
private:
	Botan::StreamCipher* m_rc4;

	uint8_t m_rc4Key[32];
	uint8_t m_xorKey[16];
	uint8_t m_hashKey[16];

public:
	ROSCryptoState(const std::string& platformKey);

	inline const uint8_t* GetXorKey()
	{
		return m_xorKey;
	}

	inline const uint8_t* GetHashKey()
	{
		return m_hashKey;
	}
};

class GameServicesBodyParser
{
private:
	std::shared_ptr<ROSCryptoState> m_cryptoState;

	fwRefContainer<net::HttpRequest> m_request;

	std::string m_parsedBody;

public:
	GameServicesBodyParser(const std::string& platformKey, fwRefContainer<net::HttpRequest> request);

	void ParseBody(const std::vector<uint8_t>& data);

	inline const std::string& GetBody()
	{
		return m_parsedBody;
	}
};

GameServicesBodyParser::GameServicesBodyParser(const std::string& platformKey, fwRefContainer<net::HttpRequest> request)
	: m_request(request)
{
	m_cryptoState = std::make_shared<ROSCryptoState>(platformKey);
}

void GameServicesBodyParser::ParseBody(const std::vector<uint8_t>& data)
{
	// stores the final RC4 key
	uint8_t rc4Key[16];

	// session key we told the client in SCUIStub.cpp
	uint8_t rc4Xor[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	// store security flag stuff
	bool hasSecurity = m_request->GetHeader("ros-SecurityFlags") == "239";

	// platform key
	auto platformKey = m_cryptoState->GetXorKey();

	// output the key
	for (int i = 0; i < sizeof(rc4Key); i++)
	{
		rc4Key[i] = platformKey[i];
		rc4Key[i] ^= data[i];

		// if security flags say we should xor, do that
		if (hasSecurity)
		{
			rc4Key[i] ^= rc4Xor[i];
		}
	}

	// initialize the RC4 decryption cipher
	Botan::StreamCipher* rc4 = Botan::get_stream_cipher("RC4")->clone();
	rc4->set_key(rc4Key, sizeof(rc4Key));

	// the client doesn't encrypt in blocks, so just ignore that and decrypt
	std::vector<uint8_t> outData(data.size() - 16 - 20);
	rc4->cipher(&data[16], &outData[0], outData.size());

	// done, clear the cipher
	delete rc4;

	// and set the result string
	m_parsedBody = std::string(outData.begin(), outData.end());
}

GameServicesHandler::GameServicesHandler(const std::string& gameName)
	: m_gameName(gameName)
{
	if (gameName == "gta5")
	{
		m_securityKey = "C4pWJwWIKGUxcHd69eGl2AOwH2zrmzZAoQeHfQFcMelybd32QFw9s10px6k0o75XZeB5YsI9Q9TdeuRgdbvKsxc=";
	}
	else if (gameName == "rdr2")
	{
		m_securityKey = "CxdAElo3H1WNntCCLZ0WEW6WaH1cFFyvF6JCK5Oo1+UqczD626BPGczMnOuv532+AqT/7n3lIQEYxO3hhuXJItk=";
	}
	else if (gameName == "gta4")
	{
		m_securityKey = "C4S+pkoYcYNOzAYOjhemqrc3Sk3az03HT2sKl51L3cJhtRmJDFEtjIZ5PG1HpS/3YaVzh4+bI4c99uuXMtgLwcE=";
	}
	else if (gameName == "launcher")
	{

	}
	//assert(gameName == "gta5" || gameName == "launcher"); // as the platform key is hardcoded
}

bool GameServicesHandler::HandleRequest(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response)
{
	// get the service name from the last two path components
	std::string path = std::string{ request->GetPath().c_str() };

	int subserviceNameOffset = path.find_last_of('/');
	int serviceNameOffset = path.find_last_of('/', subserviceNameOffset - 1);

	std::string serviceName = path.substr(serviceNameOffset + 1);

	int queryStringOffset = serviceName.find_first_of('?');
	std::string queryString;

	if (queryStringOffset != std::string::npos)
	{
		queryString = serviceName.substr(queryStringOffset + 1);
		serviceName = serviceName.substr(0, queryStringOffset);
	}

	// get the endpoint mapper and service handler
	EndpointMapper* endpointMapper = Instance<EndpointMapper>::Get();

	auto handlerContainer = endpointMapper->GetGameServiceHandler(serviceName);

	if (!handlerContainer.is_initialized())
	{
		response->SetStatusCode(404);
		response->End("Not found.");

		return true;
	}

	// is the UA ROS?
	std::string ua = request->GetHeader("User-Agent");
	bool isSecure = (ua.find("ros ") == 0);

	// get the security flags from the request header
	int securityFlags = atoi(request->GetHeader("ros-SecurityFlags", "0").c_str());

	assert(!isSecure || securityFlags == 0 || securityFlags == 239 || securityFlags == 8 /* launcher..? */);

	// if security is requested, verify that the session ticket matches the session key
	if (securityFlags != 0 && isSecure)
	{
		assert(request->GetHeader("ros-SessionTicket") == "vhASmPR0NnA7MZsdVCTCV/3XFABWGa9duCEscmAM0kcCDVEa7YR/rQ4kfHs2HIPIttq08TcxIzuwyPWbaEllvQ==");
	}

	// request handler call
	auto handleDispatch = [=] (const std::string& body)
	{
		// call the dispatch
		std::string outputData = (*handlerContainer)(body);

		if (isSecure && securityFlags != 8)
		{
			// respond to the request as appropriate
			SendResponse(request, response, outputData);
		}
		else
		{
			// raw response
			response->SetStatusCode(200);
			response->End(outputData);
		}
	};

	if (request->GetRequestMethod() == "POST")
	{
		// set up a ROS body parser for the request
		auto bodyParser = std::make_shared<GameServicesBodyParser>(m_securityKey, request);

		request->SetDataHandler([=] (const std::vector<uint8_t>& data)
		{
			if (securityFlags == 8 || serviceName.find(".svc") != std::string::npos)
			{
				handleDispatch(std::string{ data.begin(), data.end() });
			}
			else
			{
				// parse the input body
				bodyParser->ParseBody(data);

				// and dispatch the request
				handleDispatch(bodyParser->GetBody());
			}
		});
	}
	else
	{
		handleDispatch(queryString);
	}

	return true;
}

// work around a current issue with VS15.7 code generation breaking some part of this function by disabling optimization
#pragma optimize("", off)
void GameServicesHandler::SendResponse(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response, const std::string& outputData)
{
	// output stream and other state
	ROSCryptoState cryptoState(m_securityKey);
	std::stringstream outStream;

	// session key, again
	uint8_t rc4Xor[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	// store security flag stuff
	bool hasSecurity = request->GetHeader("ros-SecurityFlags") == "239";

	// get a random RC4 key
	uint8_t rc4Key[16];

	Botan::AutoSeeded_RNG rng;
	rng.randomize(rc4Key, sizeof(rc4Key));

	// create a RC4 cipher for the data
	auto rc4 = Botan::StreamCipher::create("RC4");
	rc4->set_key(rc4Key, sizeof(rc4Key));

	// store the original RC4 key (HMAC needs it later)
	uint8_t origRc4Key[16];
	memcpy(origRc4Key, rc4Key, sizeof(origRc4Key));

	// xor the RC4 key with the platform key (and optionally the session key)
	for (size_t i = 0; i < sizeof(rc4Key); i++)
	{
		rc4Key[i] ^= cryptoState.GetXorKey()[i];

		if (hasSecurity)
		{
			rc4Key[i] ^= rc4Xor[i];
		}
	}

	// append the key to the stream
	outStream << std::string(rc4Key, rc4Key + 16);

	// stream block size
	size_t blockSize = 1024;

	// append the block size to the stream
	{
		// set up endianness
		union
		{
			uint8_t u8[4];
			uint32_t u32;
		} blockSizeData;

		blockSizeData.u32 = _byteswap_ulong(blockSize);

		// swap endianness
		rc4->cipher1(blockSizeData.u8, 4);

		// append to the stream
		outStream << std::string(blockSizeData.u8, blockSizeData.u8 + 4);
	}

	Botan::secure_vector<uint8_t> challenge;
	Botan::secure_vector<uint8_t> requestHmac;

	if (hasSecurity)
	{
		std::string challengeString = request->GetHeader("ros-Challenge");
		challenge = Botan::base64_decode(challengeString);

		std::string requestHmacString = request->GetHeader("ros-HeadersHmac");
		requestHmac = Botan::base64_decode(requestHmacString);
	}

	// pass output data
	{
		std::vector<uint8_t> outputArray(outputData.begin(), outputData.end());
		size_t done = 0;

		while (done < outputArray.size())
		{
			// encrypt the passed user data using the key
			size_t remaining = outputArray.size() - done;
			size_t thisSize = std::min(remaining, blockSize);

			std::vector<uint8_t> inData(thisSize);
			memcpy(&inData[0], &outputArray[done], inData.size());

			done += thisSize;

			// encipher and append
			rc4->encipher(inData);

			// calculate the HMAC/hash before we append
			uint8_t outHash[20];

			if (hasSecurity)
			{
				auto hash = Botan::MessageAuthenticationCode::create("HMAC(SHA1)");
				hash->set_key(origRc4Key, sizeof(origRc4Key));
				hash->update(inData);
				hash->update(challenge);
				hash->update(cryptoState.GetHashKey(), 16);

				hash->final(outHash);
			}
			else
			{
				auto sha1 = Botan::HashFunction::create("SHA1");
				sha1->update(inData);
				sha1->update(cryptoState.GetHashKey(), 16);

				sha1->final(outHash);
			}

			// write the inData to the output stream
			outStream << std::string(inData.begin(), inData.end());

			// and append the HMAC as well
			outStream << std::string(outHash, outHash + sizeof(outHash));
		}
	}

	// response headers HMAC, as ROS needs this haswell (crap, that's my voice recognition failing)
	if (hasSecurity)
	{
		auto hmac = Botan::MessageAuthenticationCode::create("HMAC(SHA1)");

		// set the key
		uint8_t hmacKey[16];

		// xor the RC4 key with the platform key (and optionally the session key)
		for (size_t i = 0; i < sizeof(hmacKey); i++)
		{
			hmacKey[i] = rc4Xor[i] ^ cryptoState.GetXorKey()[i];
		}

		hmac->set_key(hmacKey, sizeof(hmacKey));

		// empty header
		hmac->update(0);

		// ros-ContentSignature (if any)
		hmac->update(0);

		// ros-ContentHash (if any)
		hmac->update(0);

		// HTTP status code (200)
		hmac->update_be(200);
		hmac->update(0);

		// challenge
		hmac->update(challenge);
		hmac->update(0);

		// request HMAC
		hmac->update(requestHmac);
		hmac->update(0);

		// platform hash key
		hmac->update(cryptoState.GetHashKey(), 16);

		// set the request header
		auto hmacValue = hmac->final();
		
		response->SetHeader("ros-HeadersHmac", Botan::base64_encode(hmacValue));
	}

	response->SetStatusCode(200);
	response->End(outStream.str());
}
#pragma optimize("", on)

ROSCryptoState::ROSCryptoState(const std::string& platformKey)
{
	// initialize the key inputs
	size_t outLength = 0;
	uint8_t* platformStr = base64_decode(platformKey.c_str(), platformKey.length(), &outLength);

	if (outLength > 0)
	{
		memcpy(m_rc4Key, &platformStr[1], sizeof(m_rc4Key));
		memcpy(m_xorKey, &platformStr[33], sizeof(m_xorKey));
		memcpy(m_hashKey, &platformStr[49], sizeof(m_hashKey));
	}

	free(platformStr);

	if (outLength == 0)
	{
		return;
	}

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
	EndpointMapper* endpointMapper = Instance<EndpointMapper>::Get();

	endpointMapper->AddPrefix("/gta5/11/gameservices/", new GameServicesHandler("gta5"));
	endpointMapper->AddPrefix("/gta4/11/gameservices/", new GameServicesHandler("gta4"));
	endpointMapper->AddPrefix("/rdr2/11/gameservices/", new GameServicesHandler("rdr2"));
	endpointMapper->AddPrefix("/rdr2/11/wcfgameservices/", new GameServicesHandler("rdr2"));
	endpointMapper->AddPrefix("/launcher/11/launcherservices/", new GameServicesHandler("launcher"));
});
