/* Copyright (C) 2009-2014, Martin Johansson <martin@fatbob.nu>
   Copyright (C) 2005-2014, Thorvald Natvig <thorvald@natvig.com>

   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   - Neither the name of the Developers nor the names of its contributors may
     be used to endorse or promote products derived from this software without
     specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "StdInc.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#include "client.h"
#include "conf.h"
#include "log.h"
#include "memory.h"
#include "timer.h"
#include "version.h"
#include "util.h"
#include "sharedmemory.h"

/* globals */
bool_t shutdown_server;
extern char *bindaddr;
extern char *bindaddr6;
extern int bindport;
extern int bindport6;
int* udpsocks;
bool_t hasv4 = true, hasv6 = true;

const int on = 1;
int nofServerSocks = 4;

#if 0
void Server_runLoop(struct pollfd* pollfds)
{
	int timeout, rc, clientcount;

	etimer_t janitorTimer;
	Timer_init(&janitorTimer);

	while (!shutdown_server) {
		struct sockaddr_storage remote;
		int i;

#ifdef USE_SHAREDMEMORY_API
    Sharedmemory_alivetick();
#endif

		for(i = 0; i < nofServerSocks; i++) {
			pollfds[i].revents = 0;
		}

		clientcount = Client_getfds(&pollfds[nofServerSocks]);

		timeout = (int)(1000000LL - (int64_t)Timer_elapsed(&janitorTimer)) / 1000LL;
		if (timeout <= 0) {
			Client_janitor();
			Timer_restart(&janitorTimer);
			timeout = (int)(1000000LL - (int64_t)Timer_elapsed(&janitorTimer)) / 1000LL;
		}
		rc = poll(pollfds, clientcount + nofServerSocks, timeout);
		if (rc == 0) {
			/* Poll timed out, do maintenance */
			Timer_restart(&janitorTimer);
			Client_janitor();
			continue;
		}
		if (rc < 0) {
			if (errno == EINTR) /* signal */
				continue;
			else
				Log_fatal("poll: error %d (%s)", errno, strerror(errno));
		}

		/* Check for new connection */
		for (i = 0; i < nofServerSocks / 2; i++) {
			if (pollfds[i].revents) {
				int tcpfd;
				uint32_t addrlen = sizeof(struct sockaddr_storage);
				tcpfd = accept(pollfds[i].fd, (struct sockaddr *)&remote, &addrlen);
				fcntl(tcpfd, F_SETFL, O_NONBLOCK);
				setsockopt(tcpfd, IPPROTO_TCP, TCP_NODELAY, (char *) &on, sizeof(int));
				char *addressString = Util_addressToString(&remote);
				Log_debug("Connection from %s port %d\n", addressString, Util_addressToPort(&remote));
				free(addressString);
				if (Client_add(tcpfd, &remote) < 0)
					close(tcpfd);
			}
		}

		for (i = nofServerSocks / 2; i < nofServerSocks; i++) {
			if (pollfds[i].revents)
				Client_read_udp(udpsocks[i - nofServerSocks / 2]);
		}

		for (i = 0; i < clientcount; i++) {
			if (pollfds[nofServerSocks + i].revents & POLLIN)
				Client_read_fd(pollfds[nofServerSocks + i].fd);

			if (pollfds[nofServerSocks + i].revents & POLLOUT)
				Client_write_fd(pollfds[nofServerSocks + i].fd);
		}
#ifdef USE_SHAREDMEMORY_API
    Sharedmemory_update();
#endif
	}
}

