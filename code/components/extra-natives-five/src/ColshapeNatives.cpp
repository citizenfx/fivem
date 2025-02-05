#include <StdInc.h>
#include <ScriptEngine.h>
#include <Resource.h>
#include <DisableChat.h>
#include <nutsnbolts.h>
#include <ICoreGameInit.h>
#include <ResourceManager.h>
#include <EntitySystem.h>
#include <ResourceEventComponent.h>
#include <ResourceCallbackComponent.h>
#include <chrono>
#include <scrEngine.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <memory>

#include <locale>
#include <codecvt>
#include <string>
#include <stdexcept>

// We are using a grid-based collision detection system instead of quadtrees because:
// - Insertion Speed: Adding 100,000 shapes took only ~0.085 seconds with the grid,
//   whereas quadtrees were significantly slower. (45 seconds+, maybe it was my implementation though)
// - Sufficient Query Performance: Although quadtrees offer faster lookup times, the grid was very close and is "good enough" in my opinion.

// Define constants/convars
#include <CoreConsole.h>


#ifdef GTA_FIVE
	static ConVar<float> colShapeMinBound("colShapeMinBound", ConVar_Archive, -10000.0f);
	static ConVar<float> colShapeMaxBound("colShapeMaxBound", ConVar_Archive, 10000.0f);
#elif defined(IS_RDR3) // rdr3 map is lager than gta5
	static ConVar<float> colShapeMinBound("colShapeMinBound", ConVar_Archive, -15000.0f);
	static ConVar<float> colShapeMaxBound("colShapeMaxBound", ConVar_Archive, 15000.0f);
#else
	static ConVar<float> colShapeMinBound("colShapeMinBound", ConVar_Archive, -10000.0f);
	static ConVar<float> colShapeMaxBound("colShapeMaxBound", ConVar_Archive, 10000.0f);
#endif


static ConVar<float> colShapeCellSize("colShapeCellSize", ConVar_Archive, 500.0f);
static ConVar<float> colShapeAutoInfiniteThreshold("colShapeAutoInfiniteThreshold", ConVar_Archive, 1000.0f);
static ConVar<float> colShapeMaxInfiniteShapeDistance("colShapeMaxInfiniteShapeDistance", ConVar_Archive, 1000.0f);

// cant be constexpr anymore but shouldnt matter
static float CELL_SIZE = colShapeCellSize.GetValue();
static float AUTO_INFINITE_THRESHOLD = colShapeAutoInfiniteThreshold.GetValue(); // Threshold to mark shapes as infinite, if they're bigger than this we move them to a seperate table
static float MAX_INFINITE_SHAPE_DISTANCE = colShapeMaxInfiniteShapeDistance.GetValue(); // Maximum distance for infinite shapes, essentially at what "range" they should get ignored

// -------------------------------------------------------------------------
// Basic Vector2 type
struct Vector2
{
	float x;
	float y;
	Vector2()
		: x(0.f), y(0.f)
	{
	}
	Vector2(float x, float y)
		: x(x), y(y)
	{
	}
};

// Enumerate collision shape types
enum class ColShapeType
{
	Circle, // 2D circle
	Cube, // 3D cube
	Cylinder, // Cylinder shape
	Rectangle, // 2D rectangle (with bottomZ + height in Z)
	Sphere, // 3D sphere
	Polyzone, // 2D polygon with minZ/maxZ
};

// -------------------------------------------------------------------------
// The main colshape data
struct ColShape
{
	int id; // auto-generated integer ID
	ColShapeType type;
	Vector3 pos1; // center or first corner
	Vector3 pos2; // second corner for cubes/rectangles
	float radius; // circle, cylinder, sphere radius
	float height; // cylinder, rectangle, or polyzone height
	bool infinite; // whether to skip the grid
	float maxDistance; // maximum distance for collision checks

	// bounding box in X and Y
	float minX, maxX;
	float minY, maxY;

	std::vector<Vector2> points; // used by polyzones

	// constructor
	ColShape(int shapeId, ColShapeType shapeType)
		: id(shapeId),
		  type(shapeType),
		  pos1({ 0, 0, 0 }),
		  pos2({ 0, 0, 0 }),
		  radius(0.0f),
		  height(0.0f),
		  infinite(false),
		  maxDistance(MAX_INFINITE_SHAPE_DISTANCE),
		  minX(0), maxX(0),
		  minY(0), maxY(0),
		  points()
	{
	}
};

