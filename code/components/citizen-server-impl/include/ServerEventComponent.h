#pragma once

#include <charconv>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include <ComponentHolder.h>

#include <ServerInstanceBase.h>

#include <msgpack.hpp>

namespace fx
{
	inline bool DecodeClientEventTargets(const void* data, size_t dataLen, std::vector<uint32_t>& outNetIds)
	{
		if (!data || dataLen == 0)
		{
			return false;
		}

		static const msgpack::unpack_limit targetLimit(65536, 0, 32, 0, 0, 1);

		try
		{
			msgpack::object_handle handle;
			msgpack::unpack(handle, static_cast<const char*>(data), dataLen, nullptr, nullptr, targetLimit);

			const msgpack::object& root = handle.get();

			if (root.type != msgpack::type::ARRAY)
			{
				return false;
			}

			const msgpack::object_array& targets = root.via.array;

			outNetIds.clear();
			outNetIds.reserve(targets.size);

			for (uint32_t i = 0; i < targets.size; ++i)
			{
				const msgpack::object& element = targets.ptr[i];
				uint64_t netId;

				if (element.type == msgpack::type::POSITIVE_INTEGER)
				{
					netId = element.via.u64;
				}
				else if (element.type == msgpack::type::NEGATIVE_INTEGER)
				{
					// Ignore -1 in this context
					if (element.via.i64 == -1)
					{
						continue;
					}

					return false;
				}
				else if (element.type == msgpack::type::STR)
				{
					const char* begin = element.via.str.ptr;
					const char* end = begin + element.via.str.size;

					if (std::string_view(begin, element.via.str.size) == "-1")
					{
						continue;
					}

					const std::from_chars_result result = std::from_chars(begin, end, netId);

					if (result.ec != std::errc{} || result.ptr != end)
					{
						return false;
					}
				}
				else
				{
					return false;
				}

				if (netId > UINT32_MAX)
				{
					return false;
				}

				outNetIds.push_back(static_cast<uint32_t>(netId));
			}

			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

	class ServerEventComponent : public fwRefCountable, public IAttached<ServerInstanceBase>
	{
	public:
		virtual void TriggerClientEvent(const std::string_view& eventName, const void* data, size_t dataLen, const std::optional<std::string_view>& targetSrc = std::optional<std::string_view>());

		void TriggerMulticastClientEvent(const std::string_view& eventName, const void* data, size_t dataLen, const std::vector<uint32_t>& targetNetIds);

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
