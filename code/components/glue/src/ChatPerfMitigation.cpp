#include <StdInc.h>
#include <queue>

#include <Resource.h>
#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include <ICoreGameInit.h>

static bool g_gameSettled;

struct ChatQueueState
{
	size_t eventCookie = 0;
	bool hasQueued = false;
};

static InitFunction initFunction([]
{
	Instance<ICoreGameInit>::Get()->OnSetVariable.Connect([](const std::string& variable, bool value)
	{
		if (variable == "gameSettled")
		{
			g_gameSettled = value;
		}
	});

	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		if (resource->GetName() != "chat")
		{
			return;
		}

		auto queueState = std::make_shared<ChatQueueState>();

		resource->OnStart.Connect([resource, queueState]()
		{
			auto eventComponent = resource->GetComponent<fx::ResourceEventComponent>();
			eventComponent->OnTriggerEvent.Connect([eventComponent, queueState](const std::string& eventName, const std::string&, const std::string&, bool*)
			{
				if (eventName == "onClientResourceStart")
				{
					if (!g_gameSettled)
					{
						if (!queueState->hasQueued)
						{
							queueState->hasQueued = true;
							queueState->eventCookie = fx::ResourceManager::GetCurrent()->OnTick.Connect([queueState, eventComponent]()
							{
								if (g_gameSettled)
								{
									bool canceled = false;
									eventComponent->QueueEvent2("onClientResourceStart", {}, "chat");

									fx::ResourceManager::GetCurrent()->OnTick.Disconnect(queueState->eventCookie);
								}
							});
						}

						return false;
					}
				}

				return true;
			}, INT32_MIN);
		}, INT32_MAX);
	});
}, INT32_MAX);