void Server_setupTCPSockets(struct sockaddr_storage* addresses[2], struct pollfd* pollfds)
{
	int yes = 1;
	int sockets[2];

	if (hasv4) {
		/* IPv4 socket setup */
		sockets[0] = socket(PF_INET, SOCK_STREAM, 0);
		if (sockets[0] < 0)
			Log_fatal("socket IPv4");
		if (setsockopt(sockets[0], SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) != 0)
			Log_fatal("setsockopt IPv4: %s", strerror(errno));
		if (bind(sockets[0], (struct sockaddr *)addresses[0], sizeof (struct sockaddr_in)) < 0) {
			char *addressString = Util_addressToString(addresses[0]);
			Log_fatal("bind %s %d: %s", addressString, Util_addressToPort(addresses[0]), strerror(errno));
			free(addressString);
		}
		if (listen(sockets[0], 3) < 0)
			Log_fatal("listen IPv4");
		fcntl(sockets[0], F_SETFL, O_NONBLOCK);

		pollfds[0].fd = sockets[0];
		pollfds[0].events = POLLIN;
	}

	if (hasv6) {
		/* IPv6 socket setup */
		sockets[1] = socket(PF_INET6, SOCK_STREAM, 0);
		if (sockets[1] < 0)
			Log_fatal("socket IPv6: %s", strerror(errno));
		if (setsockopt(sockets[1], SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) != 0)
			Log_fatal("setsockopt IPv6: %s", strerror(errno));
		if (setsockopt(sockets[1], IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(int)) != 0)
			Log_fatal("setsockopt IPv6: %s", strerror(errno));
		if (bind(sockets[1], (struct sockaddr *)addresses[1], sizeof (struct sockaddr_in6)) < 0) {
			char *addressString = Util_addressToString(addresses[1]);
			Log_fatal("bind %s %d: %s", addressString, Util_addressToPort(addresses[1]), strerror(errno));
			free(addressString);
		}
		if (listen(sockets[1], 3) < 0)
			Log_fatal("listen IPv6");
		fcntl(sockets[1], F_SETFL, O_NONBLOCK);


		/* If  there is an IPv4 address, then IPv6 will use the second socket, otherwise it uses the first */
		pollfds[(hasv4) ? 1 : 0].fd = sockets[1];
		pollfds[(hasv4) ? 1 : 0].events = POLLIN;
	}
}

void Server_setupUDPSockets(struct sockaddr_storage* addresses[2], struct pollfd* pollfds)
{
	int val = 0;
	int sockets[2] = {-1, -1};

	udpsocks = Memory_safeCalloc(nofServerSocks / 2, sizeof(int));

	if (hasv4) {
		sockets[0] = socket(PF_INET, SOCK_DGRAM, 0);
		if (bind(sockets[0], (struct sockaddr *) addresses[0], sizeof (struct sockaddr_in)) < 0) {
			char *addressString = Util_addressToString(addresses[0]);
			Log_fatal("bind %s %d: %s", addressString, Util_addressToPort(addresses[0]), strerror(errno));
			free(addressString);
		}
		val = 0xe0;
		if (setsockopt(sockets[0], IPPROTO_IP, IP_TOS, &val, sizeof(val)) < 0)
			Log_warn("Server: Failed to set TOS for UDP Socket");
		val = 0x80;
		if (setsockopt(sockets[0], IPPROTO_IP, IP_TOS, &val, sizeof(val)) < 0)
			Log_warn("Server: Failed to set TOS for UDP Socket");

		fcntl(sockets[0], F_SETFL, O_NONBLOCK);
		pollfds[(hasv6) ? 2 : 1].fd = sockets[0];
		pollfds[(hasv6) ? 2 : 1].events = POLLIN | POLLHUP | POLLERR;
		udpsocks[0] = sockets[0];
	}

	if (hasv6) {
		sockets[1] = socket(PF_INET6, SOCK_DGRAM, 0);
		if (setsockopt(sockets[1], IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(int)) != 0)
			Log_fatal("setsockopt IPv6: %s", strerror(errno));
		if (bind(sockets[1], (struct sockaddr *) addresses[1], sizeof (struct sockaddr_in6)) < 0) {
			char *addressString = Util_addressToString(addresses[1]);
			Log_fatal("bind %s %d: %s", addressString, Util_addressToPort(addresses[1]), strerror(errno));
			free(addressString);
		}
		val = 0xe0;
		if (setsockopt(sockets[1], IPPROTO_IPV6, IPV6_TCLASS, &val, sizeof(val)) < 0)
			Log_warn("Server: Failed to set TOS for UDP Socket");
		val = 0x80;
		if (setsockopt(sockets[1], IPPROTO_IPV6, IPV6_TCLASS, &val, sizeof(val)) < 0)
			Log_warn("Server: Failed to set TOS for UDP Socket");

		fcntl(sockets[1], F_SETFL, O_NONBLOCK);
		pollfds[(hasv4) ? 3 : 1].fd = sockets[1];
		pollfds[(hasv4) ? 3 : 1].events = POLLIN | POLLHUP | POLLERR;
		udpsocks[(hasv4) ? 1 : 0] = sockets[1];
	}

}
#endif