// -------------------------------------------------------------------------
// Class to manage collision shapes using an optimized grid-based approach
class ColShapeManager
{
public:
	// Singleton accessor
	static ColShapeManager& Get()
	{
		static ColShapeManager instance;
		return instance;
	}

	ColShapeManager(const ColShapeManager&) = delete;
	ColShapeManager& operator=(const ColShapeManager&) = delete;

	// Creation methods
	int CreateCircle(const Vector3& center, float radius);
	int CreateCube(const Vector3& pos1, const Vector3& pos2);
	int CreateCylinder(const Vector3& center, float radius, float height);
	int CreateRectangleZ(float x1, float y1, float x2, float y2, float bottomZ, float height);
	int CreateRectangle(float x1, float y1, float x2, float y2, float height); // alias
	int CreateSphere(const Vector3& center, float radius);
	int CreatePolyzone(const std::vector<Vector2>& points, float minZ, float maxZ);

	bool DeleteColShape(int colShapeId);
	bool Exists(int colShapeId);

	void SetEnabled(bool enabled)
	{
		enabled_ = enabled;
	}
	void SetUpdateInterval(int intervalMs)
	{
		if (intervalMs < 0)
			intervalMs = 0;
		updateIntervalMs_ = intervalMs;
	}

	bool isEnabled() const
	{
		return enabled_;
	}

	int GetUpdateInterval() const
	{
		return updateIntervalMs_;
	}
	

	inline void Update();

private:
	ColShapeManager();

	// Helpers
	void MaybeMarkInfinite(ColShape& shape);
	void AddToGrid(ColShape* shape);
	std::vector<ColShape*> GetColShapesForPosition(const Vector3& pos);
	inline bool IsPointInColShape(const Vector3& p, const ColShape& shape);

	// data
	std::unordered_map<int, std::unique_ptr<ColShape>> colShapes_;
	std::vector<ColShape*> infiniteShapes_;

	int minCx_, maxCx_, minCy_, maxCy_;
	int numCellsX_, numCellsY_;
	std::vector<std::vector<ColShape*>> grid_; // [cx * numCellsY + cy] -> shapes

	std::unordered_set<ColShape*> playerInsideColShapes_;

	int nextId_;
	bool enabled_ = false; // default is false
	int updateIntervalMs_ = 100; // 100ms update interval
};

ColShapeManager::ColShapeManager()
	: nextId_(1)
{
	#ifdef GTA_FIVE
		// Use the ConVar values instead of the fixed -10000 / 10000
		minCx_ = static_cast<int>(std::floor(static_cast<float>(colShapeMinBound.GetValue()) / CELL_SIZE));
		maxCx_ = static_cast<int>(std::floor(static_cast<float>(colShapeMaxBound.GetValue()) / CELL_SIZE));
		minCy_ = static_cast<int>(std::floor(static_cast<float>(colShapeMinBound.GetValue()) / CELL_SIZE));
		maxCy_ = static_cast<int>(std::floor(static_cast<float>(colShapeMaxBound.GetValue()) / CELL_SIZE));
	#elif defined(IS_RDR3)
		// If you used different ConVar defaults for RDR3,
		// they'll come from colShapeMinBound, colShapeMaxBound as well
		minCx_ = static_cast<int>(std::floor(static_cast<float>(colShapeMinBound.GetValue()) / CELL_SIZE));
		maxCx_ = static_cast<int>(std::floor(static_cast<float>(colShapeMaxBound.GetValue()) / CELL_SIZE));
		minCy_ = static_cast<int>(std::floor(static_cast<float>(colShapeMinBound.GetValue()) / CELL_SIZE));
		maxCy_ = static_cast<int>(std::floor(static_cast<float>(colShapeMaxBound.GetValue()) / CELL_SIZE));
	#else
		// fallback if needed
		minCx_ = static_cast<int>(std::floor(static_cast<float>(colShapeMinBound.GetValue()) / CELL_SIZE));
		maxCx_ = static_cast<int>(std::floor(static_cast<float>(colShapeMaxBound.GetValue()) / CELL_SIZE));
		minCy_ = static_cast<int>(std::floor(static_cast<float>(colShapeMinBound.GetValue()) / CELL_SIZE));
		maxCy_ = static_cast<int>(std::floor(static_cast<float>(colShapeMaxBound.GetValue()) / CELL_SIZE));
	#endif
	numCellsX_ = maxCx_ - minCx_ + 1;
	numCellsY_ = maxCy_ - minCy_ + 1;
	grid_.resize(numCellsX_ * numCellsY_);
}

