/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "TcpServerManager.h"
#include "MultiplexTcpServer.h"

#include <libssh/libssh.h>
#include <libssh/bind.h>
#include <libssh/server.h>
#include <libssh/socket.h>

class ShellService
{
private:
	fwRefContainer<net::TcpServer> m_server;

	fwRefContainer<net::MultiplexTcpServer> m_serverHost;

private:
	ssh_bind m_bind;

private:
	void InitializeNative();

	void OnConnected(fwRefContainer<net::TcpServerStream> stream);

public:
	void Initialize(fwRefContainer<net::MultiplexTcpServer> serverHost);

	~ShellService();
};

void ShellService::Initialize(fwRefContainer<net::MultiplexTcpServer> serverHost)
{
	InitializeNative();

	m_serverHost = serverHost;

	m_server = serverHost->CreateServer([] (const std::vector<uint8_t>& data)
	{
		if (data.size() < 5)
		{
			return net::MultiplexPatternMatchResult::InsufficientData;
		}

		return ((memcmp(&data[0], "SSH-2", 5) == 0) ? net::MultiplexPatternMatchResult::Match : net::MultiplexPatternMatchResult::NoMatch);
	});

	m_server->SetConnectionCallback(std::bind(&ShellService::OnConnected, this, std::placeholders::_1));
}

ShellService::~ShellService()
{
	ssh_bind_free(m_bind);
}

void ShellService::InitializeNative()
{
	m_bind = ssh_bind_new();

	// find a host key if it exists
	FILE* hostKeyFile = fopen("ssh_host_rsa", "r");

	if (hostKeyFile)
	{
		std::vector<char> hostKeyData(65536);
		fread(&hostKeyData[0], 1, hostKeyData.size(), hostKeyFile);
		fclose(hostKeyFile);

		ssh_pki_import_privkey_base64(&hostKeyData[0], nullptr, nullptr, nullptr, &m_bind->rsa);
	}
	else
	{
		ssh_pki_generate(SSH_KEYTYPE_RSA, 2048, &m_bind->rsa);

		ssh_pki_export_privkey_file(m_bind->rsa, nullptr, nullptr, nullptr, "ssh_host_rsa");
	}

	ssh_bind_set_fd(m_bind, (socket_t)1);
	ssh_bind_listen(m_bind);
}

void ShellService::OnConnected(fwRefContainer<net::TcpServerStream> stream)
{
	ssh_session session = ssh_new();

	static ssh_server_callbacks_struct callbacks = { 0 };
	callbacks.auth_password_function = [] (ssh_session session, const char* user, const char* password, void*)
	{
		return static_cast<int>(SSH_AUTH_DENIED);
	};

	ssh_socket outSocket;
	ssh_bind_accept_fd(m_bind, session, (socket_t)stream.GetRef(), &outSocket);
	ssh_set_blocking(session, false);

	ssh_callbacks_init(&callbacks);
	ssh_set_server_callbacks(session, &callbacks);
	ssh_set_auth_methods(session, SSH_AUTH_METHOD_PASSWORD);

	outSocket->is_callback_socket = true;
	outSocket->fd_out = (socket_t)stream.GetRef();
	outSocket->write_callback = [] (ssh_socket socket, const void* data, size_t size)
	{
		// get the stream
		net::TcpServerStream* stream = reinterpret_cast<net::TcpServerStream*>(socket->fd_out);

		// create a buffer
		std::vector<uint8_t> dataBuf(size);
		memcpy(&dataBuf[0], data, dataBuf.size());

		stream->Write(dataBuf);

		return static_cast<int>(size);
	};

	std::shared_ptr<bool> inInputCallback = std::make_shared<bool>(false);

	outSocket->close_callback = [=] ()
	{
		if (!*inInputCallback)
		{
			stream->Close();
		}
		else
		{
			// mark that we need to be closed
			*inInputCallback = false;
		}
	};

	ssh_handle_key_exchange(session);

	stream->SetReadCallback([=] (const std::vector<uint8_t>& data)
	{
		// flag to prevent instant closing
		*inInputCallback = true;

		// call input callback
		outSocket->input_callback(outSocket, &data[0], data.size());

		// unset if set, kill if not set
		if (*inInputCallback)
		{
			*inInputCallback = false;
		}
		else
		{
			stream->Close();
		}
	});

	stream->SetCloseCallback([=] ()
	{
		// clean up the session
		ssh_free(session);
	});
}

#include "StdInc.h"
#include <ServerInstanceBase.h>
#include <TcpListenManager.h>

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		fx::TcpListenManager* listenManager = Instance<fx::TcpListenManager>::Get(instance->GetInstanceRegistry());

		listenManager->OnInitializeMultiplexServer.Connect([=](fwRefContainer<net::MultiplexTcpServer> server)
		{
			static ShellService shellService;
			shellService.Initialize(server);

			/*_CrtMemState s3;

			_CrtMemState souter1;
			_CrtMemCheckpoint(&souter1);

			{
				fwRefContainer<net::TcpServerManager> tcpStack = new net::TcpServerManager();

				fwRefContainer<net::MultiplexTcpServer> multiplexHost = new net::MultiplexTcpServer(tcpStack);
				multiplexHost->Bind(net::PeerAddress::FromString("0.0.0.0:30125").get());

				ShellService shellService;
				shellService.Initialize(multiplexHost);

				_CrtMemState s1;
				_CrtMemCheckpoint(&s1);

				std::this_thread::sleep_for(std::chrono::seconds(3600));

				// dump all new objects
				_CrtMemDumpAllObjectsSince(&s1);

				_CrtMemState s2;

				_CrtMemCheckpoint(&s2);

				_CrtMemDifference(&s3, &s1, &s2);
			}

			{
				_CrtMemDumpStatistics(&s3);
			}

			ExitProcess(0);*/
		});
	});
});