#include <StdInc.h>
#include <ScriptEngine.h>
#include <EntitySystem.h>

#include <Hooking.h>
#include <Pool.h>

#include <GameInit.h>

static fwEntity* getAndCheckVehicle(fx::ScriptContext& context, std::string_view name)
{
    auto traceFn = [name](std::string_view msg)
    {
        trace("%s: %s\n", name, msg);
    };

    if (context.GetArgumentCount() < 1)
    {
        traceFn("At least one argument must be passed");
        return nullptr;
    }

    fwEntity* vehicle = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));

    if (!vehicle)
    {
        traceFn("No such entity");
        return nullptr;
    }

    if (!vehicle->IsOfType<CVehicle>())
    {
        traceFn("Can not read from an entity that is not a vehicle or draft vehicle");
        return nullptr;
    }

    return vehicle;
}

static hook::FlexStruct* getAndCheckVehicleFlexStruct(fx::ScriptContext& context, std::string_view name)
{
    fwEntity* vehicle = getAndCheckVehicle(context, name);
    if (!vehicle)
    {
        return nullptr;
    }
    return reinterpret_cast<hook::FlexStruct*>(vehicle);
}

static bool g_ignoreVehicleOwnershipChecksForStowing = false;

static bool (*g_origIsAllowedToInteractWithHuntingWagon)(void*, bool);

static bool IsAllowedToInteractWithHuntingWagon(void* entity, bool ownershipCheck)
{
	return g_origIsAllowedToInteractWithHuntingWagon(entity, !g_ignoreVehicleOwnershipChecksForStowing);
}

static bool (*origGetPedOfPlayerOwnerOfNetworkObject)(void* pNetObj);

static InitFunction initFunction([]()
{

    fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_ADDITIONAL_PROP_SET_HASH", [](fx::ScriptContext& context)
    {
        if (hook::FlexStruct* flexStruct = getAndCheckVehicleFlexStruct(context, "GET_VEHICLE_ADDITIONAL_PROP_SET_HASH"))
        {
            uint32_t propSetHash = flexStruct->Get<uint32_t>(0xD90);
            context.SetResult(propSetHash);
        }
        else
        {
            context.SetResult(0);
        }
    });

});

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern("E8 ? ? ? ? 84 C0 74 5C 40 84 F6 74 22");
		hook::set_call(&g_origIsAllowedToInteractWithHuntingWagon, location);
		hook::call(location, IsAllowedToInteractWithHuntingWagon);
	}

	static struct : jitasm::Frontend
	{
		static bool getIgnore()
		{
			return g_ignoreVehicleOwnershipChecksForStowing;
		}

		virtual void InternalMain() override
		{
			// * original code
			mov(rcx, rax);

			// save rax, it has the pointer to EntityNetObj
			push(rax);

			sub(rsp, 0x18);

			// call the function
			mov(rax, (uintptr_t)getIgnore);
			call(rax);

			add(rsp, 0x18);

			// compare the returned boolean, if true, skip the original code.
			cmp(al, 1);

			// restore rax
			pop(rax);

			// our check failed, run the original code!
			jne("fail");

			// success, return
			ret();

			L("fail");

			// * start of the original code

			// rbx is the first argument, move pEntityNetObj to that
			mov(rax, rbx);

			// call the function
			mov(rax, (uintptr_t)origGetPedOfPlayerOwnerOfNetworkObject);
			call(rax);

			cmp(rax, rbx);

			// * end of the original code

			// return from the function
			ret();
		}
	} asmfunc;

	{
		auto location = hook::get_pattern<char>("48 8B C8 E8 ? ? ? ? 48 3B C3 75 E1");

		hook::set_call(&origGetPedOfPlayerOwnerOfNetworkObject, location + 0x3);

		hook::nop(location, 11);

		hook::call(location, asmfunc.GetCode());
	}

	fx::ScriptEngine::RegisterNativeHandler("SET_IGNORE_VEHICLE_OWNERSHIP_FOR_STOWING", [](fx::ScriptContext& context)
	{
		g_ignoreVehicleOwnershipChecksForStowing = context.GetArgument<bool>(0);
	});

	OnKillNetworkDone.Connect([]()
	{
		g_ignoreVehicleOwnershipChecksForStowing = false;
	});
});
