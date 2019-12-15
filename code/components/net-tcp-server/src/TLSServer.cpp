/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "TLSServer.h"

#include "memdbgon.h"

#include <botan/auto_rng.h>
#include <botan/credentials_manager.h>
#include <botan/pk_algs.h>
#include <botan/pkcs8.h>
#include <botan/tls_policy.h>
#include <botan/x509self.h>
#include <botan/data_src.h>

#include <fstream>

#include <Error.h>

class CredentialManager : public Botan::Credentials_Manager
{
private:
	std::vector<Botan::X509_Certificate> m_certificates;
	std::unique_ptr<Botan::Private_Key> m_key;

public:
	CredentialManager(Botan::RandomNumberGenerator& rng, const fwPlatformString& serverCert, const fwPlatformString& serverKey, bool autoGenerate = false)
	{
		try
		{
			std::ifstream serverKeyStream(serverKey, std::ios::binary);
			std::ifstream serverCertStream(serverCert, std::ios::binary);
			
			if (serverCertStream && serverKeyStream)
			{
				Botan::DataSource_Stream ds(serverKeyStream);
				m_key.reset(Botan::PKCS8::load_key(ds, rng));

				Botan::DataSource_Stream in(serverCertStream);

				while (!in.end_of_data())
				{
					try
					{
						m_certificates.push_back(Botan::X509_Certificate(in));
					}
					catch (std::exception& e)
					{

					}
				}
			}
			else if (autoGenerate)
			{
				Botan::X509_Cert_Options options;
				options.country = "XX";
				options.common_name = "do-not-trust.citizenfx.tls.invalid";
				options.not_after("20250101000000Z");

				m_key = Botan::create_private_key("RSA", rng, "2048");

				m_certificates.push_back(Botan::X509::create_self_signed_cert(options, *(m_key.get()), "SHA-256", rng));

				std::string pemKey = Botan::PKCS8::PEM_encode(*(m_key.get()));
				std::string pemCert = m_certificates[0].PEM_encode();

				std::ofstream serverKeyOutStream(serverKey);
				serverKeyOutStream << pemKey;

				std::ofstream serverCertOutStream(serverCert);
				serverCertOutStream << pemCert;
			}
			else
			{
				FatalError("Could not open TLS certificate pair");
			}
		}
		catch (std::exception& e)
		{
			FatalError("%s", e.what());
		}
	}

	virtual std::vector<Botan::Certificate_Store*> trusted_certificate_authorities(const std::string& type, const std::string& context) override
	{
		return std::vector<Botan::Certificate_Store*>();
	}

	virtual std::vector<Botan::X509_Certificate> cert_chain(const std::vector<std::string>& cert_key_types, const std::string& type, const std::string& context) override
	{
		if (std::find(cert_key_types.begin(), cert_key_types.end(), m_key->algo_name()) != cert_key_types.end())
		{
			if (context == "" || m_certificates[0].matches_dns_name(context))
			{
				return m_certificates;
			}
		}

		return std::vector<Botan::X509_Certificate>();
	}

	virtual Botan::Private_Key* private_key_for(const Botan::X509_Certificate& cert, const std::string& type, const std::string& context) override
	{
		if (m_certificates[0] == cert)
		{
			return m_key.get();
		}

		return nullptr;
	}
};

#ifdef IS_FXSERVER
// hardened policy
class ServerTLSPolicy : public Botan::TLS::Policy
{
public:
	virtual bool abort_connection_on_undesired_renegotiation() const
	{
		return true;
	}
};
#endif

