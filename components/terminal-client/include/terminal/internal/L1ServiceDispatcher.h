/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <terminal/internal/Dispatcher.h>

#include <terminal/internal/MessageDefinition.h>

#include <concurrent_unordered_map.h>

#include <mutex>

namespace terminal
{
class ServiceMessageData
{
private:
	std::vector<uint8_t> m_bytes;

public:
	ServiceMessageData();

	ServiceMessageData(const std::vector<uint8_t>& bytes);

	inline const std::vector<uint8_t>& GetBytes() const
	{
		return m_bytes;
	}

	template<typename T>
	typename T::ProtoPtr GetMessage() const
	{
		auto pointer = T::Create();
		pointer->ParseFromArray(&m_bytes[0], m_bytes.size());

		return pointer;
	}
};

class L1ServiceDispatcher : public Dispatcher
{
private:
	std::mutex m_completionEventMutex;

	std::unordered_map<uint32_t, concurrency::task_completion_event<ServiceMessageData>> m_completionEvents;

	concurrency::concurrent_unordered_multimap<uint32_t, std::function<void(const ServiceMessageData&)>> m_typeCallbacks;

public:
	virtual void DispatchMessage(uint32_t messageType, uint32_t messageID, const std::vector<uint8_t>& messageData) override;

private:
	void SetTypeCallbackInternal(uint32_t messageType, const std::function<void(const ServiceMessageData&)>& callback);

	void SetCompletionEventInternal(uint32_t messageID, concurrency::task_completion_event<ServiceMessageData> event);

public:
	template<typename T>
	void SetTypeCallback(const std::function<void(typename T::ProtoPtr)>& callback)
	{
		SetTypeCallbackInternal(T::MessageType, [=] (const ServiceMessageData& data)
		{
			callback(data.GetMessage<T>());
		});
	}

	template<typename T>
	void SetCompletionEvent(uint32_t messageID, concurrency::task_completion_event<typename T::ProtoPtr>& event)
	{
		concurrency::task_completion_event<ServiceMessageData> intEvent;
		SetCompletionEventInternal(messageID, intEvent);

		concurrency::task<ServiceMessageData> task(intEvent);
		task.then([=] (const ServiceMessageData& data)
		{
			event.set(data.GetMessage<T>());
		});
	}
};
}