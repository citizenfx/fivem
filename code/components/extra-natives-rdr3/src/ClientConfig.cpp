#include "StdInc.h"
#include "ClientConfig.h"

#include <bitset>
#include <unordered_set>

#include "fxScripting.h"
#include "Resource.h"
#include "ScriptEngine.h"

std::bitset<256> g_clientConfigBits;
static std::unordered_map<fx::Resource*, std::unordered_set<ClientConfigFlag>> g_resourceFlags;

void SetClientConfigFlag(ClientConfigFlag flag, bool enabled)
{
	g_clientConfigBits.set(static_cast<size_t>(flag), enabled);
}

bool IsClientConfigEnabled(ClientConfigFlag flag)
{
	return g_clientConfigBits.test(static_cast<size_t>(flag));
}

void ResetClientConfigFlagsFor(fx::Resource* resource)
{
    auto it = g_resourceFlags.find(resource);
    if (it == g_resourceFlags.end())
        return;

    for (auto flag : it->second)
    {
        // Disable only the flags this resource enabled
        SetClientConfigFlag(flag, false);
    }

    g_resourceFlags.erase(it);
}

static HookFunction hookFunction([]()
{
    fx::ScriptEngine::RegisterNativeHandler("SET_CLIENT_CONFIG_BOOL", [](fx::ScriptContext& context)
    {
        int flagIndex = context.GetArgument<int>(0);
        bool enabled = context.GetArgument<bool>(1);

        ClientConfigFlag flag = static_cast<ClientConfigFlag>(flagIndex);
        SetClientConfigFlag(flag, enabled);

        fx::OMPtr<IScriptRuntime> runtime;
        if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
        {
            fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

            if (enabled)
            {
                g_resourceFlags[resource].insert(flag);
            }
            else
            {
                auto it = g_resourceFlags.find(resource);
                if (it != g_resourceFlags.end())
                {
                    it->second.erase(flag);
                    if (it->second.empty())
                        g_resourceFlags.erase(it);
                }
            }

            resource->OnStop.Connect([resource]()
            {
                ResetClientConfigFlagsFor(resource);
            });
        }
    });

	fx::ScriptEngine::RegisterNativeHandler("GET_CLIENT_CONFIG_BOOL", [](fx::ScriptContext& context)
	{
		int flagIndex = context.GetArgument<int>(0);
		ClientConfigFlag flag = static_cast<ClientConfigFlag>(flagIndex);

		bool result = IsClientConfigEnabled(flag);
		context.SetResult<bool>(result);
	});
});