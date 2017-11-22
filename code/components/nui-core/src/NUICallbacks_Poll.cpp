/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include "NUIApp.h"
#include "memdbgon.h"

class PollCallbacks
{
private:
	typedef std::map<int, std::pair<CefV8Context*, CefV8Value*>> TCallbackList;

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

				// manually add a reference so we don't release the CEF handle (workaround for process exit crash)
				context->AddRef();
				arguments[0]->AddRef();

				m_callbacks[context->GetBrowser()->GetIdentifier()] = std::make_pair(context.get(), arguments[0].get());
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