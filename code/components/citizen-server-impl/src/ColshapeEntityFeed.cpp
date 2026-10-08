#include "StdInc.h"

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>
#include <GameServer.h>

#include <state/ServerGameStatePublic.h>

#include <ColshapeManager.h>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

// the tick gathers samples and emits events; a worker runs the query in between
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
				return g_hasJob;
			});

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
	// order 100 so this runs after GameServer attaches its component (default order 0),
	// else GetComponent<fx::GameServer>() below asserts
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		static std::chrono::milliseconds lastTick{ 0 };
		static std::thread worker(ColshapeWorker);
		worker.detach();

		instance->GetComponent<fx::GameServer>()->OnTick.Connect([instance]()
		{
			auto eventManager = instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::ResourceEventManagerComponent>();
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

			auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
			if (now - lastTick < std::chrono::milliseconds{ 50 })
			{
				return;
			}

			lastTick = now;

			auto gameState = instance->GetComponent<fx::ServerGameStatePublic>();
			if (!gameState.GetRef())
			{
				return;
			}

			std::vector<fx::colshape::EntitySample> samples;
			samples.reserve(256);
			gameState->ForAllEntities([&samples](fx::sync::Entity* entity)
			{
				int type = entity->GetTypeIndex();
				if (type < 0)
				{
					return;
				}

				auto pos = entity->GetPosition();
				samples.push_back({ static_cast<int>(entity->GetScriptGuid()), type, pos.x, pos.y, pos.z });
			});

			{
				std::lock_guard lock(g_jobMutex);
				g_pendingSamples = std::move(samples);
				g_hasJob = true;
			}
			g_jobCv.notify_one();
		});
	}, 100);
});
