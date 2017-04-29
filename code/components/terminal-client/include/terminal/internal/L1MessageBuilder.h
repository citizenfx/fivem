/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <terminal/internal/Connection.h>
#include <terminal/internal/L1ServiceDispatcher.h>

#ifdef GetMessage
#undef GetMessage
#endif

namespace terminal
{
class L1MessageBase
{
protected:
	fwRefContainer<Connection> m_connection;

	fwRefContainer<L1ServiceDispatcher> m_dispatcher;

	int m_messageID;

protected:
	concurrency::task<ServiceMessageData> Send(uint32_t messageType, const std::vector<uint8_t>& data);

	void SendOneShot(uint32_t messageType, const std::vector<uint8_t>& data);

public:
	L1MessageBase(const fwRefContainer<Connection>& connection, const fwRefContainer<L1ServiceDispatcher>& dispatcher);
};

template<typename TMessage>
class L1Message : public L1MessageBase
{
private:
	typename TMessage::ProtoPtr m_message;

public:
	L1Message(const fwRefContainer<Connection>& connection, const fwRefContainer<L1ServiceDispatcher>& dispatcher)
		: L1MessageBase(connection, dispatcher)
	{
		m_message = TMessage::Create();
	}

	inline typename TMessage::ProtoPtr GetMessage()
	{
		return m_message;
	}

	template<typename TResponse>
	concurrency::task<typename TResponse::ProtoPtr> Send()
	{
		// register the completion event with the dispatcher
		concurrency::task_completion_event<typename TResponse::ProtoPtr> completionEvent;
		m_dispatcher->SetCompletionEvent<TResponse>(m_messageID, completionEvent);

		// send the message
		SendOneShot();

		// return the event
		return concurrency::task<typename TResponse::ProtoPtr>(completionEvent);
	}

	void SendOneShot()
	{
		// allocate an array big enough for the message
		int messageSize = m_message->ByteSize();
		std::vector<uint8_t> bytes((sizeof(uint32_t) * 2) + messageSize);

		// serialize the message to the footer
		m_message->SerializeToArray(&bytes[sizeof(uint32_t) * 2], messageSize);

		// set the header bytes
		*(uint32_t*)&bytes[0] = TMessage::MessageType;
		*(uint32_t*)&bytes[sizeof(uint32_t)] = m_messageID;

		// send it across the wire
		m_connection->SendMessage(bytes);
	}
};

class L1MessageBuilder : public fwRefCountable
{
private:
	fwRefContainer<Connection> m_connection;

	fwRefContainer<L1ServiceDispatcher> m_dispatcher;

public:
	void SetConnection(fwRefContainer<Connection> connection);

	void SetDispatcher(fwRefContainer<L1ServiceDispatcher> dispatcher);

	template<typename TMessage>
	L1Message<TMessage> CreateMessage()
	{
		return L1Message<TMessage>(m_connection, m_dispatcher);
	}
};
}