/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <terminal/internal/L1ServiceDispatcher.h>

namespace terminal
{
void L1ServiceDispatcher::DispatchMessage(uint32_t messageType, uint32_t messageID, const std::vector<uint8_t>& messageData)
{
	// create a ServiceMessageData instance
	ServiceMessageData data(messageData);

	// generic type callbacks
	auto typeCallbacks = m_typeCallbacks.equal_range(messageType);

	for (auto& it = typeCallbacks.first; it != typeCallbacks.second; it++)
	{
		it->second(data);
	}

	// completion callback
	m_completionEventMutex.lock();
	auto completionCallback = m_completionEvents.find(messageID);

	bool found = (completionCallback != m_completionEvents.end());
	
	m_completionEventMutex.unlock();

	if (found)
	{
		completionCallback->second.set(data);

		// erase it from the list
		m_completionEventMutex.lock();
		m_completionEvents.erase(completionCallback);
		m_completionEventMutex.unlock();
	}
}

void L1ServiceDispatcher::SetCompletionEventInternal(uint32_t messageID, concurrency::task_completion_event<ServiceMessageData> event)
{
	m_completionEventMutex.lock();
	m_completionEvents[messageID] = event;
	m_completionEventMutex.unlock();
}

void L1ServiceDispatcher::SetTypeCallbackInternal(uint32_t messageType, const std::function<void(const ServiceMessageData&)>& callback)
{
	m_typeCallbacks.insert(std::make_pair(messageType, callback));
}

ServiceMessageData::ServiceMessageData()
{

}

ServiceMessageData::ServiceMessageData(const std::vector<uint8_t>& bytes)
	: m_bytes(bytes)
{

}
}