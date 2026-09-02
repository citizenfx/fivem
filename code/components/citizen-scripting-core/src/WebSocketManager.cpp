/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "WebSocketManager.h"

#include <Resource.h>
#include <ResourceManager.h>

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <thread>
#include <future>
#include <chrono>

namespace fx
{

// Timeout for WebSocket connection attempts (in seconds)
static constexpr int kConnectionTimeoutSeconds = 30;

using WebSocketClient = websocketpp::client<websocketpp::config::asio_client>;
using WebSocketConnectionHandle = websocketpp::connection_hdl;
using WebSocketMessagePtr = websocketpp::config::asio_client::message_type::ptr;

class WebSocketManager::WebSocketConnection
{
public:
	WebSocketConnection(int id, const std::string& url, WebSocketManager* manager, Resource* resource)
		: m_id(id)
		, m_url(url)
		, m_state(WebSocketState::Connecting)
		, m_manager(manager)
		, m_resource(resource)
		, m_isRunning(false)
		, m_hasStarted(false)
	{
	}

	~WebSocketConnection()
	{
		Close();
	}

	bool Start(const WebSocketEventCallback& onEvent)
	{
		// Prevent multiple starts
		bool expected = false;
		if (!m_hasStarted.compare_exchange_strong(expected, true))
		{
			return false;
		}

		m_onEvent = onEvent;
		m_isRunning = true;

		auto connectionPromise = std::make_shared<std::promise<bool>>();
		auto connectionFuture = connectionPromise->get_future();
		auto promiseFulfilled = std::make_shared<std::atomic<bool>>(false);

		try
		{
			m_client.init_asio();

			m_client.set_open_handler([this](WebSocketConnectionHandle hdl)
			{
				m_connectionHandle = hdl;
				m_state = WebSocketState::Open;
				// Don't signal success yet - wait for first message (welcome)
				QueueEvent(WebSocketEventType::Open, "");
			});

			m_client.set_message_handler([this, connectionPromise, promiseFulfilled](WebSocketConnectionHandle hdl, WebSocketMessagePtr msg)
			{
				if (msg->get_opcode() == websocketpp::frame::opcode::TEXT ||
					msg->get_opcode() == websocketpp::frame::opcode::BINARY)
				{
					// Signal successful connection on first message (welcome reply)
					bool expected = false;
					if (promiseFulfilled->compare_exchange_strong(expected, true))
					{
						connectionPromise->set_value(true);
					}

					QueueEvent(WebSocketEventType::Message, msg->get_payload());
				}
			});

			m_client.set_close_handler([this, connectionPromise, promiseFulfilled](WebSocketConnectionHandle hdl)
			{
				m_state = WebSocketState::Closed;

				// Signal failed connection if closed before receiving welcome message
				bool expected = false;
				if (promiseFulfilled->compare_exchange_strong(expected, true))
				{
					connectionPromise->set_value(false);
				}

				QueueEvent(WebSocketEventType::Close, "");
			});

			m_client.set_fail_handler([this, connectionPromise, promiseFulfilled](WebSocketConnectionHandle hdl)
			{
				m_state = WebSocketState::Error;

				std::string errorMsg = "Connection failed";
				try
				{
					auto con = m_client.get_con_from_hdl(hdl);
					if (con)
					{
						errorMsg = con->get_ec().message();
					}
				}
				catch (...)
				{
				}

				// Signal failed connection (only once)
				bool expected = false;
				if (promiseFulfilled->compare_exchange_strong(expected, true))
				{
					connectionPromise->set_value(false);
				}

				QueueEvent(WebSocketEventType::Error, errorMsg);
			});

			// Create connection
			websocketpp::lib::error_code ec;
			auto connection = m_client.get_connection(m_url, ec);

			if (ec)
			{
				m_state = WebSocketState::Error;
				return false;
			}

			m_client.connect(connection);

			// Start the ASIO io_service run loop in a separate thread
			m_clientThread = std::thread([this, connectionPromise, promiseFulfilled]()
			{
				try
				{
					m_client.run();
				}
				catch (const std::exception& e)
				{
					m_state = WebSocketState::Error;

					// Signal failed connection if exception occurs (only once)
					bool expected = false;
					if (promiseFulfilled->compare_exchange_strong(expected, true))
					{
						connectionPromise->set_value(false);
					}

					QueueEvent(WebSocketEventType::Error, e.what());
				}
			});

			// Wait for connection result with a timeout
			auto status = connectionFuture.wait_for(std::chrono::seconds(kConnectionTimeoutSeconds));

			if (status == std::future_status::timeout)
			{
				// Connection timed out - clean up and return failure
				// Mark as fulfilled to prevent promise being set after we return
				bool expected = false;
				promiseFulfilled->compare_exchange_strong(expected, true);

				m_state = WebSocketState::Error;

				// Stop the ASIO client to clean up the thread
				try
				{
					m_client.stop();
					if (m_clientThread.joinable())
					{
						m_clientThread.join();
					}
				}
				catch (...)
				{
				}

				m_isRunning = false;
				return false;
			}

			return connectionFuture.get();
		}
		catch (const std::exception& e)
		{
			m_state = WebSocketState::Error;
			return false;
		}
	}

