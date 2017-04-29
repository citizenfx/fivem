/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <terminal/internal/Connection.h>
#include <terminal/internal/Socket.h>

namespace terminal
{
class L1FramedTcpConnection : public Connection
{
private:
	fwRefContainer<Parser> m_parser;

	fwRefContainer<StreamSocket> m_socket;

private:
	void PacketReadCallback(uint32_t expectedLength, concurrency::task<Result<SocketReadResult>> resultTask);

	void LengthReadCallback(concurrency::task<Result<SocketReadResult>> resultTask);

public:
	virtual void SetParser(fwRefContainer<Parser> parser) override;

	virtual void BindSocket(fwRefContainer<StreamSocket> socket) override;

	virtual void SendMessage(const std::vector<uint8_t>& message) override;

	virtual bool HasSocket() override;
};
}