void Server_run()
{
	//checkIPversions();

	/* max clients + server sokets + client connecting that will be disconnected */
	//pollfds = Memory_safeCalloc((getIntConf(MAX_CLIENTS) + nofServerSocks + 1) , sizeof(struct pollfd));

	/* Figure out bind address and port */
	//struct sockaddr_storage** addresses = Server_setupAddressesAndPorts();

	/* Prepare TCP sockets */
	//Server_setupTCPSockets(addresses, pollfds);

	/* Prepare UDP sockets */
	//Server_setupUDPSockets(addresses, pollfds);

	Log_info("uMurmur version %s ('%s') protocol version %d.%d.%d",
		UMURMUR_VERSION, UMURMUR_CODENAME, PROTVER_MAJOR, PROTVER_MINOR, PROTVER_PATCH);
	Log_info("Visit https://github.com/umurmur/umurmur");

	/* Main server loop */
	//Server_runLoop(pollfds);



	/* Disconnect clients and cleanup memory */
	Client_disconnect_all();
}

void Server_shutdown()
{
	shutdown_server = true;
}

#include <HttpServerManager.h>
#include <TcpServerManager.h>
#include <MultiplexTcpServer.h>

#include <ServerInstanceBase.h>
#include <UdpInterceptor.h>

#include "messages.h"

#undef PROTOCOL_VERSION
#include <TLSServer.h>

#include "client.h"
#include "messagehandler.h"

#include <thread>

bool_t checkDecrypt(client_t *client, const uint8_t *encrypted, uint8_t *plain, unsigned int len);
int Client_send_udp(client_t *client, uint8_t *data, int len);

// #TODO: Once correctness is guaranteed have servers create ETW traces to
// measure lock contention.
static std::mutex mumblePairsMutex;
static std::map<net::PeerAddress, bool> mumblePairs;

std::recursive_mutex g_mumbleClientMutex;
static std::map<net::PeerAddress, int> retryPairs;

static std::map<std::string, int> clientsPerIP;

static void SetMumbleAddress(const net::PeerAddress& address, bool isMumble)
{
	std::lock_guard _(mumblePairsMutex);
	mumblePairs.insert({ address, isMumble });
}

static void ClearMumbleAddress(const net::PeerAddress& address)
{
	std::lock_guard _(mumblePairsMutex);
	mumblePairs.erase(address);
}

static bool IsMumbleAddress(const net::PeerAddress& address, bool& isMumble)
{
	std::lock_guard _(mumblePairsMutex);
	if (auto it = mumblePairs.find(address); it != mumblePairs.end())
	{
		// if this is already known to not be a Mumble client, ignore
		if (!it->second)
		{
			return false;
		}

		isMumble = true;
	}
	return true;
}

