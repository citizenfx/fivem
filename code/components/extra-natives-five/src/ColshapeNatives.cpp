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

// Define constants

// 50 should be fine (20 times a second)
static constexpr int updateIntervalMs = 50;

#ifdef GTA_FIVE
static constexpr float CELL_SIZE = 500.0f; // 500x500 units per grid cell, in my mind it made sense to choose something close to the ~412.0 onesync range?
#elif defined(IS_RDR3)
static constexpr float CELL_SIZE = 500.0f; // 500x500 units per grid cell, no idea for RDR3!
#endif

static constexpr float AUTO_INFINITE_THRESHOLD = 1000.0f; // Threshold to mark shapes as infinite
static constexpr float MAX_INFINITE_SHAPE_DISTANCE = 1000.0f; // Maximum distance for infinite shapes, if we are further away then we don't check them at all!

// copied from RuntimeAssetNatives.cpp
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
	Cylinder, // Cylindrical shape
	Rectangle, // 2D rectangle (with bottomZ + height in Z)
	Sphere, // 3D sphere
	Polyzone, // Same as popular 'PolyZone' fivem resource
};

struct ColShape
{
	std::string id; // Unique identifier
	ColShapeType type;
	Vector3 pos1; // Center or first corner
	Vector3 pos2; // Second corner for cubes/rectangles
	float radius; // For circles, cylinders, spheres
	float height; // For cylinders, rectangles
	bool infinite; // Whether to skip the grid
	float maxDistance; // Maximum distance for collision checks

	// Bounding box in X and Y
	float minX, maxX;
	float minY, maxY;

	std::vector<Vector2> points; // For polyzones

	// Constructor
	ColShape(const std::string& shapeId, ColShapeType shapeType)
		: id(shapeId), type(shapeType), pos1({ 0, 0, 0 }), pos2({ 0, 0, 0 }), radius(0.0f), height(0.0f), infinite(false),
		  minX(0), maxX(0), minY(0), maxY(0), maxDistance(MAX_INFINITE_SHAPE_DISTANCE), points()
	{
	}
};


// Class to manage collision shapes using an optimized grid-based approach
class ColShapeManager
{
public:
	// Singleton instance accessor
	static ColShapeManager& Get()
	{
		static ColShapeManager instance;
		return instance;
	}

	// Disable copy and move semantics
	ColShapeManager(const ColShapeManager&) = delete;
	ColShapeManager& operator=(const ColShapeManager&) = delete;

	// Methods to create different types of collision shapes
	bool CreateCircle(const std::string& colShapeId, const Vector3& center, float radius, bool infinite = false, float maxDistance = MAX_INFINITE_SHAPE_DISTANCE);
	bool CreateCube(const std::string& colShapeId, const Vector3& pos1, const Vector3& pos2, bool infinite = false, float maxDistance = MAX_INFINITE_SHAPE_DISTANCE);
	bool CreateCylinder(const std::string& colShapeId, const Vector3& center, float radius, float height, bool infinite = false, float maxDistance = MAX_INFINITE_SHAPE_DISTANCE);
	bool CreateRectangleZ(const std::string& colShapeId, float x1, float y1, float x2, float y2, float bottomZ, float height, bool infinite = false, float maxDistance = MAX_INFINITE_SHAPE_DISTANCE);
	bool CreateRectangle(const std::string& colShapeId, float x1, float y1, float x2, float y2, float height, bool infinite = false, float maxDistance = MAX_INFINITE_SHAPE_DISTANCE);
	bool CreateSphere(const std::string& colShapeId, const Vector3& center, float radius, bool infinite = false, float maxDistance = MAX_INFINITE_SHAPE_DISTANCE);
	bool DeleteColShape(const std::string& colShapeId);
	bool CreatePolyzone(const std::string& colShapeId, const std::vector<Vector2>& points, float minZ, float maxZ, bool infinite = false, float maxDistance = MAX_INFINITE_SHAPE_DISTANCE);


	bool Exists(const std::string& colShapeId);

	// Update method to check player position and trigger events
	inline void Update();


private:
	ColShapeManager(); // Private constructor for singleton