// policy allowing TLS_RSA_WITH_AES_256_CBC_SHA256 since ROS SDK wants this
class TLSPolicy : public Botan::TLS::Policy
{
public:
	virtual std::vector<std::string> allowed_ciphers() const override
	{
		return {
			"ChaCha20Poly1305",
			"AES-256/GCM",
			"AES-128/GCM",
			"AES-256/CCM",
			"AES-128/CCM",
			//"AES-256/CCM(8)",
			//"AES-128/CCM(8)",
			//"Camellia-256/GCM",
			//"Camellia-128/GCM",
			//"ARIA-256/GCM",
			//"ARIA-128/GCM",
			"AES-256", // ROS wants this
			"AES-128",
			//"Camellia-256",
			//"Camellia-128",
			//"SEED"
			//"3DES",
		};
	}

	virtual std::vector<std::string> allowed_signature_methods() const override
	{
		return {
			"ECDSA",
			"RSA",
			"IMPLICIT",
		};
	}

	virtual bool acceptable_protocol_version(Botan::TLS::Protocol_Version version) const override
	{
		return true;
	}

	virtual bool acceptable_ciphersuite(const Botan::TLS::Ciphersuite& suite) const override
	{
		return Botan::TLS::Policy::acceptable_ciphersuite(suite);
	}

	virtual std::vector<std::string> allowed_key_exchange_methods() const override
	{
		return {
			//"SRP_SHA",
			//"ECDHE_PSK",
			//"DHE_PSK",
			//"PSK",
			"CECPQ1",
			"ECDH",
			"DH",
			"RSA", // ROS wants this
		};
	}
};

#include <sstream>

