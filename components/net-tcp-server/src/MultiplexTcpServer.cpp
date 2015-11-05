/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "MultiplexTcpServer.h"

#include <deque>
#include <memory>

#include "memdbgon.h"

namespace net
{
MultiplexTcpServer::MultiplexTcpServer(const fwRefContainer<TcpServerFactory>& factory)
	: m_factory(factory)
{
	
}

void MultiplexTcpServer::Bind(const PeerAddress& bindAddress)
{
	if (m_rootServer.GetRef())
	{
		trace("MultiplexTcpServer is already bound - not binding to %s.\n", bindAddress.ToString().c_str());
		return;
	}

	m_rootServer = m_factory->CreateServer(bindAddress);
	
	if (m_rootServer.GetRef())
	{
		m_rootServer->SetConnectionCallback([=] (fwRefContainer<TcpServerStream> stream)
		{
			// start the attachment process for the stream
			std::shared_ptr<std::vector<uint8_t>> recvQueue = std::make_shared<std::vector<uint8_t>>();

			TcpServerStream::TReadCallback readCallback = [=] (const std::vector<uint8_t>& data)
			{
				if (data.size() > 0)
				{
					// copy to the receive queue
					size_t origSize = recvQueue->size();
					recvQueue->resize(origSize + data.size());

					std::copy(data.begin(), data.end(), recvQueue->begin() + origSize);

					// check all the servers for a pattern match
					bool needMoreData = false;

					for (auto& server : m_childServers)
					{
						auto& matchFunction = server->GetPatternMatcher();
						auto matchResult = matchFunction(*recvQueue);

						// if we matched on this server
						if (matchResult == MultiplexPatternMatchResult::Match)
						{
							// keep a scope-local reference to the receive queue/stream (as we'll lose the stored reference soon)
							auto localStream = stream;
							auto localRecvQueue = recvQueue;

							// unset our read callback
							stream->SetReadCallback(TcpServerStream::TReadCallback());

							// forward the result
							server->AttachToResult(*localRecvQueue, localStream);

							// return so that the stream doesn't end up closed
							return;
						}
						else if (matchResult == MultiplexPatternMatchResult::InsufficientData)
						{
							needMoreData = true;
						}
					}

					if (!needMoreData)
					{
						// nobody matched, and we don't need more data - this stream is useless to us
						stream->Close();
					}
				}
			};

			stream->SetReadCallback(readCallback);
		});
	}
	else
	{
		trace("Could not bind MultiplexTcpServer to %s.\n", bindAddress.ToString().c_str());
	}
}

void MultiplexTcpChildServer::AttachToResult(const std::vector<uint8_t>& existingData, fwRefContainer<TcpServerStream> baseStream)
{
	fwRefContainer<MultiplexTcpChildServerStream> stream = new MultiplexTcpChildServerStream(this, baseStream);
	stream->SetInitialData(existingData);

	// keep a local reference to the connection
	m_connections.insert(stream);

	// invoke the connection callback
	auto connectionCallback = GetConnectionCallback();

	if (connectionCallback)
	{
		connectionCallback(stream);
	}
}

void MultiplexTcpChildServer::CloseStream(MultiplexTcpChildServerStream* stream)
{
	m_connections.erase(stream);
}

void MultiplexTcpChildServer::SetPatternMatcher(const MultiplexPatternMatchFn& function)
{
	m_patternMatcher = function;
}

MultiplexTcpChildServerStream::MultiplexTcpChildServerStream(MultiplexTcpChildServer* server, fwRefContainer<TcpServerStream> baseStream)
	: m_baseStream(baseStream), m_server(server)
{
	baseStream->SetReadCallback([=] (const std::vector<uint8_t>& data)
	{
		auto ourReadCallback = GetReadCallback();

		if (ourReadCallback)
		{
			TrySendInitialData();

			ourReadCallback(data);
		}
	});

	baseStream->SetCloseCallback([=] ()
	{
		CloseInternal();
	});
}

void MultiplexTcpChildServerStream::TrySendInitialData()
{
	auto ourReadCallback = GetReadCallback();

	if (ourReadCallback)
	{
		if (!m_initialData.empty())
		{
			ourReadCallback(m_initialData);
			m_initialData.clear();
		}
	}
}

void MultiplexTcpChildServerStream::OnFirstSetReadCallback()
{
	TrySendInitialData();
}

void MultiplexTcpChildServerStream::CloseInternal()
{
	// keep a reference to ourselves so we only free after returning
	fwRefContainer<MultiplexTcpChildServerStream> thisRef = this;

	auto ourCloseCallback = GetCloseCallback();

	if (ourCloseCallback)
	{
		SetCloseCallback(TCloseCallback());
		ourCloseCallback();
	}

	SetReadCallback(TReadCallback());

	m_server->CloseStream(this);
}

void MultiplexTcpChildServerStream::Close()
{
	// keep a reference to ourselves
	fwRefContainer<MultiplexTcpChildServerStream> thisRef = this;

	if (m_baseStream.GetRef())
	{
		m_baseStream->Close();
		m_baseStream = nullptr;
	}

	CloseInternal();
}

void MultiplexTcpChildServerStream::Write(const std::vector<uint8_t>& data)
{
	m_baseStream->Write(data);
}

PeerAddress MultiplexTcpChildServerStream::GetPeerAddress()
{
	return m_baseStream->GetPeerAddress();
}

void MultiplexTcpChildServerStream::SetInitialData(const std::vector<uint8_t>& initialData)
{
	m_initialData = initialData;
}

fwRefContainer<TcpServer> MultiplexTcpServer::CreateServer(const MultiplexPatternMatchFn& patternMatchFunction)
{
	fwRefContainer<MultiplexTcpChildServer> child = new MultiplexTcpChildServer();
	child->SetPatternMatcher(patternMatchFunction);

	m_childServers.push_back(child);

	return child;
}
}