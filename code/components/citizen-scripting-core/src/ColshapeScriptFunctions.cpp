/*
 * Colshape natives (shared client + server). The geometry/registry is pure and shared;
 * enter/exit tracking is driven by a per-side entity feed that calls
 * ColshapeManager::Update() on a tick and emits onColshapeEnter / onColshapeExit.
 */

#include "StdInc.h"

#include <ScriptEngine.h>
#include <Resource.h>
#include <ResourceManager.h>
#include <ResourceEventComponent.h>
#include <fxScripting.h>

#include <algorithm>
#include <atomic>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <thread>
#include <vector>

#include <msgpack.hpp>

#include "ColshapeManager.h"

namespace fx::colshape
{
static double NowMs()
{
	using namespace std::chrono;
	return duration<double, std::milli>(steady_clock::now().time_since_epoch()).count();
}

static std::string CurrentResourceName()
{
	fx::OMPtr<IScriptRuntime> runtime;
	if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
	{
		if (auto* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject()))
		{
			return resource->GetName();
		}
	}
	return {};
}

// codegen passes an `object` arg as a msgpack (ptr, len) pair: a flat [x1,y1,x2,y2,...]
static void ParsePolygonPoints(fx::ScriptContext& context, int argIdx, std::vector<Vec2>& out)
{
	auto data = context.GetArgument<const char*>(argIdx);
	auto length = context.GetArgument<size_t>(argIdx + 1);
	if (!data || !length)
	{
		return;
	}

	try
	{
		auto unpacked = msgpack::unpack(data, length);
		auto obj = unpacked.get();
		if (obj.type != msgpack::type::ARRAY)
		{
			return;
		}

		auto& arr = obj.via.array;
		for (uint32_t i = 0; i + 1 < arr.size; i += 2)
		{
			out.push_back({ arr.ptr[i].as<float>(), arr.ptr[i + 1].as<float>() });
		}
	}
	catch (const std::exception&)
	{
	}
}

ColshapeManager& ColshapeManager::Get()
{
	static ColshapeManager inst;
	return inst;
}

int ColshapeManager::Add(ColShape&& shape)
{
	// NaN/Inf would break the BVH's nth_element ordering, so reject malformed shapes
	if (!std::isfinite(shape.pos1.x) || !std::isfinite(shape.pos1.y) || !std::isfinite(shape.pos1.z) ||
		!std::isfinite(shape.radius) || !std::isfinite(shape.height) ||
		!std::isfinite(shape.dimW) || !std::isfinite(shape.dimD) ||
		!std::isfinite(shape.heading) || !std::isfinite(shape.minZ) || !std::isfinite(shape.maxZ))
	{
		return -1;
	}

	for (const auto& p : shape.points)
	{
		if (!std::isfinite(p.x) || !std::isfinite(p.y))
		{
			return -1;
		}
	}

	// negative extents pass the BVH's abs() bounds but make the signed narrow-phase
	// test unsatisfiable, so reject them rather than register an undetectable shape
	if (shape.radius < 0.f || shape.height < 0.f || shape.dimW < 0.f || shape.dimD < 0.f)
	{
		return -1;
	}

	std::unique_lock lock(m_mutex);

	int id;
	if (!m_freeIds.empty())
	{
		// reuse a freed id and bump its generation so old (id, generation) refs miss
		id = m_freeIds.back();
		m_freeIds.pop_back();
		m_generations[id]++;
	}
	else
	{
		// enhanced caps ids at 16 bits; we recycle freed ids, so this only trips if
		// 65536 shapes are live at once
		if (m_nextId > 0xFFFF)
		{
			return -1;
		}
		id = m_nextId++;
		m_generations.push_back(0);
		m_idToSlot.push_back(-1);
	}

	shape.id = id;
	shape.generation = m_generations[id];
	shape.owner = CurrentResourceName();
	shape.ComputeBounds();

	m_idToSlot[id] = static_cast<int32_t>(m_shapeCount);
	m_bounds.push_back({ id, shape.minX, shape.minY, shape.maxX, shape.maxY });
	m_recent.push_back(m_bounds.back());

	if (m_shapes.empty() || m_shapes.back().size() == kShapeChunkSize)
	{
		m_shapes.emplace_back();
		m_shapes.back().reserve(kShapeChunkSize);
	}
	m_shapes.back().push_back(std::move(shape));
	m_shapeCount++;

	m_lastMutationMs.store(static_cast<int64_t>(NowMs()), std::memory_order_relaxed);
	m_bvhGeneration.fetch_add(1, std::memory_order_relaxed);
	return id;
}