namespace net
{
TLSServerStream::TLSServerStream(TLSServer* server, fwRefContainer<TcpServerStream> baseStream)
	: m_parentServer(server), m_baseStream(baseStream), m_closing(false)
{
	
}

void TLSServerStream::Initialize()
{
#ifndef IS_FXSERVER
	m_policy = std::make_unique<TLSPolicy>();
#else
	m_policy = std::make_unique<ServerTLSPolicy>();
#endif

	m_sessionManager = std::make_unique<Botan::TLS::Session_Manager_In_Memory>(m_rng);

	m_tlsServer.reset(new Botan::TLS::Server(
		*this,
		*(m_sessionManager.get()),
		*m_parentServer->GetCredentials(),
		*(m_policy.get()),
		m_rng
	));

	// first set the close callback, since SetReadCallback may in fact replay queued reads
	{
		fwRefContainer<TLSServerStream> thisRef = this;

		m_baseStream->SetCloseCallback([=]()
		{
			fwRefContainer<TLSServerStream> scopedThisRef = thisRef;

			CloseInternal();
		});
	}

	m_baseStream->SetReadCallback([=] (const std::vector<uint8_t>& data)
	{
		// keep a reference to the TLS server in case we close due to a TLS alert
		fwRefContainer<TLSServerStream> self = this;

		try
		{
			self->m_tlsServer->received_data(data);
		}
		catch (std::exception& e)
		{
#ifndef IS_FXSERVER
			trace("%s\n", e.what());
#endif
		}
	});
}

PeerAddress TLSServerStream::GetPeerAddress()
{
	return m_baseStream->GetPeerAddress();
}

void TLSServerStream::Write(const std::string& data)
{
	DoWrite<decltype(data)>(data);
}

void TLSServerStream::Write(const std::vector<uint8_t>& data)
{
	DoWrite<decltype(data)>(data);
}

void TLSServerStream::Write(std::string&& data)
{
	DoWrite<decltype(data)>(std::move(data));
}

void TLSServerStream::Write(std::vector<uint8_t>&& data)
{
	DoWrite<decltype(data)>(std::move(data));
}

void TLSServerStream::Close()
{
	fwRefContainer<TLSServerStream> thisRef = this;

	ScheduleCallback([thisRef]()
	{
		auto tlsServer = thisRef->m_tlsServer;

		if (tlsServer)
		{
			try
			{
				tlsServer->close();
			}
			catch (const std::exception& e)
			{
#ifndef IS_FXSERVER
				trace("tls close: %s\n", e.what());
#endif
			}
		}
	});
}

void TLSServerStream::WriteToClient(const uint8_t buf[], size_t length)
{
	std::vector<uint8_t> data(buf, buf + length);
	
	if (m_baseStream.GetRef())
	{
		m_baseStream->Write(data);
	}
}

void TLSServerStream::ReceivedData(const uint8_t buf[], size_t length)
{
	if (GetReadCallback())
	{
		std::vector<uint8_t> data(buf, buf + length);

		GetReadCallback()(data);
	}
}

void TLSServerStream::ReceivedAlert(Botan::TLS::Alert alert, const uint8_t[], size_t)
{
	if (alert.type() == Botan::TLS::Alert::CLOSE_NOTIFY)
	{
		fwRefContainer<TLSServerStream> thisRef = this;

		if (m_baseStream.GetRef())
		{
			m_baseStream->Close();
			m_baseStream = nullptr;
		}
	}
	else
	{
#ifndef IS_FXSERVER
		trace("alert %s\n", alert.type_string().c_str());
#endif
	}
}

bool TLSServerStream::HandshakeComplete()
{
	m_parentServer->InvokeConnectionCallback(this, m_protocol);

	return true;
}

std::string TLSServerStream::tls_server_choose_app_protocol(const std::vector<std::string>& client_protos)
{
	auto protocols = m_parentServer->GetProtocolList();

	std::set<std::string> client_protos_set(client_protos.begin(), client_protos.end());

	for (auto& protocol : protocols)
	{
		if (client_protos_set.find(protocol) != client_protos_set.end())
		{
			m_protocol = protocol;
			return protocol;
		}
	}

	return "";
}

void TLSServerStream::CloseInternal()
{
	// keep a reference to ourselves so we only free after returning
	fwRefContainer<TLSServerStream> thisRef = this;

	auto ourCloseCallback = GetCloseCallback();

	if (ourCloseCallback)
	{
		SetCloseCallback(TCloseCallback());
		ourCloseCallback();
	}

	SetReadCallback(TReadCallback());

	m_parentServer->CloseStream(this);
}

void TLSServerStream::ScheduleCallback(const TScheduledCallback& callback)
{
	if (m_baseStream.GetRef())
	{
		m_baseStream->ScheduleCallback(callback);
	}
}

TLSServer::TLSServer(fwRefContainer<TcpServer> baseServer, const std::string& certificatePath, const std::string& keyPath, bool autoGenerate)
{
	// initialize credentials
	Botan::AutoSeeded_RNG rng;
	
	Initialize(baseServer, std::make_shared<CredentialManager>(rng, MakeRelativeCitPath(certificatePath), MakeRelativeCitPath(keyPath), autoGenerate));
}

void TLSServer::Initialize(fwRefContainer<TcpServer> baseServer, std::shared_ptr<Botan::Credentials_Manager> credentialManager)
{
	m_baseServer = baseServer;

	m_credentials = credentialManager;

	m_baseServer->SetConnectionCallback([=](fwRefContainer<TcpServerStream> stream)
	{
		fwRefContainer<TLSServerStream> childStream = new TLSServerStream(this, stream);
		childStream->Initialize();

		m_connections.insert(childStream);
	});
}

class TLSProtocolFakeServer : public TcpServer
{

};

fwRefContainer<TcpServer> TLSServer::GetProtocolServer(const std::string& protocol)
{
	auto it = m_protocolServers.find(protocol);
	
	if (it == m_protocolServers.end())
	{
		it = m_protocolServers.insert({ protocol, new TLSProtocolFakeServer() }).first;
	}

	return it->second;
}

void TLSServer::InvokeConnectionCallback(TLSServerStream* stream, const std::string& protocol)
{
	fwRefContainer<TcpServer> tcpServer = this;

	if (auto it = m_protocolServers.find(protocol); it != m_protocolServers.end())
	{
		tcpServer = it->second;
	}

	if (tcpServer->GetConnectionCallback())
	{
		tcpServer->GetConnectionCallback()(stream);
	}
}
}
