/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "TcpServer.h"
#include "TcpServerFactory.h"

#include <mutex>
#include <shared_mutex>

#ifdef COMPILING_NET_TCP_SERVER
#define TCP_SERVER_EXPORT DLL_EXPORT
#else
#define TCP_SERVER_EXPORT DLL_IMPORT
#endif

namespace net
{
class MultiplexTcpChildServer;

class MultiplexTcpChildServerStream : public TcpServerStream
{
private:
	std::shared_mutex m_baseStreamMutex;

	fwRefContainer<TcpServerStream> m_baseStream;

	std::vector<uint8_t> m_initialData;

	MultiplexTcpChildServer* m_server;

private:
	void TrySendInitialData();

	void CloseInternal();

	inline fwRefContainer<TcpServerStream> GetBaseStream()
	{
		std::shared_lock<std::shared_mutex> _(m_baseStreamMutex);

		return m_baseStream;
	}

protected:
	virtual void OnFirstSetReadCallback() override;

public:
	MultiplexTcpChildServerStream(MultiplexTcpChildServer* server, fwRefContainer<TcpServerStream> baseStream);

	void SetInitialData(const std::vector<uint8_t>& initialData);

	virtual PeerAddress GetPeerAddress() override;

	virtual void Write(const std::vector<uint8_t>& data, TCompleteCallback&& onComplete) override;

	virtual void Write(const std::string& data, TCompleteCallback&& onComplete) override;

	virtual void Write(std::vector<uint8_t>&& data, TCompleteCallback&& onComplete) override;

	virtual void Write(std::string&& data, TCompleteCallback&& onComplete) override;

	virtual void Write(std::unique_ptr<char[]> data, size_t len, TCompleteCallback&& onComplete) override;

	virtual void Close() override;

	virtual void ScheduleCallback(TScheduledCallback&& callback, bool performInline) override;

	void StartConnectionTimeout(std::chrono::duration<uint64_t, std::milli> timeout) override;
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

	std::mutex m_connectionsMutex;

	std::set<fwRefContainer<TcpServerStream>> m_connections;

public:
	inline const MultiplexPatternMatchFn& GetPatternMatcher()
	{
		return m_patternMatcher;
	}

	void SetPatternMatcher(const MultiplexPatternMatchFn& function);

	void AttachToResult(const std::vector<uint8_t>& existingData, fwRefContainer<TcpServerStream> baseStream);

	void CloseStream(MultiplexTcpChildServerStream* stream);
};

class TCP_SERVER_EXPORT MultiplexTcpServer : public fwRefCountable
{
private:
	std::vector<fwRefContainer<MultiplexTcpChildServer>> m_childServers;

public:
	MultiplexTcpServer();

	fwRefContainer<TcpServer> CreateServer(const MultiplexPatternMatchFn& patternMatchFunction);

	void AttachToServer(fwRefContainer<TcpServer> server);
};

class TCP_SERVER_EXPORT MultiplexTcpBindServer : public MultiplexTcpServer
{
private:
	fwRefContainer<TcpServerFactory> m_factory;
	fwRefContainer<TcpServer> m_rootServer;

public:
	MultiplexTcpBindServer(const fwRefContainer<TcpServerFactory>& factory);

	void Bind(const PeerAddress& bindAddress);
};
}
