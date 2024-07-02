#include "StdInc.h"

#include <Hooking.h>
#include <ScriptEngine.h>
#include <ScriptSerialization.h>

#include <atPool.h>
#include <Pool.h>

#ifdef IS_RDR3
#include <NativeWrappers.h>
#endif

#include <Local.h>

#ifdef GTA_FIVE
template<typename TEntry>
class RefPool
{
private:
	TEntry** m_baseAddress;
	size_t m_count;
	size_t m_freeSpace;
	char m_pad2[24];
	uint32_t* m_validBits;
	// ...

private:
	bool IsValid(int index) const
	{
		return (m_validBits[index / 32] >> (index % 32)) & 1;
	}

public:
	TEntry* GetAt(int index) const
	{
		if (!IsValid(index))
		{
			return nullptr;
		}

		return m_baseAddress[index];
	}

	size_t GetSize()
	{
		return m_count;
	}

	// copying the behaviour of atPoolBase by returning the used space
	size_t GetCountDirect()
	{
		return (m_count - m_freeSpace);
	}
};
#endif

static hook::cdecl_stub<uint32_t(fwEntity*)> getScriptGuidForEntity([]()
{
#ifdef GTA_FIVE
	return hook::get_pattern("48 F7 F9 49 8B 48 08 48 63 D0 C1 E0 08 0F B6 1C 11 03 D8", -0x68);
#elif IS_RDR3
	return hook::get_pattern("32 DB E8 ? ? ? ? 48 85 C0 75 ? 8A 05", -35);
#endif
});

struct PedPoolTraits
{
	using ObjectType = CPed;
	using PoolType = atPool<CPed>;

	static PoolType* GetPool()
	{
		return rage::GetPool<ObjectType>("Peds");
	}

	static uint32_t getScriptGuid(ObjectType* e)
	{
		return getScriptGuidForEntity((ObjectType*)e);
	}
};

#ifdef GTA_FIVE
static RefPool<CVehicle>*** g_vehiclePool;

struct VehiclePoolTraits
{
	using ObjectType = CVehicle;
	using PoolType = RefPool<CVehicle>;

	static PoolType* GetPool()
	{
		return **g_vehiclePool;
	}

	static uint32_t getScriptGuid(ObjectType* e)
	{
		return getScriptGuidForEntity((ObjectType*)e);
	}
};
#elif IS_RDR3
struct VehiclePoolTraits
{
	using ObjectType = CVehicle;
	using PoolType = atPool<CVehicle>;

	static PoolType* GetPool()
	{
		return rage::GetPool<ObjectType>("CVehicle");
	}

	static uint32_t getScriptGuid(ObjectType* e)
	{
		return getScriptGuidForEntity((ObjectType*)e);
	}
};
#endif

struct ObjectPoolTraits
{
	using ObjectType = CObject;
	using PoolType = atPool<CObject>;

	static PoolType* GetPool()
	{
		return rage::GetPool<ObjectType>("Object");
	}

	static uint32_t getScriptGuid(ObjectType* e)
	{
		return getScriptGuidForEntity((ObjectType*)e);
	}
};

struct PickupPoolTraits
{
	using ObjectType = CPickup;
	using PoolType = atPool<CPickup>;

	static PoolType* GetPool()
	{
		return rage::GetPool<ObjectType>("CPickup");
	}

	static uint32_t getScriptGuid(ObjectType* e)
	{
		return getScriptGuidForEntity((ObjectType*)e);
	}
};

template<typename TTraits>
static void SerializePool(fx::ScriptContext& context)
{
	std::vector<uint32_t> guids;

	TTraits::PoolType* pool = TTraits::GetPool();
	for (int i = 0; i < pool->GetSize(); ++i) // size_t i
	{
		TTraits::ObjectType* entry = pool->GetAt(i);
		if (entry)
		{
			uint32_t guid = TTraits::getScriptGuid(entry);
			if (guid != 0)
			{
				guids.push_back(guid);
			}
		}
	}

	context.SetResult(fx::SerializeObject(guids));
}

template<typename TTraits>
static void GetPoolSize(fx::ScriptContext& context)
{
	TTraits::PoolType* pool = TTraits::GetPool();

	context.SetResult<int>(pool->GetCountDirect());
	*context.GetArgument<int*>(1) = pool->GetSize();
}

struct FindHandle
{
	void* pool;
	int index;

	template<typename TPool>
	bool Find(int* outGuid)
	{
		auto typedPool = reinterpret_cast<TPool*>(pool);

		do
		{
			++index;

			if (index < typedPool->GetSize())
			{
				auto entry = typedPool->GetAt(index);

				if (entry)
				{
					*outGuid = getScriptGuidForEntity(entry);

					return true;
				}
			}
		} while (index < typedPool->GetSize());

		*outGuid = -1;
		return false;
	}
};

