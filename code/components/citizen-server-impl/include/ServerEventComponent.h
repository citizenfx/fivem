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
	private:
		struct pass
		{
			template<typename ...T> pass(T...) {}
		};

	public:
		virtual void TriggerClientEvent(const std::string_view& eventName, const void* data, size_t dataLen, const std::optional<std::string_view>& targetSrc = std::optional<std::string_view>(), bool replayed = false);

		inline virtual void AttachToObject(ServerInstanceBase* object) override
		{
			m_instance = object;
		}

		template<typename... TArg>
		inline void TriggerClientEvent(const std::string_view& eventName, const std::optional<std::string_view>& targetSrc, const TArg&... args)
		{
			return TriggerClientEventInternal<false>(eventName, targetSrc, args...);
		}

		template<typename... TArg>
		inline void TriggerClientEventReplayed(const std::string_view& eventName, const std::optional<std::string_view>& targetSrc, const TArg&... args)
		{
			return TriggerClientEventInternal<true>(eventName, targetSrc, args...);
		}

	private:
		template<bool Replayed, typename... TArg>
		inline void TriggerClientEventInternal(const std::string_view& eventName, const std::optional<std::string_view>& targetSrc, const TArg&... args)
		{
			msgpack::sbuffer buf;
			msgpack::packer<msgpack::sbuffer> packer(buf);

			// pack the argument pack as array
			packer.pack_array(sizeof...(args));
			pass{ (packer.pack(args), 0)... };

			TriggerClientEvent(eventName, buf.data(), buf.size(), targetSrc, Replayed);
		}

	private:
		ServerInstanceBase* m_instance;
	};
}

DECLARE_INSTANCE_TYPE(fx::ServerEventComponent);
