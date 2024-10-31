#include <StdInc.h>

#include <atArray.h>
#include <EntitySystem.h>
#include <Hooking.h>
#include <Local.h>
#include <ScriptEngine.h>
#include <Utils.h>

class CPedPropInfo
{
};

class CPedVariationInfo
{
};

class CPedVariationInfoCollection
{
public:
	atArray<CPedVariationInfo*> m_infos;
	// There are more fields after m_infos, but we don't care about them.
};

enum class VariationType : uint8_t
{
	DRAWABLE,
	PROP,
};

static uint32_t g_collectionInfoHashOffset;
static uint32_t g_dynamicEntityArchetypeOffset;
static uint32_t g_pedModelInfoVarInfoCollectionOffset;
static uint32_t g_variationInfoPropInfoOffset;

static hook::cdecl_stub<int(CPedVariationInfoCollection*, int, uint32_t, uint32_t)> g_GetGlobalDrawableIndex([]()
{
	return hook::get_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 0F B7 41 ? 33 F6 45 8B F1");
});

static hook::cdecl_stub<int(CPedVariationInfoCollection*, int, uint32_t, uint32_t)> g_GetGlobalPropIndex([]()
{
	return hook::get_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 33 FF 45 8B F1");
});

static hook::cdecl_stub<int(CPedVariationInfoCollection*, uint32_t, uint32_t)> g_GetDlcDrawableIdx([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 44 8B C3 48 8B 5D"));
});

static hook::cdecl_stub<int(CPedVariationInfoCollection*, uint32_t, uint32_t)> g_GetDlcPropIdx([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 45 33 F6 44 8B E8 44 38 75"));
});

static hook::cdecl_stub<uint8_t(CPedVariationInfo*, uint32_t)> g_GetMaxNumDrawables([]()
{
	return hook::get_pattern("48 89 5C 24 ? 57 48 83 EC ? 8B DA 48 8B F9 E8 ? ? ? ? 48 85 C0 74 ? 8B D3");
});

static hook::cdecl_stub<uint8_t(CPedPropInfo*, uint32_t)> g_GetMaxNumProps([]()
{
	return hook::get_pattern("48 89 5C 24 ? 0F B7 41 ? 45 33 C0 45 8B C8 45 8B D0 8B D8");
});

static hook::cdecl_stub<CPedVariationInfo*(CPedVariationInfoCollection*, uint32_t, uint32_t)> g_GetVariationInfoFromDrawableIdx([]()
{
	return hook::get_pattern("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 56 48 83 EC ? 33 DB 41 8B F0 8B EA 48 8B F9 66 3B 59 ? 73 ? 48 8B 0F");
});

static hook::cdecl_stub<CPedVariationInfo*(CPedVariationInfoCollection*, uint32_t, uint32_t)> g_GetVariationInfoFromPropIdx([]()
{
	return hook::get_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B7 41 ? 33 DB");
});

static hook::cdecl_stub<const char*(CPedVariationInfo*)> g_GetCollectionName([]()
{
	return hook::get_pattern("8B 51 ? 85 D2 75 ? 33 C0");
});

static const char* GetCollectionName(CPedVariationInfo* info)
{
	const char* collectionName = g_GetCollectionName(info);
	if (collectionName == nullptr)
	{
		// g_GetCollectionName returns nullptr for empty string which corresponds to the base game collection.
		return "";
	}
	else
	{
		return collectionName;
	}
}

static uint32_t GetCollectionHash(CPedVariationInfo* info)
{
	return *(uint32_t*)((uintptr_t)info + g_collectionInfoHashOffset);
};

static CPedVariationInfoCollection* GetPedVariationInfoCollection(uint32_t pedId)
{
	auto ped = rage::fwScriptGuid::GetBaseFromGuid(pedId);
	if (!ped || !ped->IsOfType<CPed>())
	{
		return nullptr;
	}

	auto pedModelInfo = *(void**)((uintptr_t)ped + g_dynamicEntityArchetypeOffset);
	return *(CPedVariationInfoCollection**)((uintptr_t)pedModelInfo + g_pedModelInfoVarInfoCollectionOffset);
}