// -------------------------------------------------------------------------
// Check if ID exists
bool ColShapeManager::Exists(int colShapeId)
{
	auto it = colShapes_.find(colShapeId);
	return (it != colShapes_.end());
}

// -------------------------------------------------------------------------
// Helper: mark infinite if bounding box is huge
void ColShapeManager::MaybeMarkInfinite(ColShape& shape)
{
	float width = shape.maxX - shape.minX;
	float height = shape.maxY - shape.minY;

	if (width >= AUTO_INFINITE_THRESHOLD || height >= AUTO_INFINITE_THRESHOLD)
	{
		shape.infinite = true;
	}
}

// -------------------------------------------------------------------------
void ColShapeManager::AddToGrid(ColShape* shape)
{
	int startCx = static_cast<int>(std::floor(shape->minX / CELL_SIZE));
	int endCx = static_cast<int>(std::floor(shape->maxX / CELL_SIZE));
	int startCy = static_cast<int>(std::floor(shape->minY / CELL_SIZE));
	int endCy = static_cast<int>(std::floor(shape->maxY / CELL_SIZE));

	// clamp
	startCx = std::max(startCx, minCx_);
	endCx = std::min(endCx, maxCx_);
	startCy = std::max(startCy, minCy_);
	endCy = std::min(endCy, maxCy_);

	for (int cx = startCx; cx <= endCx; ++cx)
	{
		for (int cy = startCy; cy <= endCy; ++cy)
		{
			int index = (cx - minCx_) * numCellsY_ + (cy - minCy_);
			if (index >= 0 && index < (int)grid_.size())
			{
				grid_[index].push_back(shape);
			}
		}
	}
}

// -------------------------------------------------------------------------
// Create Circle
int ColShapeManager::CreateCircle(const Vector3& center, float radius)
{
	// You could decide to fail if radius <= 0, etc.
	// if (radius <= 0.f) return -1;

	int newId = nextId_++;
	auto shape = std::make_unique<ColShape>(newId, ColShapeType::Circle);
	shape->pos1 = center;
	shape->radius = radius;

	shape->minX = center.x - radius;
	shape->maxX = center.x + radius;
	shape->minY = center.y - radius;
	shape->maxY = center.y + radius;

	MaybeMarkInfinite(*shape);

	ColShape* rawPtr = shape.get();
	colShapes_.emplace(newId, std::move(shape));

	if (!rawPtr->infinite)
	{
		AddToGrid(rawPtr);
	}
	else
	{
		infiniteShapes_.push_back(rawPtr);
	}

	return newId;
}

// -------------------------------------------------------------------------
// Create Cube
int ColShapeManager::CreateCube(const Vector3& pos1, const Vector3& pos2)
{
	int newId = nextId_++;
	auto shape = std::make_unique<ColShape>(newId, ColShapeType::Cube);
	shape->pos1 = pos1;
	shape->pos2 = pos2;

	shape->minX = std::min(pos1.x, pos2.x);
	shape->maxX = std::max(pos1.x, pos2.x);
	shape->minY = std::min(pos1.y, pos2.y);
	shape->maxY = std::max(pos1.y, pos2.y);

	MaybeMarkInfinite(*shape);

	ColShape* rawPtr = shape.get();
	colShapes_.emplace(newId, std::move(shape));

	if (!rawPtr->infinite)
	{
		AddToGrid(rawPtr);
	}
	else
	{
		infiniteShapes_.push_back(rawPtr);
	}

	return newId;
}

// -------------------------------------------------------------------------
// Create Cylinder
int ColShapeManager::CreateCylinder(const Vector3& center, float radius, float height)
{
	int newId = nextId_++;
	auto shape = std::make_unique<ColShape>(newId, ColShapeType::Cylinder);
	shape->pos1 = center;
	shape->radius = radius;
	shape->height = height;

	shape->minX = center.x - radius;
	shape->maxX = center.x + radius;
	shape->minY = center.y - radius;
	shape->maxY = center.y + radius;

	MaybeMarkInfinite(*shape);

	ColShape* rawPtr = shape.get();
	colShapes_.emplace(newId, std::move(shape));

	if (!rawPtr->infinite)
	{
		AddToGrid(rawPtr);
	}
	else
	{
		infiniteShapes_.push_back(rawPtr);
	}

	return newId;
}

