/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "UvTcpServer.h"
#include "TcpServerManager.h"
#include "UvLoopManager.h"
#include "memdbgon.h"

#include <botan/auto_rng.h>

#ifdef _WIN32
#pragma comment(lib, "ntdll")
#include <winternl.h>

EXTERN_C NTSTATUS NTSYSAPI NTAPI NtSetInformationFile(
	_In_  HANDLE                 FileHandle,
	_Out_ PIO_STATUS_BLOCK       IoStatusBlock,
	_In_  PVOID                  FileInformation,
	_In_  ULONG                  Length,
	_In_  FILE_INFORMATION_CLASS FileInformationClass
);

struct FILE_COMPLETION_INFORMATION
{
	DWORD_PTR Port;
	DWORD_PTR Key;
};

#define FileReplaceCompletionInformation (FILE_INFORMATION_CLASS)61

#define STATUS_INVALID_INFO_CLASS 0xC0000003

#endif

namespace net
{
UvTcpServer::UvTcpServer(TcpServerManager* manager)
	: m_manager(manager), m_dispatchIndex(0), m_tryDetachFromIOCP(true)
{
	m_pipeName = fmt::sprintf(
#ifdef _WIN32
		"\\\\.\\pipe\\fxserver_%d%d",
#else
		"/tmp/fxserver_%d%d",
#endif
		time(NULL), rand()
	);
	
	Botan::AutoSeeded_RNG rng;
	rng.randomize(m_helloMessage.data(), m_helloMessage.size());
}

UvTcpServer::~UvTcpServer()
{
	m_dispatchPipes.clear();
	m_createdPipes.clear();

	m_listenPipe = {};

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

	auto pipe = m_server->loop().resource<uvw::PipeHandle>();
	pipe->bind(m_pipeName);
	pipe->listen();

	pipe->on<uvw::ListenEvent>([this](const uvw::ListenEvent& event, uvw::PipeHandle& handle)
	{
		OnListenPipe(handle);
	});

	m_listenPipe = pipe;

	auto threadCount = std::min(std::max(int(std::thread::hardware_concurrency() / 2), 1), 8);

	for (int i = 0; i < threadCount; i++)
	{
		auto cs = std::make_shared<UvTcpChildServer>(this, m_pipeName, m_helloMessage, i);
		cs->Listen();

		m_childServers.insert(cs);
	}

	return true;
}

void UvTcpServer::OnListenPipe(uvw::PipeHandle& handle)
{
	auto pipe = m_server->loop().resource<uvw::PipeHandle>(true);
	std::weak_ptr<uvw::PipeHandle> pipeWeak(pipe);

	pipe->on<uvw::DataEvent>([this, pipeWeak](const uvw::DataEvent& event, uvw::PipeHandle& handle)
	{
		if (event.length == 0)
		{
			m_createdPipes.erase(pipeWeak.lock());
			return;
		}

		// #TODOTCP: handle partial reads
		if (event.length != m_helloMessage.size())
		{
			m_createdPipes.erase(pipeWeak.lock());
			return;
		}

		auto pipe = pipeWeak.lock();

		if (pipe)
		{
			if (memcmp(event.data.get(), m_helloMessage.data(), m_helloMessage.size()) != 0)
			{
				m_createdPipes.erase(pipe);
				return;
			}

			m_dispatchPipes.push_back(pipe);
		}
	});

	pipe->on<uvw::EndEvent>([this, pipeWeak](const uvw::EndEvent& event, uvw::PipeHandle& handle)
	{
		m_createdPipes.erase(pipeWeak.lock());
	});

	handle.accept(*pipe);
	pipe->read();

	m_createdPipes.insert(pipe);
}

void UvTcpServer::OnConnection(int status)
{
	// check for error conditions
	if (status < 0)
	{
		trace("error on connection: %s\n", uv_strerror(status));
		return;
	}

	auto clientHandle = m_server->loop().resource<uvw::TCPHandle>();
	m_server->accept(*clientHandle);

	// if not set up yet, don't accept
	if (m_dispatchPipes.empty())
	{
		clientHandle->close();
		return;
	}

	// get a pipe
	auto index = m_dispatchIndex++ % m_dispatchPipes.size();

	if (index == m_dispatchPipes.size())
	{
		// TODO: log this? handle locally?
		return;
	}

#ifdef _WIN32
	if (m_tryDetachFromIOCP)
	{
		auto fd = clientHandle->fileno();

		FILE_COMPLETION_INFORMATION info = { 0 };
		IO_STATUS_BLOCK block;

		if (NtSetInformationFile(fd, &block, &info, sizeof(info), FileReplaceCompletionInformation) == STATUS_INVALID_INFO_CLASS)
		{
			m_tryDetachFromIOCP = false;
		}
	}
#endif

	static char dummyMessage[] = { 1, 2, 3, 4 };
	m_dispatchPipes[index]->write(*clientHandle, dummyMessage, sizeof(dummyMessage));

	clientHandle->close();
}

UvTcpChildServer::UvTcpChildServer(UvTcpServer* parent, const std::string& pipeName, const std::array<uint8_t, 16>& pipeMessage, int idx)
	: m_parent(parent), m_pipeName(pipeName), m_pipeMessage(pipeMessage)
{
	m_uvLoopName = fmt::sprintf("tcp%d", idx);

	m_uvLoop = Instance<net::UvLoopManager>::Get()->GetOrCreate(m_uvLoopName);
}

void UvTcpChildServer::Listen()
{
	m_uvLoop->EnqueueCallback([this]()
	{
		auto loop = m_uvLoop->Get();

		auto pipe = loop->resource<uvw::PipeHandle>(true);
		pipe->connect(m_pipeName);

		m_dispatchPipe = pipe;

		pipe->on<uvw::DataEvent>([this](const uvw::DataEvent& ev, uvw::PipeHandle& handle)
		{
			OnConnection(0);
		});

		pipe->on<uvw::ConnectEvent>([this](const uvw::ConnectEvent& ev, uvw::PipeHandle& handle)
		{
			handle.read();

			auto msg = std::unique_ptr<char[]>(new char[m_pipeMessage.size()]);
			memcpy(msg.get(), m_pipeMessage.data(), m_pipeMessage.size());

			handle.write(std::move(msg), m_pipeMessage.size());
		});
	});
}

net::TcpServerManager* UvTcpChildServer::GetManager() const
{
	return m_parent->GetManager();
}

void UvTcpChildServer::OnConnection(int status)
{
	// initialize a handle for the client
	auto clientHandle = m_dispatchPipe->loop().resource<uvw::TCPHandle>();

	// create a stream instance and associate
	fwRefContainer<UvTcpServerStream> stream(new UvTcpServerStream(this));

	// attempt accepting the connection
	if (stream->Accept(std::move(clientHandle)))
	{
		m_clients.insert(stream);

		// invoke the connection callback
		if (m_parent->GetConnectionCallback())
		{
			m_parent->GetConnectionCallback()(stream);
		}
	}
	else
	{
		stream = nullptr;
	}
}

void UvTcpChildServer::RemoveStream(UvTcpServerStream* stream)
{
	m_clients.erase(stream);
}

UvTcpServerStream::UvTcpServerStream(UvTcpChildServer* server)
	: m_server(server), m_closingClient(false), m_threadId(std::this_thread::get_id())
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

