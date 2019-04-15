#include <StdInc.h>
#include <uv.h>

#include <queue>

#include <UvLoopManager.h>
#include <UvTcpServer.h>

#define ENET_BUILDING_LIB 1
#include "enet/enet.h"

static std::chrono::nanoseconds timeBase{ 0 };

extern "C" int
enet_initialize(void)
{
	timeBase = std::chrono::nanoseconds(uv_hrtime());

	return 0;
}

extern "C" void
enet_deinitialize(void)
{

}

extern "C" enet_uint32
enet_time_get(void)
{
	auto time = std::chrono::nanoseconds(uv_hrtime());
	return static_cast<enet_uint32>(std::chrono::duration_cast<std::chrono::milliseconds>(time - timeBase).count());
}

extern "C" enet_uint32
enet_host_random_seed(void)
{
	return static_cast<enet_uint32>(uv_hrtime());
}

struct Datagram
{
	sockaddr_in6 from;
	std::unique_ptr<char[]> data;
	ssize_t read;
};

struct UdpSocket
{
	UvHandleContainer<uv_udp_t> udp;
	std::deque<Datagram> recvQueue;
};

static std::unordered_map<ENetSocket, std::shared_ptr<UdpSocket>> g_sockets;
static int g_curFd;

extern "C" ENetSocket
enet_socket_create(ENetSocketType type)
{
	assert(type == ENET_SOCKET_TYPE_DATAGRAM);

	auto socketData = std::make_shared<UdpSocket>();

	g_curFd += 4;
	auto fd = (ENetSocket)g_curFd;

	g_sockets[fd] = socketData;

	uv_udp_init(Instance<net::UvLoopManager>::Get()->GetOrCreate("svNetwork")->GetLoop(), &socketData->udp);
	
	return fd;
}

extern "C" void
enet_socket_destroy(ENetSocket socket)
{
	g_sockets.erase(socket);
}

extern "C" int
enet_socket_set_option(ENetSocket socket, ENetSocketOption option, int value)
{
	return 0;
}

static void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	*buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
}

fwEvent<> OnEnetReceive;

extern "C" int
enet_socket_bind(ENetSocket socket, const ENetAddress* address)
{
	struct sockaddr_in6 sin;

	memset(&sin, 0, sizeof(struct sockaddr_in6));

	sin.sin6_family = AF_INET6;

	if (address != NULL)
	{
		sin.sin6_port = ENET_HOST_TO_NET_16(address->port);
		sin.sin6_addr = address->host;
		sin.sin6_scope_id = address->sin6_scope_id;
	}
	else
	{
		sin.sin6_port = 0;
		sin.sin6_addr = in6addr_any;
		sin.sin6_scope_id = 0;
	}

	auto socketIt = g_sockets.find(socket);

	if (socketIt == g_sockets.end())
	{
		return -1;
	}

	auto sd = socketIt->second;

	int rv = uv_udp_bind(&sd->udp, (sockaddr*)&sin, 0);

	if (rv >= 0)
	{
		sd->udp.get()->data = sd.get();

		uv_udp_recv_start(&sd->udp, alloc_buffer, [](uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf,
			const struct sockaddr* addr, unsigned flags)
		{
			auto udpSocket = (UdpSocket*)handle->data;

			if (addr && addr->sa_family == AF_INET6)
			{
				const sockaddr_in6* in6 = (const sockaddr_in6*)addr;

				Datagram dgram;

				if (nread > 0)
				{
					dgram.data = std::unique_ptr<char[]>(new char[nread]);
					memcpy(dgram.data.get(), buf->base, nread);
				}

				dgram.read = nread;
				dgram.from = *in6;

				udpSocket->recvQueue.push_back(std::move(dgram));
			}

			OnEnetReceive();

			free(buf->base);
		});
	}

	return rv;
}

extern "C" int
enet_socket_get_address(ENetSocket socket, ENetAddress* address)
{
	auto socketIt = g_sockets.find(socket);

	if (socketIt == g_sockets.end())
	{
		return -1;
	}

	auto sd = socketIt->second;

	struct sockaddr_in6 sin;
	int sinLength = sizeof(struct sockaddr_in6);

	if (uv_udp_getsockname(&sd->udp, (struct sockaddr*)&sin, &sinLength) < 0)
		return -1;

	address->host = sin.sin6_addr;
	address->port = ENET_NET_TO_HOST_16(sin.sin6_port);
	address->sin6_scope_id = sin.sin6_scope_id;

	return 0;
}

extern "C" int
enet_socket_send(ENetSocket socket,
	const ENetAddress* address,
	const ENetBuffer* buffers,
	size_t bufferCount)
{
	struct sockaddr_in6 sin;

	if (address != NULL)
	{
		memset(&sin, 0, sizeof(struct sockaddr_in6));

		sin.sin6_family = AF_INET6;
		sin.sin6_port = ENET_HOST_TO_NET_16(address->port);
		sin.sin6_addr = address->host;
		sin.sin6_scope_id = address->sin6_scope_id;
	}

	auto socketIt = g_sockets.find(socket);

	if (socketIt == g_sockets.end())
	{
		return -1;
	}

	auto sd = socketIt->second;

	int sentLength = 0;

	std::vector<uv_buf_t> uvBuffers(bufferCount);
	for (size_t i = 0; i < bufferCount; i++)
	{
		uvBuffers[i].base = (char*)buffers[i].data;
		uvBuffers[i].len = buffers[i].dataLength;

		sentLength += buffers[i].dataLength;
	}

	auto sendReq = std::make_unique<uv_udp_send_t>();

	// since std::move in construction may execute before we do
	auto orderReq = sendReq.get();

	uv_udp_send(orderReq, &sd->udp, uvBuffers.data(), bufferCount, (sockaddr*)&sin, UvCallbackArgs<int>::Get(orderReq, [sendReq = std::move(sendReq)](uv_udp_send_t*, int)
	{
		// no-op
	}));

	return sentLength;
}

extern "C" int
enet_socket_receive(ENetSocket socket,
	ENetAddress* address,
	ENetBuffer* buffers,
	size_t bufferCount)
{
	auto socketIt = g_sockets.find(socket);

	if (socketIt == g_sockets.end())
	{
		return -1;
	}

	auto sd = socketIt->second;

	if (sd->recvQueue.empty())
	{
		return 0;
	}

	assert(bufferCount == 1);

	auto dgram = std::move(*sd->recvQueue.begin());
	sd->recvQueue.pop_front();

	auto nread = dgram.read;

	const auto& sin = dgram.from;

	if (address != NULL)
	{
		address->host = sin.sin6_addr;
		address->port = ENET_NET_TO_HOST_16(sin.sin6_port);
		address->sin6_scope_id = sin.sin6_scope_id;
	}

	if (dgram.data)
	{
		nread = std::min(buffers[0].dataLength, static_cast<size_t>(nread));

		memcpy(buffers[0].data, dgram.data.get(), nread);
	}

	return nread;
}

extern "C" int
enet_socketset_select(ENetSocket maxSocket, ENetSocketSet* readSet, ENetSocketSet* writeSet, enet_uint32 timeout)
{
	return -1;
}

extern "C" int
enet_socket_wait(ENetSocket socket, enet_uint32* condition, enet_uint32 timeout)
{
	*condition = 0;

	return 0;
}
