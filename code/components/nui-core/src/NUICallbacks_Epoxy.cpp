/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */
#include "StdInc.h"
#include "NUIApp.h"
#include "memdbgon.h"
#include <NUIWindowManager.h>

//  
// Handle private messaging to and from the root UI and epoxy scripts
// We can't use postMessage without risking breaking popular resources that assume that the only messages recieved are relevant to them
// So we handle epoxy message callbacks here in a way that doesn't upset CORS and allows us to maintain backwards compatability.
//

class EpoxyCallbacks
{
private:
	typedef std::unordered_map<std::string, std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value>>> TCallbackList;
	TCallbackList m_messageCallbacks;

	// Queue for any handlers not yet registered
	typedef std::queue<std::pair<std::string, CefRefPtr<CefV8Value>>> TCallbackQueue;
	std::unordered_map<std::string, TCallbackQueue> m_messageQueue;
public:
	void Initialize()
	{
		auto nuiApp = Instance<NUIApp>::Get();

		nuiApp->AddV8Handler("registerEpoxyHandler", [=](const CefV8ValueList& arguments, CefString& exception)
		{
			if (arguments.size() == 1 && arguments[0]->IsFunction())
			{
				auto context = CefV8Context::GetCurrentContext();
				auto frame = context->GetFrame();
				std::string frameName = frame->GetName().ToString();
				
				m_messageCallbacks[frameName] = std::make_pair(context, arguments[0]);

				// Execute any queued calls that were sent before the epoxy script was fully loaded (e.g. setHandoverData).
				if (auto it = m_messageQueue.find(frameName); it != m_messageQueue.end())
				{
					auto& queue = it->second;
					while (!queue.empty())
					{
						auto& [name, value] = queue.front();

						if (context->IsValid())
						{
							CefV8ValueList args;
							args.push_back(CefV8Value::CreateString(name));
							if (value && value->IsObject())
							{
								args.push_back(value);
							}

							arguments[0]->ExecuteFunctionWithContext(context, nullptr, args);
						}
						queue.pop();
					}

					m_messageQueue.erase(frameName);
				}
			}

			return CefV8Value::CreateNull();
		});

		nuiApp->AddV8Handler("sendEpoxyMessage", [=](const CefV8ValueList& arguments, CefString& exception)
		{
			if (arguments.size() < 2 || !arguments[0]->IsString() || !arguments[1]->IsString())
			{
				return CefV8Value::CreateBool(false);
			}

			CefString targetWindow = arguments[0]->GetStringValue();
			CefString messageType = arguments[1]->GetStringValue();

			if (messageType.empty() || targetWindow.empty())
			{
				return CefV8Value::CreateBool(false);
			}

			auto it = m_messageCallbacks.find(targetWindow);
			if (it != m_messageCallbacks.end())
			{
				auto context = it->second.first;
				auto callback = it->second.second;

				if (context->IsValid())
				{
					CefV8ValueList args;
					args.push_back(CefV8Value::CreateString(messageType));
					if (arguments.size() >= 3 && arguments[2]->IsObject())
					{
						args.push_back(arguments[2]);
					}

					callback->ExecuteFunctionWithContext(context, nullptr, args);
					return CefV8Value::CreateBool(true);
				}
				else
				{
					// Target window context is no longer valid, resource was likely restarted since then.
					m_messageCallbacks.erase(it);
					return CefV8Value::CreateBool(false);
				}
			}

			auto& queue = m_messageQueue[targetWindow];
			queue.push(std::make_pair(messageType, (arguments.size() >= 3 && arguments[2]->IsObject()) ? arguments[2] : nullptr));
			// Message is queued and will be executed when epoxy handler is registered.
			return CefV8Value::CreateBool(true);
		});

		nuiApp->AddContextReleaseHandler([=](CefRefPtr<CefV8Context> context)
		{
			for (auto it = m_messageCallbacks.begin(); it != m_messageCallbacks.end();)
			{
				if (it->second.first.get() == context.get())
				{
					it = m_messageCallbacks.erase(it);
				}
				else
				{
					++it;
				}
			}
		});
	}
};

static EpoxyCallbacks g_epoxyCallbacks;

static InitFunction initFunction([]()
{
	g_epoxyCallbacks.Initialize();
},
1);
