/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "UvTcpServer.h"
#include "TcpServerManager.h"
#include "memdbgon.h"

namespace net
{
UvTcpServer::UvTcpServer(TcpServerManager* manager)
	: m_manager(manager)
{

}

UvTcpServer::~UvTcpServer()
{
	m_clients.clear();

	auto server = m_server;

	if (server)
	{
		server->close();
		m_server = {};
	}
}

bool UvTcpServer::Listen(std::shared_ptr<uvw::TCPHandle>&& server)
{
	m_server = std::move(server);

	m_server->on<uvw::ListenEvent>([this](const uvw::ListenEvent& event, uvw::TCPHandle& handle)
	{
		OnConnection(0);
	});

	m_server->on<uvw::ErrorEvent>([this](const uvw::ErrorEvent& event, uvw::TCPHandle& handle)
	{
		trace("Listening on socket failed - libuv error %s.\n", event.name());

		OnConnection(event.code());
	});

	m_server->listen();

	return true;
}

void UvTcpServer::OnConnection(int status)
{
	// check for error conditions
	if (status < 0)
	{
		trace("error on connection: %s\n", uv_strerror(status));
		return;
	}

	// initialize a handle for the client
	auto clientHandle = m_manager->GetWrapLoop()->resource<uvw::TCPHandle>();

	// create a stream instance and associate
	fwRefContainer<UvTcpServerStream> stream(new UvTcpServerStream(this));

	// attempt accepting the connection
	if (stream->Accept(std::move(clientHandle)))
	{
		m_clients.insert(stream);
		
		// invoke the connection callback
		if (GetConnectionCallback())
		{
			GetConnectionCallback()(stream);
		}
	}
	else
	{
		stream = nullptr;
	}
}

void UvTcpServer::RemoveStream(UvTcpServerStream* stream)
{
	m_clients.erase(stream);
}

UvTcpServerStream::UvTcpServerStream(UvTcpServer* server)
	: m_server(server), m_closingClient(false)
{

}

UvTcpServerStream::~UvTcpServerStream()
{
	CloseClient();
}

void UvTcpServerStream::CloseClient()
{
	auto client = m_client;

	if (client && !m_closingClient)
	{
		m_closingClient = true;

		decltype(m_writeCallback) writeCallback;

		{
			std::unique_lock<std::shared_mutex> lock(m_writeCallbackMutex);
			writeCallback = std::move(m_writeCallback);
		}

		// before closing (but after eating the write callback!), drain the write list
		HandlePendingWrites();

		if (writeCallback)
		{
			writeCallback->clear();
			writeCallback->close();
		}

		client->clear();

		client->stop();

		client->close();

		m_client = {};
	}
}

bool UvTcpServerStream::Accept(std::shared_ptr<uvw::TCPHandle>&& client)
{
	m_client = std::move(client);

	m_client->noDelay(true);

	// initialize a write callback handle
	{
		std::unique_lock<std::shared_mutex> lock(m_writeCallbackMutex);

		fwRefContainer<UvTcpServerStream> thisRef(this);

		m_writeCallback = m_client->loop().resource<uvw::AsyncHandle>();
		m_writeCallback->on<uvw::AsyncEvent>([thisRef](const uvw::AsyncEvent& event, uvw::AsyncHandle& handle)
		{
			thisRef->HandlePendingWrites();
		});
	}

	m_client->on<uvw::DataEvent>([this](const uvw::DataEvent& event, uvw::TCPHandle& handle)
	{
		HandleRead(event.length, event.data);
	});

	m_client->on<uvw::EndEvent>([this](const uvw::EndEvent& event, uvw::TCPHandle& handle)
	{
		HandleRead(0, nullptr);
	});

	m_client->on<uvw::ErrorEvent>([this](const uvw::ErrorEvent& event, uvw::TCPHandle& handle)
	{
		HandleRead(-1, nullptr);
	});

	// accept and read
	m_server->GetServer()->accept(*m_client);
	m_client->read();

	return true;
}

void UvTcpServerStream::HandleRead(ssize_t nread, const std::unique_ptr<char[]>& buf)
{
	if (nread > 0)
	{
		std::vector<uint8_t> targetBuf(nread);
		memcpy(&targetBuf[0], buf.get(), targetBuf.size());

		if (GetReadCallback())
		{
			GetReadCallback()(targetBuf);
		}
	}
	else if (nread <= 0)
	{
		// hold a reference to ourselves while in this scope
		fwRefContainer<UvTcpServerStream> tempContainer = this;

		Close();
	}
}

PeerAddress UvTcpServerStream::GetPeerAddress()
{
	auto client = m_client;

	if (!client)
	{
		return PeerAddress{};
	}

	sockaddr_storage addr;
	int len = sizeof(addr);

	uv_tcp_getpeername(client->raw(), reinterpret_cast<sockaddr*>(&addr), &len);

	return PeerAddress(reinterpret_cast<sockaddr*>(&addr), static_cast<socklen_t>(len));
}

void UvTcpServerStream::Write(const std::string& data)
{
	auto dataRef = std::unique_ptr<char[]>(new char[data.size()]);
	memcpy(dataRef.get(), data.data(), data.size());

	WriteInternal(std::move(dataRef), data.size());
}

void UvTcpServerStream::Write(const std::vector<uint8_t>& data)
{
	auto dataRef = std::unique_ptr<char[]>(new char[data.size()]);
	memcpy(dataRef.get(), data.data(), data.size());

	WriteInternal(std::move(dataRef), data.size());
}

void UvTcpServerStream::Write(std::string&& data)
{
	auto dataRef = std::unique_ptr<char[]>(new char[data.size()]);
	memcpy(dataRef.get(), data.data(), data.size());

	WriteInternal(std::move(dataRef), data.size());
}

void UvTcpServerStream::Write(std::vector<uint8_t>&& data)
{
	auto dataRef = std::unique_ptr<char[]>(new char[data.size()]);
	memcpy(dataRef.get(), data.data(), data.size());

	WriteInternal(std::move(dataRef), data.size());
}

// blindly copypasted from StackOverflow (to allow std::function to store the funcref types with their move semantics)
// TODO: we use this *three times* now, time for a shared header?
template<class F>
struct shared_function
{
	std::shared_ptr<F> f;
	shared_function() = default;
	shared_function(F&& f_) : f(std::make_shared<F>(std::move(f_))) {}
	shared_function(shared_function const&) = default;
	shared_function(shared_function&&) = default;
	shared_function& operator=(shared_function const&) = default;
	shared_function& operator=(shared_function&&) = default;