	// Helper methods
	void MaybeMarkInfinite(ColShape& shape);
	void AddToGrid(ColShape* shape);
	std::vector<ColShape*> GetColShapesForPosition(const Vector3& pos);
	inline bool IsPointInColShape(const Vector3& p, const ColShape& shape);

	// Data Structures
	std::unordered_map<std::string, std::unique_ptr<ColShape>> colShapes_; // colShapeId -> ColShape
	std::vector<ColShape*> infiniteShapes_; // Shapes treated as infinite

	// Grid Implementation: Flat 1D vector
	int minCx_, maxCx_, minCy_, maxCy_;
	int numCellsX_, numCellsY_;
	std::vector<std::vector<ColShape*>> grid_; // [cx * numCellsY + cy] -> shapes

	// Track which shapes the player is currently inside
	std::unordered_set<ColShape*> playerInsideColShapes_;


	//mutable std::mutex mutex_; removed cause we're not using threads anymore
};


ColShapeManager::ColShapeManager()
{
	// Im pretty confident on the values for GTA 5 (this is essentially map corners), i made it a little bit larger for RDR3 but am unsure if it's fine or not.
	#ifdef GTA_FIVE
		minCx_ = static_cast<int>(std::floor(-10000.0f / CELL_SIZE));
		maxCx_ = static_cast<int>(std::floor(10000.0f / CELL_SIZE));
		minCy_ = static_cast<int>(std::floor(-10000.0f / CELL_SIZE));
		maxCy_ = static_cast<int>(std::floor(10000.0f / CELL_SIZE));
	#elif IS_RDR3
		minCx_ = static_cast<int>(std::floor(-15000.0f / CELL_SIZE));
		maxCx_ = static_cast<int>(std::floor(15000.0f / CELL_SIZE));
		minCy_ = static_cast<int>(std::floor(-15000.0f / CELL_SIZE));
		maxCy_ = static_cast<int>(std::floor(15000.0f / CELL_SIZE));
	#else // fallback i guess.
		minCx_ = static_cast<int>(std::floor(-10000.0f / CELL_SIZE));
		maxCx_ = static_cast<int>(std::floor(10000.0f / CELL_SIZE));
		minCy_ = static_cast<int>(std::floor(-10000.0f / CELL_SIZE));
		maxCy_ = static_cast<int>(std::floor(10000.0f / CELL_SIZE));
	#endif

	numCellsX_ = maxCx_ - minCx_ + 1;
	numCellsY_ = maxCy_ - minCy_ + 1;

	// Initialize the grid with empty vectors
	grid_.resize(numCellsX_ * numCellsY_, std::vector<ColShape*>());
}

// Helper to mark shapes as infinite based on size
void ColShapeManager::MaybeMarkInfinite(ColShape& shape)
{
	float width = shape.maxX - shape.minX;
	float height = shape.maxY - shape.minY;

	if (width >= AUTO_INFINITE_THRESHOLD || height >= AUTO_INFINITE_THRESHOLD)
	{
		shape.infinite = true;
	}
}

// Helper to check if id is taken
bool ColShapeManager::Exists(const std::string& colShapeId)
{

	return colShapes_.find(colShapeId) != colShapes_.end();
}

// Add a shape to the grid
void ColShapeManager::AddToGrid(ColShape* shape)
{
	int startCx = static_cast<int>(std::floor(shape->minX / CELL_SIZE));
	int endCx = static_cast<int>(std::floor(shape->maxX / CELL_SIZE));
	int startCy = static_cast<int>(std::floor(shape->minY / CELL_SIZE));
	int endCy = static_cast<int>(std::floor(shape->maxY / CELL_SIZE));

	// Clamp to grid bounds
	startCx = std::max(startCx, minCx_);
	endCx = std::min(endCx, maxCx_);
	startCy = std::max(startCy, minCy_);
	endCy = std::min(endCy, maxCy_);

	for (int cx = startCx; cx <= endCx; ++cx)
	{
		for (int cy = startCy; cy <= endCy; ++cy)
		{
			int index = (cx - minCx_) * numCellsY_ + (cy - minCy_);
			if (index >= 0 && index < static_cast<int>(grid_.size()))
			{
				grid_[index].push_back(shape);
			}
		}
	}
}

