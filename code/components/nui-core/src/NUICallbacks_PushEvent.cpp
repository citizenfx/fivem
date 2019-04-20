#include "StdInc.h"
#include "NUIApp.h"
#include "memdbgon.h"

class PushCallbacks
{
private:
	using TCallbackList = std::map<int, std::pair<CefV8Context*, CefV8Value*>>;

	TCallbackList m_pushCallbacks;

public:
	void Initialize()
	{
		auto pushCB = [=](CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> message)
		{
			auto it = m_pushCallbacks.find(browser->GetIdentifier());

			if (it != m_pushCallbacks.end())
			{
				auto context = it->second.first;
				auto callback = it->second.second;

				CefV8ValueList arguments;

				auto argList = message->GetArgumentList();

				for (int arg = 0; arg < argList->GetSize(); arg++)
				{
					arguments.push_back(CefV8Value::CreateString(message->GetArgumentList()->GetString(arg)));
				}

				callback->ExecuteFunctionWithContext(context, nullptr, arguments);
			}

			return true;
		};

		auto nuiApp = Instance<NUIApp>::Get();
		nuiApp->AddProcessMessageHandler("pushEvent", pushCB);

		nuiApp->AddV8Handler("registerPushFunction", [=](const CefV8ValueList& arguments, CefString& exception)
		{
			if (arguments.size() == 1 && arguments[0]->IsFunction())
			{
				auto context = CefV8Context::GetCurrentContext();

				context->AddRef();
				arguments[0]->AddRef();

				m_pushCallbacks[context->GetBrowser()->GetIdentifier()] = std::make_pair(context.get(), arguments[0].get());
			}

			return CefV8Value::CreateNull();
		});

		nuiApp->AddContextReleaseHandler([=](CefRefPtr<CefV8Context> context)
		{
			for (auto it = m_pushCallbacks.begin(); it != m_pushCallbacks.end(); )
			{
				if (it->second.first == context.get())
				{
					it = m_pushCallbacks.erase(it);
				}
				else
				{
					++it;
				}
			}
		});
	}
};

static PushCallbacks g_pushCallbacks;

static InitFunction initFunction([]()
{
	g_pushCallbacks.Initialize();
}, 1);
