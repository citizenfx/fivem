/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "TcpServer.h"
#include "TcpServerFactory.h"

namespace net
{
class MultiplexTcpChildServerStream : public TcpServerStream
{
private:
	fwRefContainer<TcpServerStream> m_baseStream;

	std::vector<uint8_t> m_initialData;

private:
	void TrySendInitialData();

public:
	MultiplexTcpChildServerStream(fwRefContainer<TcpServerStream> baseStream);

	void SetInitialData(const std::vector<uint8_t>& initialData);

	virtual PeerAddress GetPeerAddress() override;

	virtual void OnFirstSetReadCallback() override;
};

enum class MultiplexPatternMatchResult
{
	NoMatch,
	Match,
	InsufficientData
};

typedef std::function<MultiplexPatternMatchResult(const std::vector<uint8_t>& bytes)> MultiplexPatternMatchFn;

class MultiplexTcpChildServer : public TcpServer
{
private:
	MultiplexPatternMatchFn m_patternMatcher;

	std::vector<fwRefContainer<TcpServerStream>> m_connections;

public:
	inline const MultiplexPatternMatchFn& GetPatternMatcher()
	{
		return m_patternMatcher;
	}

	void SetPatternMatcher(const MultiplexPatternMatchFn& function);

	void AttachToResult(const std::vector<uint8_t>& existingData, fwRefContainer<TcpServerStream> baseStream);
};

class MultiplexTcpServer : public fwRefCountable
{
private:
	fwRefContainer<TcpServerFactory> m_factory;
	fwRefContainer<TcpServer> m_rootServer;

private:
	std::vector<fwRefContainer<MultiplexTcpChildServer>> m_childServers;

public:
	MultiplexTcpServer(const fwRefContainer<TcpServerFactory>& factory);

	void Bind(const PeerAddress& bindAddress);

	fwRefContainer<TcpServer> CreateServer(const MultiplexPatternMatchFn& patternMatchFunction);
};
}