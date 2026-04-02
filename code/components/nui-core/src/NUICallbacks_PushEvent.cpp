#include "StdInc.h"
#include "NUIApp.h"
#include "memdbgon.h"

class PushCallbacks
{
private:
	using TCallbackList = std::unordered_map<int, std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value>>>;

	TCallbackList m_pushCallbacks;
public:
	void Initialize()
	{
		auto pushCB = [&](CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> message)
		{
			auto it = m_pushCallbacks.find(browser->GetIdentifier());
			if (it == m_pushCallbacks.end())
			{
				return true;
			}
			auto& [context, callback] = it->second;

			CefV8ValueList arguments;
			auto argList = message->GetArgumentList();
			for (size_t arg = 0; arg < argList->GetSize(); arg++)
			{
				arguments.push_back(CefV8Value::CreateString(argList->GetString(arg)));
			}
			callback->ExecuteFunctionWithContext(context, nullptr, arguments);

			return true;
		};

		auto nuiApp = Instance<NUIApp>::Get();
		nuiApp->AddProcessMessageHandler("pushEvent", pushCB);

		nuiApp->AddV8Handler("registerPushFunction", [&](const CefV8ValueList& arguments, CefString& exception)
		{
			if (arguments.size() == 1 && arguments[0]->IsFunction())
			{
				auto context = CefV8Context::GetCurrentContext();
				m_pushCallbacks.insert_or_assign(context->GetBrowser()->GetIdentifier(), std::make_pair(context, arguments[0]));
			}

			return CefV8Value::CreateNull();
		});

		nuiApp->AddContextReleaseHandler([&](CefRefPtr<CefV8Context> context)
		{
			for (auto it = m_pushCallbacks.begin(); it != m_pushCallbacks.end(); )
			{
				if (it->second.first.get() == context.get())
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
