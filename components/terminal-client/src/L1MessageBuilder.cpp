/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <terminal/internal/L1MessageBuilder.h>

namespace terminal
{
static LONG g_messageID;

L1MessageBase::L1MessageBase(const fwRefContainer<Connection>& connection, const fwRefContainer<L1ServiceDispatcher>& dispatcher)
	: m_connection(connection), m_dispatcher(dispatcher)
{
	m_messageID = InterlockedIncrement(&g_messageID);
}

void L1MessageBuilder::SetConnection(fwRefContainer<Connection> connection)
{
	m_connection = connection;
}

void L1MessageBuilder::SetDispatcher(fwRefContainer<L1ServiceDispatcher> dispatcher)
{
	m_dispatcher = dispatcher;
}
}