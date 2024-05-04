#include <StdInc.h>

#include "ServerEventComponent.h"

#include "ServerEventComponentInstance.h"

void fx::ServerEventComponent::TriggerClientEvent(const std::string_view& eventName, const void* data, size_t dataLen,
                                                  const std::optional<std::string_view>& targetSrc)
{
	std::optional<std::string> targetSourceStr;
	if (auto target = targetSrc)
	{
		targetSourceStr.emplace(target.value());
	}
	ServerEventComponentInstance::lastClientEvent.emplace(std::string(eventName),
	                                                      std::vector(static_cast<const uint8_t*>(data),
	                                                                  static_cast<const uint8_t*>(data) + dataLen),
	                                                      targetSourceStr);
}

fx::ServerEventComponent* fx::ServerEventComponentInstance::Create()
{
	return new ServerEventComponent();
}
