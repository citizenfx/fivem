#pragma once

#include <HttpServer.h>
#include <optional>

#include <json.hpp>

using json = nlohmann::json;

namespace fx
{
	class ClientMethodRegistry : public fwRefCountable
	{
	public:
		using TCallback = std::function<void(const json&)>;

		using THandler = std::function<void(std::map<std::string, std::string>&, const fwRefContainer<net::HttpRequest>&, const TCallback&)>;

		std::optional<THandler> GetHandler(const std::string& method);

		void AddHandler(const std::string& method, const THandler& handler);

	private:
		std::map<std::string, THandler> m_methods;
	};
}

DECLARE_INSTANCE_TYPE(fx::ClientMethodRegistry);
