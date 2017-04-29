/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

class TerminalClient
{
private:
	fwRefContainer<terminal::IClient> m_client;

public:
	inline fwRefContainer<terminal::IClient> GetClient() const
	{
		return m_client;
	}

	inline void SetClient(fwRefContainer<terminal::IClient> client)
	{
		m_client = client;
	}
};

DECLARE_INSTANCE_TYPE(TerminalClient);