bool ColshapeManager::Delete(int id)
{
	std::unique_lock lock(m_mutex);

	if (id < 0 || static_cast<size_t>(id) >= m_idToSlot.size())
	{
		return false;
	}

	int32_t slot = m_idToSlot[id];
	if (slot < 0)
	{
		return false;
	}

	size_t last = m_shapeCount - 1;
	if (static_cast<size_t>(slot) != last)
	{
		int movedId = ShapeAt(last).id;
		ShapeAt(slot) = std::move(ShapeAt(last));
		m_bounds[slot] = m_bounds[last];
		m_idToSlot[movedId] = slot;
	}

	m_shapes.back().pop_back();
	if (m_shapes.back().empty())
	{
		m_shapes.pop_back();
	}
	m_shapeCount--;

	m_bounds.pop_back();
	m_idToSlot[id] = -1;
	m_freeIds.push_back(id);

	// stale entries left in m_recent/the tree are fine - queries re-validate via Find()
	m_lastMutationMs.store(static_cast<int64_t>(NowMs()), std::memory_order_relaxed);
	m_bvhGeneration.fetch_add(1, std::memory_order_relaxed);
	return true;
}

const ColShape* ColshapeManager::Find(int id) const
{
	if (id < 0 || static_cast<size_t>(id) >= m_idToSlot.size())
	{
		return nullptr;
	}

	int32_t slot = m_idToSlot[id];
	return (slot >= 0) ? &ShapeAt(slot) : nullptr;
}

ColShape* ColshapeManager::Find(int id)
{
	return const_cast<ColShape*>(static_cast<const ColshapeManager*>(this)->Find(id));
}

bool ColshapeManager::IsLive(int id, uint32_t generation) const
{
	std::shared_lock lock(m_mutex);
	const ColShape* s = Find(id);
	return s != nullptr && s->generation == generation;
}

void ColshapeManager::SetEntityType(int id, int type, bool value)
{
	std::unique_lock lock(m_mutex);

	if (auto* shape = Find(id))
	{
		shape->SetEntityType(type, value);
	}
}

void ColshapeManager::DeleteByOwner(const std::string& owner)
{
	std::vector<int> ids;
	{
		std::shared_lock lock(m_mutex);
		for (size_t i = 0; i < m_shapeCount; i++)
		{
			if (ShapeAt(i).owner == owner)
			{
				ids.push_back(ShapeAt(i).id);
			}
		}
	}

	for (int id : ids)
	{
		Delete(id);
	}
}

// static + reads only the passed-in snapshot, so it is safe on a background thread
std::shared_ptr<ColshapeManager::Bvh> ColshapeManager::BuildBvh(std::vector<BvhInput>& input)
{
	auto bvh = std::make_shared<Bvh>();
	if (input.empty())
	{
		return bvh;
	}

	// leaf size 4 => at most N-1 nodes
	bvh->nodes.reserve(input.size());
	bvh->ids.reserve(input.size());
	BuildBvhNode(*bvh, input, 0, static_cast<int>(input.size()));
	return bvh;
}

