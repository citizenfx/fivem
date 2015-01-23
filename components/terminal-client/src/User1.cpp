/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <terminal/internal/User1.h>

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
		AuthenticateDetail detail(message->npid());
		
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

uint64_t User1::GetNPID()
{
	return m_npID;
}
}