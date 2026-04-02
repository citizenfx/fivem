/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIApp.h"
#include "memdbgon.h"

class FrameCallbacks
{
private:
	using TCallbackList = std::unordered_map<int, std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value>>>;

	TCallbackList m_frameCallbacks;
public:
	void Initialize()
	{
		auto frameCB = [&](CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> message)
		{
			auto it = m_frameCallbacks.find(browser->GetIdentifier());
			if (it == m_frameCallbacks.end())
			{
				return true;
			}
			auto& [context, callback] = it->second;

			const auto& name = message->GetName();
			CefV8ValueList arguments;
			arguments.push_back(CefV8Value::CreateString(name));
			arguments.push_back(CefV8Value::CreateString(message->GetArgumentList()->GetString(0)));

			if (name == "createFrame")
			{
				arguments.push_back(CefV8Value::CreateString(message->GetArgumentList()->GetString(1)));
			}

			callback->ExecuteFunctionWithContext(context, nullptr, arguments);

			return true;
		};

		auto nuiApp = Instance<NUIApp>::Get();
		nuiApp->AddProcessMessageHandler("createFrame", frameCB);
		nuiApp->AddProcessMessageHandler("destroyFrame", frameCB);

		nuiApp->AddV8Handler("registerFrameFunction", [=] (const CefV8ValueList& arguments, CefString& exception)
		{
			if (arguments.size() == 1 && arguments[0]->IsFunction())
			{
				auto context = CefV8Context::GetCurrentContext();
				m_frameCallbacks.try_emplace(context->GetBrowser()->GetIdentifier(), context, arguments[0]);
			}

			return CefV8Value::CreateNull();
		});

		nuiApp->AddContextReleaseHandler([=](CefRefPtr<CefV8Context> context)
		{
			for (auto it = m_frameCallbacks.begin(); it != m_frameCallbacks.end(); )
			{
				if (it->second.first.get() == context.get())
				{
					it = m_frameCallbacks.erase(it);
				}
				else
				{
					++it;
				}
			}
		});
	}
};

static FrameCallbacks g_frameCallbacks;

static InitFunction initFunction([] ()
{
	g_frameCallbacks.Initialize();
}, 1);