	template<class...As>
	auto operator()(As&&...as) const
	{
		return (*f)(std::forward<As>(as)...);
	}
};

template<class F>
shared_function<std::decay_t<F>> make_shared_function(F&& f)
{
	return { std::forward<F>(f) };
}

void UvTcpServerStream::WriteInternal(std::unique_ptr<char[]> data, size_t size)
{
	if (!m_client)
	{
		return;
	}

	std::shared_lock<std::shared_mutex> lock(m_writeCallbackMutex);

	auto writeCallback = m_writeCallback;

	if (writeCallback)
	{
		// submit the write request
		m_pendingRequests.push(make_shared_function([this, data = std::move(data), size]() mutable
		{
			auto client = m_client;

			if (client)
			{
				client->write(std::move(data), size);
			}
		}));

		// wake the callback
		writeCallback->send();
	}
}

void UvTcpServerStream::ScheduleCallback(const TScheduledCallback& callback)
{
	std::shared_lock<std::shared_mutex> lock(m_writeCallbackMutex);

	auto writeCallback = m_writeCallback;

	if (writeCallback)
	{
		m_pendingRequests.push(callback);

		writeCallback->send();
	}
}

void UvTcpServerStream::HandlePendingWrites()
{
	if (!m_client)
	{
		return;
	}

	// as a possible result is closing self, keep reference
	fwRefContainer<UvTcpServerStream> selfRef = this;

	// dequeue pending writes
	std::function<void()> request;

	while (m_pendingRequests.try_pop(request))
	{
		if (m_client)
		{
			request();
		}
	}
}

void UvTcpServerStream::Close()
{
	// NOTE: potential data race if this gets zeroed/closed right when we try to do something
	if (!m_client)
	{
		return;
	}

	std::shared_ptr<uvw::AsyncHandle> wc;

	{
		std::shared_lock<std::shared_mutex> lock(m_writeCallbackMutex);

		wc = m_writeCallback;
	}

	if (!wc)
	{
		return;
	}

	m_pendingRequests.push([=]()
	{
		// keep a reference in scope
		fwRefContainer<UvTcpServerStream> selfRef = this;

		CloseClient();

		SetReadCallback(TReadCallback());

		// get it locally as we may recurse
		auto closeCallback = GetCloseCallback();

		if (closeCallback)
		{
			SetCloseCallback(TCloseCallback());

			closeCallback();
		}

		m_server->RemoveStream(this);
	});

	// wake the callback
	wc->send();
}
}