static CPedVariationInfo* GetVariationInfoFromCollection(CPedVariationInfoCollection* self, const char* collectionName)
{
	uint32_t collectionNameHash = HashString(collectionName);
	for (int i = 0; i < self->m_infos.GetCount(); i++)
	{
		auto variationInfo = self->m_infos[i];
		if (variationInfo && GetCollectionHash(variationInfo) == collectionNameHash)
		{
			return variationInfo;
		}
	}
	return nullptr;
}

static uint8_t GetMaxNumberOfDrawables(CPedVariationInfo* info, uint32_t componentId)
{
	return g_GetMaxNumDrawables(info, componentId);
}

static uint8_t GetMaxNumberOfProps(CPedVariationInfo* info, uint32_t anchorPoint)
{
	auto propInfo = (CPedPropInfo*)((uintptr_t)info + g_variationInfoPropInfoOffset);
	return g_GetMaxNumProps(propInfo, anchorPoint);
}

static int GetGlobalIndex(uint32_t pedId, int slotId, const char* collectionName, int localIndex, VariationType variationType)
{
	auto variationInfoCollection = GetPedVariationInfoCollection(pedId);
	if (!variationInfoCollection)
	{
		return -1;
	}

	auto variationInfo = GetVariationInfoFromCollection(variationInfoCollection, collectionName);
	// Collection does not exist.
	if (!variationInfo)
	{
		return -1;
	}

	int maxNumVariations = 0;
	if (variationType == VariationType::DRAWABLE)
	{
		maxNumVariations = static_cast<int>(GetMaxNumberOfDrawables(variationInfo, slotId));
	}
	else
	{
		maxNumVariations = static_cast<int>(GetMaxNumberOfProps(variationInfo, slotId));
	}

	// Local index is out of bounds.
	if (localIndex < 0 || localIndex >= maxNumVariations)
	{
		return -1;
	}

	uint32_t collectionNameHash = HashString(collectionName);
	if (variationType == VariationType::DRAWABLE)
	{
		return g_GetGlobalDrawableIndex(variationInfoCollection, localIndex, slotId, collectionNameHash);
	}
	else
	{
		return g_GetGlobalPropIndex(variationInfoCollection, localIndex, slotId, collectionNameHash);
	}
}

template<typename T>
static int VariadicGetArgument(fx::ScriptContext& context, fx::ScriptContextBuffer& newContext, int index)
{
	newContext.Push(context.GetArgument<T>(index));
	return index + 1;
}

template<typename T, typename... AdditionalArguments>
static void RedirectNativeCallWithGlobalIndex(fx::ScriptContext& context, VariationType variationType, uint64_t nativeIdentifier)
{
	uint32_t pedId = context.GetArgument<uint32_t>(0);
	int componentId = context.GetArgument<int>(1);
	int globalIndex = GetGlobalIndex(pedId, componentId, context.CheckArgument<const char*>(2), context.GetArgument<int>(3), variationType);
	if (globalIndex == -1)
	{
		if constexpr (!std::is_same<T, void>::value)
		{
			context.SetResult<T>(T());
		}
		return;
	}

	fx::ScriptContextBuffer newContext;
	newContext.Push(pedId);
	newContext.Push(componentId);
	newContext.Push(globalIndex);
	int index = 4;
	((index = VariadicGetArgument<AdditionalArguments>(context, newContext, index)), ...);

	fx::ScriptEngine::CallNativeHandler(nativeIdentifier, newContext);
	if constexpr (!std::is_same<T, void>::value)
	{
		context.SetResult<T>(newContext.GetResult<T>());
	}
}