// -------------------------------------------------------------------------
// Create Rectangle w/ explicit bottomZ
int ColShapeManager::CreateRectangleZ(float x1, float y1, float x2, float y2, float bottomZ, float height)
{
	int newId = nextId_++;
	auto shape = std::make_unique<ColShape>(newId, ColShapeType::Rectangle);
	shape->pos1 = { x1, y1, bottomZ };
	shape->pos2 = { x2, y2, bottomZ };
	shape->height = height;

	shape->minX = std::min(x1, x2);
	shape->maxX = std::max(x1, x2);
	shape->minY = std::min(y1, y2);
	shape->maxY = std::max(y1, y2);

	MaybeMarkInfinite(*shape);

	ColShape* rawPtr = shape.get();
	colShapes_.emplace(newId, std::move(shape));

	if (!rawPtr->infinite)
	{
		AddToGrid(rawPtr);
	}
	else
	{
		infiniteShapes_.push_back(rawPtr);
	}
	return newId;
}

// -------------------------------------------------------------------------
// Create Rectangle w/ default bottomZ=0
int ColShapeManager::CreateRectangle(float x1, float y1, float x2, float y2, float height)
{
	// just call CreateRectangleZ with bottomZ = 0
	return CreateRectangleZ(x1, y1, x2, y2, 0.0f, height);
}

// -------------------------------------------------------------------------
// Create Sphere
int ColShapeManager::CreateSphere(const Vector3& center, float radius)
{
	int newId = nextId_++;
	auto shape = std::make_unique<ColShape>(newId, ColShapeType::Sphere);
	shape->pos1 = center;
	shape->radius = radius;

	shape->minX = center.x - radius;
	shape->maxX = center.x + radius;
	shape->minY = center.y - radius;
	shape->maxY = center.y + radius;

	MaybeMarkInfinite(*shape);

	ColShape* rawPtr = shape.get();
	colShapes_.emplace(newId, std::move(shape));

	if (!rawPtr->infinite)
	{
		AddToGrid(rawPtr);
	}
	else
	{
		infiniteShapes_.push_back(rawPtr);
	}
	return newId;
}

// -------------------------------------------------------------------------
// Create Polyzone
int ColShapeManager::CreatePolyzone(const std::vector<Vector2>& points, float minZ, float maxZ)
{
	if (points.empty())
	{
		// no shape
		return -1;
	}

	int newId = nextId_++;
	auto shape = std::make_unique<ColShape>(newId, ColShapeType::Polyzone);
	shape->points = points;

	// store minZ in pos1.z, then store height as (maxZ - minZ)
	shape->pos1.z = minZ;
	shape->height = (maxZ - minZ);

	float minX = points[0].x;
	float maxX = points[0].x;
	float minY = points[0].y;
	float maxY = points[0].y;

	for (const auto& pt : points)
	{
		minX = std::min(minX, pt.x);
		maxX = std::max(maxX, pt.x);
		minY = std::min(minY, pt.y);
		maxY = std::max(maxY, pt.y);
	}

	shape->minX = minX;
	shape->maxX = maxX;
	shape->minY = minY;
	shape->maxY = maxY;

	MaybeMarkInfinite(*shape);

	ColShape* rawPtr = shape.get();
	colShapes_.emplace(newId, std::move(shape));

	if (!rawPtr->infinite)
	{
		AddToGrid(rawPtr);
	}
	else
	{
		infiniteShapes_.push_back(rawPtr);
	}

	return newId;
}

// -------------------------------------------------------------------------
// Delete a shape by ID
bool ColShapeManager::DeleteColShape(int colShapeId)
{
	auto it = colShapes_.find(colShapeId);
	if (it == colShapes_.end())
	{
		return false;
	}

	ColShape* shapePtr = it->second.get();

	if (shapePtr->infinite)
	{
		auto eraseIt = std::find(infiniteShapes_.begin(), infiniteShapes_.end(), shapePtr);
		if (eraseIt != infiniteShapes_.end())
		{
			infiniteShapes_.erase(eraseIt);
		}
	}
	else
	{
		int startCx = static_cast<int>(std::floor(shapePtr->minX / CELL_SIZE));
		int endCx = static_cast<int>(std::floor(shapePtr->maxX / CELL_SIZE));
		int startCy = static_cast<int>(std::floor(shapePtr->minY / CELL_SIZE));
		int endCy = static_cast<int>(std::floor(shapePtr->maxY / CELL_SIZE));

		startCx = std::max(startCx, minCx_);
		endCx = std::min(endCx, maxCx_);
		startCy = std::max(startCy, minCy_);
		endCy = std::min(endCy, maxCy_);

		for (int cx = startCx; cx <= endCx; ++cx)
		{
			for (int cy = startCy; cy <= endCy; ++cy)
			{
				int index = (cx - minCx_) * numCellsY_ + (cy - minCy_);
				if (index >= 0 && index < (int)grid_.size())
				{
					auto& cellShapes = grid_[index];
					cellShapes.erase(
					std::remove(cellShapes.begin(), cellShapes.end(), shapePtr),
					cellShapes.end());
				}
			}
		}
	}

	playerInsideColShapes_.erase(shapePtr);
	colShapes_.erase(it);
	return true;
}

