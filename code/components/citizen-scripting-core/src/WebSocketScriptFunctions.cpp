/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#ifndef IS_FXSERVER

#include <ScriptEngine.h>

#include <Resource.h>
#include <ResourceManager.h>
#include <fxScripting.h>

#include <ResourceCallbackComponent.h>
#include <ScriptSerialization.h>
#include <SharedFunction.h>

#include "WebSocketManager.h"

static InitFunction initFunction([]()
{
	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* resman)
	{
		resman->OnTick.Connect([]()
		{
			fx::WebSocketManager::GetInstance()->ProcessEvents();
		});
	});

	fx::ScriptEngine::RegisterNativeHandler("SOCKET_CONNECT", [](fx::ScriptContext& context)
	{
		const char* url = context.CheckArgument<const char*>(0);
		auto cbRef = fx::FunctionRef{ context.CheckArgument<const char*>(1) };

		fx::OMPtr<IScriptRuntime> runtime;

		if (!FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			context.SetResult(-1);
			return;
		}

		fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

		if (!resource)
		{
			context.SetResult(-1);
			return;
		}

		auto resourceManager = resource->GetManager();
		std::string urlStr = url;

		fx::WebSocketEventCallback eventCallback = make_shared_function([resourceManager, cbRef = std::move(cbRef)](const std::string& eventType, const std::string& data) mutable
		{
			try
			{
				resourceManager->CallReference<void>(cbRef.GetRef(), eventType, data);
			}
			catch (const std::exception& e)
			{
				trace("WebSocket event callback error: %s\n", e.what());
			}
		});

		int socketId = fx::WebSocketManager::GetInstance()->Connect(urlStr, eventCallback, resource);

		if (socketId > 0)
		{
			// Hook into resource stop to cleanup connections
			resource->OnStop.Connect([resource]()
			{
				fx::WebSocketManager::GetInstance()->CleanupResource(resource);
			}, INT32_MAX);
		}

		context.SetResult(socketId);
	});

	fx::ScriptEngine::RegisterNativeHandler("SOCKET_DISCONNECT", [](fx::ScriptContext& context)
	{
		int socketId = context.GetArgument<int>(0);

		bool success = fx::WebSocketManager::GetInstance()->Disconnect(socketId);

		context.SetResult(success);
	});

	fx::ScriptEngine::RegisterNativeHandler("SOCKET_LIST_CONNECTIONS", [](fx::ScriptContext& context)
	{
		auto connections = fx::WebSocketManager::GetInstance()->ListConnections();

		context.SetResult(fx::SerializeObject(connections));
	});

	fx::ScriptEngine::RegisterNativeHandler("SOCKET_ON", [](fx::ScriptContext& context)
	{
		int socketId = context.GetArgument<int>(0);
		const char* eventType = context.CheckArgument<const char*>(1);
		auto cbRef = fx::FunctionRef{ context.CheckArgument<const char*>(2) };

		fx::OMPtr<IScriptRuntime> runtime;

		if (!FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			context.SetResult(false);
			return;
		}

		fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

		if (!resource)
		{
			context.SetResult(false);
			return;
		}

		auto resourceManager = resource->GetManager();

		fx::WebSocketEventType wsEventType;
		if (!fx::WebSocketEventTypeFromString(eventType, wsEventType))
		{
			trace("WebSocket: Unknown event type '%s'. Valid types are: open, message, close, error\n", eventType);
			context.SetResult(false);
			return;
		}

		fx::WebSocketEventHandler handler = make_shared_function([resourceManager, cbRef = std::move(cbRef)](const std::string& data) mutable
		{
			try
			{
				resourceManager->CallReference<void>(cbRef.GetRef(), data);
			}
			catch (const std::exception& e)
			{
				trace("WebSocket event handler error: %s\n", e.what());
			}
		});

		bool success = fx::WebSocketManager::GetInstance()->RegisterEventHandler(socketId, wsEventType, handler);

		context.SetResult(success);
	});

	fx::ScriptEngine::RegisterNativeHandler("SOCKET_SEND", [](fx::ScriptContext& context)
	{
		int socketId = context.GetArgument<int>(0);
		const char* data = context.CheckArgument<const char*>(1);

		bool success = fx::WebSocketManager::GetInstance()->Send(socketId, data);

		context.SetResult(success);
	});
});

#endif