int ColshapeManager::BuildBvhNode(Bvh& bvh, std::vector<BvhInput>& input, int begin, int end)
{
	int nodeIdx = static_cast<int>(bvh.nodes.size());
	bvh.nodes.push_back({});

	float minX = FLT_MAX, minY = FLT_MAX, maxX = -FLT_MAX, maxY = -FLT_MAX;
	for (int i = begin; i < end; i++)
	{
		minX = std::min(minX, input[i].minX);
		minY = std::min(minY, input[i].minY);
		maxX = std::max(maxX, input[i].maxX);
		maxY = std::max(maxY, input[i].maxY);
	}

	bvh.nodes[nodeIdx].minX = minX;
	bvh.nodes[nodeIdx].minY = minY;
	bvh.nodes[nodeIdx].maxX = maxX;
	bvh.nodes[nodeIdx].maxY = maxY;

	int n = end - begin;
	if (n <= 4)
	{
		bvh.nodes[nodeIdx].start = static_cast<int>(bvh.ids.size());
		bvh.nodes[nodeIdx].count = n;
		for (int i = begin; i < end; i++)
		{
			bvh.ids.push_back(input[i].id);
		}
		return nodeIdx;
	}

	bool splitX = (maxX - minX) >= (maxY - minY);
	int mid = begin + n / 2;
	std::nth_element(input.data() + begin, input.data() + mid, input.data() + end, [splitX](const BvhInput& a, const BvhInput& b)
	{
		if (splitX)
		{
			return (a.minX + a.maxX) < (b.minX + b.maxX);
		}
		return (a.minY + a.maxY) < (b.minY + b.maxY);
	});

	BuildBvhNode(bvh, input, begin, mid);
	int right = BuildBvhNode(bvh, input, mid, end);
	bvh.nodes[nodeIdx].right = right;
	return nodeIdx;
}

template<typename TFn>
void ColshapeManager::QueryBvh(const Bvh& bvh, float px, float py, const TFn& fn)
{
	if (bvh.nodes.empty())
	{
		return;
	}

	int stack[128];
	int sp = 0;
	stack[sp++] = 0;

	while (sp > 0)
	{
		int nodeIdx = stack[--sp];
		const BvhNode& node = bvh.nodes[nodeIdx];

		if (px < node.minX || px > node.maxX || py < node.minY || py > node.maxY)
		{
			continue;
		}

		if (node.count > 0)
		{
			for (int i = 0; i < node.count; i++)
			{
				fn(bvh.ids[node.start + i]);
			}
		}
		else
		{
			stack[sp++] = nodeIdx + 1;
			stack[sp++] = node.right;
		}
	}
}

// builds+swaps the tree on a background thread; the current tree serves queries meanwhile
void ColshapeManager::KickBvhBuild()
{
	if (m_bvhBuiltGeneration.load() == m_bvhGeneration.load(std::memory_order_relaxed))
	{
		return;
	}

	// let mutations settle before rebuilding so a burst can't trigger back-to-back
	// builds; the staleness cap still indexes a steady trickle
	int64_t nowMs = static_cast<int64_t>(NowMs());
	bool quiet = (nowMs - m_lastMutationMs.load(std::memory_order_relaxed)) >= 150;
	bool tooStale = (nowMs - m_lastBvhKickMs.load()) >= 2000;
	if (!quiet && !tooStale)
	{
		return;
	}

	bool expected = false;
	if (!m_bvhBuilding.compare_exchange_strong(expected, true))
	{
		return;
	}

	m_lastBvhKickMs.store(nowMs);

	uint64_t buildingGen = m_bvhGeneration.load(std::memory_order_relaxed);

	auto input = std::make_shared<std::vector<BvhInput>>(m_bounds);

	// the tree covers exactly the m_recent entries that existed now; new shapes only
	// ever append, so the covered ones are this leading count (ids are recycled, so a
	// covered-max-id test would wrongly prune a recycled low id created mid-build)
	size_t coveredRecent = m_recent.size();

	std::thread([this, input, buildingGen, coveredRecent]()
	{
		auto built = BuildBvh(*input);

		{
			std::lock_guard swap(m_bvhSwapMutex);
			m_bvh = built;
		}
		m_bvhBuiltGeneration.store(buildingGen);

		// drop the m_recent prefix the new tree now covers (separate lock scope, so
		// m_mutex and the swap mutex are never held together)
		{
			std::unique_lock lock(m_mutex);
			size_t covered = std::min(coveredRecent, m_recent.size());
			m_recent.erase(m_recent.begin(), m_recent.begin() + covered);
		}

		m_bvhBuilding.store(false);
	}).detach();
}

