#include "StdInc.h"

#include <Hooking.h>
#include <ScriptEngine.h>

#include <atPool.h>
#include <Pool.h>

#include <NativeWrappers.h>

struct PedPoolTraits
{
	using ObjectType = CPed;
	using PoolType = atPool<CPed>;

	static PoolType* GetPool()
	{
		return rage::GetPool<ObjectType>("Peds");
	}
};

struct VehiclePoolTraits
{
	using ObjectType = CVehicle;
	using PoolType = atPool<CVehicle>;

	static PoolType* GetPool()
	{
		return rage::GetPool<ObjectType>("CVehicle");
	}
};

struct ObjectPoolTraits
{
	using ObjectType = CObject;
	using PoolType = atPool<CObject>;

	static PoolType* GetPool()
	{
		return rage::GetPool<ObjectType>("Object");
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
};

static hook::cdecl_stub<uint32_t(void*)> getScriptGuidForEntity([]()
{
	return hook::get_pattern("32 DB E8 ? ? ? ? 48 85 C0 75 ? 8A 05", -35);
});

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
		}
		while (index < typedPool->GetSize());

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
});
