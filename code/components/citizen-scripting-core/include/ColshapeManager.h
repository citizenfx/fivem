#pragma once

#include <atomic>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "ComponentExport.h"

namespace fx::colshape
{
enum class ColShapeType
{
	Circle,
	Cuboid,
	Cylinder,
	Rectangle,
	Sphere,
	Polygon,
};


struct Vec3
{
	float x = 0.f, y = 0.f, z = 0.f;
};

struct Vec2
{
	float x = 0.f, y = 0.f;
};

struct EntitySample
{
	int handle;
	int type;
	float x, y, z;
};

struct ColShape
{
	int id = -1;

	// bumped every time this id is reused, so tracked (id, generation) pairs from a
	// previous shape don't alias a new shape that happens to reuse the freed id
	uint32_t generation = 0;

	ColShapeType type;

	// resource that created this shape, so it can be dropped when that resource stops
	std::string owner;

	Vec3 pos1;
	float radius = 0.f;
	float radiusSq = 0.f;
	float height = 0.f;
	float dimW = 0.f;
	float dimD = 0.f;
	float heading = 0.f;
	float cosHeading = 1.f;
	float sinHeading = 0.f;
	float minZ = 0.f, maxZ = 0.f;
	std::vector<Vec2> points;

	float minX = 0.f, maxX = 0.f, minY = 0.f, maxY = 0.f;

	// bit per NetObjEntityType; a set bit excludes that type from detection
	uint64_t disabledEntityTypes = 0;

	explicit ColShape(ColShapeType t)
		: type(t)
	{
	}

	void SetEntityType(int type, bool value)
	{
		if (type < 0 || type >= 64)
		{
			return;
		}

		if (value)
		{
			disabledEntityTypes &= ~(1ull << type);
		}
		else
		{
			disabledEntityTypes |= (1ull << type);
		}
	}

	bool AcceptsEntityType(int type) const
	{
		if (type < 0 || type >= 64)
		{
			return true;
		}

		return (disabledEntityTypes & (1ull << type)) == 0;
	}

	void ComputeBounds()
	{
		radiusSq = radius * radius;

		// abs() so a negative radius/dimension can't invert the AABB
		switch (type)
		{
			case ColShapeType::Circle:
			case ColShapeType::Cylinder:
			case ColShapeType::Sphere:
			{
				float r = std::fabs(radius);
				minX = pos1.x - r;
				maxX = pos1.x + r;
				minY = pos1.y - r;
				maxY = pos1.y + r;
				break;
			}
			case ColShapeType::Cuboid:
			{
				float hw = std::fabs(dimW) * 0.5f;
				float hd = std::fabs(dimD) * 0.5f;
				minX = pos1.x - hw;
				maxX = pos1.x + hw;
				minY = pos1.y - hd;
				maxY = pos1.y + hd;
				break;
			}
			case ColShapeType::Rectangle:
			{
				cosHeading = std::cos(heading);
				sinHeading = std::sin(heading);

				// exact rotated-rect AABB (tighter than a circumcircle bound)
				float hw = std::fabs(dimW) * 0.5f;
				float hd = std::fabs(dimD) * 0.5f;
				float ex = std::fabs(cosHeading) * hw + std::fabs(sinHeading) * hd;
				float ey = std::fabs(sinHeading) * hw + std::fabs(cosHeading) * hd;
				minX = pos1.x - ex;
				maxX = pos1.x + ex;
				minY = pos1.y - ey;
				maxY = pos1.y + ey;
				break;
			}
			case ColShapeType::Polygon:
			{
				if (points.empty())
				{
					minX = maxX = pos1.x;
					minY = maxY = pos1.y;
					break;
				}

				minX = maxX = points[0].x;
				minY = maxY = points[0].y;
				for (const auto& p : points)
				{
					minX = std::min(minX, p.x);
					maxX = std::max(maxX, p.x);
					minY = std::min(minY, p.y);
					maxY = std::max(maxY, p.y);
				}
				break;
			}
		}
	}

	bool ContainsPoint(float px, float py, float pz) const
	{
		switch (type)
		{
			case ColShapeType::Circle:
			{
				float dx = px - pos1.x, dy = py - pos1.y;
				return dx * dx + dy * dy <= radiusSq;
			}
			case ColShapeType::Sphere:
			{
				float dx = px - pos1.x, dy = py - pos1.y, dz = pz - pos1.z;
				return dx * dx + dy * dy + dz * dz <= radiusSq;
			}
			case ColShapeType::Cylinder:
			{
				float dz = pz - pos1.z;
				if (dz < -height * 0.5f || dz > height * 0.5f)
				{
					return false;
				}

				float dx = px - pos1.x, dy = py - pos1.y;
				return dx * dx + dy * dy <= radiusSq;
			}
			case ColShapeType::Cuboid:
			{
				return px >= pos1.x - dimW * 0.5f && px <= pos1.x + dimW * 0.5f
					&& py >= pos1.y - dimD * 0.5f && py <= pos1.y + dimD * 0.5f
					&& pz >= pos1.z - height * 0.5f && pz <= pos1.z + height * 0.5f;
			}
			case ColShapeType::Rectangle:
			{
				float dx = px - pos1.x, dy = py - pos1.y;
				float lx = dx * cosHeading + dy * sinHeading;
				float ly = -dx * sinHeading + dy * cosHeading;
				return lx >= -dimW * 0.5f && lx <= dimW * 0.5f
					&& ly >= -dimD * 0.5f && ly <= dimD * 0.5f;
			}
			case ColShapeType::Polygon:
			{
				if (pz < minZ || pz > maxZ)
				{
					return false;
				}

				return PointInPoly(px, py);
			}
		}
		return false;
	}

