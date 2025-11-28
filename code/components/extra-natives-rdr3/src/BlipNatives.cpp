#include "StdInc.h"

#include <Hooking.h>
#include <ScriptEngine.h>
#include <ScriptSerialization.h>

#include <atArray.h>
#include <Pool.h>

#include <GameInit.h>
#include <Local.h>

struct BlipData
{
    char pad1[32]; // 0x0000-0x001F (32 bytes)
    float x; // 0x0020 (4 bytes)
    float y; // 0x0024 (4 bytes)
    float z; // 0x0028 (4 bytes)
    char vpad[4]; // 0x002C-0x002F (4 bytes)
    uint16_t rotation; // 0x0030 (2 bytes)
    char pad2[542]; // 0x0032-0x024F (542 bytes)
    atArray<uint32_t> modifiers; // 0x0250-0x025F (16 bytes)
    char pad3[92]; // 0x0260-0x02BB (92 bytes)
    uint32_t blipSprite; // 0x02BC (4 bytes)
    char pad4[15]; // 0x02C0-0x02CE (15 bytes)
    uint8_t blipFlags; // 0x02CF - bitfield containing blip type/flags
    char pad5[16]; // 0x02D0-0x02DF (16 bytes)
    void* vtable; // 0x02F0 (8 bytes)
    void* unk; // 0x02F8 (8 bytes)
};

static auto GetBlipPool()
{
    static auto pool = rage::GetPoolBase("fwuiBlip");
    return pool;
}

static BlipData* GetBlipData(uint32_t blipHandle)
{
    if (blipHandle == 0)
        return nullptr;

    auto pool = GetBlipPool();
    if (!pool)
        return nullptr;

    auto blip = pool->GetAtHandle<BlipData>(blipHandle);
    return blip;
}

static InitFunction initFunction([]()
{
    fx::ScriptEngine::RegisterNativeHandler("GET_BLIP_SPRITE", [](fx::ScriptContext& context)
    {
        uint32_t blipHandle = context.GetArgument<uint32_t>(0);
        
        auto blip = GetBlipData(blipHandle);
        if (blip)
        {
            context.SetResult(blip->blipSprite);
        }
        else
        {
            context.SetResult(0);
        }
    });

    fx::ScriptEngine::RegisterNativeHandler("GET_BLIP_ROTATION", [](fx::ScriptContext& context)
    {
        uint32_t blipHandle = context.GetArgument<uint32_t>(0);
        
        auto blip = GetBlipData(blipHandle);
        if (blip)
        {
            float rotation = static_cast<float>(blip->rotation) * (360.0f / 65535.0f);
            context.SetResult(rotation);
        }
        else
        {
            context.SetResult(0.0f);
        }
    });

    fx::ScriptEngine::RegisterNativeHandler("GET_BLIP_MODIFIERS", [](fx::ScriptContext& context)
    {
        uint32_t blipHandle = context.GetArgument<uint32_t>(0);
        
        std::vector<uint32_t> modifiers;
        auto blip = GetBlipData(blipHandle);
        if (blip)
        {
            for (int i = 0; i < blip->modifiers.GetCount(); ++i)
            {
                modifiers.push_back(blip->modifiers[i]);
            }
        }
        context.SetResult(fx::SerializeObject(modifiers));
    });

});