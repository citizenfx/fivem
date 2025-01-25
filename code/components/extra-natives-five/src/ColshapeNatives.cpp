/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#if defined(GTA_FIVE)
#include <ScriptEngine.h>

#include <Resource.h>
#include <fxScripting.h>

#include <DisableChat.h>
#include <nutsnbolts.h>

#include <ICoreGameInit.h>

#include <ResourceManager.h>
#include <EntitySystem.h>
#include <ResourceEventComponent.h>
#include <ResourceCallbackComponent.h>

#include <chrono>  


/*
lua:

AddEventHandler('myCustomHandler, function(data)
    print('Received event: ' .. data)
end)

*/

static void TriggerMyCustomEvent()
{

    auto resman = Instance<fx::ResourceManager>::Get();
    if (!resman) return;

    auto rec = resman->GetComponent<fx::ResourceEventManagerComponent>();
    if (!rec) return;

    // Queue the event
    rec->QueueEvent2("myCustomhandler", {}, "Periodic event fired!");
}

static InitFunction initFunction([]()
{
    static bool s_init = false;
    static auto s_lastTriggerTime = std::chrono::steady_clock::now();

    OnMainGameFrame.Connect([]()
    {
        if (!s_init)
        {
            s_init = true;
            s_lastTriggerTime = std::chrono::steady_clock::now();
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lastTriggerTime).count();

        if (elapsed >= 500)
        {
            s_lastTriggerTime = now;
            TriggerMyCustomEvent();
        }
    });
});

#endif
