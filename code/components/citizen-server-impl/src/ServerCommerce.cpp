#include <StdInc.h>

#include <MakeClientFunction.h>
#include <ScriptEngine.h>
#include <ScriptDeprecations.h>

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("CAN_PLAYER_START_COMMERCE_SESSION", [] (fx::ScriptContext& context)
	{
		fx::WarningDeprecationf<fx::ScriptDeprecations::CAN_PLAYER_START_COMMERCE_SESSION>("natives", "CAN_PLAYER_START_COMMERCE_SESSION is deprecated.");
		return false;
	});

	fx::ScriptEngine::RegisterNativeHandler("LOAD_PLAYER_COMMERCE_DATA", [] (fx::ScriptContext& context)
	{
		fx::WarningDeprecationf<fx::ScriptDeprecations::LOAD_PLAYER_COMMERCE_DATA>("natives", "LOAD_PLAYER_COMMERCE_DATA is deprecated. Use LOAD_PLAYER_COMMERCE_DATA_EXT instead.");
		return false;
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_PLAYER_COMMERCE_INFO_LOADED", [] (fx::ScriptContext& context)
	{
		fx::WarningDeprecationf<fx::ScriptDeprecations::IS_PLAYER_COMMERCE_INFO_LOADED>("natives", "IS_PLAYER_COMMERCE_INFO_LOADED is deprecated. Use IS_PLAYER_COMMERCE_INFO_LOADED_EXT instead.");
		return false;
	});

	fx::ScriptEngine::RegisterNativeHandler("DOES_PLAYER_OWN_SKU", [] (fx::ScriptContext& context)
	{
		fx::WarningDeprecationf<fx::ScriptDeprecations::DOES_PLAYER_OWN_SKU>("natives", "DOES_PLAYER_OWN_SKU is deprecated. Use DOES_PLAYER_OWN_SKU_EXT instead.");
		return false;
	});

	fx::ScriptEngine::RegisterNativeHandler("REQUEST_PLAYER_COMMERCE_SESSION", [] (fx::ScriptContext& context)
	{
		fx::WarningDeprecationf<fx::ScriptDeprecations::REQUEST_PLAYER_COMMERCE_SESSION>("natives", "REQUEST_PLAYER_COMMERCE_SESSION is deprecated.");
		return false;
	});
});