		decltype(m_writeTimeout) writeTimeout;

		{
			writeTimeout = std::move(m_writeTimeout);
		}

		if (writeTimeout)
		{
			writeTimeout->clear();
			writeTimeout->close();
		}

		client->stop();
		client->shutdown();
		client->close();

		m_client = {};
	}
}

void UvTcpServerStream::ResetWriteTimeout()
{
	m_writeTimeout->start(std::chrono::seconds{ 30 }, std::chrono::milliseconds{ 0 });
}

bool UvTcpServerStream::Accept(std::shared_ptr<uvw::TCPHandle>&& client)
{
	// accept early
	m_server->GetServer()->accept(*client);

	// rate limiter start
	auto manager = m_server->GetManager();

	sockaddr_storage addr;
	int len = sizeof(addr);

	uv_tcp_getpeername(client->raw(), reinterpret_cast<sockaddr*>(&addr), &len);

	PeerAddress peer{ reinterpret_cast<sockaddr*>(&addr), static_cast<socklen_t>(len) };

	if (!manager->OnStartConnection(peer))
	{
		client->closeReset();
		return false;
	}

	// continue connection
	m_writeTimeout = client->loop().resource<uvw::TimerHandle>();

	fwRefContainer<UvTcpServerStream> thisRef(this);

	m_writeTimeout->once<uvw::TimerEvent>([thisRef](const uvw::TimerEvent& event, uvw::TimerHandle& handle)
	{
		thisRef->Close();
	});

	m_client = std::move(client);

	m_client->noDelay(true);

	// initialize a write callback handle
	{
		std::unique_lock<std::shared_mutex> lock(m_writeCallbackMutex);

		m_writeCallback = m_client->loop().resource<uvw::AsyncHandle>();
		m_writeCallback->on<uvw::AsyncEvent>([thisRef](const uvw::AsyncEvent& event, uvw::AsyncHandle& handle)
		{
			thisRef->HandlePendingWrites();
		});
	}

	m_client->on<uvw::WriteEvent>([thisRef](const uvw::WriteEvent& event, uvw::TCPHandle& handle)
	{
		if (thisRef->m_closingClient)
		{
			return;
		}

		if (thisRef->m_pendingWrites.fetch_sub(1) == 0)
		{
			thisRef->m_writeTimeout->stop();
		}
		else
		{
			thisRef->ResetWriteTimeout();
		}
	});

	m_client->on<uvw::DataEvent>([thisRef](const uvw::DataEvent& event, uvw::TCPHandle& handle)
	{
		if (thisRef->m_closingClient)
		{
			return;
		}

		thisRef->HandleRead(event.length, event.data);
	});

	m_client->on<uvw::EndEvent>([thisRef](const uvw::EndEvent& event, uvw::TCPHandle& handle)
	{
		if (thisRef->m_closingClient)
		{
			return;
		}

		thisRef->HandleRead(0, nullptr);
	});

	m_client->on<uvw::ErrorEvent>([thisRef](const uvw::ErrorEvent& event, uvw::TCPHandle& handle)
	{
		if (thisRef->m_closingClient)
		{
			return;
		}

		thisRef->HandleRead(-1, nullptr);
	});

	// rate limiter close event
	m_client->on<uvw::CloseEvent>([manager, peer](const uvw::CloseEvent&, uvw::TCPHandle& handle)
	{
		manager->OnCloseConnection(peer);
	});

	// read
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

void UvTcpServerStream::Write(const std::string& data, TCompleteCallback&& onComplete)
{
	auto dataRef = std::unique_ptr<char[]>(new char[data.size()]);
	memcpy(dataRef.get(), data.data(), data.size());

	WriteInternal(std::move(dataRef), data.size(), std::move(onComplete));
}

void UvTcpServerStream::Write(const std::vector<uint8_t>& data, TCompleteCallback&& onComplete)
{
	auto dataRef = std::unique_ptr<char[]>(new char[data.size()]);
	memcpy(dataRef.get(), data.data(), data.size());

	WriteInternal(std::move(dataRef), data.size(), std::move(onComplete));
}

void UvTcpServerStream::Write(std::string&& data, TCompleteCallback&& onComplete)
{
	auto dataRef = std::unique_ptr<char[]>(new char[data.size()]);
	memcpy(dataRef.get(), data.data(), data.size());

	WriteInternal(std::move(dataRef), data.size(), std::move(onComplete));
}

void UvTcpServerStream::Write(std::vector<uint8_t>&& data, TCompleteCallback&& onComplete)
{
	auto dataRef = std::unique_ptr<char[]>(new char[data.size()]);
	memcpy(dataRef.get(), data.data(), data.size());

	WriteInternal(std::move(dataRef), data.size(), std::move(onComplete));
}

void UvTcpServerStream::Write(std::unique_ptr<char[]> data, size_t size, TCompleteCallback&& onComplete)
{
	WriteInternal(std::move(data), size, std::move(onComplete));
}

void UvTcpServerStream::WriteInternal(std::unique_ptr<char[]> data, size_t size, TCompleteCallback&& onComplete)
{
	if (!m_client)
	{
		return;
	}

	auto doWrite = [this](const std::shared_ptr<uvw::TCPHandle>& client, TCompleteCallback&& onComplete, std::unique_ptr<char[]> data, size_t size)
	{
		if (onComplete)
		{
			auto cb = std::make_shared<std::function<void(bool)>>(make_shared_function([onComplete = std::move(onComplete)](bool success) mutable
			{
				onComplete(success);
			}));

			auto c1 = client->once<uvw::EndEvent>([cb](const uvw::EndEvent& e, uvw::TCPHandle& h) mutable
			{
				auto c = *cb;

				if (c)
				{
					c(false);
					*cb = {};
				}
			});

			auto c2 = client->once<uvw::ErrorEvent>([cb](const uvw::ErrorEvent& e, uvw::TCPHandle& h) mutable
			{
				auto c = *cb;

				if (c)
				{
					c(false);
					*cb = {};
				}
			});

			std::weak_ptr<uvw::TCPHandle> clientWeak(client);

			client->once<uvw::WriteEvent>([clientWeak, cb, c1 = std::move(c1), c2 = std::move(c2)](const uvw::WriteEvent& e, uvw::TCPHandle& h) mutable
			{
				auto client = clientWeak.lock();

				if (client)
				{
					client->erase<uvw::EndEvent>(c1);
					client->erase<uvw::ErrorEvent>(c2);
				}

				auto c = *cb;

				if (c)
				{
					c(true);
					*cb = {};
				}
			});
		}

		m_pendingWrites++;
		ResetWriteTimeout();

		client->write(std::move(data), size);
	};

	if (std::this_thread::get_id() == m_threadId)
	{
		auto client = m_client;

		if (client)
		{
			doWrite(client, std::move(onComplete), std::move(data), size);
		}

		return;
	}

	std::shared_lock<std::shared_mutex> lock(m_writeCallbackMutex);

	auto writeCallback = m_writeCallback;

	if (writeCallback)
	{
		// submit the write request
		std::weak_ptr<uvw::TCPHandle> weakClient(m_client);

		m_pendingRequests.push(make_shared_function([weakClient, doWrite, data = std::move(data), onComplete = std::move(onComplete), size]() mutable
		{
			auto client = weakClient.lock();

			if (client)
			{
				doWrite(client, std::move(onComplete), std::move(data), size);
			}
		}));

		// wake the callback
		writeCallback->send();
	}
}

void UvTcpServerStream::ScheduleCallback(TScheduledCallback&& callback, bool performInline)
{
	if (performInline && std::this_thread::get_id() == m_threadId)
	{
		callback();
		return;
	}

	std::shared_lock<std::shared_mutex> lock(m_writeCallbackMutex);

	auto writeCallback = m_writeCallback;

	if (writeCallback)
	{
		m_pendingRequests.push(std::move(callback));

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
	TScheduledCallback request;

	while (!m_pendingRequests.empty())
	{
		while (m_pendingRequests.try_pop(request))
		{
			if (m_client)
			{
				request();
			}
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
