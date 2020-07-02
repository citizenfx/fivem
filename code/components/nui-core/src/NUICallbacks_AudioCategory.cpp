#include "StdInc.h"

#include "NUIApp.h"
#include "NUIClient.h"

class AudioCallbacks
{
public:
	void Initialize()
	{
		auto nuiApp = Instance<NUIApp>::Get();

		NUIClient::OnClientCreated.Connect([](NUIClient* client)
		{
			client->AddProcessMessageHandler("nuiDoSetAudioCategory", [client](CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> message)
			{
				auto arguments = message->GetArgumentList();
				client->OnAudioCategoryConfigure(arguments->GetString(0), arguments->GetString(1));

				return true;
			});
		});

		nuiApp->AddV8Handler("nuiSetAudioCategory", [=](const CefV8ValueList& arguments, CefString& exception)
		{
			auto context = CefV8Context::GetCurrentContext();
			auto frame = context->GetFrame();

			// try finding the second-topmost frame
			auto refFrame = frame;

			for (auto f = refFrame; f->GetParent(); f = f->GetParent())
			{
				if (!f->GetParent()->GetParent())
				{
					refFrame = f;
					break;
				}
			}

			// get frame name
			auto frameName = refFrame->GetName();
			
			// get the audio category, if any
			if (arguments.size() >= 1 && arguments[0]->IsString())
			{
				auto message = CefProcessMessage::Create("nuiDoSetAudioCategory");
				auto argumentList = message->GetArgumentList();

				argumentList->SetString(0, frameName);
				argumentList->SetString(1, arguments[0]->GetStringValue());

				frame->SendProcessMessage(PID_BROWSER, message);
			}

			return CefV8Value::CreateUndefined();
		});
	}
};

static AudioCallbacks g_AudioCallbacks;

static InitFunction initFunction([]()
{
	g_AudioCallbacks.Initialize();
}, 1);
