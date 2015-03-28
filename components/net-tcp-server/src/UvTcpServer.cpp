/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "UvTcpServer.h"
#include "TcpServerManager.h"

template<typename Handle, class Class, typename T1, void(Class::*Callable)(T1)>
void UvCallback(Handle* handle, T1 a1)
{
	(reinterpret_cast<Class*>(handle->data)->*Callable)(a1);
}

template<typename Handle, class Class, typename T1, typename T2, void(Class::*Callable)(T1, T2)>
void UvCallback(Handle* handle, T1 a1, T2 a2)
{
	(reinterpret_cast<Class*>(handle->data)->*Callable)(a1, a2);
}

namespace net
{
UvTcpServer::UvTcpServer(TcpServerManager* manager)
	: m_manager(manager)
{

}

UvTcpServer::~UvTcpServer()
{
	m_clients.clear();

	if (m_server.get())
	{
		uv_close(reinterpret_cast<uv_handle_t*>(m_server.get()), nullptr);
	}
}

bool UvTcpServer::Listen(std::unique_ptr<uv_tcp_t>&& server)
{
	m_server = std::move(server);

	int result = uv_listen(reinterpret_cast<uv_stream_t*>(m_server.get()), 0, UvCallback<uv_stream_t, UvTcpServer, int, &UvTcpServer::OnConnection>);

	bool retval = (result == 0);

	if (!retval)
	{
		trace("Listening on socket failed - libuv error %s.\n", uv_strerror(result));
	}

	return retval;
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
	std::unique_ptr<uv_tcp_t> clientHandle = std::make_unique<uv_tcp_t>();
	uv_tcp_init(m_manager->GetLoop(), clientHandle.get());

	// create a stream instance and associate
	fwRefContainer<UvTcpServerStream> stream(new UvTcpServerStream(this));
	clientHandle->data = stream.GetRef();

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

UvTcpServerStream::UvTcpServerStream(UvTcpServer* server)
	: m_server(server)
{

}

UvTcpServerStream::~UvTcpServerStream()
{
	if (m_client.get())
	{
		uv_close(reinterpret_cast<uv_handle_t*>(m_client.get()), nullptr);
	}
}

bool UvTcpServerStream::Accept(std::unique_ptr<uv_tcp_t>&& client)
{
	m_client = std::move(client);

	int result = uv_accept(reinterpret_cast<uv_stream_t*>(m_server->GetServer()),
						   reinterpret_cast<uv_stream_t*>(m_client.get()));

	if (result == 0)
	{
		uv_read_start(reinterpret_cast<uv_stream_t*>(m_client.get()), [] (uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf)
		{
			buf->base = new char[suggestedSize];
			buf->len = suggestedSize;
		}, UvCallback<uv_stream_t, UvTcpServerStream, ssize_t, const uv_buf_t*, &UvTcpServerStream::HandleRead>);
	}

	return (result == 0);
}

void UvTcpServerStream::HandleRead(ssize_t nread, const uv_buf_t* buf)
{
	if (nread > 0)
	{
		std::vector<uint8_t> targetBuf(nread);
		memcpy(&targetBuf[0], buf->base, targetBuf.size());

		delete[] buf->base;

		if (GetReadCallback())
		{
			GetReadCallback()(targetBuf);
		}
	}
	else if (nread < 0)
	{
		trace("read error: %s\n", uv_strerror(nread));

		// TODO: handle close
	}
}

PeerAddress UvTcpServerStream::GetPeerAddress()
{
	sockaddr_storage addr;
	int len = sizeof(addr);

	uv_tcp_getpeername(m_client.get(), reinterpret_cast<sockaddr*>(&addr), &len);

	return PeerAddress(reinterpret_cast<sockaddr*>(&addr), static_cast<socklen_t>(len));
}
}