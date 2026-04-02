/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIApp.h"
#include "memdbgon.h"

class PollCallbacks
{
private:
	using TCallbackList = std::unordered_map<int, std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value>>>;

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

				CefV8ValueList arguments;
				arguments.push_back(CefV8Value::CreateString(message->GetArgumentList()->GetString(0)));

                callback->ExecuteFunctionWithContext(context, nullptr, arguments);
			}

			return true;
		});

		nuiApp->AddV8Handler("registerPollFunction", [=] (const CefV8ValueList& arguments, CefString& exception)
		{
			if (arguments.size() == 1 && arguments[0]->IsFunction())
			{
				auto context = CefV8Context::GetCurrentContext();
				m_callbacks.try_emplace(context->GetBrowser()->GetIdentifier(), std::make_pair(context, arguments[0]));
			}

			return CefV8Value::CreateNull();
		});

		nuiApp->AddContextReleaseHandler([&](CefRefPtr<CefV8Context> context)
		{
			for (auto it = m_callbacks.begin(); it != m_callbacks.end();)
			{
				if (it->second.first.get() == context.get())
				{
					it = m_callbacks.erase(it);
				}
				else
				{
					++it;
				}
			}
		});
	}
};

static PollCallbacks g_pollCallbacks;

static InitFunction initFunction([] ()
{
	g_pollCallbacks.Initialize();
}, 1);
