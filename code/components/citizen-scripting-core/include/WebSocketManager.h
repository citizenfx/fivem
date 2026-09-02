/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <vector>

#include <msgpack.hpp>

#include <tbb/concurrent_queue.h>

namespace fx
{
class Resource;

enum class WebSocketState
{
	Connecting,
	Open,
	Closing,
	Closed,
	Error
};

inline const char* WebSocketStateToString(WebSocketState state)
{
	switch (state)
	{
		case WebSocketState::Connecting: return "connecting";
		case WebSocketState::Open: return "open";
		case WebSocketState::Closing: return "closing";
		case WebSocketState::Closed: return "closed";
		case WebSocketState::Error: return "error";
		default: return "unknown";
	}
}

enum class WebSocketEventType
{
	Open,
	Message,
	Close,
	Error
};

inline const char* WebSocketEventTypeToString(WebSocketEventType type)
{
	switch (type)
	{
		case WebSocketEventType::Open: return "open";
		case WebSocketEventType::Message: return "message";
		case WebSocketEventType::Close: return "close";
		case WebSocketEventType::Error: return "error";
		default: return "unknown";
	}
}

inline bool WebSocketEventTypeFromString(const std::string& type, WebSocketEventType& outType)
{
	if (type == "open") { outType = WebSocketEventType::Open; return true; }
	if (type == "message") { outType = WebSocketEventType::Message; return true; }
	if (type == "close") { outType = WebSocketEventType::Close; return true; }
	if (type == "error") { outType = WebSocketEventType::Error; return true; }
	outType = WebSocketEventType::Error;
	return false;
}

struct WebSocketConnectionInfo
{
	int id;
	std::string url;
	std::string state;

	MSGPACK_DEFINE_MAP(id, url, state);
};

using WebSocketEventCallback = std::function<void(const std::string& eventType, const std::string& data)>;

using WebSocketEventHandler = std::function<void(const std::string& data)>;

struct WebSocketEvent
{
	int socketId;
	WebSocketEventType type;
	std::string data;
};

class WebSocketManager
{
public:
	WebSocketManager();
	~WebSocketManager();

	int Connect(const std::string& url, const WebSocketEventCallback& onEvent, Resource* resource);

	bool Disconnect(int socketId);

	std::vector<WebSocketConnectionInfo> ListConnections();

	bool RegisterEventHandler(int socketId, WebSocketEventType eventType, const WebSocketEventHandler& handler);

	bool Send(int socketId, const std::string& data);

	void ProcessEvents();

	void CleanupResource(Resource* resource);

	static WebSocketManager* GetInstance();

private:
	class WebSocketConnection;

	void QueueEvent(const WebSocketEvent& event);

	std::unordered_map<int, std::unique_ptr<WebSocketConnection>> m_connections;
	std::mutex m_connectionsMutex;

	tbb::concurrent_queue<WebSocketEvent> m_eventQueue;

	std::atomic<int> m_nextSocketId;

	static std::unique_ptr<WebSocketManager> s_instance;
	static std::mutex s_instanceMutex;
};

}
