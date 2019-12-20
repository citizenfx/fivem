#include <StdInc.h>

#if defined(GTA_FIVE)
#include <EntitySystem.h>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>
#include <ResourceCallbackComponent.h>

static InitFunction initFunction([]()
{
	OnCreatePopulationPed.Connect([](PopulationCreationState* state)
	{
		auto resman = Instance<fx::ResourceManager>::Get();
		
		auto rec = resman->GetComponent<fx::ResourceEventManagerComponent>();
		auto cbComponent = resman->GetComponent<fx::ResourceCallbackComponent>();

		std::map<std::string, fx::ResourceCallbackComponent::CallbackRef> overrideCalls;
		overrideCalls["setPosition"] = cbComponent->CreateCallback([=](const msgpack::v1::unpacked& unpacked)
		{
			auto args = unpacked.get().as<std::vector<msgpack::object>>();

			if (args.size() == 3)
			{
				state->position[0] = args[0].as<float>();
				state->position[1] = args[1].as<float>();
				state->position[2] = args[2].as<float>();
			}
		});

		overrideCalls["setModel"] = cbComponent->CreateCallback([=](const msgpack::v1::unpacked& unpacked)
		{
			auto args = unpacked.get().as<std::vector<msgpack::object>>();

			if (args.size() == 1)
			{
				if (args[0].type == msgpack::type::STR)
				{
					state->model = HashString(args[0].as<std::string>().c_str());
				}
				else
				{
					state->model = args[0].as<uint32_t>();
				}
			}
		});

		state->allowed = rec->TriggerEvent2("populationPedCreating", {}, state->position[0], state->position[1], state->position[2], state->model, overrideCalls) && state->allowed;
	});
});
#endif
