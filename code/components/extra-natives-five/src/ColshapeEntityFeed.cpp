#include "StdInc.h"

#include <Hooking.h>
#include <nutsnbolts.h>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include <EntitySystem.h>
#include <Pool.h>
#include <netObject.h>
#include <net/NetObjEntityType.h>

#include <ColshapeManager.h>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

// defined in PoolTraversalNatives.cpp (owns the vehicle-pool pointer scan)
void ForAllVehicles(const std::function<void(fwEntity*)>& fn);

// networked entities carry their NetObjEntityType; local-only ones fall back per pool
static void AddSample(fwEntity* entity, fx::sync::NetObjEntityType fallback, std::vector<fx::colshape::EntitySample>& out)
{
	if (!entity)
	{
		return;
	}

	int handle = rage::fwScriptGuid::GetGuidFromBase(entity);
	if (handle == 0)
	{
		return;
	}

	int type = static_cast<int>(fallback);
	if (auto netObj = static_cast<rage::netObject*>(entity->GetNetObject()))
	{
		type = netObj->GetObjectType();
	}

	auto pos = entity->GetPosition();
	out.push_back({ handle, type, pos.x, pos.y, pos.z });
}

template<typename T>
static void GatherPool(const char* poolName, fx::sync::NetObjEntityType fallback, std::vector<fx::colshape::EntitySample>& out)
{
	auto pool = rage::GetPool<T>(poolName);
	if (!pool)
	{
		return;
	}

	for (int i = 0; i < pool->GetSize(); i++)
	{
		AddSample(pool->GetAt(i), fallback, out);
	}
}

static void GatherVehicles(std::vector<fx::colshape::EntitySample>& out)
{
	ForAllVehicles([&out](fwEntity* entity)
	{
		AddSample(entity, fx::sync::NetObjEntityType::Automobile, out);
	});
}

// the game frame gathers samples and emits events; a worker runs the query in between
struct PendingEvent
{
	const char* name;
	int entity;
	int shape;
	uint32_t generation;
};

static std::mutex g_jobMutex;
static std::condition_variable g_jobCv;
static std::vector<fx::colshape::EntitySample> g_pendingSamples;
static bool g_hasJob = false;
static bool g_workerStop = false;

static std::mutex g_resultMutex;
static std::vector<PendingEvent> g_results;

static void ColshapeWorker()
{
	for (;;)
	{
		std::vector<fx::colshape::EntitySample> samples;

		{
			std::unique_lock lock(g_jobMutex);
			g_jobCv.wait(lock, []()
			{
				return g_hasJob || g_workerStop;
			});

			if (g_workerStop)
			{
				return;
			}

			samples = std::move(g_pendingSamples);
			g_pendingSamples.clear();
			g_hasJob = false;
		}

		std::vector<PendingEvent> found;
		fx::colshape::ColshapeManager::Get().Update(samples, [&found](const char* event, int entity, int shape, uint32_t generation)
		{
			found.push_back({ event, entity, shape, generation });
		});

		if (!found.empty())
		{
			std::lock_guard lock(g_resultMutex);
			g_results.insert(g_results.end(), found.begin(), found.end());
		}
	}
}

static InitFunction initFunction([]()
{
	static std::chrono::milliseconds lastTick{ 0 };
	static std::thread worker(ColshapeWorker);
	worker.detach();

	OnGameFrame.Connect([]()
	{
		auto resourceManager = fx::ResourceManager::GetCurrent();
		if (resourceManager)
		{
			auto eventManager = resourceManager->GetComponent<fx::ResourceEventManagerComponent>();
			if (eventManager.GetRef())
			{
				std::vector<PendingEvent> results;
				{
					std::lock_guard lock(g_resultMutex);
					results = std::move(g_results);
					g_results.clear();
				}

				for (const auto& r : results)
				{
					// drop events whose shape id was freed (or freed+reused) since detection
					if (!fx::colshape::ColshapeManager::Get().IsLive(r.shape, r.generation))
					{
						continue;
					}

					eventManager->QueueEvent2(r.name, {}, r.entity, r.shape);
				}
			}
		}

		auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
		if (now - lastTick < std::chrono::milliseconds{ 50 })
		{
			return;
		}

		lastTick = now;

		std::vector<fx::colshape::EntitySample> samples;
		samples.reserve(512);

		GatherPool<CPed>("Peds", fx::sync::NetObjEntityType::Ped, samples);
		GatherVehicles(samples);
		GatherPool<CObject>("Object", fx::sync::NetObjEntityType::Object, samples);

		{
			std::lock_guard lock(g_jobMutex);
			g_pendingSamples = std::move(samples);
			g_hasJob = true;
		}
		g_jobCv.notify_one();
	});
});