// Create methods for different shapes
bool ColShapeManager::CreateCircle(const std::string& colShapeId, const Vector3& center, float radius, bool infinite, float maxDistance)
{

	if (this->Exists(colShapeId))
		return false;

	auto shape = std::make_unique<ColShape>(colShapeId, ColShapeType::Circle);
	shape->pos1 = center;
	shape->radius = radius;
	if (infinite)
	{
		shape->maxDistance = maxDistance;
	}

	// Compute bounding box
	shape->minX = center.x - radius;
	shape->maxX = center.x + radius;
	shape->minY = center.y - radius;
	shape->maxY = center.y + radius;

	// Auto-detect if bounding box is huge -> force infinite
	MaybeMarkInfinite(*shape);

	ColShape* rawPtr = shape.get();
	colShapes_.emplace(colShapeId, std::move(shape));

	if (!rawPtr->infinite)
	{
		AddToGrid(rawPtr);
	}
	else
	{
		infiniteShapes_.push_back(rawPtr);
	}
	return true;
}

bool ColShapeManager::CreateCube(const std::string& colShapeId, const Vector3& pos1, const Vector3& pos2, bool infinite, float maxDistance)
{

	if (this->Exists(colShapeId))
		return false;

	auto shape = std::make_unique<ColShape>(colShapeId, ColShapeType::Cube);
	shape->pos1 = pos1;
	shape->pos2 = pos2;
	if (infinite)
	{
		shape->maxDistance = maxDistance;
	}

	// Compute bounding box
	shape->minX = std::min(pos1.x, pos2.x);
	shape->maxX = std::max(pos1.x, pos2.x);
	shape->minY = std::min(pos1.y, pos2.y);
	shape->maxY = std::max(pos1.y, pos2.y);

	// Auto-detect if bounding box is huge -> force infinite
	MaybeMarkInfinite(*shape);

	ColShape* rawPtr = shape.get();
	colShapes_.emplace(colShapeId, std::move(shape));

	if (!rawPtr->infinite)
	{
		AddToGrid(rawPtr);
	}
	else
	{
		infiniteShapes_.push_back(rawPtr);
	}
	return true;
}

bool ColShapeManager::CreateCylinder(const std::string& colShapeId, const Vector3& center, float radius, float height, bool infinite, float maxDistance)
{

	if (this->Exists(colShapeId))
		return false;

	auto shape = std::make_unique<ColShape>(colShapeId, ColShapeType::Cylinder);
	shape->pos1 = center;
	shape->radius = radius;
	shape->height = height;
	if (infinite)
	{
		shape->maxDistance = maxDistance;
	}

	// Compute bounding box
	shape->minX = center.x - radius;
	shape->maxX = center.x + radius;
	shape->minY = center.y - radius;
	shape->maxY = center.y + radius;

	// Auto-detect if bounding box is huge -> force infinite
	MaybeMarkInfinite(*shape);

	ColShape* rawPtr = shape.get();
	colShapes_.emplace(colShapeId, std::move(shape));

	if (!rawPtr->infinite)
	{
		AddToGrid(rawPtr);
	}
	else
	{
		infiniteShapes_.push_back(rawPtr);
	}
	return true;
}

bool ColShapeManager::CreateRectangleZ(const std::string& colShapeId, float x1, float y1, float x2, float y2, float bottomZ, float height, bool infinite, float maxDistance)
{

	if (this->Exists(colShapeId))
		return false;

	auto shape = std::make_unique<ColShape>(colShapeId, ColShapeType::Rectangle);
	shape->pos1 = { x1, y1, bottomZ };
	shape->pos2 = { x2, y2, bottomZ };
	shape->height = height;
	if (infinite)
	{
		shape->maxDistance = maxDistance;
	}

	// Compute bounding box
	shape->minX = std::min(x1, x2);
	shape->maxX = std::max(x1, x2);
	shape->minY = std::min(y1, y2);
	shape->maxY = std::max(y1, y2);

	// Auto-detect if bounding box is huge -> force infinite
	MaybeMarkInfinite(*shape);

	ColShape* rawPtr = shape.get();
	colShapes_.emplace(colShapeId, std::move(shape));

	if (!rawPtr->infinite)
	{
		AddToGrid(rawPtr);
	}
	else
	{
		infiniteShapes_.push_back(rawPtr);
	}
	return true;
}

