/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <HttpServer.h>

#include <boost/optional.hpp>

using TGameServiceHandler = std::function<std::string(const std::string&)>;

class EndpointMapper : public net::HttpHandler
{
private:
	std::map<std::string, fwRefContainer<net::HttpHandler>> m_prefixes;

	std::map<std::string, TGameServiceHandler> m_gameServices;

public:
	bool HandleRequest(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response) override;

	void AddPrefix(const std::string& routeOrigin, fwRefContainer<net::HttpHandler> handler);

	void AddGameService(const std::string& serviceName, const TGameServiceHandler& handler);

	boost::optional<TGameServiceHandler> GetGameServiceHandler(const std::string& serviceName);
};

DECLARE_INSTANCE_TYPE(EndpointMapper);