/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <terminal/internal/Dispatcher.h>

namespace terminal
{
class ServiceMessageData
{
private:
	std::vector<uint8_t> m_bytes;

public:
	ServiceMessageData(const std::vector<uint8_t>& bytes);

	inline const std::vector<uint8_t>& GetBytes() const
	{
		return m_bytes;
	}

	template<typename T>
	T GetMessage()
	{

	}
};

class L1ServiceDispatcher : public Dispatcher
{
public:
	virtual void DispatchMessage(uint32_t messageType, uint32_t messageID, const std::vector<uint8_t>& messageData) override;

	void SetTypeCallback(uint32_t messageID, const std::function<void(const ServiceMessageData&)>& callback);

	void SetCompletionEvent(uint32_t messageID, concurrency::task_completion_event<ServiceMessageData>& event);
};
}