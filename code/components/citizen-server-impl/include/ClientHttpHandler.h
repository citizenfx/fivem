#pragma once

#include <HttpServer.h>
#include <optional>
#include <variant>

#include <json.hpp>

#include <rapidjson/document.h>

using json = nlohmann::json;

namespace fx
{
	class ClientMethodRegistry : public fwRefCountable
	{
	public:
		using TCallback = std::function<void(const json&)>;

		using TCallbackFast = std::function<void(const rapidjson::Document&)>;

		template<typename TCb>
		using THandler = std::function<void(const std::map<std::string, std::string>&, const fwRefContainer<net::HttpRequest>&, const TCb&)>;

		template<typename TJson, typename TCb>
		using TFilter = std::function<void(const TJson&, const std::map<std::string, std::string>&, const fwRefContainer<net::HttpRequest>&, const TCb&)>;

		virtual std::optional<std::variant<THandler<TCallback>, THandler<TCallbackFast>>> GetHandler(const std::string& method);

		template<typename TCb>
		void AddHandlerInt(const std::string& method, const THandler<TCb>& handler)
		{
			m_methods.emplace(method, handler);
		}

		template<typename TJson, typename TCb>
		void AddAfterFilterInt(const std::string& method, const TFilter<TJson, TCb>& handler)
		{
			auto it = m_methods.find(method);

			assert(it != m_methods.end());

			auto lastValue = std::get<THandler<TCb>>(it->second);

			it->second = [=](const std::map<std::string, std::string>& a1, const fwRefContainer<net::HttpRequest>& a2, const TCb& a3)
			{
				lastValue(a1, a2, [=](const TJson& data)
				{
					handler(data, a1, a2, a3);
				});
			};
		}

		inline void AddHandler(const std::string& method, const THandler<TCallback>& handler)
		{
			AddHandlerInt(method, handler);
		}

		inline void AddHandler(const std::string& method, const THandler<TCallbackFast>& handler)
		{
			AddHandlerInt(method, handler);
		}

		inline void AddAfterFilter(const std::string& method, const TFilter<json, TCallback>& handler)
		{
			AddAfterFilterInt(method, handler);
		}

		inline void AddAfterFilter(const std::string& method, const TFilter<rapidjson::Document, TCallbackFast>& handler)
		{
			AddAfterFilterInt(method, handler);
		}

	private:
		std::map<std::string, std::variant<THandler<TCallback>, THandler<TCallbackFast>>> m_methods;
	};
}

DECLARE_INSTANCE_TYPE(fx::ClientMethodRegistry);
