/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <terminal/internal/L1FramedTcpConnection.h>

namespace terminal
{
void L1FramedTcpConnection::SendMessage(const std::vector<uint8_t>& message)
{
	// build the frame header
	struct
	{
		uint32_t signature;
		uint32_t length;
	} lengthHeader;

	lengthHeader.signature = 0xDEADC0DE;
	lengthHeader.length = message.size() - 8;

	// build the full message by copying
	std::vector<uint8_t> outMessage(sizeof(lengthHeader) + message.size());
	memcpy(&outMessage[0], &lengthHeader, sizeof(lengthHeader));
	memcpy(&outMessage[sizeof(lengthHeader)], &message[0], message.size());

	// and send it across the socket
	m_socket->Write(outMessage);
}

void L1FramedTcpConnection::BindSocket(fwRefContainer<StreamSocket> socket)
{
	m_socket = socket;

	m_socket->Read(8).then(std::bind(&L1FramedTcpConnection::LengthReadCallback, this, std::placeholders::_1));
}

void L1FramedTcpConnection::PacketReadCallback(uint32_t expectedLength, concurrency::task<Result<SocketReadResult>> resultTask)
{
	auto result = resultTask.get();

	if (!result.HasSucceeded())
	{
		// TODO: propagate error
	}
	else
	{
		auto byteArray = result.GetDetail().GetBuffer();

		// if the size is as expected
		if (byteArray.size() == expectedLength)
		{
			// pass it on to the parset (ha!)
			m_parser->HandleIncomingMessage(byteArray);
		}

		// continue reading the stream
		m_socket->Read(8).then(std::bind(&L1FramedTcpConnection::LengthReadCallback, this, std::placeholders::_1));
	}
}

void L1FramedTcpConnection::LengthReadCallback(concurrency::task<Result<SocketReadResult>> resultTask)
{
	auto result = resultTask.get();

	if (!result.HasSucceeded())
	{
		// TODO: propagate error

	}
	else
	{
		auto byteArray = result.GetDetail().GetBuffer();

		// if the size is sufficient for a length header
		if (byteArray.size() == 8)
		{
			struct
			{
				uint32_t signature;
				uint32_t length;
			} lengthHeader;

			memcpy(&lengthHeader, &byteArray[0], sizeof(lengthHeader));

			// if the signature is valid
			if (lengthHeader.signature == 0xDEADC0DE)
			{
				// read the expected data
				size_t readLength = lengthHeader.length + 8;

				m_socket->Read(readLength).then(std::bind(&L1FramedTcpConnection::PacketReadCallback, this, readLength, std::placeholders::_1));
			}
		}
	}
}

void L1FramedTcpConnection::SetParser(fwRefContainer<Parser> parser)
{
	m_parser = parser;
}

bool L1FramedTcpConnection::HasSocket()
{
	return m_socket.GetRef();
}
}