// Alias that defaults bottomZ = 0.0f
bool ColShapeManager::CreateRectangle(const std::string& colShapeId, float x1, float y1, float x2, float y2, float height, bool infinite, float maxDistance)
{

	return CreateRectangleZ(colShapeId, x1, y1, x2, y2, 0.0f, height, infinite, maxDistance);
}

bool ColShapeManager::CreateSphere(const std::string& colShapeId, const Vector3& center, float radius, bool infinite, float maxDistance)
{
	if (this->Exists(colShapeId))
		return false;

	auto shape = std::make_unique<ColShape>(colShapeId, ColShapeType::Sphere);
	shape->pos1 = center;
	shape->radius = radius;
	if (infinite)
	{
		shape->maxDistance = maxDistance;
	}

	// Compute bounding box
	shape->minX = center.x - radius;
	shape->maxX = center.x + radius;
	shape->minY = center.y - radius;
	shape->maxY = center.y + radius;

	// Auto-detect if bounding box is huge -> force infinite
	MaybeMarkInfinite(*shape);

	ColShape* rawPtr = shape.get();
	colShapes_.emplace(colShapeId, std::move(shape));

	if (!rawPtr->infinite)
	{
		AddToGrid(rawPtr);
	}
	else
	{
		infiniteShapes_.push_back(rawPtr);
	}
	return true;
}


bool ColShapeManager::CreatePolyzone(const std::string& colShapeId, const std::vector<Vector2>& points, float minZ, float maxZ, bool infinite, float maxDistance)

{
	if (this->Exists(colShapeId))
		return false;

	auto shape = std::make_unique<ColShape>(colShapeId, ColShapeType::Polyzone);
	shape->points = points;

	// We use pos1.z as minZ and height as (maxZ - minZ)
	shape->pos1.z = minZ;
	shape->height = maxZ - minZ;

	// Compute the 2D bounding box from the points.
	if (!points.empty())
	{
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
	}
	else
	{
		return false;
	}

	MaybeMarkInfinite(*shape);

	ColShape* rawPtr = shape.get();
	colShapes_.emplace(colShapeId, std::move(shape));

	if (!rawPtr->infinite)
	{
		AddToGrid(rawPtr);
	}
	else
	{
		infiniteShapes_.push_back(rawPtr);
	}
	return true;
}



// Delete a colShape by ID
bool ColShapeManager::DeleteColShape(const std::string& colShapeId)
{

	auto it = colShapes_.find(colShapeId);
	if (it == colShapes_.end())
	{
		return false;
	}
	ColShape* shapePtr = it->second.get();

	if (shapePtr->infinite)
	{
		// Remove from infiniteShapes_
		auto eraseIt = std::find(infiniteShapes_.begin(), infiniteShapes_.end(), shapePtr);
		if (eraseIt != infiniteShapes_.end())
		{
			infiniteShapes_.erase(eraseIt);
		}
	}
	else
	{
		// Remove from grid cells
		int startCx = static_cast<int>(std::floor(shapePtr->minX / CELL_SIZE));
		int endCx = static_cast<int>(std::floor(shapePtr->maxX / CELL_SIZE));
		int startCy = static_cast<int>(std::floor(shapePtr->minY / CELL_SIZE));
		int endCy = static_cast<int>(std::floor(shapePtr->maxY / CELL_SIZE));

		// Clamp to grid bounds
		startCx = std::max(startCx, minCx_);
		endCx = std::min(endCx, maxCx_);
		startCy = std::max(startCy, minCy_);
		endCy = std::min(endCy, maxCy_);

		for (int cx = startCx; cx <= endCx; ++cx)
		{
			for (int cy = startCy; cy <= endCy; ++cy)
			{
				int index = (cx - minCx_) * numCellsY_ + (cy - minCy_);
				if (index >= 0 && index < static_cast<int>(grid_.size()))
				{
					auto& cellShapes = grid_[index];
					cellShapes.erase(std::remove(cellShapes.begin(), cellShapes.end(), shapePtr), cellShapes.end());
				}
			}
		}
	}

	// Remove from playerInsideColShapes_ if present
	playerInsideColShapes_.erase(shapePtr);

	// Remove from master map
	colShapes_.erase(it);
	return true;
}

