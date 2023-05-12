#pragma once

#include <UvTcpServer.h>

#include <deque>
#include <uvw.hpp>

#ifdef COMPILING_NET_TCP_SERVER
#define TCP_SERVER_EXPORT DLL_EXPORT
#else
#define TCP_SERVER_EXPORT DLL_IMPORT
#endif

namespace net
{
class ReverseTcpServer;

class ReverseTcpServerStream : public TcpServerStream
{
	friend class ReverseTcpServer;

public:
	ReverseTcpServerStream(ReverseTcpServer* server, const std::shared_ptr<uvw::TCPHandle>& tcp);

	virtual ~ReverseTcpServerStream() override;

	inline virtual PeerAddress GetPeerAddress() override
	{
		return m_remotePeerAddress;
	}

	virtual void Close() override;

	virtual void Write(const std::vector<uint8_t>& data, TCompleteCallback&& onComplete) override;

	virtual void ScheduleCallback(TScheduledCallback&& callback, bool performInline) override;

protected:
	void ConsumeData(const void* data, size_t length);

private:
	ReverseTcpServer* m_server;
	std::weak_ptr<uvw::TCPHandle> m_tcp;

	net::PeerAddress m_remotePeerAddress;

	std::shared_ptr<uvw::AsyncHandle> m_writeCallback;

	tbb::concurrent_queue<TScheduledCallback> m_pendingRequests;
};

class TcpServerManager;

class TCP_SERVER_EXPORT ReverseTcpServer : public TcpServer
{
public:
	ReverseTcpServer();

	virtual ~ReverseTcpServer();

	void Listen(const std::string& remote, const std::string& regToken);

private:
	void CreateWorkerConnection();

	void RemoveWorker(const std::shared_ptr<uvw::TCPHandle>& worker);

	void Reconnect();

	void ReconnectWithPeer(const net::PeerAddress& peer);

private:
	std::shared_ptr<uvw::TimerHandle> m_reconnectTimer;

	std::shared_ptr<uvw::TCPHandle> m_control;

	std::shared_ptr<uvw::GetAddrInfoReq> m_addr;

	std::shared_ptr<uvw::Loop> m_loop;

	std::set<std::shared_ptr<uvw::TCPHandle>> m_work;

	std::map<std::shared_ptr<uvw::TCPHandle>, fwRefContainer<ReverseTcpServerStream>> m_streams;

	bool m_loggedIn;

	std::string m_remote;

	net::PeerAddress m_curRemote;
	std::string m_token;
	std::string m_regToken;
};
}