static HookFunction hookFunction([]()
{
	g_collectionInfoHashOffset = *hook::get_pattern<uint8_t>("8B 51 ? 85 D2 75 ? 33 C0", 2);
	g_dynamicEntityArchetypeOffset = *hook::get_pattern<uint8_t>("4C 8B 61 ? 48 8B F9 4D 63 E9", 3);
	g_pedModelInfoVarInfoCollectionOffset = *hook::get_pattern<uint32_t>("49 8B 8C 24 ? ? ? ? 49 8B E8", 4);
	g_variationInfoPropInfoOffset = *hook::get_pattern<uint8_t>("48 83 C1 ? E8 ? ? ? ? 48 8D 7F", 3);

	// Natives to iterate over all collections.
	fx::ScriptEngine::RegisterNativeHandler("GET_PED_COLLECTIONS_COUNT", [](fx::ScriptContext& context)
	{
		auto variationInfoCollection = GetPedVariationInfoCollection(context.GetArgument<uint32_t>(0));
		if (variationInfoCollection)
		{
			context.SetResult<int>(variationInfoCollection->m_infos.GetCount());
		}
		else
		{
			context.SetResult<int>(0);
		}
	});
	fx::ScriptEngine::RegisterNativeHandler("GET_PED_COLLECTION_NAME", [](fx::ScriptContext& context)
	{
		auto variationInfoCollection = GetPedVariationInfoCollection(context.GetArgument<uint32_t>(0));
		int index = context.GetArgument<int>(1);
		if (variationInfoCollection && index >= 0 && index < variationInfoCollection->m_infos.GetCount())
		{
			auto variationInfo = variationInfoCollection->m_infos[context.GetArgument<int>(1)];
			context.SetResult<const char*>(GetCollectionName(variationInfo));
		}
		else
		{
			context.SetResult<const char*>(nullptr);
		}
	});

	// Natives to convert between global drawable/prop indices and local collections drawable/prop indices.
	fx::ScriptEngine::RegisterNativeHandler("GET_PED_COLLECTION_NAME_FROM_DRAWABLE", [](fx::ScriptContext& context)
	{
		auto variationInfoCollection = GetPedVariationInfoCollection(context.GetArgument<uint32_t>(0));
		if (variationInfoCollection)
		{
			auto variationInfo = g_GetVariationInfoFromDrawableIdx(variationInfoCollection, context.GetArgument<int>(1), context.GetArgument<int>(2));
			if (variationInfo)
			{
				context.SetResult<const char*>(GetCollectionName(variationInfo));
				return;
			}
		}
		context.SetResult<const char*>(nullptr);
	});
	fx::ScriptEngine::RegisterNativeHandler("GET_PED_COLLECTION_NAME_FROM_PROP", [](fx::ScriptContext& context)
	{
		auto variationInfoCollection = GetPedVariationInfoCollection(context.GetArgument<uint32_t>(0));
		if (variationInfoCollection)
		{
			auto variationInfo = g_GetVariationInfoFromPropIdx(variationInfoCollection, context.GetArgument<int>(1), context.GetArgument<int>(2));
			if (variationInfo)
			{
				context.SetResult<const char*>(GetCollectionName(variationInfo));
				return;
			}
		}
		context.SetResult<const char*>(nullptr);
	});
	fx::ScriptEngine::RegisterNativeHandler("GET_PED_COLLECTION_LOCAL_INDEX_FROM_DRAWABLE", [](fx::ScriptContext& context)
	{
		auto variationInfoCollection = GetPedVariationInfoCollection(context.GetArgument<uint32_t>(0));
		if (variationInfoCollection)
		{
			// Validate that the drawable index is within bounds. If variationInfo is null then input is invalid.
			auto variationInfo = g_GetVariationInfoFromDrawableIdx(variationInfoCollection, context.GetArgument<int>(1), context.GetArgument<int>(2));
			if (variationInfo)
			{
				context.SetResult<int>(g_GetDlcDrawableIdx(variationInfoCollection, context.GetArgument<int>(1), context.GetArgument<int>(2)));
				return;
			}
		}
		context.SetResult<int>(-1);
	});
	fx::ScriptEngine::RegisterNativeHandler("GET_PED_COLLECTION_LOCAL_INDEX_FROM_PROP", [](fx::ScriptContext& context)
	{
		auto variationInfoCollection = GetPedVariationInfoCollection(context.GetArgument<uint32_t>(0));
		if (variationInfoCollection)
		{
			// Validate that the prop index is within bounds. If variationInfo is null then input is invalid.
			auto variationInfo = g_GetVariationInfoFromPropIdx(variationInfoCollection, context.GetArgument<int>(1), context.GetArgument<int>(2));
			if (variationInfo)
			{
				context.SetResult<int>(g_GetDlcPropIdx(variationInfoCollection, context.GetArgument<int>(1), context.GetArgument<int>(2)));
				return;
			}
		}
		context.SetResult<int>(-1);
	});
	fx::ScriptEngine::RegisterNativeHandler("GET_PED_DRAWABLE_GLOBAL_INDEX_FROM_COLLECTION", [](fx::ScriptContext& context)
	{
		context.SetResult<int>(GetGlobalIndex(context.GetArgument<uint32_t>(0), context.GetArgument<int>(1), context.CheckArgument<const char*>(2), context.GetArgument<int>(3), VariationType::DRAWABLE));
	});
	fx::ScriptEngine::RegisterNativeHandler("GET_PED_PROP_GLOBAL_INDEX_FROM_COLLECTION", [](fx::ScriptContext& context)
	{
		context.SetResult<int>(GetGlobalIndex(context.GetArgument<uint32_t>(0), context.GetArgument<int>(1), context.CheckArgument<const char*>(2), context.GetArgument<int>(3), VariationType::PROP));
	});

	// Natives to set component/prop variation using collections.
	fx::ScriptEngine::RegisterNativeHandler("SET_PED_COLLECTION_COMPONENT_VARIATION", [](fx::ScriptContext& context)
	{
		// Call SET_PED_COMPONENT_VARIATION using the global index obtained from the collection name and local index.
		RedirectNativeCallWithGlobalIndex<void, int, int>(context, VariationType::DRAWABLE, 0x262B14F48D29DE80);
	});
	fx::ScriptEngine::RegisterNativeHandler("SET_PED_COLLECTION_PROP_INDEX", [](fx::ScriptContext& context)
	{
		// Call SET_PED_PROP_INDEX using the global index obtained from the collection name and local index.
		RedirectNativeCallWithGlobalIndex<void, int, bool>(context, VariationType::PROP, 0x93376B65A266EB5F);
	});
	fx::ScriptEngine::RegisterNativeHandler("SET_PED_COLLECTION_PRELOAD_VARIATION_DATA", [](fx::ScriptContext& context)
	{
		// Call SET_PED_PRELOAD_VARIATION_DATA using the global index obtained from the collection name and local index.
		RedirectNativeCallWithGlobalIndex<void, int>(context, VariationType::DRAWABLE, 0x39D55A620FCB6A3A);
	});
	fx::ScriptEngine::RegisterNativeHandler("SET_PED_COLLECTION_PRELOAD_PROP_DATA", [](fx::ScriptContext& context)
	{
		// Call SET_PED_PROP_INDEX using the global index obtained from the collection name and local index.
		RedirectNativeCallWithGlobalIndex<void, int>(context, VariationType::PROP, 0x2B16A3BFF1FBCE49);
	});

	// Natives to get component/prop variation using collections.
	fx::ScriptEngine::RegisterNativeHandler("GET_NUMBER_OF_PED_COLLECTION_DRAWABLE_VARIATIONS", [](fx::ScriptContext& context)
	{
		auto variationInfoCollection = GetPedVariationInfoCollection(context.GetArgument<uint32_t>(0));
		if (variationInfoCollection)
		{
			auto variationInfo = GetVariationInfoFromCollection(variationInfoCollection, context.CheckArgument<const char*>(2));
			if (variationInfo)
			{
				context.SetResult<int>(static_cast<int>(GetMaxNumberOfDrawables(variationInfo, context.GetArgument<int>(1))));
				return;
			}
		}
		context.SetResult<int>(0);
	});
	fx::ScriptEngine::RegisterNativeHandler("GET_NUMBER_OF_PED_COLLECTION_PROP_DRAWABLE_VARIATIONS", [](fx::ScriptContext& context)
	{
		auto variationInfoCollection = GetPedVariationInfoCollection(context.GetArgument<uint32_t>(0));
		if (variationInfoCollection)
		{
			auto variationInfo = GetVariationInfoFromCollection(variationInfoCollection, context.CheckArgument<const char*>(2));
			if (variationInfo)
			{
				context.SetResult<int>(static_cast<int>(GetMaxNumberOfProps(variationInfo, context.GetArgument<int>(1))));
				return;
			}
		}
		context.SetResult<int>(0);
	});
	fx::ScriptEngine::RegisterNativeHandler("GET_NUMBER_OF_PED_COLLECTION_TEXTURE_VARIATIONS", [](fx::ScriptContext& context)
	{
		// Call GET_NUMBER_OF_PED_TEXTURE_VARIATIONS using the global index obtained from the collection name and local index.
		RedirectNativeCallWithGlobalIndex<int>(context, VariationType::DRAWABLE, 0x8F7156A3142A6BAD);
	});
	fx::ScriptEngine::RegisterNativeHandler("GET_NUMBER_OF_PED_COLLECTION_PROP_TEXTURE_VARIATIONS", [](fx::ScriptContext& context)
	{
		// Call GET_NUMBER_OF_PED_PROP_TEXTURE_VARIATIONS using the global index obtained from the collection name and local index.
		RedirectNativeCallWithGlobalIndex<int>(context, VariationType::PROP, 0xA6E7F1CEB523E171);
	});
	fx::ScriptEngine::RegisterNativeHandler("IS_PED_COLLECTION_COMPONENT_VARIATION_VALID", [](fx::ScriptContext& context)
	{
		// Call IS_PED_COMPONENT_VARIATION_VALID using the global index obtained from the collection name and local index.
		RedirectNativeCallWithGlobalIndex<bool, int>(context, VariationType::DRAWABLE, 0xE825F6B6CEA7671D);
	});
	fx::ScriptEngine::RegisterNativeHandler("IS_PED_COLLECTION_COMPONENT_VARIATION_GEN9_EXCLUSIVE", [](fx::ScriptContext& context)
	{
		// Call IS_PED_COMPONENT_VARIATION_GEN9_EXCLUSIVE using the global index obtained from the collection name and local index.
		RedirectNativeCallWithGlobalIndex<bool>(context, VariationType::DRAWABLE, 0xC767B581);
	});
	fx::ScriptEngine::RegisterNativeHandler("GET_PED_DRAWABLE_VARIATION_COLLECTION_LOCAL_INDEX", [](fx::ScriptContext& context)
	{
		uint32_t pedId = context.GetArgument<uint32_t>(0);
		int componentId = context.GetArgument<int>(1);
		auto variationInfoCollection = GetPedVariationInfoCollection(pedId);
		if (!variationInfoCollection)
		{
			context.SetResult<int>(-1);
			return;
		}

		fx::ScriptContextBuffer newContext;
		newContext.Push(pedId);
		newContext.Push(componentId);
		// Call GET_PED_DRAWABLE_VARIATION to get global drawable index.
		fx::ScriptEngine::CallNativeHandler(0x67F3780DD425D4FC, newContext);
		int globalDrawableIndex = newContext.GetResult<int>();
		context.SetResult<int>(g_GetDlcDrawableIdx(variationInfoCollection, componentId, globalDrawableIndex));
	});
	fx::ScriptEngine::RegisterNativeHandler("GET_PED_DRAWABLE_VARIATION_COLLECTION_NAME", [](fx::ScriptContext& context)
	{
		uint32_t pedId = context.GetArgument<uint32_t>(0);
		int componentId = context.GetArgument<int>(1);
		auto variationInfoCollection = GetPedVariationInfoCollection(pedId);
		if (!variationInfoCollection)
		{
			context.SetResult<const char*>(nullptr);
			return;
		}

		fx::ScriptContextBuffer newContext;
		newContext.Push(pedId);
		newContext.Push(componentId);
		// Call GET_PED_DRAWABLE_VARIATION to get global drawable index.
		fx::ScriptEngine::CallNativeHandler(0x67F3780DD425D4FC, newContext);
		int globalDrawableIndex = newContext.GetResult<int>();
		auto variationInfo = g_GetVariationInfoFromDrawableIdx(variationInfoCollection, componentId, globalDrawableIndex);
		if (!variationInfo)
		{
			context.SetResult<const char*>(nullptr);
			return;
		}

		context.SetResult<const char*>(GetCollectionName(variationInfo));
	});
});
