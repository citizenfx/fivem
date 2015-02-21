/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <terminal/internal/User1.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

namespace terminal
{
enum EAuthenticateResult
{
	AuthenticateResultOK = 0,
	AuthenticateResultBadDetails = 1,
	AuthenticateResultServiceUnavailable = 2,
	AuthenticateResultBanned = 3,
	AuthenticateResultAlreadyLoggedIn = 4,
	AuthenticateResultUnknown = 9999
};

User1::User1(Client* client)
	: m_client(client), m_npID(0), m_authenticated(false)
{

}

void User1::AuthenticateCB(concurrency::task_completion_event<Result<AuthenticateDetail>> event, concurrency::task<std::shared_ptr<AuthenticateResultMessage>> task)
{
	auto message = task.get();

	// if we're authenticated just fine
	if (message->result() == AuthenticateResultOK)
	{
		// mark internal state
		m_npID = message->npid();
		m_authenticated = true;

		// set our task result
		AuthenticateDetail detail(message->npid(), message->sessiontoken());
		
		event.set(Result<AuthenticateDetail>(detail));
	}
	// or if we didn't
	else
	{
		ErrorCode errorCode = ErrorCode::Unknown;

		switch (message->result())
		{
			case AuthenticateResultBadDetails:
				errorCode = ErrorCode::InvalidAuthDetails;
				break;
			case AuthenticateResultBanned:
				errorCode = ErrorCode::AuthDetailsBanned;
				break;
			case AuthenticateResultServiceUnavailable:
				errorCode = ErrorCode::AuthenticationServiceUnavailable;
				break;
		}

		event.set(Result<void>(errorCode));
	}
}

concurrency::task<Result<AuthenticateDetail>> User1::AuthenticateWithLicenseKey(const char* licenseKey)
{
	// get a new message and set the license key
	fwRefContainer<L1MessageBuilder> builder = m_client->GetBuilder();
	auto message = builder->CreateMessage<msg::RPCAuthenticateWithKeyMessage>();

	auto data = message.GetMessage();
	data->set_licensekey(licenseKey);

	// bind a task for the message, send and set the handler
	concurrency::task_completion_event<Result<AuthenticateDetail>> event;
	message.Send<msg::RPCAuthenticateResultMessage>().then(std::bind(&User1::AuthenticateCB, this, event, std::placeholders::_1));

	return concurrency::task<Result<AuthenticateDetail>>(event);
}

concurrency::task<Result<AuthenticateDetail>> User1::AuthenticateWithTokenBag(const TokenBag& tokenBag)
{
	// get a new message and set the license key
	fwRefContainer<L1MessageBuilder> builder = m_client->GetBuilder();
	auto message = builder->CreateMessage<msg::RPCAuthenticateWithTokenMessage>();

	std::string tokenString = tokenBag.ToString();

	auto data = message.GetMessage();
	data->set_token(tokenString);

	// bind a task for the message, send and set the handler
	concurrency::task_completion_event<Result<AuthenticateDetail>> event;
	message.Send<msg::RPCAuthenticateResultMessage>().then(std::bind(&User1::AuthenticateCB, this, event, std::placeholders::_1));

	return concurrency::task<Result<AuthenticateDetail>>(event);
}

uint64_t User1::GetNPID()
{
	return m_npID;
}

std::vector<uint8_t> User1::GetUserTicket(uint64_t remoteServerId)
{
	// allocate the array to return and cast it to a pointer
	std::vector<uint8_t> ticketArray(sizeof(NPAuthenticateTicket));
	NPAuthenticateTicket* ticket = reinterpret_cast<NPAuthenticateTicket*>(&ticketArray[0]);

	ticket->version = 1;
	ticket->clientID = GetNPID();
	ticket->serverID = remoteServerId;
	ticket->time = 0;

	return ticketArray;
}

// detail structs
AuthenticateDetail::AuthenticateDetail(uint64_t npID, const std::string& sessionTokenString)
	: m_npID(npID)
{
	// if this is a NPv2 token...
	if (sessionTokenString.find("npv2ids:") == 0)
	{
		// ... strip the string
		std::string tokenData = sessionTokenString.substr(8);

		// and parse the document
		rapidjson::Document document;
		document.Parse(tokenData.c_str());

		// if there's no errors
		if (!document.HasParseError())
		{
			// and it's an array
			if (document.IsArray())
			{
				// loop through the array members
				for (auto it = document.Begin(); it != document.End(); it++)
				{
					// if the member itself is an array too, and contains 2 elements
					auto& member = *it;

					if (member.IsArray() && member.Size() == 2)
					{
						// test if both members are a string and add them
						auto& key = member[(size_t)0];
						auto& value = member[1];

						if (key.IsString() && value.IsString())
						{
							m_identifiers[key.GetString()] = value.GetString();
						}
					}
				}
			}
		}
	}
}

std::string TokenBag::ToString() const
{
	// create a new document
	rapidjson::Document outDocument;
	outDocument.SetArray();

	// iterate through our tokens
	for (auto& token : m_tokens)
	{
		// create a new value containing our data
		rapidjson::Value tokenValue;
		tokenValue.SetArray();

		tokenValue.PushBack(rapidjson::Value((int)token.first).Move(), outDocument.GetAllocator());
		tokenValue.PushBack(rapidjson::Value(token.second.c_str(), outDocument.GetAllocator()).Move(), outDocument.GetAllocator());

		// and add it to the document
		outDocument.PushBack(tokenValue, outDocument.GetAllocator());
	}

	// write the document
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);

	outDocument.Accept(writer);

	// and return the string
	return std::string("npv2tkn:") + s.GetString();
}
}