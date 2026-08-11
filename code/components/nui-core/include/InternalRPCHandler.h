/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <functional>

namespace nui
{
class RPCHandlerManager
{
public:
	typedef std::function<void(const std::string& result)> TCallbackFn;
	typedef std::function<void(std::string functionName, std::string additionalPath, std::map<std::string, std::string> postFields, TCallbackFn cb)> TEndpointFn;

public:
	virtual void RegisterEndpoint(std::string endpointName, TEndpointFn callback) = 0;
};
}

DECLARE_INSTANCE_TYPE(nui::RPCHandlerManager);