	void Close()
	{
		if (!m_isRunning)
		{
			return;
		}

		m_isRunning = false;

		try
		{
			if (m_state == WebSocketState::Open)
			{
				m_state = WebSocketState::Closing;

				websocketpp::lib::error_code ec;
				m_client.close(m_connectionHandle, websocketpp::close::status::normal, "Closing", ec);
			}

			m_client.stop();

			if (m_clientThread.joinable())
			{
				m_clientThread.join();
			}
		}
		catch (...)
		{
		}

		m_state = WebSocketState::Closed;
	}

	bool Send(const std::string& data)
	{
		if (m_state != WebSocketState::Open)
		{
			return false;
		}

		try
		{
			websocketpp::lib::error_code ec;
			m_client.send(m_connectionHandle, data, websocketpp::frame::opcode::TEXT, ec);
			return !ec;
		}
		catch (...)
		{
			return false;
		}
	}

	void AddEventHandler(WebSocketEventType type, const WebSocketEventHandler& handler)
	{
		std::lock_guard<std::mutex> lock(m_handlersMutex);
		m_eventHandlers[type].push_back(handler);
	}

	void InvokeEventHandlers(WebSocketEventType type, const std::string& data)
	{
		if (m_onEvent)
		{
			try
			{
				m_onEvent(WebSocketEventTypeToString(type), data);
			}
			catch (...)
			{
			}
		}

		std::lock_guard<std::mutex> lock(m_handlersMutex);
		auto it = m_eventHandlers.find(type);
		if (it != m_eventHandlers.end())
		{
			for (const auto& handler : it->second)
			{
				try
				{
					handler(data);
				}
				catch (...)
				{
				}
			}
		}
	}

	int GetId() const { return m_id; }
	const std::string& GetUrl() const { return m_url; }
	WebSocketState GetState() const { return m_state; }
	Resource* GetResource() const { return m_resource; }

private:
	void QueueEvent(WebSocketEventType type, const std::string& data)
	{
		WebSocketEvent event;
		event.socketId = m_id;
		event.type = type;
		event.data = data;
		m_manager->QueueEvent(event);
	}

	int m_id;
	std::string m_url;
	WebSocketState m_state;
	WebSocketManager* m_manager;
	Resource* m_resource;

	WebSocketClient m_client;
	WebSocketConnectionHandle m_connectionHandle;
	std::thread m_clientThread;
	std::atomic<bool> m_isRunning;
	std::atomic<bool> m_hasStarted;