// -------------------------------------------------------------------------
// Return shapes from the relevant grid cell
std::vector<ColShape*> ColShapeManager::GetColShapesForPosition(const Vector3& pos)
{
	std::vector<ColShape*> result;
	int cx = static_cast<int>(std::floor(pos.x / CELL_SIZE));
	int cy = static_cast<int>(std::floor(pos.y / CELL_SIZE));

	if (cx < minCx_ || cx > maxCx_ || cy < minCy_ || cy > maxCy_)
		return result;

	int index = (cx - minCx_) * numCellsY_ + (cy - minCy_);
	if (index >= 0 && index < (int)grid_.size())
	{
		const auto& cellShapes = grid_[index];
		result.reserve(cellShapes.size());
		for (auto shape : cellShapes)
		{
			result.push_back(shape);
		}
	}
	return result;
}

// -------------------------------------------------------------------------
// Check if a point is inside a given shape
inline bool ColShapeManager::IsPointInColShape(const Vector3& p, const ColShape& shape)
{
	switch (shape.type)
	{
		case ColShapeType::Circle:
		{
			float dx = p.x - shape.pos1.x;
			float dy = p.y - shape.pos1.y;
			return (dx * dx + dy * dy) <= (shape.radius * shape.radius);
		}
		case ColShapeType::Cube:
		{
			float minX = std::min(shape.pos1.x, shape.pos2.x);
			float maxX = std::max(shape.pos1.x, shape.pos2.x);
			float minY = std::min(shape.pos1.y, shape.pos2.y);
			float maxY = std::max(shape.pos1.y, shape.pos2.y);
			float minZ = std::min(shape.pos1.z, shape.pos2.z);
			float maxZ = std::max(shape.pos1.z, shape.pos2.z);

			return (p.x >= minX && p.x <= maxX && p.y >= minY && p.y <= maxY && p.z >= minZ && p.z <= maxZ);
		}
		case ColShapeType::Cylinder:
		{
			float dx = p.x - shape.pos1.x;
			float dy = p.y - shape.pos1.y;
			if ((dx * dx + dy * dy) > (shape.radius * shape.radius))
				return false;

			float bottomZ = shape.pos1.z;
			float topZ = shape.pos1.z + shape.height;
			if (topZ < bottomZ)
				std::swap(topZ, bottomZ);

			return (p.z >= bottomZ && p.z <= topZ);
		}
		case ColShapeType::Rectangle:
		{
			float minX = std::min(shape.pos1.x, shape.pos2.x);
			float maxX = std::max(shape.pos1.x, shape.pos2.x);
			float minY = std::min(shape.pos1.y, shape.pos2.y);
			float maxY = std::max(shape.pos1.y, shape.pos2.y);

			float bottomZ = shape.pos1.z;
			float topZ = shape.pos1.z + shape.height;
			if (topZ < bottomZ)
				std::swap(topZ, bottomZ);

			bool inside2D = (p.x >= minX && p.x <= maxX && p.y >= minY && p.y <= maxY);
			bool insideZ = (p.z >= bottomZ && p.z <= topZ);
			return (inside2D && insideZ);
		}
		case ColShapeType::Sphere:
		{
			float dx = p.x - shape.pos1.x;
			float dy = p.y - shape.pos1.y;
			float dz = p.z - shape.pos1.z;
			float distSq = dx * dx + dy * dy + dz * dz;
			float rSq = shape.radius * shape.radius;
			return distSq <= rSq;
		}
		case ColShapeType::Polyzone:
		{
			float bottomZ = shape.pos1.z;
			float topZ = bottomZ + shape.height;
			if (p.z < bottomZ || p.z > topZ)
				return false;

			const auto& poly = shape.points;
			bool inside = false;
			for (size_t i = 0, j = poly.size() - 1; i < poly.size(); j = i++)
			{
				bool intersect = ((poly[i].y > p.y) != (poly[j].y > p.y)) && (p.x < (poly[j].x - poly[i].x) * (p.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x);
				if (intersect)
					inside = !inside;
			}
			return inside;
		}
		default:
			return false;
	}
}

// -------------------------------------------------------------------------
// Main update
inline void ColShapeManager::Update()
{
	static auto resman = Instance<fx::ResourceManager>::Get();
	if (!resman)
		return;

	static auto rec = resman->GetComponent<fx::ResourceEventManagerComponent>();

#ifdef GTA_FIVE
	constexpr uint64_t HASH_PLAYER_PED_ID = 0xD80958FC74E988A6;
	constexpr uint64_t HASH_GET_ENTITY_COORDS = 0x3FEF770D40960D5A;
#elif defined(IS_RDR3)
	constexpr uint64_t HASH_PLAYER_PED_ID = 0xC190F27E12443814;
	constexpr uint64_t HASH_GET_ENTITY_COORDS = 0xA86D5F069399F44D;
#endif

	static auto getPlayerPed = fx::ScriptEngine::GetNativeHandler(HASH_PLAYER_PED_ID);
	static auto getEntityCoords = fx::ScriptEngine::GetNativeHandler(HASH_GET_ENTITY_COORDS);

	int playerPedId = FxNativeInvoke::Invoke<int>(getPlayerPed);
	if (playerPedId == 0)
		return;

	scrVector coords = FxNativeInvoke::Invoke<scrVector>(getEntityCoords, playerPedId, true);
	Vector3 playerPos{ coords.x, coords.y, coords.z };

	// gather potential shapes from the grid
	std::vector<ColShape*> currentInside;
	auto cellShapes = GetColShapesForPosition(playerPos);
	currentInside.reserve(cellShapes.size());

	for (ColShape* shape : cellShapes)
	{
		if (IsPointInColShape(playerPos, *shape))
		{
			currentInside.push_back(shape);
		}
	}

	// check infinite shapes
	for (ColShape* infShape : infiniteShapes_)
	{
		float dx = playerPos.x - infShape->pos1.x;
		float dy = playerPos.y - infShape->pos1.y;
		float dz = playerPos.z - infShape->pos1.z;
		float distanceSq = dx * dx + dy * dy + dz * dz;

		if (distanceSq > (infShape->maxDistance * infShape->maxDistance))
			continue;

		if (IsPointInColShape(playerPos, *infShape))
		{
			currentInside.push_back(infShape);
		}
	}

	// figure out newly entered / newly left
	std::vector<ColShape*> newlyEntered;
	std::vector<ColShape*> newlyLeft;

	// shapes now inside but not previously
	for (ColShape* shape : currentInside)
	{
		if (playerInsideColShapes_.find(shape) == playerInsideColShapes_.end())
		{
			newlyEntered.push_back(shape);
		}
	}

	// shapes previously inside but not now
	for (ColShape* shape : playerInsideColShapes_)
	{
		if (std::find(currentInside.begin(), currentInside.end(), shape) == currentInside.end())
		{
			newlyLeft.push_back(shape);
		}
	}

	// update the set
	playerInsideColShapes_.clear();
	for (ColShape* shape : currentInside)
	{
		playerInsideColShapes_.insert(shape);
	}

	// trigger events
	for (ColShape* shape : newlyEntered)
	{
		rec->QueueEvent2("onPlayerEnterColshape", {}, shape->id);
	}
	for (ColShape* shape : newlyLeft)
	{
		rec->QueueEvent2("onPlayerLeaveColshape", {}, shape->id);
	}
}

// -------------------------------------------------------------------------
// Polyzone creation limits
static const uint32_t MAX_MSGPACK_SIZE = 1048576;
static const size_t MAX_VERTICES = 1000;

// -------------------------------------------------------------------------
// Natives binding
static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("DOES_COLSHAPE_EXIST", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 1)
		{
			trace("DOES_COLSHAPE_EXIST: Invalid argument count\n");
			context.SetResult(false);
			return;
		}

		int shapeId = context.GetArgument<int>(0);
		bool exists = ColShapeManager::Get().Exists(shapeId);
		context.SetResult<bool>(exists);
	});

	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_CIRCLE", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 4)
		{
			trace("CREATE_COLSHAPE_CIRCLE: Invalid argument count\n");
			context.SetResult<int>(-1);
			return;
		}
		float x = context.GetArgument<float>(0);
		float y = context.GetArgument<float>(1);
		float z = context.GetArgument<float>(2);
		float radius = context.GetArgument<float>(3);

		int newId = ColShapeManager::Get().CreateCircle({ x, y, z }, radius);
		context.SetResult<int>(newId);
	});

	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_CUBE", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 6)
		{
			trace("CREATE_COLSHAPE_CUBE: Invalid argument count\n");
			context.SetResult<int>(-1);
			return;
		}
		float x1 = context.GetArgument<float>(0);
		float y1 = context.GetArgument<float>(1);
		float z1 = context.GetArgument<float>(2);
		float x2 = context.GetArgument<float>(3);
		float y2 = context.GetArgument<float>(4);
		float z2 = context.GetArgument<float>(5);

		int newId = ColShapeManager::Get().CreateCube({ x1, y1, z1 }, { x2, y2, z2 });
		context.SetResult<int>(newId);
	});

	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_CYLINDER", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 5)
		{
			trace("CREATE_COLSHAPE_CYLINDER: Invalid argument count\n");
			context.SetResult<int>(-1);
			return;
		}

		float x = context.GetArgument<float>(0);
		float y = context.GetArgument<float>(1);
		float z = context.GetArgument<float>(2);
		float radius = context.GetArgument<float>(3);
		float height = context.GetArgument<float>(4);
		int newId = ColShapeManager::Get().CreateCylinder({ x, y, z }, radius, height);
		context.SetResult<int>(newId);
	});

	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_RECTANGLE", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 6)
		{
			trace("CREATE_COLSHAPE_RECTANGLE: Invalid argument count\n");
			context.SetResult<int>(-1);
			return;
		}
		float x1 = context.GetArgument<float>(0);
		float y1 = context.GetArgument<float>(1);
		float x2 = context.GetArgument<float>(2);
		float y2 = context.GetArgument<float>(3);
		float bottomZ = context.GetArgument<float>(4);
		float height = context.GetArgument<float>(5);

		int newId = ColShapeManager::Get().CreateRectangleZ(x1, y1, x2, y2, bottomZ, height);
		context.SetResult<int>(newId);
	});

	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_SPHERE", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 4)
		{
			trace("CREATE_COLSHAPE_SPHERE: Invalid argument count\n");
			context.SetResult<int>(-1);
			return;
		}
		float x = context.GetArgument<float>(0);
		float y = context.GetArgument<float>(1);
		float z = context.GetArgument<float>(2);
		float radius = context.GetArgument<float>(3);

		int newId = ColShapeManager::Get().CreateSphere({ x, y, z }, radius);
		context.SetResult<int>(newId);
	});

	fx::ScriptEngine::RegisterNativeHandler("DELETE_COLSHAPE", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 1)
		{
			trace("DELETE_COLSHAPE: Invalid argument count\n");
			context.SetResult(false);
			return;
		}
		int shapeId = context.GetArgument<int>(0);
		bool success = ColShapeManager::Get().DeleteColShape(shapeId);
		context.SetResult<bool>(success);
	});

	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_POLYZONE", [](fx::ScriptContext& context)
	{
		int argCount = context.GetArgumentCount();
		if (argCount != 2 && argCount != 4)
		{
			trace("CREATE_COLSHAPE_POLYZONE: Invalid argument count. Must be 2 or 4.\n");
			context.SetResult<int>(-1);
			return;
		}

		const char* packedData = context.CheckArgument<const char*>(0);
		uint32_t dataLen = context.GetArgument<uint32_t>(1);

		// default Z range
		float minZ = -10000.0f;
		float maxZ = 10000.0f;

		if (argCount == 4)
		{
			minZ = context.GetArgument<float>(2);
			maxZ = context.GetArgument<float>(3);
			// looks like this gets set to 0 if not provided.
			if (minZ == 0.0f && maxZ == 0.0f)
			{
				// essentially ignore z if left out.
				minZ = -10000.0f;
				maxZ = 10000.0f;
			}
		}

		if (dataLen == 0 || dataLen > MAX_MSGPACK_SIZE)
		{
			trace("CREATE_COLSHAPE_POLYZONE: dataLen=%u out of range (max %u)\n", dataLen, MAX_MSGPACK_SIZE);
			context.SetResult<int>(-1);
			return;
		}

		msgpack::unpacked unpacked;
		try
		{
			unpacked = msgpack::unpack(packedData, dataLen);
		}
		catch (const std::exception& e)
		{
			trace("CREATE_COLSHAPE_POLYZONE: Failed to unpack: %s\n", e.what());
			context.SetResult<int>(-1);
			return;
		}

		msgpack::object obj = unpacked.get();
		if (obj.type != msgpack::type::ARRAY)
		{
			trace("CREATE_COLSHAPE_POLYZONE: Expected array of points.\n");
			context.SetResult<int>(-1);
			return;
		}

		if (obj.via.array.size > MAX_VERTICES)
		{
			trace("CREATE_COLSHAPE_POLYZONE: Too many vertices (%zu, max %zu)\n",
			obj.via.array.size, MAX_VERTICES);
			context.SetResult<int>(-1);
			return;
		}

		std::vector<Vector2> points;
		points.reserve(obj.via.array.size);

		for (size_t i = 0; i < obj.via.array.size; ++i)
		{
			msgpack::object& element = obj.via.array.ptr[i];
			float x = 0.f, y = 0.f;

			if (element.type == msgpack::type::ARRAY)
			{
				auto& arr = element.via.array;
				if (arr.size >= 2)
				{
					x = arr.ptr[0].as<float>();
					y = arr.ptr[1].as<float>();
				}
				else
				{
					trace("CREATE_COLSHAPE_POLYZONE: array[%zu] has <2 floats.\n", i);
					context.SetResult<int>(-1);
					return;
				}
			}
			else if (element.type == msgpack::type::MAP)
			{
				for (size_t j = 0; j < element.via.map.size; ++j)
				{
					auto& kv = element.via.map.ptr[j];
					if (kv.key.type == msgpack::type::STR)
					{
						std::string key(kv.key.via.str.ptr, kv.key.via.str.size);
						if (key == "x")
							x = kv.val.as<float>();
						if (key == "y")
							y = kv.val.as<float>();
					}
				}
			}
			else if (element.type == msgpack::type::EXT)
			{
				int8_t extType = element.via.ext.type();
				size_t sz = element.via.ext.size;
				const char* dataPtr = element.via.ext.ptr;

				// vector2 => extType=20, size=8
				// vector3 => extType=21, size=12
				// vector4 => extType=22, size=16
				if ((extType == 20 && sz == 8) || (extType == 21 && sz == 12) || (extType == 22 && sz == 16))
				{
					// skip first byte cause serializer included it
					dataPtr++;
					float xx, yy;
					memcpy(&xx, dataPtr + 0, 4);
					memcpy(&yy, dataPtr + 4, 4);
					x = xx;
					y = yy;
				}
				else
				{
					trace("CREATE_COLSHAPE_POLYZONE: Unexpected EXT type/size at %zu\n", i);
					context.SetResult<int>(-1);
					return;
				}
			}
			else
			{
				trace("CREATE_COLSHAPE_POLYZONE: Unexpected element type at index %zu\n", i);
				context.SetResult<int>(-1);
				return;
			}

			points.push_back(Vector2(x, y));
		}

		int newId = ColShapeManager::Get().CreatePolyzone(points, minZ, maxZ);
		context.SetResult<int>(newId);
	});

	fx::ScriptEngine::RegisterNativeHandler("DELETE_COLSHAPE", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 1)
		{
			trace("DELETE_COLSHAPE: Invalid argument count\n");
			context.SetResult(false);
			return;
		}
		int shapeId = context.GetArgument<int>(0);
		bool success = ColShapeManager::Get().DeleteColShape(shapeId);
		context.SetResult<bool>(success);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_COLSHAPE_SYSTEM_ENABLED", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 1)
		{
			trace("SET_COLSHAPE_SYSTEM_ENABLED: Invalid argument count\n");
			return;
		}
		bool enabled = context.GetArgument<bool>(0);
		ColShapeManager::Get().SetEnabled(enabled);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_IS_COLSHAPE_SYSTEM_ENABLED", [](fx::ScriptContext& context) {
		context.SetResult(ColShapeManager::Get().isEnabled());
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_COLSHAPE_SYSTEM_UPDATE_INTERVAL", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 1)
		{
			trace("SET_COLSHAPE_SYSTEM_UPDATE_INTERVAL: Invalid argument count\n");
			return;
		}

		int intervalMs = context.GetArgument<int>(0);
		ColShapeManager::Get().SetUpdateInterval(intervalMs);
	});

	static auto lastUpdateTime = std::chrono::high_resolution_clock::now();
	OnMainGameFrame.Connect([]()
	{
		static auto* colshapeMgr = &ColShapeManager::Get();
		if (!colshapeMgr->isEnabled())
			return;

		auto now = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime).count() >= colshapeMgr->GetUpdateInterval())
		{
			lastUpdateTime = now;
			colshapeMgr->Update();
		}
	});
});