static FindHandle g_handles[64];

static FindHandle* GetFindHandle()
{
	for (auto& handle : g_handles)
	{
		if (!handle.pool)
		{
			return &handle;
		}
	}

	return nullptr;
}

template<typename TTraits>
static void FindFirstHandler(fx::ScriptContext& context)
{
	auto handle = GetFindHandle();

	if (handle)
	{
		handle->pool = TTraits::GetPool();
		handle->index = -1;

		if (handle->Find<TTraits::PoolType>(context.GetArgument<int*>(0)))
		{
			context.SetResult(handle - g_handles);
			return;
		}
		handle->pool = nullptr;
	}

	context.SetResult(-1);
}

template<typename TTraits>
static void FindNextHandler(fx::ScriptContext& context)
{
	int handleIdx = context.GetArgument<uint32_t>(0);

	if (handleIdx >= _countof(g_handles))
	{
		context.SetResult(false);
		return;
	}

	auto handle = &g_handles[handleIdx];
	context.SetResult(handle->Find<TTraits::PoolType>(context.GetArgument<int*>(1)));
}

static void CloseFindHandler(fx::ScriptContext& context)
{
	int handleIdx = context.GetArgument<uint32_t>(0);

	if (handleIdx < _countof(g_handles))
	{
		auto handle = &g_handles[handleIdx];
		handle->pool = nullptr;
	}
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("FIND_FIRST_PED", FindFirstHandler<PedPoolTraits>);
	fx::ScriptEngine::RegisterNativeHandler("FIND_NEXT_PED", FindNextHandler<PedPoolTraits>);
	fx::ScriptEngine::RegisterNativeHandler("END_FIND_PED", CloseFindHandler);

	fx::ScriptEngine::RegisterNativeHandler("FIND_FIRST_VEHICLE", FindFirstHandler<VehiclePoolTraits>);
	fx::ScriptEngine::RegisterNativeHandler("FIND_NEXT_VEHICLE", FindNextHandler<VehiclePoolTraits>);
	fx::ScriptEngine::RegisterNativeHandler("END_FIND_VEHICLE", CloseFindHandler);

	fx::ScriptEngine::RegisterNativeHandler("FIND_FIRST_OBJECT", FindFirstHandler<ObjectPoolTraits>);
	fx::ScriptEngine::RegisterNativeHandler("FIND_NEXT_OBJECT", FindNextHandler<ObjectPoolTraits>);
	fx::ScriptEngine::RegisterNativeHandler("END_FIND_OBJECT", CloseFindHandler);

	fx::ScriptEngine::RegisterNativeHandler("FIND_FIRST_PICKUP", FindFirstHandler<PickupPoolTraits>);
	fx::ScriptEngine::RegisterNativeHandler("FIND_NEXT_PICKUP", FindNextHandler<PickupPoolTraits>);
	fx::ScriptEngine::RegisterNativeHandler("END_FIND_PICKUP", CloseFindHandler);

	fx::ScriptEngine::RegisterNativeHandler("GET_GAME_POOL", [](fx::ScriptContext& context)
	{
		std::string_view pool = context.CheckArgument<const char*>(0);

		if (pool == "CPed")
		{
			SerializePool<PedPoolTraits>(context);
		}
		else if (pool == "CObject")
		{
			SerializePool<ObjectPoolTraits>(context);
		}
		else if (pool == "CPickup")
		{
			SerializePool<PickupPoolTraits>(context);
		}
		else if (pool == "CVehicle")
		{
			SerializePool<VehiclePoolTraits>(context);
		}
		else
		{
			throw std::runtime_error(va("Invalid pool: %s", pool));
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_GAME_POOL_SIZE", [](fx::ScriptContext& context)
	{
		std::string_view pool = context.CheckArgument<const char*>(0);

		if (pool == "CPed")
		{
			GetPoolSize<PedPoolTraits>(context);
		}
		else if (pool == "CObject")
		{
			GetPoolSize<ObjectPoolTraits>(context);
		}
		else if (pool == "CPickup")
		{
			GetPoolSize<PickupPoolTraits>(context);
		}
		else if (pool == "CVehicle")
		{
			GetPoolSize<VehiclePoolTraits>(context);
		}
		else
		{
			throw std::runtime_error(va("Invalid pool: %s", pool));
		}
	});
});

#ifdef GTA_FIVE
static HookFunction hookFunction([]()
{
	g_vehiclePool = hook::get_address<decltype(g_vehiclePool)>(hook::get_pattern("48 8B 05 ? ? ? ? F3 0F 59 F6 48 8B 08", 3));
});
#endif