// Get all shapes in the grid cell where the position is located, this is unlocked since it's called from Update which is locked already
std::vector<ColShape*> ColShapeManager::GetColShapesForPosition(const Vector3& pos)
{
	std::vector<ColShape*> result;
	int cx = static_cast<int>(std::floor(pos.x / CELL_SIZE));
	int cy = static_cast<int>(std::floor(pos.y / CELL_SIZE));

	if (cx < minCx_ || cx > maxCx_ || cy < minCy_ || cy > maxCy_)
		return result; // Out of grid bounds

	int index = (cx - minCx_) * numCellsY_ + (cy - minCy_);
	if (index >= 0 && index < static_cast<int>(grid_.size()))
	{
		const auto& cellShapes = grid_[index];
		result.reserve(cellShapes.size());
		for (const auto& shape : cellShapes)
		{
			result.push_back(shape);
		}
	}
	return result;
}

// Check if a point is inside a given ColShape
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
			{
				//trace("Polyzone: Z out of bounds\n");
				return false;
			}
			const std::vector<Vector2>& poly = shape.points;
			bool inside = false;
			for (size_t i = 0, j = poly.size() - 1; i < poly.size(); j = i++)
			{
				if (((poly[i].y > p.y) != (poly[j].y > p.y)) && (p.x < (poly[j].x - poly[i].x) * (p.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x))
				{
					inside = !inside;
				}
			}
			return inside;
		}
		default:
			return false;
	}
}

// Update method to check player's current position against collision shapes
inline void ColShapeManager::Update()
{

	static auto resman = Instance<fx::ResourceManager>::Get();
	if (!resman)
		return;

	static auto rec = resman->GetComponent<fx::ResourceEventManagerComponent>();

	#ifdef GTA_FIVE
		constexpr uint64_t HASH_PLAYER_PED_ID = 0xD80958FC74E988A6;
		constexpr uint64_t HASH_GET_ENTITY_COORDS = 0x3FEF770D40960D5A;
	#elif IS_RDR3
		constexpr uint64_t HASH_PLAYER_PED_ID = 0xC190F27E12443814;
		constexpr uint64_t HASH_GET_ENTITY_COORDS = 0xA86D5F069399F44D;
	#endif

	static auto getPlayerPed = fx::ScriptEngine::GetNativeHandler(HASH_PLAYER_PED_ID);
	static auto getEntityCoords = fx::ScriptEngine::GetNativeHandler(HASH_GET_ENTITY_COORDS);

	int playerPedId = FxNativeInvoke::Invoke<int>(getPlayerPed);
	if (playerPedId == 0)
	{
		return;
	}
	scrVector coords = FxNativeInvoke::Invoke<scrVector>(getEntityCoords, playerPedId, true);
	Vector3 playerPos{ coords.x, coords.y, coords.z };

	// Collect shapes in the relevant grid cell
	std::vector<ColShape*> currentInside;

	// Determine grid cell
	int cx = static_cast<int>(std::floor(playerPos.x / CELL_SIZE));
	int cy = static_cast<int>(std::floor(playerPos.y / CELL_SIZE));

	// Get shapes from grid
	auto cellShapes = GetColShapesForPosition(playerPos);
	currentInside.reserve(cellShapes.size());

	for (ColShape* shape : cellShapes)
	{
		if (IsPointInColShape(playerPos, *shape))
		{
			currentInside.push_back(shape);
		}
	}

	// Check infinite shapes
	for (ColShape* infShape : infiniteShapes_)
	{
		// Distance culling
		float dx = playerPos.x - infShape->pos1.x;
		float dy = playerPos.y - infShape->pos1.y;
		float dz = playerPos.z - infShape->pos1.z;
		float distanceSq = dx * dx + dy * dy + dz * dz;

		if (distanceSq > (infShape->maxDistance * infShape->maxDistance))
			continue; // Skip shapes beyond maxDistance

		if (IsPointInColShape(playerPos, *infShape))
		{
			currentInside.push_back(infShape);
		}
	}

	// Determine newly entered and left shapes
	std::vector<ColShape*> newlyEntered;
	std::vector<ColShape*> newlyLeft;

	// Shapes that are now inside but weren't before
	for (ColShape* shape : currentInside)
	{
		if (playerInsideColShapes_.find(shape) == playerInsideColShapes_.end())
		{
			newlyEntered.push_back(shape);
		}
	}

	// Shapes that were inside before but aren't anymore
	for (ColShape* shape : playerInsideColShapes_)
	{
		if (std::find(currentInside.begin(), currentInside.end(), shape) == currentInside.end())
		{
			newlyLeft.push_back(shape);
		}
	}

	// Update the set of shapes the player is inside
	playerInsideColShapes_.clear();
	for (ColShape* shape : currentInside)
	{
		playerInsideColShapes_.insert(shape);
	}

	// Trigger events
	for (ColShape* shape : newlyEntered)
	{
		rec->QueueEvent2("onPlayerEnterColshape", {}, shape->id.c_str());
	}
	for (ColShape* shape : newlyLeft)
	{
		rec->QueueEvent2("onPlayerLeaveColshape", {}, shape->id.c_str());
	}
}

