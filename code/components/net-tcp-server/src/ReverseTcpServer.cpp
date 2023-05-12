#include <StdInc.h>

#include <CoreConsole.h>
#include <ReverseTcpServer.h>

#include <UvLoopManager.h>

#include <json.hpp>

using json = nlohmann::json;

namespace net
{
	ReverseTcpServerStream::ReverseTcpServerStream(ReverseTcpServer* server, const std::shared_ptr<uvw::TCPHandle>& tcp)
		: m_server(server), m_tcp(tcp)
	{
		m_writeCallback = tcp->loop().resource<uvw::AsyncHandle>();
		m_writeCallback->data(std::make_shared<fwRefContainer<ReverseTcpServerStream>>(this));

		m_writeCallback->on<uvw::AsyncEvent>([](const uvw::AsyncEvent& e, uvw::AsyncHandle& h)
		{
			// as a possible result is closing self, keep reference
			auto refRef = h.data<fwRefContainer<ReverseTcpServerStream>>();

			if (!refRef)
			{
				return;
			}

			fwRefContainer<ReverseTcpServerStream> selfRef = *refRef;

			// dequeue pending writes
			TScheduledCallback request;

			while (selfRef->m_pendingRequests.try_pop(request))
			{
				request();
			}
		});

		m_writeCallback->on<uvw::CloseEvent>([](const uvw::CloseEvent& e, uvw::AsyncHandle& h)
		{
			h.data({});
		});
	}

	ReverseTcpServerStream::~ReverseTcpServerStream()
	{
		
	}

	void ReverseTcpServerStream::Write(const std::vector<uint8_t>& data, TCompleteCallback&& onComplete)
	{
		auto worker = m_tcp.lock();
		auto writeCallback = m_writeCallback;

		if (worker && writeCallback)
		{
			m_pendingRequests.push([worker, data, onComplete = std::move(onComplete)]() mutable
			{
				std::unique_ptr<char[]> msg{ new char[data.size()] };
				memcpy(msg.get(), data.data(), data.size());

				if (onComplete)
				{
					worker->once<uvw::WriteEvent>(make_shared_function([onComplete = std::move(onComplete)](const uvw::WriteEvent& e, uvw::TCPHandle& h) mutable
					{
						onComplete(true);
					}));
				}

				worker->write(std::move(msg), data.size());
			});

			writeCallback->send();
		}
	}

	void ReverseTcpServerStream::ScheduleCallback(TScheduledCallback&& callback, bool performInline)
	{
		auto writeCallback = m_writeCallback;

		if (writeCallback)
		{
			m_pendingRequests.push(std::move(callback));
			writeCallback->send();
		}
	}

	void ReverseTcpServerStream::ConsumeData(const void* data, size_t length)
	{
		auto rcb = GetReadCallback();

		if (rcb)
		{
			std::vector<uint8_t> d(length);
			memcpy(&d[0], data, length);

			rcb(d);
		}
	}

	void ReverseTcpServerStream::Close()
	{
		auto worker = m_tcp.lock();

		if (worker)
		{
			auto writeCallback = m_writeCallback;

			if (writeCallback)
			{
				fwRefContainer thisRef = this;

				m_pendingRequests.push([thisRef, writeCallback, worker]()
				{
					worker->once<uvw::ShutdownEvent>([](const uvw::ShutdownEvent& e, uvw::TCPHandle& h)
					{
						h.close();
					});

					worker->shutdown();

					writeCallback->once<uvw::CloseEvent>([writeCallback](const uvw::CloseEvent&, uvw::AsyncHandle& h)
					{
						(void)writeCallback;
					});
					writeCallback->close();

					thisRef->m_writeCallback = {};
				});

				m_writeCallback->send();
			}
		}
	}

	ReverseTcpServer::ReverseTcpServer()
		: m_loggedIn(false)
	{

	}

	ReverseTcpServer::~ReverseTcpServer()
	{

	}

	struct MessageHandler
	{
	 	bool ReceivedData(const void* data, size_t length);

	 	bool ReceivedMessage(const std::vector<uint8_t>& messageData);

		void WriteMessage(uvw::TCPHandle& tcp, const json& json);

		std::deque<uint8_t> GetAndResetLeftoverData();

		fwEvent<const json&> OnMessage;

	private:
		std::deque<uint8_t> m_readQueue;
	};

	void MessageHandler::WriteMessage(uvw::TCPHandle& tcp, const json& json)
	{
		auto jsonDump = json.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace);

		size_t len = jsonDump.length() + sizeof(uint32_t);
		std::unique_ptr<char[]> msg{ new char[len] };

		*(uint32_t*)(&msg.get()[0]) = uint32_t(jsonDump.length());
		memcpy(&msg.get()[4], jsonDump.data(), jsonDump.length());