static void CleanupMumbleAddresses()
{
	{
		std::lock_guard _(mumblePairsMutex);
		for (auto it = mumblePairs.begin(); it != mumblePairs.end(); )
		{
			if (!it->second)
			{
				it = mumblePairs.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}

static bool mumbleServerInitialized;

static void EnsureServerInitialized()
{
	if (!mumbleServerInitialized)
	{
		Chan_init();
		Client_init();
		// Ban_init();
		mumbleServerInitialized = true;
	}
}

std::shared_ptr<ConVar<bool>> mumble_disableServer;
std::shared_ptr<ConVar<int>> mumble_maxClientsPerIP;

static InitFunction initFunction([]()
{
	mumble_disableServer = std::make_shared<ConVar<bool>>("mumble_disableServer", ConVar_None, false);
	mumble_maxClientsPerIP = std::make_shared<ConVar<int>>("mumble_maxClientsPerIP", ConVar_None, 32);

	OnCreateTlsMultiplex.Connect([=](fwRefContainer<net::MultiplexTcpServer> multiplex)
	{
		if (mumble_disableServer->GetValue())
			return;

		EnsureServerInitialized();

		auto server = multiplex->CreateServer([](const std::vector<uint8_t>& bytes)
		{
			if (bytes.size() > 6)
			{
				if (bytes[0] == '\0' && bytes[1] == '\0')
				{
					uint32_t len = htonl(*(uint32_t*)&bytes[2]);

					// hacky, actually should check parsing
					if (bytes.size() < (len + 6))
					{
						return net::MultiplexPatternMatchResult::InsufficientData;
					}

					auto version = std::make_unique<MumbleProto::Version>();
					return (version->ParseFromArray(&bytes[6], len)) ? net::MultiplexPatternMatchResult::Match : net::MultiplexPatternMatchResult::NoMatch;
				}

				return net::MultiplexPatternMatchResult::NoMatch;
			}

			return net::MultiplexPatternMatchResult::InsufficientData;
		});

		server->SetConnectionCallback([=](fwRefContainer<net::TcpServerStream> stream)
		{
			client_t* client;

			{
				std::lock_guard _(g_mumbleClientMutex);

				auto hostIP = stream->GetPeerAddress().GetHost();
				auto mapEntry = clientsPerIP.find(hostIP);
				if (mapEntry != clientsPerIP.end())
				{
					auto maxClientCount = mumble_maxClientsPerIP->GetValue();
					if (mapEntry->second >= maxClientCount)
					{
						trace("IP %s already has %d Mumble connections active, rejecting client. This limit can be changed using the mumble_maxClientsPerIP ConVar.\n",
						hostIP.c_str(), maxClientCount);
						return;
					}
					mapEntry->second++;
				}
				else
				{
					clientsPerIP.insert({ hostIP, 1 });
				}

				Client_add(stream, &client);
			}

			stream->SetReadCallback([=](const std::vector<uint8_t>& data)
			{
				std::lock_guard _(g_mumbleClientMutex);

				Timer_restart(&client->lastActivity);

#if 0
				message_t *msg;

				memcpy(&client->rxbuf[client->rxcount], data.data(), data.size());
				client->rxcount += data.size();
				if (!client->msgsize && client->rxcount >= 6) {
					uint32_t msgLen;
					memcpy(&msgLen, &client->rxbuf[2], sizeof(uint32_t));
					client->msgsize = ntohl(msgLen);
				}
				if (client->msgsize > BUFSIZE - 6) {
					/* XXX - figure out how to handle this. A large size here can represent two cases:
					* 1. A valid size. The only message that is this big is UserState message with a big texture
					* 2. An invalid size = protocol error, e.g. connecting with a 1.1.x client
					*/
					Log_warn("Too big message received (%d bytes). Playing safe and disconnecting client %s",
						client->msgsize, client->remote_tcp.ToString().c_str());
					stream->Close();
					/* client->rxcount = client->msgsize = 0; */
				}
				else if (client->rxcount == client->msgsize + 6) { /* Got all of the message */
					msg = Msg_networkToMessage(client->rxbuf, client->msgsize + 6);
					/* pass messsage to handler */
					if (msg)
						Mh_handle_message(client, msg);
					client->rxcount = client->msgsize = 0;
				}
#endif

				auto& readQueue = client->rcvbuf;

				size_t origSize = readQueue.size();
				readQueue.resize(origSize + data.size());

				// close the stream if the length is too big
				if (readQueue.size() > (1024 * 1024 * 25))
				{
					stream->Close();
					return;
				}

				if (data.empty())
				{
					return;
				}

				std::copy(data.begin(), data.end(), readQueue.begin() + origSize);

				while (readQueue.size() > 6)
				{
					uint8_t lenBit[4];
					std::copy(readQueue.begin() + 2, readQueue.begin() + 6, lenBit);

					uint32_t msgLen = ntohl(*(uint32_t*)lenBit);

					if (readQueue.size() >= msgLen + 6)
					{
						// copy the deque into a vector for data purposes, again
						std::vector<uint8_t> rxbuf(readQueue.begin(), readQueue.begin() + msgLen + 6);

						// remove the original bytes from the queue
						readQueue.erase(readQueue.begin(), readQueue.begin() + msgLen + 6);

						if (rxbuf.size() > BUFSIZE)
						{
							Log_warn("Too big message received (%d bytes). Playing safe and disconnecting client %s",
								rxbuf.size(), client->remote_tcp.ToString().c_str());
							stream->Close();

							return;
						}

						memcpy(client->rxbuf, rxbuf.data(), rxbuf.size());
						client->msgsize = rxbuf.size();

						auto msg = Msg_networkToMessage(client->rxbuf, client->msgsize);
						/* pass messsage to handler */
						if (msg)
							Mh_handle_message(client, msg);
						client->rxcount = client->msgsize = 0;
					}
					else
					{
						break;
					}
				}

				// close stream if shutting down
				if (client->shutdown_wait)
				{
					client->stream->Close();
				}
			});

			stream->SetCloseCallback([=]()
			{
				ClearMumbleAddress(client->remote_udp);

				std::lock_guard _(g_mumbleClientMutex);
				auto mapEntry = clientsPerIP.find(client->remote_tcp.GetHost());
				if (mapEntry->second == 1)
					clientsPerIP.erase(mapEntry);
				else
					mapEntry->second--;

				// It is assumed Client_free is only ever called from this location.
				Client_free(client);
			});
		});

		std::thread([=]()
		{
			using namespace std::chrono_literals;
			SetThreadName(-1, "[Mumble] Worker thread");

			size_t i = 0;

			while (true)
			{
				std::this_thread::sleep_for(1000ms);
				std::lock_guard _(g_mumbleClientMutex);

				Client_janitor();

				++i;

				// clean up pairs every ~20 seconds
				if (i > 20)
				{
					CleanupMumbleAddresses();
					retryPairs.clear();
					i = 0;
				}
			}
		}).detach();
	});

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		if (mumble_disableServer->GetValue())
			return;

		EnsureServerInitialized();

		auto interceptor = instance->GetComponent<fx::UdpInterceptor>();

		interceptor->OnIntercept.Connect([interceptor](const net::PeerAddress& address, const uint8_t* data, size_t len, bool* intercepted)
		{
			bool known = false;

			if (!IsMumbleAddress(address, known))
			{
				return;
			}

			// is this a Mumble ping?
			if (len == 12 && *(uint32_t*)data == 0)
			{
				uint32_t ping[6];
				memcpy(ping, data, len);

				ping[0] = htonl((uint32_t)((PROTVER_MAJOR << 16) | (PROTVER_MINOR << 8) | (PROTVER_PATCH)));
				ping[3] = htonl((uint32_t)Client_count());
				ping[4] = htonl((uint32_t)getIntConf(MAX_CLIENTS));
				ping[5] = htonl((uint32_t)getIntConf(MAX_BANDWIDTH));

				*intercepted = true;

				interceptor->Send(address, ping, sizeof(ping));

				return;
			}

			auto fromAddress = address.GetHostBytes();

			// Mumble clients are expected to be connected to TCP already - if this is not a known Mumble TCP pair, mark and drop
			client_t* client = nullptr;

			uint8_t buffer[1024] = { 0 };

			// Lock moved from the interceptor assignment below to this location
			// to avoid conflict with Client_free while ensuring retryPairs does
			// not race with the Client_janitor thread.
			//
			// This may add a little bit more contention when processing unknown
			// addresses. Servers testing this did not report any noticeable
			// change in performance.
			std::lock_guard _(g_mumbleClientMutex);

			if (!known)
			{
				// try finding a client for whom the packet will decrypt correctly
				client_t* itr = nullptr;
				bool found = false;

				while (Client_iterate(&itr) != nullptr)
				{
					if (itr->remote_udp == net::PeerAddress{} &&
						(itr->remote_tcp.GetHostBytes() == fromAddress ||
						 itr->remote_tcp.GetHostBytesV6() == fromAddress))
					{
						known = true;

						// try decrypting the packet, maybe it'll be mumble
						if (checkDecrypt(itr, data, buffer, std::min(len, sizeof(buffer))))
						{
							// it's mumble!
							retryPairs[address] = 0;

							found = true;

							break;
						}
					}
				}

				// if no client matched even on IP, mark this pair as dead
				if (!known)
				{
					SetMumbleAddress(address, false);
					return;
				}

				// wasn't mumble, note down a failure and return
				if (!found)
				{
					// quite certainly not mumble
					if (retryPairs[address] > 5)
					{
						// permanently note down this pair as not-mumble
						SetMumbleAddress(address, false);

						return;
					}

					// note this client has been mumble
					retryPairs[address]++;
					return;
				}

				// mumble! let's mark it
				SetMumbleAddress(address, true);

				itr->remote_udp = address;

				client = itr;
			}
			else
			{
				client_t* itr = nullptr;

				// find the right client from our client list
				while (Client_iterate(&itr) != nullptr)
				{
					if (itr->remote_udp == address)
					{
						client = itr;
						break;
					}
				}

				if (!client)
				{
					return;
				}

				if (!checkDecrypt(client, data, buffer, std::min(len, sizeof(buffer))))
				{
					*intercepted = true;
					return;
				}
			}

			if (!client)
			{
				return;
			}

			client->interceptor = interceptor.GetRef();

			*intercepted = true;

			client->bUDP = true;
			len -= 4; /* Adjust for crypt header */
			auto msgType = (UDPMessageType_t)((buffer[0] >> 5) & 0x7);

			switch (msgType) {
			case UDPVoiceSpeex:
			case UDPVoiceCELTAlpha:
			case UDPVoiceCELTBeta:
				break;
			case UDPVoiceOpus:
				Client_voiceMsg(client, buffer, len);
				break;
			case UDPPing:
				Log_debug("UDP Ping reply len %d", len);
				Client_send_udp(client, buffer, len);
				break;
			default:
			{
				auto clientAddressString = Util_clientAddressToString(client);
				Log_debug("Unknown UDP message type from %s port %d", clientAddressString, address.GetPort());
				free(clientAddressString);
				break;
			}
			}
		});
	}, 999);
});
