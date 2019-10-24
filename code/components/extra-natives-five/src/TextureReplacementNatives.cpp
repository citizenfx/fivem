#include <StdInc.h>

#include <ScriptEngine.h>

#include <grcTexture.h>
#include <Streaming.h>
#include <DrawCommands.h>

#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE

#define RAGE_FORMATS_IN_GAME
#include <gtaDrawable.h>

rage::grcTexture* LookupTexture(const std::string& txd, const std::string& txn)
{
	streaming::Manager* streaming = streaming::Manager::GetInstance();
	auto txdStore = streaming->moduleMgr.GetStreamingModule("ytd");

	uint32_t id = -1;
	txdStore->FindSlotFromHashKey(&id, txd.c_str());

	if (id != 0xFFFFFFFF)
	{
		auto txdRef = (rage::five::pgDictionary<rage::grcTexture>*)txdStore->GetPtr(id);

		if (txdRef)
		{
			return txdRef->Get(txn.c_str());
		}
	}

	return nullptr;
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("ADD_REPLACE_TEXTURE", [](fx::ScriptContext& context)
	{
		std::string origTxd = context.GetArgument<const char*>(0);
		std::string origTxn = context.GetArgument<const char*>(1);
		std::string newTxd = context.GetArgument<const char*>(2);
		std::string newTxn = context.GetArgument<const char*>(3);

		auto origTexture = LookupTexture(origTxd, origTxn);
		auto newTexture = LookupTexture(newTxd, newTxn);

		if (!origTexture || !newTexture)
		{
			context.SetResult<uint64_t>(false);
			return;
		}

		AddTextureOverride(origTexture, newTexture);
	});

	fx::ScriptEngine::RegisterNativeHandler("REMOVE_REPLACE_TEXTURE", [](fx::ScriptContext& context)
	{
		std::string origTxd = context.GetArgument<const char*>(0);
		std::string origTxn = context.GetArgument<const char*>(1);

		auto origTexture = LookupTexture(origTxd, origTxn);

		if (!origTexture)
		{
			context.SetResult<uint64_t>(false);
			return;
		}

		RemoveTextureOverride(origTexture);
	});
});
