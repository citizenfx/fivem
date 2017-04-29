/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "MumbleClientImpl.h"
#include <thread>

static __declspec(thread) MumbleClient* g_currentMumbleClient;

void MumbleClient::Initialize()
{
	if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)))
	{
		return;
	}

	m_beginConnectEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	m_socketConnectEvent = nullptr;
	m_socketReadEvent = nullptr;
	m_idleEvent = CreateWaitableTimer(nullptr, FALSE, nullptr);

	m_mumbleThread = std::thread(ThreadFunc, this);

	m_audioInput.Initialize();
	m_audioInput.SetClient(this);

	WSADATA wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);
}

concurrency::task<MumbleConnectionInfo*> MumbleClient::ConnectAsync(const char* hostname, uint16_t port, const wchar_t* username)
{
	m_connectionInfo.hostname = hostname;
	m_connectionInfo.port = port;
	m_connectionInfo.username = username;

	m_state.SetClient(this);
	m_state.SetUsername(std::wstring(username));

	SetEvent(m_beginConnectEvent);

	m_completionEvent = concurrency::task_completion_event<MumbleConnectionInfo*>();

	return concurrency::task<MumbleConnectionInfo*>(m_completionEvent);
}

concurrency::task<void> MumbleClient::DisconnectAsync()
{
	return concurrency::task<void>();
}

void MumbleClient::ThreadFuncImpl()
{
	SetThreadName(-1, "[Mumble] Network Thread");

	while (true)
	{
		ClientTask task = WaitForTask();

		try
		{
			switch (task)
			{
				case ClientTask::BeginConnect:
				{
					auto hostName = m_connectionInfo.hostname.c_str();
					uint32_t addr = inet_addr(hostName);

					/*if (entity == nullptr)
					{
					trace("[mumble] gethostbyname(%s) failed\n", hostName);

					break;
					}*/

					//if (entity->h_addrtype == AF_INET)
					if (true)
					{
						sockaddr_in inetAddr = { 0 };
						inetAddr.sin_family = AF_INET;
						inetAddr.sin_port = htons(m_connectionInfo.port);
						inetAddr.sin_addr.s_addr = addr;
						//memcpy(&inetAddr.sin_addr, entity->h_addr, entity->h_length);

						m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

						m_socketConnectEvent = WSACreateEvent();
						m_socketReadEvent = WSACreateEvent();

						WSAEventSelect(m_socket, m_socketConnectEvent, FD_CONNECT);

						//u_long nonBlocking = 1;
						//ioctlsocket(m_socket, FIONBIO, &nonBlocking);

						connect(m_socket, (sockaddr*)&inetAddr, sizeof(inetAddr));

						trace("[mumble] connecting to %s...\n", hostName);
					}

					break;
				}

				case ClientTask::EndConnect:
				{
					if (!m_connectionInfo.isConnected)
					{
						// TODO: reconnecting?
						trace("[mumble] connecting failed: %d\n", WSAGetLastError());

						break;
					}

					WSACloseEvent(m_socketConnectEvent);
					m_socketConnectEvent = INVALID_HANDLE_VALUE;

					LARGE_INTEGER waitTime;
					waitTime.QuadPart = -20000000LL;

					SetWaitableTimer(m_idleEvent, &waitTime, 0, nullptr, nullptr, 0);

					m_handler.Reset();

					WSAEventSelect(m_socket, m_socketReadEvent, FD_READ);

					m_sessionManager = std::make_unique<Botan::TLS::Session_Manager_In_Memory>(m_rng);

					m_credentials = std::make_unique<MumbleCredentialsManager>();

					std::string hostNameStr(m_connectionInfo.hostname.c_str());

					m_tlsClient = std::make_shared<Botan::TLS::Client>(std::bind(&MumbleClient::WriteToSocket, this, std::placeholders::_1, std::placeholders::_2),
																	   std::bind(&MumbleClient::OnReceive, this, std::placeholders::_1, std::placeholders::_2),
																	   std::bind(&MumbleClient::OnAlert, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
																	   std::bind(&MumbleClient::OnHandshake, this, std::placeholders::_1),
																	   *(m_sessionManager.get()),
																	   *(m_credentials.get()),
																	   m_policy,
																	   m_rng,
																	   Botan::TLS::Server_Information(hostNameStr, m_connectionInfo.port)
																	   );

					break;
				}

				case ClientTask::Idle:
				{
					if (m_tlsClient->is_active())
					{
						MumbleProto::Ping ping;
						ping.set_timestamp(GetTickCount64());
						ping.set_tcp_ping_avg(13.37f);
						ping.set_tcp_ping_var(13.37f);

						Send(MumbleMessageType::Ping, ping);

						LARGE_INTEGER waitTime;
						waitTime.QuadPart = -20000000LL;

						SetWaitableTimer(m_idleEvent, &waitTime, 0, nullptr, nullptr, 0);
					}

					break;
				}

				case ClientTask::RecvData:
				{
					uint8_t buffer[16384];
					int len = recv(m_socket, (char*)buffer, sizeof(buffer), 0);

					ResetEvent(m_socketReadEvent);

					if (len > 0)
					{
						m_clientMutex.lock();
						m_tlsClient->received_data(buffer, len);
						m_clientMutex.unlock();
					}
					else if (len == 0)
					{
						// TCP close, graceful?
						trace("[mumble] tcp close :(\n");
					}
					else
					{
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							// TCP close, error state
							trace("[mumble] tcp error :(\n");
						}
					}

					break;
				}

			}
		}
		catch (std::exception& e)
		{
			trace("lolexception %s", e.what());
		}
	}
}

