/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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