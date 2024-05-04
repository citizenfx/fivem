#pragma once

namespace fx
{
	class ServerEventComponent;

	class ServerEventComponentInstance
	{
	public:
		struct ClientEventData
		{
			std::string eventName;
			std::vector<uint8_t> data;
			std::optional<std::string> targetSrc;

			ClientEventData(std::string event_name, const std::vector<uint8_t>& data,
			                const std::optional<std::string>& target_src)
				: eventName(std::move(event_name)),
				  data(data),
				  targetSrc(target_src)
			{
			}
		};
		
		static inline std::optional<ClientEventData> lastClientEvent;
		
		static ServerEventComponent* Create();
	};
}
