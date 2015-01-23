/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <terminal/internal/Client.h>

namespace terminal
{
class User1 : public IUser1, public fwRefCountable
{
private:
	Client* m_client;

	bool m_authenticated;

	uint64_t m_npID;

private:
	void AuthenticateCB(concurrency::task_completion_event<Result<AuthenticateDetail>> event, concurrency::task<std::shared_ptr<AuthenticateResultMessage>> message);

public:
	User1(Client* client);

	virtual concurrency::task<Result<AuthenticateDetail>> AuthenticateWithLicenseKey(const char* licenseKey) override;

	virtual uint64_t GetNPID() override;
};
}