	bool PointInPoly(float px, float py) const
	{
		// ray-cast even-odd rule
		bool in = false;
		size_t n = points.size();
		for (size_t i = 0, j = n - 1; i < n; j = i++)
		{
			const auto& a = points[i];
			const auto& b = points[j];
			if (((a.y > py) != (b.y > py)) &&
				(px < (b.x - a.x) * (py - a.y) / (b.y - a.y) + a.x))
			{
				in = !in;
			}
		}
		return in;
	}
};

class ColshapeManager
{
public:
	// generation lets the feed drop an event whose shape id was freed and reused
	// between detection and emission
	using EventFn = std::function<void(const char* event, int entity, int shape, uint32_t generation)>;

	COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE) static ColshapeManager& Get();

	int Add(ColShape&& shape);
	bool Delete(int id);

	// the returned pointer is only valid while no writer runs. callers on the script
	// thread (the native handlers) are safe because Add/Delete/SetEntityType also run
	// there; the detection worker calls this under m_mutex (see Update). do not call
	// from any other thread without holding m_mutex.
	COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE) ColShape* Find(int id);
	COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE) const ColShape* Find(int id) const;

	// true if id currently maps to a live shape with this exact generation
	COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE) bool IsLive(int id, uint32_t generation) const;

	void SetEntityType(int id, int type, bool value);
	void DeleteByOwner(const std::string& owner);

	COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE) void Update(const std::vector<EntitySample>& entities, const EventFn& emit);

private:
	struct BvhNode
	{
		float minX, minY, maxX, maxY;

		int32_t right = 0;
		int32_t start = 0;
		int32_t count = 0;
	};

	// self-contained (nodes own their bounds, leaves store shape ids) so it can be
	// built on a background thread and swapped in without touching manager state
	struct Bvh
	{
		std::vector<BvhNode> nodes;
		std::vector<int> ids;
	};

	struct BvhInput
	{
		int id;
		float minX, minY, maxX, maxY;
	};

	static std::shared_ptr<Bvh> BuildBvh(std::vector<BvhInput>& input);
	static int BuildBvhNode(Bvh& bvh, std::vector<BvhInput>& input, int begin, int end);

	template<typename TFn>
	static void QueryBvh(const Bvh& bvh, float px, float py, const TFn& fn);

	void KickBvhBuild();

	// an id plus the generation the shape had when it was recorded; the pair detects
	// an id that has since been freed and reused for a different shape
	struct ShapeRef
	{
		int id;
		uint32_t generation;

		bool operator<(const ShapeRef& o) const { return id < o.id; }
	};

	// sorted by id so the per-tick merge-diff is one pass and enter/exit order is stable
	struct EntityInsideState
	{
		uint64_t epoch = 0;
		std::vector<ShapeRef> shapes;
	};

	int m_nextId = 0;
	uint64_t m_epoch = 0;

	// freed ids waiting to be reused (each reuse bumps that id's generation), so the
	// id space is recycled instead of exhausting at 65535 under create/delete churn
	std::vector<int> m_freeIds;
	std::vector<uint32_t> m_generations;

	// chunked so growth never relocates existing shapes (Find pointers stay stable);
	// ids resolve through a flat id -> slot table (-1 = deleted)
	static constexpr size_t kShapeChunkShift = 13;
	static constexpr size_t kShapeChunkSize = size_t(1) << kShapeChunkShift;

	std::vector<std::vector<ColShape>> m_shapes;
	size_t m_shapeCount = 0;
	std::vector<int32_t> m_idToSlot;

	ColShape& ShapeAt(size_t slot)
	{
		return m_shapes[slot >> kShapeChunkShift][slot & (kShapeChunkSize - 1)];
	}

	const ColShape& ShapeAt(size_t slot) const
	{
		return m_shapes[slot >> kShapeChunkShift][slot & (kShapeChunkSize - 1)];
	}

	// (id, bounds) in lockstep with m_shapes, so the BVH snapshot is a vector copy
	std::vector<BvhInput> m_bounds;

	// shapes newer than the current tree; scanned alongside it so they're detectable
	// before the next rebuild
	std::vector<BvhInput> m_recent;

	// mismatched generations => the tree is stale and a rebuild is due
	std::shared_ptr<Bvh> m_bvh;
	std::atomic<uint64_t> m_bvhGeneration{ 0 };
	std::atomic<uint64_t> m_bvhBuiltGeneration{ 0 };
	std::atomic<bool> m_bvhBuilding{ false };
	std::atomic<int64_t> m_lastBvhKickMs{ 0 };
	std::atomic<int64_t> m_lastMutationMs{ 0 };
	std::mutex m_bvhSwapMutex;

	std::unordered_map<int, EntityInsideState> m_entityInside;
	mutable std::shared_mutex m_mutex;
};
}
