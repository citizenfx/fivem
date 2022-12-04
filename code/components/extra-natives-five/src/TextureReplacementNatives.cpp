#include <StdInc.h>

#include <ScriptEngine.h>

#include <grcTexture.h>
#include <Streaming.h>
#include <DrawCommands.h>

#include <boost/bimap.hpp>

#include <GameInit.h>
#include <unordered_set>

#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE

#define RAGE_FORMATS_IN_GAME
#include <gtaDrawable.h>
#include <fragType.h>

rage::grcTexture* LookupTexture(const std::string& txd, const std::string& txn)
{
	streaming::Manager* streaming = streaming::Manager::GetInstance();
	auto txdStore = streaming->moduleMgr.GetStreamingModule("ytd");

	uint32_t id = -1;
	txdStore->FindSlot(&id, txd.c_str());

	if (id != 0xFFFFFFFF)
	{
		auto txdRef = (rage::five::pgDictionary<rage::grcTexture>*)txdStore->GetPtr(id);

		if (txdRef)
		{
			return txdRef->Get(txn.c_str());
		}
	}

	auto drbStore = streaming->moduleMgr.GetStreamingModule("ydr");

	id = -1;
	drbStore->FindSlot(&id, txd.c_str());

	if (id != 0xFFFFFFFF)
	{
		auto txdRef = (rage::five::gtaDrawable*)drbStore->GetPtr(id);

		if (txdRef && txdRef->GetShaderGroup() && txdRef->GetShaderGroup()->GetTextures())
		{
			return (rage::grcTexture*)txdRef->GetShaderGroup()->GetTextures()->Get(txn.c_str());
		}
	}

	auto fragStore = streaming->moduleMgr.GetStreamingModule("yft");

	id = -1;
	fragStore->FindSlot(&id, txd.c_str());

	if (id != 0xFFFFFFFF)
	{
		auto txdRef = (rage::five::fragType*)fragStore->GetPtr(id);

		if (txdRef && txdRef->GetPrimaryDrawable() && txdRef->GetPrimaryDrawable()->GetShaderGroup() && txdRef->GetPrimaryDrawable()->GetShaderGroup()->GetTextures())
		{
			return (rage::grcTexture*)txdRef->GetPrimaryDrawable()->GetShaderGroup()->GetTextures()->Get(txn.c_str());
		}
	}

	return nullptr;
}

static std::unordered_set<rage::grcTexture*> texturesToRemove;
static boost::bimap<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>> g_replaceTxdPairs;

void TextureReplacement_OnTextureCreate(const std::string& txd, const std::string& txn)
{
	// TODO: this could be used (with some changes) when grcTexture gets placed as well
	if (auto it = g_replaceTxdPairs.right.find(std::make_tuple(txd, txn)); it != g_replaceTxdPairs.right.end())
	{
		auto origTexture = LookupTexture(std::get<0>(it->get_left()), std::get<1>(it->get_left()));
		auto newTexture = LookupTexture(std::get<0>(it->get_right()), std::get<1>(it->get_right()));

		if (origTexture && newTexture)
		{
			texturesToRemove.insert(origTexture);
			AddTextureOverride(origTexture, newTexture);
		}
	}
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

		// add to the pending TXD list
		g_replaceTxdPairs.insert({ { origTxd, origTxn }, { newTxd, newTxn } });

		// if either texture does not exist, skip
		if (!origTexture || !newTexture)
		{
			return;
		}

		texturesToRemove.insert(origTexture);
		AddTextureOverride(origTexture, newTexture);
	});

	fx::ScriptEngine::RegisterNativeHandler("REMOVE_REPLACE_TEXTURE", [](fx::ScriptContext& context)
	{
		std::string origTxd = context.GetArgument<const char*>(0);
		std::string origTxn = context.GetArgument<const char*>(1);

		// remove the pending txd
		g_replaceTxdPairs.left.erase(std::make_tuple(origTxd, origTxn));

		// remove any txd overridden now
		auto origTexture = LookupTexture(origTxd, origTxn);

		if (!origTexture)
		{
			context.SetResult<uint64_t>(false);
			return;
		}

		texturesToRemove.erase(origTexture);
		RemoveTextureOverride(origTexture);
	});

	OnKillNetworkDone.Connect([]()
	{
		for (auto texture : texturesToRemove)
		{
			RemoveTextureOverride(texture);
		}

		texturesToRemove.clear();
		g_replaceTxdPairs.clear();
	});
});