		tcp.write(std::move(msg), len);
	}

	bool MessageHandler::ReceivedMessage(const std::vector<uint8_t>& messageData)
	{
		try
		{
			auto message = json::parse(messageData.begin(), messageData.end());

			if (!OnMessage(message))
			{
				return false;
			}
		}
		catch (std::exception& e)
		{
			trace("ReceivedControlMessage - %s\n", e.what());
		}

		return true;
	}

	bool MessageHandler::ReceivedData(const void* data, size_t length)
	{
		// append data to queue
		size_t origSize = m_readQueue.size();
		m_readQueue.resize(origSize + length);

		std::copy(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + length, m_readQueue.begin() + origSize);

		// read data, if sufficient
		while (m_readQueue.size() >= 4) // length prefix
		{
			uint8_t lenBit[4];
			std::copy(m_readQueue.begin(), m_readQueue.begin() + 4, lenBit);

			auto msgLength = *(uint32_t*)lenBit;

			if (m_readQueue.size() >= msgLength + 4)
			{
				// copy the deque into a vector for data purposes
				std::vector<uint8_t> rxbuf(m_readQueue.begin() + 4, m_readQueue.begin() + msgLength + 4);

				// remove the original bytes from the queue
				m_readQueue.erase(m_readQueue.begin(), m_readQueue.begin() + msgLength + 4);

				// invoke message handler
				if (!ReceivedMessage(rxbuf))
				{
					return false;
				}
			}
			else
			{
				break;
			}
		}

		return true;
	}

	std::deque<uint8_t> MessageHandler::GetAndResetLeftoverData()
	{
		return std::move(m_readQueue);
	}

	void ReverseTcpServer::CreateWorkerConnection()
	{
		// if we already have X spare workers, we don't need any more
		if ((m_work.size() - m_streams.size()) > 10)
		{
			return;
		}

		auto loopHolder = Instance<net::UvLoopManager>::Get()->GetOrCreate("default");
		auto loop = loopHolder->Get();

		auto worker = loop->resource<uvw::TCPHandle>();

		auto messageHandler = std::make_shared<MessageHandler>();
		fwRefContainer<ReverseTcpServer> thisRef(this);

		worker->data(messageHandler);

		worker->on<uvw::ConnectEvent>([thisRef](const uvw::ConnectEvent& e, uvw::TCPHandle& tcp)
		{
			tcp.data<MessageHandler>()->WriteMessage(tcp, json::array({
				"newWorker",
				json::object({ { "token", thisRef->m_token } })
			}));

			tcp.read();
		});

		std::weak_ptr<uvw::TCPHandle> weakWorker{ worker };

		worker->on<uvw::DataEvent>([thisRef, weakWorker](const uvw::DataEvent& e, uvw::TCPHandle& tcp)
		{
			auto worker = weakWorker.lock();

			if (worker)
			{
				auto it = thisRef->m_streams.find(worker);

				if (it == thisRef->m_streams.end())
				{
					if (!tcp.data<MessageHandler>()->ReceivedData(e.data.get(), e.length))
					{
						// want to escape into a stream
						it = thisRef->m_streams.find(worker);

						// process leftover data
						auto leftovers = tcp.data<MessageHandler>()->GetAndResetLeftoverData();

						if (it != thisRef->m_streams.end())
						{
							// copy into a contiguous buffer
							std::vector<uint8_t> data(leftovers.begin(), leftovers.end());

							it->second->ConsumeData(data.data(), data.size());
						}
					}
				}
				else
				{
					it->second->ConsumeData(e.data.get(), e.length);
				}
			}
		});

		messageHandler->OnMessage.Connect([thisRef, weakWorker](const json& message)
		{
			auto type = message[0].get<std::string>();

			if (type == "startWorker")
			{
				auto worker = weakWorker.lock();

				if (worker)
				{
					fwRefContainer<ReverseTcpServerStream> stream{ new ReverseTcpServerStream(thisRef.GetRef(), worker) };

					auto address = net::PeerAddress::FromString(message[1].value("address", "127.0.0.1:65535"), 30120, net::PeerAddress::LookupType::NoResolution);

					if (address)
					{
						stream->m_remotePeerAddress = *address;
					}

					thisRef->m_streams[worker] = stream;

					auto ccb = thisRef->GetConnectionCallback();

					if (ccb)
					{
						ccb(stream);
					}

					return false;
				}
			}

			return true;
		});

		worker->on<uvw::EndEvent>([](const uvw::EndEvent& e, uvw::TCPHandle& tcp)
		{
			tcp.close();
		});

		worker->on<uvw::ErrorEvent>([](const uvw::ErrorEvent& e, uvw::TCPHandle& tcp)
		{
			tcp.close();
		});

		worker->on<uvw::CloseEvent>([weakWorker, thisRef](const uvw::CloseEvent& e, uvw::TCPHandle& tcp)
		{
			thisRef->RemoveWorker(weakWorker.lock());
		});

		worker->keepAlive(true, std::chrono::duration<unsigned int>{5});
		worker->connect(*m_curRemote.GetSocketAddress());

		m_work.insert(worker);
	}

	void ReverseTcpServer::RemoveWorker(const std::shared_ptr<uvw::TCPHandle>& worker)
	{
		auto it = m_streams.find(worker);

		if (it != m_streams.end())
		{
			auto ccb = it->second->GetCloseCallback();
			it->second->SetCloseCallback({});

			if (ccb)
			{
				ccb();
			}

			auto writeCallback = it->second->m_writeCallback;

			if (writeCallback)
			{
				writeCallback->close();
			}
		}

		m_streams.erase(worker);
		m_work.erase(worker);
	}

	void ReverseTcpServer::Listen(const std::string& remote, const std::string& regToken)
	{
		m_remote = remote;
		m_regToken = regToken;

		auto loopHolder = Instance<net::UvLoopManager>::Get()->GetOrCreate("default");
		auto loop = loopHolder->Get();
		m_loop = loop;

		fwRefContainer<ReverseTcpServer> thisRef(this);

		loopHolder->EnqueueCallback([thisRef, loop]()
		{
			thisRef->m_reconnectTimer = loop->resource<uvw::TimerHandle>();

			thisRef->m_reconnectTimer->on<uvw::TimerEvent>([thisRef](const uvw::TimerEvent& e, uvw::TimerHandle& timer)
			{
				thisRef->Reconnect();
			});

			thisRef->Reconnect();
		});
	}

	void ReverseTcpServer::Reconnect()
	{
		using namespace std::chrono_literals;

		m_control = {};

		m_addr = m_loop->resource<uvw::GetAddrInfoReq>();

		auto hostStr = m_remote.substr(0, m_remote.find_last_of(':'));
		std::string portStr;

		if (auto colonPos = m_remote.find_last_of(':'); colonPos != std::string::npos)
		{
			portStr = m_remote.substr(colonPos + 1);
		}

		fwRefContainer<ReverseTcpServer> thisRef(this);

		m_addr->on<uvw::ErrorEvent>([thisRef](const uvw::ErrorEvent& e, uvw::GetAddrInfoReq& req)
		{
			thisRef->m_reconnectTimer->start(2500ms, 0ms);
		});

		m_addr->on<uvw::AddrInfoEvent>([thisRef](const uvw::AddrInfoEvent& e, uvw::GetAddrInfoReq& req)
		{
			if (e.data)
			{
				net::PeerAddress peerAddress(e.data->ai_addr, e.data->ai_addrlen);
				thisRef->ReconnectWithPeer(peerAddress);

				return;	
			}

			thisRef->m_reconnectTimer->start(2500ms, 0ms);
		});

		m_addr->addrInfo(hostStr, portStr);
	}

	void ReverseTcpServer::ReconnectWithPeer(const net::PeerAddress& peer)
	{
		using namespace std::chrono_literals;

		m_control = m_loop->resource<uvw::TCPHandle>();

		fwRefContainer<ReverseTcpServer> thisRef(this);

		auto messageHandler = std::make_shared<MessageHandler>();

		messageHandler->OnMessage.Connect([thisRef](const json& message)
		{
			auto type = message[0].get<std::string>();

			if (type == "loginSuccess")
			{
				thisRef->m_loggedIn = true;

				thisRef->m_token = message[1].value("token", "");
			}
			else if (type == "newWorkerReq")
			{
				thisRef->CreateWorkerConnection();
			}
			else if (type == "error")
			{
				console::DPrintf("net", "Proxy error: %s\n", message[1].get<std::string>());
			}
		});

		m_control->data(messageHandler);

		m_control->on<uvw::ConnectEvent>([thisRef](const uvw::ConnectEvent& e, uvw::TCPHandle& tcp)
		{
			tcp.data<MessageHandler>()->WriteMessage(tcp, json::array({
				"login",
				json::object({ { "token", thisRef->m_regToken } })
				}));

			tcp.read();
		});

		m_control->on<uvw::DataEvent>([thisRef](const uvw::DataEvent& e, uvw::TCPHandle& tcp)
		{
			tcp.data<MessageHandler>()->ReceivedData(e.data.get(), e.length);
		});

		m_control->on<uvw::EndEvent>([](const uvw::EndEvent& e, uvw::TCPHandle& tcp)
		{
			tcp.close();
		});

		m_control->on<uvw::ErrorEvent>([thisRef](const uvw::ErrorEvent& e, uvw::TCPHandle& tcp)
		{
			tcp.close();
		});

		m_control->on<uvw::CloseEvent>([thisRef](const uvw::CloseEvent& e, uvw::TCPHandle& tcp)
		{
			thisRef->m_reconnectTimer->start(2500ms, 0ms);
		});

		m_control->keepAlive(true, std::chrono::duration<unsigned int>{5});
		m_control->connect(*peer.GetSocketAddress());
		m_curRemote = peer;
	}
}
