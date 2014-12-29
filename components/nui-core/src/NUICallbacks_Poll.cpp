/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIApp.h"

class PollCallbacks
{
private:
	typedef std::map<int, std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value>>> TCallbackList;

	TCallbackList m_callbacks;

public:
	void Initialize()
	{
		auto nuiApp = Instance<NUIApp>::Get();

		nuiApp->AddProcessMessageHandler("doPoll", [=] (CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> message)
		{
			auto it = m_callbacks.find(browser->GetIdentifier());

			if (it != m_callbacks.end())
			{
				auto context = it->second.first;
				auto callback = it->second.second;

				context->Enter();

				CefV8ValueList arguments;
				arguments.push_back(CefV8Value::CreateString(message->GetArgumentList()->GetString(0)));

				callback->ExecuteFunction(nullptr, arguments);

				context->Exit();
			}

			return true;
		});

		nuiApp->AddV8Handler("registerPollFunction", [=] (const CefV8ValueList& arguments, CefString& exception)
		{
			if (arguments.size() == 1 && arguments[0]->IsFunction())
			{
				auto context = CefV8Context::GetCurrentContext();

				m_callbacks[context->GetBrowser()->GetIdentifier()] = std::make_pair(context, arguments[0]);
			}

			return CefV8Value::CreateNull();
		});
	}
};

static PollCallbacks g_pollCallbacks;

static InitFunction initFunction([] ()
{
	g_pollCallbacks.Initialize();
}, 1);