void ColshapeManager::Update(const std::vector<EntitySample>& entities, const EventFn& emit)
{
	std::shared_lock lock(m_mutex);

	if (entities.empty() && m_entityInside.empty())
	{
		return;
	}

	KickBvhBuild();

	std::shared_ptr<Bvh> bvh;
	{
		std::lock_guard swap(m_bvhSwapMutex);
		bvh = m_bvh;
	}

	const uint64_t epoch = ++m_epoch;

	// beyond this, scanning the tail per entity per tick starves the create path, so
	// new shapes wait for the next rebuild instead
	constexpr size_t kMaxRecentScan = 8192;
	const bool scanRecent = m_recent.size() <= kMaxRecentScan;

	std::vector<ShapeRef> current;
	for (const auto& e : entities)
	{
		current.clear();

		auto test = [&](int shapeId)
		{
			const ColShape* s = Find(shapeId);
			if (s && s->AcceptsEntityType(e.type) && s->ContainsPoint(e.x, e.y, e.z))
			{
				current.push_back({ shapeId, s->generation });
			}
		};

		if (bvh)
		{
			QueryBvh(*bvh, e.x, e.y, test);
		}

		if (scanRecent)
		{
			for (const auto& r : m_recent)
			{
				if (e.x >= r.minX && e.x <= r.maxX && e.y >= r.minY && e.y <= r.maxY)
				{
					test(r.id);
				}
			}
		}

		std::sort(current.begin(), current.end());
		current.erase(std::unique(current.begin(), current.end(),
			[](const ShapeRef& a, const ShapeRef& b) { return a.id == b.id; }), current.end());

		auto stateIt = m_entityInside.find(e.handle);
		if (stateIt == m_entityInside.end())
		{
			if (current.empty())
			{
				continue;
			}
			stateIt = m_entityInside.emplace(e.handle, EntityInsideState{}).first;
		}

		auto& state = stateIt->second;
		state.epoch = epoch;

		// merge-diff vs last tick by id: only-in-new -> enter, only-in-old -> exit. a
		// same id with a bumped generation is a delete+recreate, so exit then enter.
		const auto& prev = state.shapes;
		size_t ci = 0, pi = 0;
		while (ci < current.size() || pi < prev.size())
		{
			if (pi == prev.size() || (ci < current.size() && current[ci].id < prev[pi].id))
			{
				emit("onColshapeEnter", e.handle, current[ci].id, current[ci].generation);
				ci++;
			}
			else if (ci == current.size() || prev[pi].id < current[ci].id)
			{
				emit("onColshapeExit", e.handle, prev[pi].id, prev[pi].generation);
				pi++;
			}
			else
			{
				if (current[ci].generation != prev[pi].generation)
				{
					emit("onColshapeExit", e.handle, prev[pi].id, prev[pi].generation);
					emit("onColshapeEnter", e.handle, current[ci].id, current[ci].generation);
				}
				ci++;
				pi++;
			}
		}

		state.shapes.swap(current);
		if (state.shapes.empty())
		{
			m_entityInside.erase(stateIt);
		}
	}

	// entities missing this tick despawned/disconnected -> exit them from every shape
	for (auto it = m_entityInside.begin(); it != m_entityInside.end(); )
	{
		if (it->second.epoch != epoch)
		{
			for (const ShapeRef& s : it->second.shapes)
			{
				emit("onColshapeExit", it->first, s.id, s.generation);
			}
			it = m_entityInside.erase(it);
		}
		else
		{
			++it;
		}
	}
}
}

