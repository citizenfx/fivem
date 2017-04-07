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
#include <botan/pkcs8.h>
#include <botan/tls_policy.h>

#include <fstream>

#include <Error.h>

class CredentialManager : public Botan::Credentials_Manager
{
private:
	std::vector<Botan::X509_Certificate> m_certificates;
	std::shared_ptr<Botan::Private_Key> m_key;

public:
	CredentialManager(Botan::RandomNumberGenerator& rng, const fwPlatformString& serverCert, const fwPlatformString& serverKey)
	{
		try
		{
			std::ifstream serverKeyStream(serverKey);
			std::ifstream serverCertStream(serverCert);

			m_key.reset(Botan::PKCS8::load_key(Botan::DataSource_Stream(serverKeyStream), rng));

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

class TLSPolicy : public Botan::TLS::Policy
{
public:
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
			"RSA",
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
	m_policy = std::make_unique<TLSPolicy>();
	m_sessionManager = std::make_unique<Botan::TLS::Session_Manager_In_Memory>(m_rng);

	m_tlsServer.reset(new Botan::TLS::Server(
		*this,
		*(m_sessionManager.get()),
		*m_parentServer->GetCredentials(),
		*(m_policy.get()),
		m_rng
	));

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
			trace("%s\n", e.what());
		}
	});

	m_baseStream->SetCloseCallback([=] ()
	{
		CloseInternal();
	});
}

PeerAddress TLSServerStream::GetPeerAddress()
{
	return m_baseStream->GetPeerAddress();
}

void TLSServerStream::Write(const std::vector<uint8_t>& data)
{
	m_tlsServer->send(data);
}

void TLSServerStream::Close()
{
	m_tlsServer->close();
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
	trace("alert %s\n", alert.type_string().c_str());

	if (alert.type() == Botan::TLS::Alert::CLOSE_NOTIFY)
	{
		fwRefContainer<TLSServerStream> thisRef = this;

		if (m_baseStream.GetRef())
		{
			m_baseStream->Close();
			m_baseStream = nullptr;
		}
	}
}

bool TLSServerStream::HandshakeComplete(const Botan::TLS::Session& session)
{
	m_parentServer->InvokeConnectionCallback(this);

	return true;
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

TLSServer::TLSServer(fwRefContainer<TcpServer> baseServer, const std::string& certificatePath, const std::string& keyPath)
	: m_baseServer(baseServer)
{
	// initialize credentials
	Botan::AutoSeeded_RNG rng;
	m_credentials = std::make_shared<CredentialManager>(rng, MakeRelativeCitPath(certificatePath), MakeRelativeCitPath(keyPath));
	
	m_baseServer->SetConnectionCallback([=] (fwRefContainer<TcpServerStream> stream)
	{
		fwRefContainer<TLSServerStream> childStream = new TLSServerStream(this, stream);
		childStream->Initialize();

		m_connections.insert(childStream);
	});
}
}