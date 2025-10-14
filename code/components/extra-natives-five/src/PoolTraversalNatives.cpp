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
	uint32_t m_count;
	char m_pad[36];
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

#ifdef IS_RDR3
struct BlipPoolTraits
{
	using PoolType = atPoolBase;

	static PoolType* GetPool()
	{
		return rage::GetPoolBase("fwuiBlip");
	}

	static uint32_t getScriptGuid(void* entry, int index)
	{
		auto pool = GetPool();
		if (!pool)
			return 0;
			
		// Blips use flag value 2
		// The handle format is: (index << 8) | flag
		uint32_t handle = (index << 8) | 2;
		
		return handle;
	}
};
#endif


template<typename TTraits, bool NetworkOnly = false>
static void SerializePool(fx::ScriptContext& context)
{
	std::vector<uint32_t> guids;

	TTraits::PoolType* pool = TTraits::GetPool();
	for (int i = 0; i < pool->GetSize(); ++i) // size_t i
	{
		TTraits::ObjectType* entry = pool->GetAt(i);
		if (entry)
		{
			if constexpr (NetworkOnly)
			{
				if (!entry->GetNetObject())
				{
					continue;
				}
			}

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
static void SerializePoolBase(fx::ScriptContext& context)
{
	std::vector<uint32_t> guids;

	auto pool = static_cast<atPoolBase*>(TTraits::GetPool());
	for (int i = 0; i < pool->GetCountDirect(); ++i)
	{
		auto entry = pool->GetAt<void>(i);
		if (entry)
		{
			uint32_t guid = TTraits::getScriptGuid(entry, i);
			if (guid != 0)
			{
				guids.push_back(guid);
			}
		}
	}

	context.SetResult(fx::SerializeObject(guids));
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
		std::string pool = context.CheckArgument<const char*>(0);
		if (pool.compare("CPed") == 0)
			SerializePool<PedPoolTraits>(context);
		else if (pool.compare("CObject") == 0)
			SerializePool<ObjectPoolTraits>(context);
		else if (pool.compare("CNetObject") == 0)
			SerializePool<ObjectPoolTraits, true>(context);
		else if (pool.compare("CPickup") == 0)
			SerializePool<PickupPoolTraits>(context);
		else if (pool.compare("CVehicle") == 0)
			SerializePool<VehiclePoolTraits>(context);
#ifdef IS_RDR3
		else if (pool.compare("CBlip") == 0)
			SerializePoolBase<BlipPoolTraits>(context);
#endif
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
