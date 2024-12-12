#pragma once

#include <optional>
#include <string_view>

#include <ComponentHolder.h>

#include <ServerInstanceBase.h>

#include <msgpack.hpp>

namespace fx
{
	class ServerEventComponent : public fwRefCountable, public IAttached<ServerInstanceBase>
	{
	public:
		virtual void TriggerClientEvent(const std::string_view& eventName, const void* data, size_t dataLen, const std::optional<std::string_view>& targetSrc = std::optional<std::string_view>(), bool reliable = true);

		inline virtual void AttachToObject(ServerInstanceBase* object) override
		{
			m_instance = object;
		}

		template<typename... TArg>
		inline void TriggerClientEvent(const std::string_view& eventName, const std::optional<std::string_view>& targetSrc, const TArg&... args)
		{
			return TriggerClientEventInternal(eventName, targetSrc, args...);
		}

	private:
		template<typename... TArg>
		inline void TriggerClientEventInternal(const std::string_view& eventName, const std::optional<std::string_view>& targetSrc, const TArg&... args)
		{
			msgpack::sbuffer buf;
			msgpack::packer<msgpack::sbuffer> packer(buf);

			// pack the argument pack as array
			packer.pack_array(sizeof...(args));
			(packer.pack(args), ...);

			TriggerClientEvent(eventName, buf.data(), buf.size(), targetSrc);
		}

	private:
		ServerInstanceBase* m_instance;
	};
}

DECLARE_INSTANCE_TYPE(fx::ServerEventComponent);