	WebSocketEventCallback m_onEvent;
	std::unordered_map<WebSocketEventType, std::vector<WebSocketEventHandler>> m_eventHandlers;
	std::mutex m_handlersMutex;
};

std::unique_ptr<WebSocketManager> WebSocketManager::s_instance;
std::mutex WebSocketManager::s_instanceMutex;

WebSocketManager::WebSocketManager()
	: m_nextSocketId(1)
{
}

WebSocketManager::~WebSocketManager()
{
	std::lock_guard<std::mutex> lock(m_connectionsMutex);
	m_connections.clear();
}

WebSocketManager* WebSocketManager::GetInstance()
{
	std::lock_guard<std::mutex> lock(s_instanceMutex);
	if (!s_instance)
	{
		s_instance = std::make_unique<WebSocketManager>();
	}
	return s_instance.get();
}

static bool IsValidWebSocketUrl(const std::string& url)
{
	if (url.empty())
	{
		return false;
	}

	if (url.length() < 6)
	{
		return false;
	}

	// Check for valid scheme (ws:// or wss://)
	bool isWs = (url.length() >= 5 && url.compare(0, 5, "ws://") == 0);
	bool isWss = (url.length() >= 6 && url.compare(0, 6, "wss://") == 0);

	if (!isWs && !isWss)
	{
		return false;
	}

	size_t schemeLen = isWss ? 6 : 5;
	if (url.length() <= schemeLen)
	{
		return false;
	}

	char firstHostChar = url[schemeLen];
	if (firstHostChar == '/' || firstHostChar == ':')
	{
		return false;
	}

	static const std::string allowedChars =
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"0123456789"
		".-_:/?&=#%+@[]";

	for (char c : url)
	{
		if (allowedChars.find(c) == std::string::npos)
		{
			return false;
		}
	}

	return true;
}

int WebSocketManager::Connect(const std::string& url, const WebSocketEventCallback& onEvent, Resource* resource)
{
	if (!IsValidWebSocketUrl(url))
	{
		return -1;
	}

	int socketId = m_nextSocketId++;

	auto connection = std::make_unique<WebSocketConnection>(socketId, url, this, resource);

	if (!connection->Start(onEvent))
	{
		return -1;
	}

	{
		std::lock_guard<std::mutex> lock(m_connectionsMutex);
		m_connections[socketId] = std::move(connection);
	}

	return socketId;
}

bool WebSocketManager::Disconnect(int socketId)
{
	std::lock_guard<std::mutex> lock(m_connectionsMutex);

	auto it = m_connections.find(socketId);
	if (it == m_connections.end())
	{
		return false;
	}

	it->second->Close();
	m_connections.erase(it);
	return true;
}

std::vector<WebSocketConnectionInfo> WebSocketManager::ListConnections()
{
	std::vector<WebSocketConnectionInfo> result;

	std::lock_guard<std::mutex> lock(m_connectionsMutex);
	for (const auto& pair : m_connections)
	{
		WebSocketConnectionInfo info;
		info.id = pair.second->GetId();
		info.url = pair.second->GetUrl();
		info.state = WebSocketStateToString(pair.second->GetState());
		result.push_back(info);
	}

	return result;
}

bool WebSocketManager::RegisterEventHandler(int socketId, WebSocketEventType eventType, const WebSocketEventHandler& handler)
{
	std::lock_guard<std::mutex> lock(m_connectionsMutex);

	auto it = m_connections.find(socketId);
	if (it == m_connections.end())
	{
		return false;
	}

	it->second->AddEventHandler(eventType, handler);
	return true;
}

bool WebSocketManager::Send(int socketId, const std::string& data)
{
	std::lock_guard<std::mutex> lock(m_connectionsMutex);

	auto it = m_connections.find(socketId);
	if (it == m_connections.end())
	{
		return false;
	}

	return it->second->Send(data);
}

void WebSocketManager::QueueEvent(const WebSocketEvent& event)
{
	m_eventQueue.push(event);
}

void WebSocketManager::ProcessEvents()
{
	WebSocketEvent event;
	while (m_eventQueue.try_pop(event))
	{
		std::lock_guard<std::mutex> lock(m_connectionsMutex);

		auto it = m_connections.find(event.socketId);
		if (it != m_connections.end())
		{
			it->second->InvokeEventHandlers(event.type, event.data);
		}
	}
}

void WebSocketManager::CleanupResource(Resource* resource)
{
	std::lock_guard<std::mutex> lock(m_connectionsMutex);

	for (auto it = m_connections.begin(); it != m_connections.end();)
	{
		if (it->second->GetResource() == resource)
		{
			it->second->Close();
			it = m_connections.erase(it);
		}
		else
		{
			++it;
		}
	}
}

}
