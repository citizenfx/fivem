/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIApp.h"
#include "memdbgon.h"

class FrameCallbacks
{
private:
	typedef std::map<int, std::pair<CefV8Context*, CefV8Value*>> TCallbackList;

	TCallbackList m_frameCallbacks;

public:
	void Initialize()
	{
		auto frameCB = [=] (CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> message)
		{
			auto it = m_frameCallbacks.find(browser->GetIdentifier());

			if (it != m_frameCallbacks.end())
			{
				auto context = it->second.first;
				auto callback = it->second.second;

				context->Enter();

				CefV8ValueList arguments;
				arguments.push_back(CefV8Value::CreateString(message->GetName()));
				arguments.push_back(CefV8Value::CreateString(message->GetArgumentList()->GetString(0)));

				if (message->GetName() == "createFrame")
				{
					arguments.push_back(CefV8Value::CreateString(message->GetArgumentList()->GetString(1)));
				}

				callback->ExecuteFunction(nullptr, arguments);

				context->Exit();
			}

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

				context->AddRef();
				arguments[0]->AddRef();

				m_frameCallbacks[context->GetBrowser()->GetIdentifier()] = std::make_pair(context.get(), arguments[0].get());
			}

			return CefV8Value::CreateNull();
		});
	}
};

static FrameCallbacks g_frameCallbacks;

static InitFunction initFunction([] ()
{
	g_frameCallbacks.Initialize();
}, 1);