void MumbleClient::Send(MumbleMessageType type, const char* buf, size_t size)
{
	MumblePacketHeader header;
	header.SetPacketType((uint16_t)type);
	header.SetPacketLength(size);

	Send((const char*)&header, sizeof(header));
	Send(buf, size);
}

void MumbleClient::Send(const char* buf, size_t size)
{
	m_clientMutex.lock();
	m_tlsClient->send((const uint8_t*)buf, size);
	m_clientMutex.unlock();
}

void MumbleClient::WriteToSocket(const uint8_t buf[], size_t length)
{
	send(m_socket, (const char*)buf, length, 0);
}

void MumbleClient::OnAlert(Botan::TLS::Alert alert, const uint8_t[], size_t)
{
	trace("[mumble] TLS alert: %s\n", alert.type_string().c_str());
}

void MumbleClient::OnReceive(const uint8_t buf[], size_t length)
{
	trace("omg data: %d bytes\n", length);

	g_currentMumbleClient = this;

	m_handler.HandleIncomingData(buf, length);
}

bool MumbleClient::OnHandshake(const Botan::TLS::Session& session)
{
	trace("got session %s %s\n", session.version().to_string().c_str(), session.ciphersuite().to_string().c_str());

	return true;
}

fwRefContainer<MumbleClient> MumbleClient::GetCurrent()
{
	return g_currentMumbleClient;
}

ClientTask MumbleClient::WaitForTask()
{
	HANDLE waitHandles[16];
	int waitCount = 0;

	waitHandles[waitCount] = m_beginConnectEvent;
	waitCount++;

	if (m_socketConnectEvent && m_socketConnectEvent != INVALID_HANDLE_VALUE)
	{
		waitHandles[waitCount] = m_socketConnectEvent;
		waitCount++;
	}

	if (m_socketReadEvent && m_socketReadEvent != INVALID_HANDLE_VALUE)
	{
		waitHandles[waitCount] = m_socketReadEvent;
		waitCount++;
	}

	waitHandles[waitCount] = m_idleEvent;
	waitCount++;

	DWORD waitResult = WaitForMultipleObjects(waitCount, waitHandles, FALSE, INFINITE);

	if (waitResult >= WAIT_OBJECT_0 && waitResult <= (WAIT_OBJECT_0 + waitCount))
	{
		HANDLE compareHandle = waitHandles[waitResult - WAIT_OBJECT_0];

		if (compareHandle == m_beginConnectEvent)
		{
			return ClientTask::BeginConnect;
		}
		else if (compareHandle == m_socketConnectEvent)
		{
			return ClientTask::EndConnect;
		}
		else if (compareHandle == m_socketReadEvent)
		{
			return ClientTask::RecvData;
		}
		else if (compareHandle == m_idleEvent)
		{
			return ClientTask::Idle;
		}
	}
}

void MumbleClient::ThreadFunc(MumbleClient* client)
{
	client->ThreadFuncImpl();
}

fwRefContainer<IMumbleClient> CreateMumbleClient()
{
	return new MumbleClient();
}