// some limits for polyzone msgpack stuff
static const uint32_t MAX_MSGPACK_SIZE = 1048576; 
static const size_t MAX_VERTICES = 1000;

static InitFunction initFunction([]()
{
	//static ColShapeThread colShapeThread;


	// 
	fx::ScriptEngine::RegisterNativeHandler("DOES_COLSHAPE_EXIST", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 1)
		{
			trace("DOES_COLSHAPE_EXIST: Invalid argument count\n");
			context.SetResult(false);
			return;
		}

		std::string colShapeId = context.CheckArgument<const char*>(0);

		bool exists = ColShapeManager::Get().Exists(colShapeId);

		context.SetResult(exists);
	});

	// COLSHAPE_CIRCLE => Creates a 2D circle shape
	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_CIRCLE", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 5)
		{
			trace("CREATE_COLSHAPE_CIRCLE: Invalid argument count\n");
			context.SetResult(false);
			return;
		}
		// Args: colShapeId, x, y, z, radius, (bool infinite)
		std::string colShapeId = context.CheckArgument<const char*>(0);
		float x = context.GetArgument<float>(1);
		float y = context.GetArgument<float>(2);
		float z = context.GetArgument<float>(3);
		float radius = context.GetArgument<float>(4);

		bool infinite = false; // this used to be a parameter but I've changed it to automatically determine

		Vector3 center{ x, y, z };
		bool success = ColShapeManager::Get().CreateCircle(colShapeId, center, radius, infinite);
		context.SetResult<bool>(success);
	});

	// COLSHAPE_CUBE => Creates a 3D cube shape
	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_CUBE", [](fx::ScriptContext& context)
	{
		// Args: colShapeId, x1, y1, z1, x2, y2, z2, (bool infinite)

		if (context.GetArgumentCount() < 7)
		{
			trace("CREATE_COLSHAPE_CUBE: Invalid argument count\n");
			context.SetResult(false);
			return;
		}

		std::string colShapeId = context.CheckArgument<const char*>(0);
		float x1 = context.GetArgument<float>(1);
		float y1 = context.GetArgument<float>(2);
		float z1 = context.GetArgument<float>(3);
		float x2 = context.GetArgument<float>(4);
		float y2 = context.GetArgument<float>(5);
		float z2 = context.GetArgument<float>(6);

		bool infinite = false;

		Vector3 pos1{ x1, y1, z1 };
		Vector3 pos2{ x2, y2, z2 };
		bool success = ColShapeManager::Get().CreateCube(colShapeId, pos1, pos2, infinite);
		context.SetResult<bool>(success);
	});

	// COLSHAPE_CYLINDER => Creates a cylinder shape
	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_CYLINDER", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 6)
		{
			trace("CREATE_COLSHAPE_CYLINDER: Invalid argument count\n");
			context.SetResult(false);
			return;
		}

		// Args: colShapeId, x, y, z, radius, height, (bool infinite)
		std::string colShapeId = context.CheckArgument<const char*>(0);
		float x = context.GetArgument<float>(1);
		float y = context.GetArgument<float>(2);
		float z = context.GetArgument<float>(3);
		float radius = context.GetArgument<float>(4);
		float height = context.GetArgument<float>(5);

		bool infinite = false;

		Vector3 center{ x, y, z };
		bool success = ColShapeManager::Get().CreateCylinder(colShapeId, center, radius, height, infinite);
		context.SetResult<bool>(success);
	});

	// COLSHAPE_RECTANGLE => Creates a rectangle with bottomZ and height in Z
	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_RECTANGLE", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 7)
		{
			trace("CREATE_COLSHAPE_RECTANGLE: Invalid argument count\n");
			context.SetResult(false);
			return;
		}
		// Args: colShapeId, x1, y1, x2, y2, bottomZ, height, (bool infinite)
		std::string colShapeId = context.CheckArgument<const char*>(0);
		float x1 = context.GetArgument<float>(1);
		float y1 = context.GetArgument<float>(2);
		float x2 = context.GetArgument<float>(3);
		float y2 = context.GetArgument<float>(4);
		float bottomZ = context.GetArgument<float>(5);
		float height = context.GetArgument<float>(6);

		bool infinite = false;

		bool success = ColShapeManager::Get().CreateRectangleZ(colShapeId, x1, y1, x2, y2, bottomZ, height, infinite);
		context.SetResult<bool>(success);
	});

	// COLSHAPE_SPHERE => Creates a 3D sphere shape
	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_SPHERE", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 5)
		{
			trace("CREATE_COLSHAPE_SPHERE: Invalid argument count\n");
			context.SetResult(false);
			return;
		}
		// Args: colShapeId, x, y, z, radius, (bool infinite)
		std::string colShapeId = context.CheckArgument<const char*>(0);
		float x = context.GetArgument<float>(1);
		float y = context.GetArgument<float>(2);
		float z = context.GetArgument<float>(3);
		float radius = context.GetArgument<float>(4);

		bool infinite = false;

		Vector3 center{ x, y, z };
		bool success = ColShapeManager::Get().CreateSphere(colShapeId, center, radius, infinite);
		context.SetResult<bool>(success);
	});

	// COLSHAPE_DELETE => Deletes a colShape by ID
	fx::ScriptEngine::RegisterNativeHandler("DELETE_COLSHAPE", [](fx::ScriptContext& context)
	{
		if (context.GetArgumentCount() < 1)
		{
			trace("DELETE_COLSHAPE: Invalid argument count\n");
			context.SetResult(false);
			return;
		}
		// Args: colShapeId
		std::string colShapeId = context.CheckArgument<const char*>(0);
		bool success = ColShapeManager::Get().DeleteColShape(colShapeId);
		context.SetResult<bool>(success);
	});

	// COLSHAPE_POLYZONE => Creates a 2D polygon shape
	fx::ScriptEngine::RegisterNativeHandler("CREATE_COLSHAPE_POLYZONE", [](fx::ScriptContext& context)
	{
		// Expect either 3 or 5 arguments:
		//  [0] string:  colShapeId
		//  [1] char*:    packed MessagePack data
		//  [2] uint32_t: length of the packed data
		// If 5 arguments:
		//  [3] float: minZ
		//  [4] float: maxZ
		int argCount = context.GetArgumentCount();
		trace("argcount is %d\n", argCount);

		// Only allow 3 or 5.
		if (argCount != 3 && argCount != 5)
		{
			trace("CREATE_COLSHAPE_POLYZONE: Invalid argument count. Must be 3 or 5.\n");
			context.SetResult(false);
			return;
		}

		std::string colShapeId = context.CheckArgument<const char*>(0);
		const char* packedData = context.CheckArgument<const char*>(1);
		uint32_t dataLen = context.GetArgument<uint32_t>(2);

		// Optional Z bounds if argCount == 5
		// essentially, if someone decides to not use arg 4 and 5 we want the "polyzone" to have "infinite height", so just some large numbers should do the trick.

		float minZ, maxZ;

		minZ = context.GetArgument<float>(3);
		maxZ = context.GetArgument<float>(4);

		if (minZ == 0.0f && maxZ == 0.0f)
		{
			// this should only be the case if param 4,5 are not used but for some WEIRD reason
			// even when u leave them empty in lua this function still gets 5 parameters, so we just do this as an alternative!
			minZ = -10000.0f;
			maxZ = 10000.0f;
		}

		if (dataLen == 0 || dataLen > MAX_MSGPACK_SIZE)
		{
			trace("CREATE_COLSHAPE_POLYZONE: Data length %u out of range (max %u)\n", dataLen, MAX_MSGPACK_SIZE);
			context.SetResult(false);
			return;
		}

		msgpack::unpacked unpacked;
		try
		{
			unpacked = msgpack::unpack(packedData, dataLen);
		}
		catch (const std::exception& e)
		{
			trace("CREATE_COLSHAPE_POLYZONE: Failed to unpack data: %s\n", e.what());
			context.SetResult(false);
			return;
		}

		msgpack::object obj = unpacked.get();
		if (obj.type != msgpack::type::ARRAY)
		{
			trace("CREATE_COLSHAPE_POLYZONE: Expected an array of points.\n");
			context.SetResult(false);
			return;
		}

		if (obj.via.array.size > MAX_VERTICES)
		{
			trace("CREATE_COLSHAPE_POLYZONE: Too many vertices (%zu, max is %zu)\n",
			obj.via.array.size, MAX_VERTICES);
			context.SetResult(false);
			return;
		}

		std::vector<Vector2> points;
		points.reserve(obj.via.array.size);

		for (size_t i = 0; i < obj.via.array.size; ++i)
		{
			msgpack::object& element = obj.via.array.ptr[i];
			float x = 0.0f, y = 0.0f;

			// makes it possible to have arrays in lua tables (if for some reason you want to do that, mostly you'd use x = 1, y = 2 or vector2(1,2) etc..)
			if (element.type == msgpack::type::ARRAY)
			{
				// If array of [x,y] or [x,y,z]
				auto& arr = element.via.array;
				if (arr.size >= 2)
				{
					// x, y
					x = arr.ptr[0].as<float>();
					y = arr.ptr[1].as<float>();
					// if arr.size == 3 => there's a z, but we ignore it
				}
				else
				{
					trace("CREATE_COLSHAPE_POLYZONE: Array element at %zu has <2 floats.\n", i);
					context.SetResult(false);
					return;
				}
			}
			// this is essentially { x = 1.0, y= 2.0 }
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
				size_t dataSize = element.via.ext.size;
				const char* dataPtr = element.via.ext.ptr;
				
				// just to make sure its vector2, vector3 or vector4
				// vector2 is 20, found in MsgPackSerializer.cs in the clrcore project (packer.PackExtendedTypeValue(20, ms.ToArray());)
				// vector3 is 21, vector 4 is 22 etc..
				if (dataSize <= 16 && dataSize >= 8 && ((extType == 20 && dataSize == 8) || (extType == 21 && dataSize == 12) || (extType == 22 && dataSize == 16)))
				{
					dataPtr++; // skip the first byte as it contains information about type/size which we don't need here
					float x_val, y_val;
					memcpy(&x_val, dataPtr, sizeof(float));
					memcpy(&y_val, dataPtr + 4, sizeof(float));
					x = x_val;
					y = y_val;
				}
				else
				{
					trace("CREATE_COLSHAPE_POLYZONE: Unexpected EXT type or size at %zu\n", i);
					context.SetResult(false);
					return;
				}
			}
			else
			{
				trace("CREATE_COLSHAPE_POLYZONE: Unexpected element type at index %zu\n", i);
				context.SetResult(false);
				return;
			}

			points.push_back(Vector2(x, y));
		}
		bool success = ColShapeManager::Get().CreatePolyzone(colShapeId, points, minZ, maxZ);
		context.SetResult<bool>(success);
	});


	// use game thread instead of seperate thread as suggested by Heron
	static auto lastUpdateTime = std::chrono::high_resolution_clock::now();
	OnMainGameFrame.Connect([]()
	{
		auto now = std::chrono::high_resolution_clock::now();

		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime).count() >= updateIntervalMs)
		{
			lastUpdateTime = now;
			ColShapeManager::Get().Update();
		}
	});


});