static InitFunction initFunction([]()
{
	using namespace fx::colshape;

	auto& mgr = ColshapeManager::Get();

	// shapes die with the resource that created them
	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		std::string resourceName = resource->GetName();
		resource->OnStop.Connect([resourceName]()
		{
			ColshapeManager::Get().DeleteByOwner(resourceName);
		});
	});

	// ---- creators ----
	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_SPHERE", [](fx::ScriptContext& context)
	{
		ColShape s(ColShapeType::Sphere);
		s.pos1 = { context.GetArgument<float>(0), context.GetArgument<float>(1), context.GetArgument<float>(2) };
		s.radius = context.GetArgument<float>(3);
		context.SetResult<int>(ColshapeManager::Get().Add(std::move(s)));
	});

	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_CIRCLE", [](fx::ScriptContext& context)
	{
		ColShape s(ColShapeType::Circle);
		s.pos1 = { context.GetArgument<float>(0), context.GetArgument<float>(1), 0.0f };
		s.radius = context.GetArgument<float>(2);
		context.SetResult<int>(ColshapeManager::Get().Add(std::move(s)));
	});

	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_CYLINDER", [](fx::ScriptContext& context)
	{
		ColShape s(ColShapeType::Cylinder);
		s.pos1 = { context.GetArgument<float>(0), context.GetArgument<float>(1), context.GetArgument<float>(2) };
		s.radius = context.GetArgument<float>(3);
		s.height = context.GetArgument<float>(4);
		context.SetResult<int>(ColshapeManager::Get().Add(std::move(s)));
	});

	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_CUBOID", [](fx::ScriptContext& context)
	{
		ColShape s(ColShapeType::Cuboid);
		s.pos1 = { context.GetArgument<float>(0), context.GetArgument<float>(1), context.GetArgument<float>(2) };
		s.dimW = context.GetArgument<float>(3);
		s.dimD = context.GetArgument<float>(4);
		s.height = context.GetArgument<float>(5);
		context.SetResult<int>(ColshapeManager::Get().Add(std::move(s)));
	});

	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_RECTANGLE", [](fx::ScriptContext& context)
	{
		ColShape s(ColShapeType::Rectangle);
		s.pos1 = { context.GetArgument<float>(0), context.GetArgument<float>(1), context.GetArgument<float>(2) };
		s.dimW = context.GetArgument<float>(3);
		s.dimD = context.GetArgument<float>(4);
		s.heading = context.GetArgument<float>(5);
		context.SetResult<int>(ColshapeManager::Get().Add(std::move(s)));
	});

	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_POLYGON", [](fx::ScriptContext& context)
	{
		ColShape s(ColShapeType::Polygon);
		s.minZ = context.GetArgument<float>(0);
		s.maxZ = context.GetArgument<float>(1);
		ParsePolygonPoints(context, 2, s.points);

		if (s.points.size() < 3)
		{
			context.SetResult<int>(-1);
			return;
		}

		context.SetResult<int>(ColshapeManager::Get().Add(std::move(s)));
	});

	// ---- manage ----
	fx::ScriptEngine::RegisterNativeHandler("DELETE_COLSHAPE", [](fx::ScriptContext& context)
	{
		context.SetResult<bool>(ColshapeManager::Get().Delete(context.GetArgument<int>(0)));
	});

	fx::ScriptEngine::RegisterNativeHandler("DOES_COLSHAPE_EXIST", [](fx::ScriptContext& context)
	{
		context.SetResult<bool>(ColshapeManager::Get().Find(context.GetArgument<int>(0)) != nullptr);
	});

	auto registerIsType = [](const char* name, ColShapeType type)
	{
		fx::ScriptEngine::RegisterNativeHandler(name, [type](fx::ScriptContext& context)
		{
			auto* s = ColshapeManager::Get().Find(context.GetArgument<int>(0));
			context.SetResult<bool>(s != nullptr && s->type == type);
		});
	};
	registerIsType("IS_COLSHAPE_SPHERE", ColShapeType::Sphere);
	registerIsType("IS_COLSHAPE_CIRCLE", ColShapeType::Circle);
	registerIsType("IS_COLSHAPE_CYLINDER", ColShapeType::Cylinder);
	registerIsType("IS_COLSHAPE_CUBOID", ColShapeType::Cuboid);
	registerIsType("IS_COLSHAPE_RECTANGLE", ColShapeType::Rectangle);
	registerIsType("IS_COLSHAPE_POLYGON", ColShapeType::Polygon);

	fx::ScriptEngine::RegisterNativeHandler("IS_POINT_INSIDE_COLSHAPE", [](fx::ScriptContext& context)
	{
		auto* s = ColshapeManager::Get().Find(context.GetArgument<int>(0));
		bool inside = s != nullptr && s->ContainsPoint(context.GetArgument<float>(1), context.GetArgument<float>(2), context.GetArgument<float>(3));
		context.SetResult<bool>(inside);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_COLSHAPE_ENTITY_TYPE", [](fx::ScriptContext& context)
	{
		ColshapeManager::Get().SetEntityType(context.GetArgument<int>(0), context.GetArgument<int>(1), context.GetArgument<bool>(2));
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_COLSHAPE_ENTITY_TYPE_SET", [](fx::ScriptContext& context)
	{
		auto* s = ColshapeManager::Get().Find(context.GetArgument<int>(0));
		context.SetResult<bool>(s != nullptr && s->AcceptsEntityType(context.GetArgument<int>(1)));
	});

	// getters return ok + float* out-params (Lua: ok, out1, ..), zeroed on failure
	fx::ScriptEngine::RegisterNativeHandler("GET_COLSHAPE_SPHERE_DATA", [](fx::ScriptContext& context)
	{
		auto* s = ColshapeManager::Get().Find(context.GetArgument<int>(0));
		bool ok = s != nullptr && s->type == ColShapeType::Sphere;
		*context.GetArgument<float*>(1) = ok ? s->pos1.x : 0.0f;
		*context.GetArgument<float*>(2) = ok ? s->pos1.y : 0.0f;
		*context.GetArgument<float*>(3) = ok ? s->pos1.z : 0.0f;
		*context.GetArgument<float*>(4) = ok ? s->radius : 0.0f;
		context.SetResult<bool>(ok);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_COLSHAPE_CIRCLE_DATA", [](fx::ScriptContext& context)
	{
		auto* s = ColshapeManager::Get().Find(context.GetArgument<int>(0));
		bool ok = s != nullptr && s->type == ColShapeType::Circle;
		*context.GetArgument<float*>(1) = ok ? s->pos1.x : 0.0f;
		*context.GetArgument<float*>(2) = ok ? s->pos1.y : 0.0f;
		*context.GetArgument<float*>(3) = ok ? s->radius : 0.0f;
		context.SetResult<bool>(ok);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_COLSHAPE_CYLINDER_DATA", [](fx::ScriptContext& context)
	{
		auto* s = ColshapeManager::Get().Find(context.GetArgument<int>(0));
		bool ok = s != nullptr && s->type == ColShapeType::Cylinder;
		*context.GetArgument<float*>(1) = ok ? s->pos1.x : 0.0f;
		*context.GetArgument<float*>(2) = ok ? s->pos1.y : 0.0f;
		*context.GetArgument<float*>(3) = ok ? s->pos1.z : 0.0f;
		*context.GetArgument<float*>(4) = ok ? s->radius : 0.0f;
		*context.GetArgument<float*>(5) = ok ? s->height : 0.0f;
		context.SetResult<bool>(ok);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_COLSHAPE_CUBOID_DATA", [](fx::ScriptContext& context)
	{
		auto* s = ColshapeManager::Get().Find(context.GetArgument<int>(0));
		bool ok = s != nullptr && s->type == ColShapeType::Cuboid;
		*context.GetArgument<float*>(1) = ok ? s->pos1.x : 0.0f;
		*context.GetArgument<float*>(2) = ok ? s->pos1.y : 0.0f;
		*context.GetArgument<float*>(3) = ok ? s->pos1.z : 0.0f;
		*context.GetArgument<float*>(4) = ok ? s->dimW : 0.0f;
		*context.GetArgument<float*>(5) = ok ? s->dimD : 0.0f;
		*context.GetArgument<float*>(6) = ok ? s->height : 0.0f;
		context.SetResult<bool>(ok);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_COLSHAPE_RECTANGLE_DATA", [](fx::ScriptContext& context)
	{
		auto* s = ColshapeManager::Get().Find(context.GetArgument<int>(0));
		bool ok = s != nullptr && s->type == ColShapeType::Rectangle;
		*context.GetArgument<float*>(1) = ok ? s->pos1.x : 0.0f;
		*context.GetArgument<float*>(2) = ok ? s->pos1.y : 0.0f;
		*context.GetArgument<float*>(3) = ok ? s->pos1.z : 0.0f;
		*context.GetArgument<float*>(4) = ok ? s->dimW : 0.0f;
		*context.GetArgument<float*>(5) = ok ? s->dimD : 0.0f;
		*context.GetArgument<float*>(6) = ok ? s->heading : 0.0f;
		context.SetResult<bool>(ok);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_COLSHAPE_POLYGON_DATA", [](fx::ScriptContext& context)
	{
		auto* s = ColshapeManager::Get().Find(context.GetArgument<int>(0));
		bool ok = s != nullptr && s->type == ColShapeType::Polygon;
		*context.GetArgument<float*>(1) = ok ? s->minZ : 0.0f;
		*context.GetArgument<float*>(2) = ok ? s->maxZ : 0.0f;
		context.SetResult<bool>(ok);
	});
});
