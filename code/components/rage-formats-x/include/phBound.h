/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <stdint.h>
#include <vector>

#include <pgBase.h>
#include <pgContainers.h>

#define RAGE_FORMATS_FILE phBound
#include <formats-header.h>

#ifdef RAGE_FORMATS_OK
#if defined(RAGE_FORMATS_GAME_NY)
#define RAGE_FORMATS_ny_phBound 1
#elif defined(RAGE_FORMATS_GAME_PAYNE)
#define RAGE_FORMATS_payne_phBound 1
#elif defined(RAGE_FORMATS_GAME_RDR3)
#define RAGE_FORMATS_rdr3_phBound 1
#endif

#if defined(RAGE_FORMATS_GAME_FIVE)
#define RAGE_FORMATS_five_phBound 1
#endif

template<typename T, size_t Size, size_t ActualSize = sizeof(T)>
inline void ValidateSizePh()
{
	static_assert(Size == ActualSize, "Invalid size.");
}

enum class phBoundType : uint8_t
{
#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_PAYNE)
	Sphere = 0,
	Capsule = 1,
	Box = 3,
	Geometry = 4,
	BVH = 8,
	Composite = 10
#elif defined(RAGE_FORMATS_GAME_RDR3)
	Sphere = 0,
	Capsule,
	TaperedCapsule,
	Box,
	Geometry,
	BVH,
	Composite,
	Triangle,
	Disc,
	Cylinder,
	Plane,
	Unknown,
	Ring
#elif defined(RAGE_FORMATS_GAME_NY)
	Sphere = 0,
	Capsule = 1,
	Box = 3,
	Geometry = 4,
	BVH = 10,
	Composite = 12
#endif
};

struct phVector3
{
	float x;
	float y;
	float z;

#if defined(RAGE_FORMATS_GAME_NY) || defined(RAGE_FORMATS_GAME_PAYNE)
private:
	float _pad;

public:
#endif

	inline phVector3()
		: x(0.0f), y(0.0f), z(0.0f)
#ifdef RAGE_FORMATS_GAME_NY
			, _pad(NAN)
#endif
	{

	}

	inline phVector3(float x, float y, float z)
		: x(x), y(y), z(z)
#ifdef RAGE_FORMATS_GAME_NY
			, _pad(NAN)
#endif
	{

	}
};

struct phBoundMaterial1
{
	uint8_t materialIdx;
#ifdef RAGE_FORMATS_GAME_NY
	uint8_t pad;
	uint16_t roomId : 5;
	uint16_t f5 : 1; // "sidewalk spec 1"
	uint16_t f6 : 1; // "sidewalk spec 2"
	uint16_t f7 : 1;
	uint16_t stairs : 1;
	uint16_t blockGrip : 1;
	uint16_t blockClimb : 1;
	uint16_t shootThrough : 1;
	uint16_t blockJumpOver : 1;
	uint16_t f13 : 1; // "sidewalk spec 3"
	uint16_t seeThrough : 1;
	uint16_t f15 : 1;
#elif defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_PAYNE) || defined(RAGE_FORMATS_GAME_RDR3)
	uint8_t proceduralId;

	// TODO: double-check order
	uint8_t roomId : 5;
	uint8_t pedDensity : 3;

	uint8_t stairs : 1;
	uint8_t blockClimb : 1;
	uint8_t seeThrough : 1;
	uint8_t shootThrough : 1;
	uint8_t notCover : 1;
	uint8_t walkablePath : 1;
	uint8_t noCamCollision : 1;
	uint8_t shootThroughFx : 1;
#endif
};

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_PAYNE) || defined(RAGE_FORMATS_GAME_RDR3)
struct phBoundMaterial2
{
	uint8_t noDecal : 1;
	uint8_t noNavmesh : 1;
	uint8_t noRagdoll : 1;
	uint8_t vehicleWheel : 1;
	uint8_t noPtfx : 1;
	uint8_t tooSteepForPlayer : 1;
	uint8_t noNetworkSpawn : 1;
	uint8_t noCamCollisionAllowClipping : 1;
	uint8_t materialColorIdx;
	uint16_t unknown;
};
#endif

struct phBoundMaterial
{
	phBoundMaterial1 mat1;

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_PAYNE) || defined(RAGE_FORMATS_GAME_RDR3)
	phBoundMaterial2 mat2;
#endif
};

class phBound
#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_PAYNE) || defined(RAGE_FORMATS_GAME_RDR3)
	: public pgBase
#else
	: public datBase
#endif
{
private:
	phBoundType m_boundType;
	uint8_t m_unk1;
	uint16_t m_unk2;

	float m_radius;

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_RDR3)
	uintptr_t m_pad;
#elif defined(RAGE_FORMATS_GAME_PAYNE)
#else
	float m_worldRadius;
#endif

	phVector3 m_aabbMax;

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_RDR3)
	float m_margin;
#endif
	phVector3 m_aabbMin;

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_RDR3)
	uint32_t m_unkCount;
#endif

	phVector3 m_centroid;

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_RDR3)
	phBoundMaterial1 m_material;
#elif defined(RAGE_FORMATS_GAME_NY)
	phVector3 m_unkVector;
#endif

	phVector3 m_cg; // center of gravity

#if defined(RAGE_FORMATS_GAME_FIVE)
	phBoundMaterial2 m_material2;
#endif

	phVector3 m_unkVector2;

#if defined(RAGE_FORMATS_GAME_NY) || defined(RAGE_FORMATS_GAME_PAYNE)
	float m_margin[3];
	uint32_t m_unkCount;
#elif defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_RDR3)
	float m_unkFloat;
#endif

public:
	inline phBound()
	{
		m_unk1 = 0;
		m_unk2 = 0;

		m_radius = 0;

		m_unkCount = 1;

#if defined(RAGE_FORMATS_GAME_FIVE)
		m_material.materialIdx = 0;
		m_material.pedDensity = 0;
		m_material.proceduralId = 0;
		m_material.roomId = 0;
		m_material.stairs = 0;
		m_material.seeThrough = 0;
		m_material.blockClimb = 0;
		m_material.shootThrough = 0;
		m_material.notCover = 0;
		m_material.walkablePath = 0;
		m_material.noCamCollision = 0;
		m_material.shootThroughFx = 0;

		m_material2.noDecal = 0;
		m_material2.noNavmesh = 0;
		m_material2.noRagdoll = 0;
		m_material2.vehicleWheel = 0;
		m_material2.noPtfx = 0;
		m_material2.tooSteepForPlayer = 0;
		m_material2.noNetworkSpawn = 0;
		m_material2.noCamCollisionAllowClipping = 0;
		m_material2.materialColorIdx = 0;
		m_material2.unknown = 0;

		m_pad = 0;

		m_unkFloat = 1.0f;

		m_unkVector2 = phVector3(1.0f, 1.0f, 1.0f);
#elif defined(RAGE_FORMATS_GAME_NY)
		m_unk1 = 1;
		m_unk2 = 1;

		m_worldRadius = sqrt(3000 * 3000 + 3000 * 3000); // why not (just temp stuff)
#endif
	}

	inline void SetUnkVector(const phVector3& vector)
	{
#if defined(RAGE_FORMATS_GAME_FIVE)
		m_unkVector2 = vector;
#elif defined(RAGE_FORMATS_GAME_NY)
		m_unkVector = vector;
#endif
	}

#ifdef RAGE_FORMATS_GAME_NY
	inline void SetUnkVector2(const phVector3& vector)
	{
		m_unkVector2 = vector;
	}
#endif

	inline float GetRadius() const
	{
		return m_radius;
	}

	inline void SetRadius(float radius)
	{
		m_radius = radius;
	}

	inline float GetMargin() const
	{
#if defined(RAGE_FORMATS_GAME_NY) || defined(RAGE_FORMATS_GAME_PAYNE)
		return m_margin[0];
#else
		return m_margin;
#endif
	}

	inline void SetMargin(float value)
	{
#if defined(RAGE_FORMATS_GAME_NY) || defined(RAGE_FORMATS_GAME_PAYNE)
		m_margin[0] = value;
		m_margin[1] = value;
		m_margin[2] = value;
#else
		m_margin = value;
#endif
	}

#ifdef RAGE_FORMATS_GAME_FIVE
	inline void SetUnkFloat(float value)
	{
		m_unkFloat = value;
	}

	inline phBoundMaterial GetMaterial()
	{
		phBoundMaterial mat;
		mat.mat1 = m_material;
		mat.mat2 = m_material2;

		return mat;
	}

	inline void SetMaterial(const phBoundMaterial& material)
	{
		m_material = material.mat1;
		m_material2 = material.mat2;
	}
#endif

	inline const phVector3& GetCG() const
	{
		return m_cg;
	}

	inline void SetCG(const phVector3& vector)
	{
		m_cg = vector;
	}

	inline const phVector3& GetCentroid() const
	{
		return m_centroid;
	}

	inline void SetCentroid(const phVector3& centroid)
	{
		m_centroid = centroid;
	}

	inline const phVector3& GetAABBMin() const
	{
		return m_aabbMin;
	}

	inline const phVector3& GetAABBMax() const
	{
		return m_aabbMax;
	}

	inline void SetAABBMin(const phVector3& vector)
	{
		m_aabbMin = vector;
	}

	inline void SetAABBMax(const phVector3& vector)
	{
		m_aabbMax = vector;
	}

	inline phBoundType GetType() const
	{
		return m_boundType;
	}

	inline void SetType(phBoundType type)
	{
		m_boundType = type;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		
	}
};

class phBoundCapsule : public phBound
{
public:
	inline phBoundCapsule()
		: phBound()
	{
		SetType(phBoundType::Capsule);

#ifdef RAGE_FORMATS_GAME_FIVE
		m_halfHeight = 0;
		memset(m_capsulePad, 0, sizeof(m_capsulePad));
#endif
	}

#ifdef RAGE_FORMATS_GAME_FIVE
public:
	inline float GetHalfHeight()
	{
		return m_halfHeight;
	}

	inline void SetHalfHeight(float halfHeight)
	{
		m_halfHeight = halfHeight;
	}

private:
	float m_halfHeight;
	uint32_t m_capsulePad[3];
#endif

#ifdef RAGE_FORMATS_GAME_NY
public:
	inline phBoundMaterial GetMaterial()
	{
		phBoundMaterial mat;
		mat.mat1 = m_material;
		return mat;
	}

	inline float GetHeight()
	{
		return m_height.x;
	}

	inline float GetCapsuleRadius()
	{
		return m_capsuleRadius.x;
	}

private:
	Vector3 m_capsuleRadius;
	Vector3 m_height;
	uint8_t m_capsulePad2[48];
	phBoundMaterial1 m_material;
	uint8_t m_capsulePad3[12];
#endif
};

class phBoundSphere : public phBound
{
public:
	inline phBoundSphere()
		: phBound()
	{
		SetType(phBoundType::Sphere);
	}

	// no extra members in V

#ifdef RAGE_FORMATS_GAME_NY
public:
	inline phBoundMaterial GetMaterial()
	{
		phBoundMaterial mat;
		mat.mat1 = m_material;
		return mat;
	}

private:
	Vector3 m_radius;
	uint32_t m_spherePad;
	phBoundMaterial1 m_material;
	uint32_t m_spherePad2[2];
#endif
};

class phBoundBox : public phBound
{
public:
	inline phBoundBox()
		: phBound()
	{
		SetType(phBoundType::Box);
	}

	// no extra members in V

#ifdef RAGE_FORMATS_GAME_NY
public:
	inline phBoundMaterial GetMaterial()
	{
		phBoundMaterial mat;
		mat.mat1 = m_material;
		return mat;
	}

private:
	uint8_t m_boxPad[0x1A0];
	phBoundMaterial1 m_material;
	uint8_t m_boxPad2[28];
#endif
};

struct phBoundFlagEntry
{
	uint32_t m_0; // boundflags value?
	uint32_t m_4; // defaults to -1 during import, though other values are also seen
};

struct phBVHNode : public pgStreamableBase
{
	int16_t m_quantizedAabbMin[3];
	int16_t m_quantizedAabbMax[3];

	uint16_t m_escapeIndexOrTriangleIndex; // if leaf, the start in phBoundPolyhedron poly array; if node, the amount of child nodes, including this node

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_RDR3)
	uint16_t m_count; // 0 if node, >=1 if leaf
#elif defined(RAGE_FORMATS_GAME_NY)
	uint8_t m_count;  // 0 if node, >=1 if leaf
	uint8_t m_pad;
#endif
};

struct phBVHSubTree : public pgStreamableBase
{
	int16_t m_quantizedAabbMin[3];
	int16_t m_quantizedAabbMax[3];

	uint16_t m_firstNode;
	uint16_t m_lastNode;
};

inline float fclamp(float val, float min, float max)
{
	if (val < min)
	{
		val = min;
	}
	else if (val > max)
	{
		val = max;
	}

	return val;
}

class phBVH : public pgStreamableBase
{
#if defined(RAGE_FORMATS_GAME_RDR3)
public:
#else
private:
#endif
#if !defined(RAGE_FORMATS_GAME_RDR3)
	pgArray<phBVHNode, uint32_t> m_nodes;

	uint32_t m_depth;

#if defined(RAGE_FORMATS_GAME_FIVE)
	uint32_t m_pad[3];
#endif

	Vector3 m_aabbMin;
	Vector3 m_aabbMax;

#if defined(RAGE_FORMATS_GAME_FIVE)
	Vector3 m_center;
#endif

	Vector3 m_divisor; // m_scale = (1.0f / m_divisor)
	Vector3 m_scale;

	pgArray<phBVHSubTree> m_subTrees;
#else
	phVector3 m_aabbMin;
	uint32_t m_depth;

	phVector3 m_aabbMax;
	float m_pad0;

	phVector3 m_center;
	float m_pad1;

	phVector3 m_divisor; // m_scale = (1.0f / m_divisor)
	float m_pad2;

	phVector3 m_scale;
	float m_pad3;

	pgArray<phBVHNode, uint32_t> m_nodes;
#endif

public:
	inline phBVH()
	{
#ifdef RAGE_FORMATS_GAME_FIVE
		m_depth = 0;
		m_pad[0] = 0;

		m_aabbMin = { FLT_MIN, FLT_MIN, FLT_MIN };
		m_aabbMax = { FLT_MAX, FLT_MAX, FLT_MAX };
		m_center = { 0.0f, 0.0f, 0.0f };
#endif
	}

#ifdef RAGE_FORMATS_GAME_FIVE
	inline void SetBVH(uint32_t numNodes, phBVHNode* nodes, uint32_t numSubTrees, phBVHSubTree* subTrees)
	{
		m_nodes.SetFrom(nodes, numNodes);

		if (subTrees && numSubTrees)
		{
			m_subTrees.SetFrom(subTrees, numSubTrees);
		}
	}

	inline void SetAABB(const Vector3& aabbMin, const Vector3& aabbMax)
	{
		m_aabbMin = aabbMin;
		m_aabbMax = aabbMax;

#ifdef RAGE_FORMATS_GAME_FIVE
		m_center = { (aabbMax.x + aabbMin.x) / 2.0f, (aabbMax.y + aabbMin.y) / 2.0f, (aabbMax.z + aabbMin.z) / 2.0f };
#endif

		Vector3 size = { aabbMax.x - aabbMin.x, aabbMax.y - aabbMin.y, aabbMax.z - aabbMin.z };
		m_divisor = { 65534.0f / size.x, 65534.0f / size.y, 65534.0f / size.z };
		m_scale = { 1.0f / m_divisor.x, 1.0f / m_divisor.y, 1.0f / m_divisor.z };
	}
#endif

#ifdef RAGE_FORMATS_GAME_FIVE
	inline Vector3 Quantize(const Vector3& pos)
	{
		return
		{
			fclamp((pos.x - m_center.x) * m_divisor.x, -32768.0f, 32767.0f),
			fclamp((pos.y - m_center.y) * m_divisor.y, -32768.0f, 32767.0f),
			fclamp((pos.z - m_center.z) * m_divisor.z, -32768.0f, 32767.0f)
		};
	}

	inline void QuantizePolyMin(const Vector3& aabbMin, int16_t* outQuantized)
	{
		auto quantized = Quantize(aabbMin);
		outQuantized[0] = static_cast<int16_t>(floorf(quantized.x));
		outQuantized[1] = static_cast<int16_t>(floorf(quantized.y));
		outQuantized[2] = static_cast<int16_t>(floorf(quantized.z));
	}

	inline void QuantizePolyMax(const Vector3& aabbMax, int16_t* outQuantized)
	{
		auto quantized = Quantize(aabbMax);
		outQuantized[0] = static_cast<int16_t>(ceilf(quantized.x));
		outQuantized[1] = static_cast<int16_t>(ceilf(quantized.y));
		outQuantized[2] = static_cast<int16_t>(ceilf(quantized.z));
	}

	inline void QuantizePolyCenter(const Vector3& center, int16_t* outQuantized)
	{
		auto quantized = Quantize(center);
		outQuantized[0] = static_cast<int16_t>(quantized.x + ((quantized.x < 0.0f) ? -0.5f : 0.5f));
		outQuantized[1] = static_cast<int16_t>(quantized.y + ((quantized.y < 0.0f) ? -0.5f : 0.5f));
		outQuantized[2] = static_cast<int16_t>(quantized.z + ((quantized.z < 0.0f) ? -0.5f : 0.5f));
	}
#endif

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_nodes.Resolve(blockMap);

#ifndef RAGE_FORMATS_GAME_RDR3
		m_subTrees.Resolve(blockMap);
#endif
	}
};

#ifdef RAGE_FORMATS_GAME_NY
struct phBoundAABB : public pgStreamableBase
{
	Vector3 min;
	Vector3 max;
};
#else
struct phBoundAABB : public pgStreamableBase
{
	phVector3 min;
	uint32_t intUnk;
	phVector3 max;
	float floatUnk;
};
#endif

struct Matrix3x4 : public pgStreamableBase
{
	Vector3 _1;
	Vector3 _2;
	Vector3 _3;
	Vector3 _4;
};

class phBoundComposite : public phBound
{
private:
	pgPtr<pgPtr<phBound>> m_childBounds;

	pgPtr<Matrix3x4> m_childMatrices;
	pgPtr<Matrix3x4> m_childMatricesInternal; // copied from child matrices, only if 'allowinternalmotion:' is set

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_PAYNE) || defined(RAGE_FORMATS_GAME_RDR3)
	pgPtr<phBoundAABB> m_childAABBs;

#ifndef RAGE_FORMATS_GAME_RDR3
	pgPtr<phBoundFlagEntry> m_boundFlags; // not set by V import function; might be finalbuild cut, Payne does set it but doesn't do much else with it; still set in V data files
#endif

	pgArray<phBoundFlagEntry> m_childArray; // contains child count/size, and pointer points to the same as m_boundFlags

	pgPtr<phBVH> m_bvh; // only set if > 5 child bounds - yes phBoundComposite has its very own BVH nowadays
#else
	pgArray<phBoundAABB> m_childArray;

	uint32_t m_compositePad[3];
#endif

public:
	inline phBoundComposite()
		: phBound()
	{
		SetType(phBoundType::Composite);
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		phBound::Resolve(blockMap);

		m_childBounds.Resolve(blockMap);
		m_childMatrices.Resolve(blockMap);
		m_childMatricesInternal.Resolve(blockMap);

#ifdef RAGE_FORMATS_GAME_FIVE
		m_childAABBs.Resolve(blockMap);
		m_boundFlags.Resolve(blockMap);
		m_bvh.Resolve(blockMap);
#endif

		m_childArray.Resolve(blockMap);

		for (int i = 0; i < m_childArray.GetCount(); i++)
		{
			(*m_childBounds)[i].Resolve(blockMap);

			// we don't resolve the actual child bound here; that'd require weird virtual typecasting and setting virtual tables, which will be evil for 32-bit files on 64-bit architectures
		}
	}

	inline uint16_t GetNumChildBounds()
	{
		return m_childArray.GetSize();
	}

	inline phBound* GetChildBound(uint16_t index)
	{
		return *((*m_childBounds)[index]);
	}

	inline void SetChildBounds(uint16_t count, phBound** children)
	{
		pgPtr<phBound>* childEntries = new(false) pgPtr<phBound>[count];

		for (uint16_t i = 0; i < count; i++)
		{
			childEntries[i] = children[i];
		}

		m_childBounds = childEntries;
	}

	inline Matrix3x4* GetChildMatrices()
	{
		return *m_childMatrices;
	}

	inline void SetChildMatrices(uint16_t count, Matrix3x4* data)
	{
		Matrix3x4* outData = new(false) Matrix3x4[count];

		memcpy(outData, data, sizeof(Matrix3x4) * count);

		m_childMatrices = outData;
		m_childMatricesInternal = outData;
	}

	inline phBoundAABB* GetChildAABBs()
	{
#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_PAYNE) || defined(RAGE_FORMATS_GAME_RDR3)
		return *m_childAABBs;
#else
		return &m_childArray.Get(0);
#endif
	}

	inline void SetChildAABBs(uint16_t count, phBoundAABB* data)
	{
#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_PAYNE) || defined(RAGE_FORMATS_GAME_RDR3)
		phBoundAABB* outData = new(false) phBoundAABB[count];

		memcpy(outData, data, sizeof(phBoundAABB) * count);

		m_childAABBs = outData;
#else
		m_childArray.SetFrom(data, count);
#endif
	}

#ifdef RAGE_FORMATS_GAME_FIVE
	inline phBoundFlagEntry* GetBoundFlags()
	{
		return *m_boundFlags;
	}

	inline void SetBoundFlags(uint16_t count, phBoundFlagEntry* data)
	{
		std::vector<phBoundFlagEntry> outEntry(count);
		memcpy(&outEntry[0], data, sizeof(phBoundFlagEntry) * count);

		m_childArray.SetFrom(&outEntry[0], count);
		m_boundFlags = &m_childArray.Get(0);
	}
#endif
};

#ifndef RAGE_FORMATS_GAME_NY
// RDR+ allegedly, confirmed in Payne/Five
struct phBoundPoly : public pgStreamableBase
{
public:
	union
	{
		struct  
		{
#ifdef RAGE_FORMATS_GAME_RDR3
			uint32_t typePad : 24;
			uint32_t type : 3;
#else
#ifdef RAGE_FORMATS_GAME_FIVE
			uint32_t type : 3; // 0: triangle, 1: sphere, 2: capsule, 3: box, 4: cylinder
#else
			uint32_t type : 2; // 0: triangle, 1: sphere, 2: capsule, 3: box
#endif
#endif
		};

		struct  
		{
			uint16_t type; // 1
			uint16_t index;

			float radius;
		} sphere;

		struct  
		{
			uint16_t type; // 2
			uint16_t index;

			float length;
			int16_t indexB;
		} capsule;

		struct  
		{
			uint32_t type; // 3

			int16_t indices[4];
		} box;

		struct  
		{
			float triangleArea;

			int16_t v1;
			int16_t v2;
			int16_t v3;
			int16_t e1;
			int16_t e2;
			int16_t e3;
		} poly;
	};
};
#else
struct phBoundPoly : public pgStreamableBase
{
	float normal[3];
	union
	{
		uint8_t material;
		float polyArea;
	};

	int16_t indices[4];
	int16_t edges[4];
};
#endif

struct phBoundVertex : public pgStreamableBase
{
	// quantized
	int16_t x;
	int16_t y;
	int16_t z;
};

class phBoundPolyhedron : public phBound
{
private:
	uint32_t m_unk1; // 112

	pgPtr<void> m_unkPtr1;

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_PAYNE) || defined(RAGE_FORMATS_GAME_RDR3)
	uint16_t m_unkShort1;

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_RDR3)
	uint8_t m_numUnksPerVertex; // can be 0 without any ill effect, it seems
#endif

	uint16_t m_numVerticesShort;
#elif defined(RAGE_FORMATS_GAME_NY)
	pgPtr<void> m_unkPtr2;
#endif

	pgPtr<phBoundPoly> m_polyEntries;

	Vector4 m_vertexScale;
	Vector4 m_vertexOffset;

	pgPtr<phBoundVertex> m_vertices;

#ifndef RAGE_FORMATS_GAME_PAYNE
	pgPtr<void> m_vertexUnks;
#endif

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_RDR3)
	pgPtr<void> m_unkPtr3;

	pgPtr<pgPtr<void>> m_unkPtr4;
#elif defined(RAGE_FORMATS_GAME_NY)
	uint8_t m_val;
	uint8_t m_polyPad[3];

	uint32_t m_polyPad2[3];
#endif

	uint32_t m_numVertices;
	uint32_t m_numPolys;

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_RDR3)
	uint64_t m_unkVal1;
	uint64_t m_unkVal2;
	uint64_t m_unkVal3;
#elif defined(RAGE_FORMATS_GAME_PAYNE)
	uint32_t m_unkVal1;
#endif

public:
	inline phBoundPolyhedron()
		: phBound()
	{
#ifdef RAGE_FORMATS_GAME_FIVE
		m_unkVal1 = 0;
		m_unkVal2 = 0;
		m_unkVal3 = 0;

		m_numUnksPerVertex = 0;
		m_numVertices = 0;
		m_numVerticesShort = 0;
		m_numPolys = 0;

		m_unk1 = 0;
		m_unkShort1 = 0;
#elif defined(RAGE_FORMATS_GAME_NY)
		m_unk1 = 0;

		m_val = 1;

		m_polyPad2[0] = -1;
		m_polyPad2[1] = 0;
		m_polyPad2[2] = 0;
#endif
	}

	inline uint32_t GetNumVertices()
	{
		return m_numVertices;
	}

	inline void SetPolys(uint32_t numPolys, phBoundPoly* polys)
	{
		phBoundPoly* data = new(false) phBoundPoly[numPolys];

		memcpy(data, polys, sizeof(phBoundPoly) * numPolys);

		m_polyEntries = data;
		m_numPolys = numPolys;
	}

	inline phBoundVertex* GetVertices()
	{
		return *m_vertices;
	}

	inline void SetVertices(uint16_t numVertices, phBoundVertex* vertices)
	{
		phBoundVertex* data = new(false) phBoundVertex[numVertices];

		memcpy(data, vertices, sizeof(phBoundVertex) * numVertices);

		m_vertices = data;
		m_numVertices = numVertices;

#ifdef RAGE_FORMATS_GAME_FIVE
		m_numVerticesShort = numVertices;
#endif
	}

	inline const Vector4& GetVertexOffset() const
	{
		return m_vertexOffset;
	}

	inline void SetVertexOffset(const Vector4& vector)
	{
		m_vertexOffset = vector;
	}

	inline const Vector4& GetQuantum() const
	{
		return m_vertexScale;
	}

	inline void SetQuantum(const Vector4& vector)
	{
		m_vertexScale = vector;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		phBound::Resolve(blockMap);

		m_unkPtr1.Resolve(blockMap);
		m_polyEntries.Resolve(blockMap);
		m_vertices.Resolve(blockMap);
#if !defined(RAGE_FORMATS_GAME_PAYNE)
		m_vertexUnks.Resolve(blockMap);
#endif

#ifdef RAGE_FORMATS_GAME_FIVE
		m_unkPtr3.Resolve(blockMap);
		m_unkPtr4.Resolve(blockMap);
#endif
	}

	inline uint32_t GetNumPolygons() const
	{
		return m_numPolys;
	}

	inline phBoundPoly* GetPolygons()
	{
		return *m_polyEntries;
	}
};

class phBoundGeometry : public phBoundPolyhedron
{
private:
	pgPtr<phBoundMaterial> m_materials;

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_RDR3)
	pgPtr<uint32_t> m_materialColors;

	uintptr_t m_pad1;

	pgPtr<void> m_secondSurfaceData;

	uint32_t m_numSecondSurfaceVertices;

	pgPtr<uint8_t> m_polysToMaterials;
#elif defined(RAGE_FORMATS_GAME_PAYNE)
	uint32_t m_pad1;

	pgPtr<uint8_t> m_polysToMaterials;
#elif defined(RAGE_FORMATS_GAME_NY)
	pgPtr<void> m_geomUnkPtr;
#endif

	uint8_t m_numMaterials;

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_RDR3)
	uint8_t m_numMaterialColors;

	uint32_t m_pad2;

	uint16_t m_geomUnkShort1;
	uint16_t m_geomUnkShort2;
	uint16_t m_geomUnkShort3;
#elif defined(RAGE_FORMATS_GAME_NY)
	uint16_t m_geomPad[3];
#endif

public:
	inline phBoundGeometry()
		: phBoundPolyhedron()
	{
		SetType(phBoundType::Geometry);

#ifdef RAGE_FORMATS_GAME_FIVE
		m_pad1 = 0;
		m_numSecondSurfaceVertices = 0;
		m_numMaterials = 0;
		m_numMaterialColors = 0;
		m_pad2 = 0;
		m_geomUnkShort1 = 0;
		m_geomUnkShort2 = 0;
		m_geomUnkShort3 = 0;
#endif
	}

#ifdef RAGE_FORMATS_GAME_FIVE
	inline uint32_t* GetMaterialColors()
	{
		return *m_materialColors;
	}

	inline void SetMaterialColors(uint8_t count, const uint32_t* inColors)
	{
		uint32_t* colors = (uint32_t*)pgStreamManager::Allocate(count * sizeof(uint32_t), false, nullptr);
		memcpy(colors, inColors, count * sizeof(uint32_t));

		m_materialColors = colors;
		m_numMaterialColors = count;
	}

	inline void SetPolysToMaterials(const uint8_t* data)
	{
		uint32_t count = GetNumPolygons();
		uint8_t* mapping = (uint8_t*)pgStreamManager::Allocate(count, false, nullptr);

		memcpy(mapping, data, count);

		m_polysToMaterials = mapping;
	}
#endif

#ifndef RAGE_FORMATS_GAME_NY
	inline uint8_t* GetPolysToMaterials()
	{
		return *m_polysToMaterials;
	}
#endif

	inline uint8_t GetNumMaterials()
	{
		return m_numMaterials;
	}

	inline phBoundMaterial* GetMaterials()
	{
		return *m_materials;
	}

	// temporary
	inline void SetMaterials(uint8_t count, const phBoundMaterial* data)
	{
		phBoundMaterial* materials = (phBoundMaterial*)pgStreamManager::Allocate(sizeof(*data) * count, false, nullptr);
		memcpy(materials, data, sizeof(*data) * count);

		m_materials = materials;

		m_numMaterials = count;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		phBoundPolyhedron::Resolve(blockMap);

		m_materials.Resolve(blockMap);

#ifdef RAGE_FORMATS_GAME_FIVE
		m_materialColors.Resolve(blockMap);
		m_secondSurfaceData.Resolve(blockMap);

		m_polysToMaterials.Resolve(blockMap);
#elif defined(RAGE_FORMATS_GAME_NY)
		m_geomUnkPtr.Resolve(blockMap);
#endif
	}
};

class phBoundBVH : public phBoundGeometry
{
private:
	pgPtr<phBVH> m_bvh;

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_PAYNE) || defined(RAGE_FORMATS_GAME_RDR3)
	uint64_t m_unkBvhPtr1;

	uint16_t m_unkBvhShort1;
	
	char m_bvhPad[14];
#elif defined(RAGE_FORMATS_GAME_NY)
	uint32_t m_bvhPad[3];
#endif

public:
	inline phBoundBVH()
		: phBoundGeometry()
	{
#ifdef RAGE_FORMATS_GAME_FIVE
		m_unkBvhShort1 = 0xFFFF;
		m_unkBvhPtr1 = 0;
#endif

		memset(m_bvhPad, 0, sizeof(m_bvhPad));

		SetType(phBoundType::BVH);
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		phBoundGeometry::Resolve(blockMap);

		m_bvh.Resolve(blockMap);
		m_bvh->Resolve(blockMap);
	}

	inline phBVH* GetBVH()
	{
		return *m_bvh;
	}

	inline void SetBVH(phBVH* bvh)
	{
		m_bvh = bvh;
	}
};

// not a game struct
struct phBVHPolyBounds
{
	int16_t min[3];
	int16_t center[3];
	int16_t max[3];
	uint16_t idx;
};

#ifdef RAGE_FORMATS_GAME_FIVE
struct phBVHBuildState
{
	std::vector<phBVHNode> nodes;
	std::vector<phBVHSubTree> subTrees;

	int curNodeIndex;
	int subTreeCount;
};

inline void ComputeBVHInternal(phBVHBuildState& state, std::vector<phBVHPolyBounds>& polys, size_t start, size_t count, int maxPerNode, int axis)
{
	size_t end = start + count;

	auto swapLeafNodes = [&](int i, int splitIndex)
	{
		std::swap(polys[i], polys[splitIndex]);
	};

	auto getAabbMin = [&](int p)
	{
		return Vector3{ polys[p].min[0] / 256.0f, polys[p].min[1] / 256.0f,polys[p].min[2] / 256.0f };
	};

	auto getAabbMax = [&](int p)
	{
		return Vector3{ polys[p].max[0] / 256.0f, polys[p].max[1] / 256.0f,polys[p].max[2] / 256.0f };
	};

	auto mergeInternalNodeAabb = [&](int n, const Vector3& min, const Vector3& max)
	{
		state.nodes[n].m_quantizedAabbMin[0] = std::min(state.nodes[n].m_quantizedAabbMin[0], static_cast<int16_t>(min.x * 256.0f));
		state.nodes[n].m_quantizedAabbMin[1] = std::min(state.nodes[n].m_quantizedAabbMin[1], static_cast<int16_t>(min.y * 256.0f));
		state.nodes[n].m_quantizedAabbMin[2] = std::min(state.nodes[n].m_quantizedAabbMin[2], static_cast<int16_t>(min.z * 256.0f));

		state.nodes[n].m_quantizedAabbMax[0] = std::max(state.nodes[n].m_quantizedAabbMax[0], static_cast<int16_t>(max.x * 256.0f));
		state.nodes[n].m_quantizedAabbMax[1] = std::max(state.nodes[n].m_quantizedAabbMax[1], static_cast<int16_t>(max.y * 256.0f));
		state.nodes[n].m_quantizedAabbMax[2] = std::max(state.nodes[n].m_quantizedAabbMax[2], static_cast<int16_t>(max.z * 256.0f));
	};

	// deeper
	if (count > 1) // other values not supported yet!
	{
		// calculate the splitting axis
		int axis;

		{
			int i;

			Vector3 means(0., 0., 0.);
			Vector3 variance(0., 0., 0.);
			
			auto startIndex = start;
			auto endIndex = start + count;

			for (i = startIndex; i < endIndex; i++)
			{
				Vector3 center{polys[i].center[0] / 256.0f, polys[i].center[1] / 256.0f, polys[i].center[2] / 256.0f };
				means += center;
			}
			means *= (1. / count);

			for (i = startIndex; i < endIndex; i++)
			{
				Vector3 center{ polys[i].center[0] / 256.0f, polys[i].center[1] / 256.0f, polys[i].center[2] / 256.0f };
				Vector3 diff2 = center - means;
				diff2 = diff2 * diff2;
				variance += diff2;
			}
			variance *= (1.f / (count - 1));

			axis = variance.MaxAxis();
		}

		// calculate the split index
		int splitIndex;

		{
			int i;
			splitIndex = start;
			int numIndices = count;
			float splitValue;

			Vector3 means(0.0f, 0.0f, 0.0f);
			for (i = start; i < start + count; i++)
			{
				Vector3 center{ polys[i].center[0] / 256.0f, polys[i].center[1] / 256.0f, polys[i].center[2] / 256.0f };
				means += center;
			}
			means *= (1.f / numIndices);

			splitValue = means[axis];

			//sort leafNodes so all values larger then splitValue comes first, and smaller values start from 'splitIndex'.
			for (i = start; i < start + count; i++)
			{
				Vector3 center{ polys[i].center[0] / 256.0f, polys[i].center[1] / 256.0f, polys[i].center[2] / 256.0f };
				if (center[axis] > splitValue)
				{
					//swap
					swapLeafNodes(i, splitIndex);
					splitIndex++;
				}
			}

			//if the splitIndex causes unbalanced trees, fix this by using the center in between startIndex and endIndex
			//otherwise the tree-building might fail due to stack-overflows in certain cases.
			//unbalanced1 is unsafe: it can cause stack overflows
			//bool unbalanced1 = ((splitIndex==startIndex) || (splitIndex == (endIndex-1)));

			//unbalanced2 should work too: always use center (perfect balanced trees)	
			//bool unbalanced2 = true;

			//this should be safe too:
			int rangeBalancedIndices = numIndices / 3;
			bool unbalanced = ((splitIndex <= (start + rangeBalancedIndices)) || (splitIndex >= ((start + count) - 1 - rangeBalancedIndices)));

			if (unbalanced)
			{
				splitIndex = start + (numIndices >> 1);
			}

			bool unbal = (splitIndex == start) || (splitIndex == (start + count));
			(void)unbal;
			assert(!unbal);

		}

		int internalNodeIndex = state.curNodeIndex;

		for (int i = start; i < start + count; i++)
		{
			mergeInternalNodeAabb(internalNodeIndex, getAabbMin(i), getAabbMax(i));
		}

		state.curNodeIndex++;

		int leftChildNodeIndex = state.curNodeIndex;
		ComputeBVHInternal(state, polys, start, splitIndex - start, maxPerNode, axis);

		int rightChildNodeIndex = state.curNodeIndex;
		ComputeBVHInternal(state, polys, splitIndex, end - splitIndex, maxPerNode, axis);

		int escapeIndex = state.curNodeIndex - internalNodeIndex;

		if ((sizeof(phBVHNode) * escapeIndex) > 2048)
		{
			phBVHNode& leftChildNode = state.nodes[leftChildNodeIndex];
			int leftSubTreeSize = leftChildNode.m_count > 0 ? 1 : leftChildNode.m_escapeIndexOrTriangleIndex;
			int leftSubTreeSizeInBytes = leftSubTreeSize * static_cast<int>(sizeof(phBVHNode));

			phBVHNode& rightChildNode = state.nodes[rightChildNodeIndex];
			int rightSubTreeSize = rightChildNode.m_count > 0 ? 1 : rightChildNode.m_escapeIndexOrTriangleIndex;
			int rightSubTreeSizeInBytes = rightSubTreeSize * static_cast<int>(sizeof(phBVHNode));

			if (leftSubTreeSizeInBytes <= 2048)
			{
				auto& subtree = state.subTrees[state.subTreeCount];
				subtree.m_quantizedAabbMin[0] = leftChildNode.m_quantizedAabbMin[0];
				subtree.m_quantizedAabbMin[1] = leftChildNode.m_quantizedAabbMin[1];
				subtree.m_quantizedAabbMin[2] = leftChildNode.m_quantizedAabbMin[2];

				subtree.m_quantizedAabbMax[0] = leftChildNode.m_quantizedAabbMax[0];
				subtree.m_quantizedAabbMax[1] = leftChildNode.m_quantizedAabbMax[1];
				subtree.m_quantizedAabbMax[2] = leftChildNode.m_quantizedAabbMax[2];
				subtree.m_firstNode = leftChildNodeIndex;
				subtree.m_lastNode = leftChildNodeIndex + leftSubTreeSize;

				state.subTreeCount++;
			}

			if (rightSubTreeSizeInBytes <= 2048)
			{
				auto& subtree = state.subTrees[state.subTreeCount];
				subtree.m_quantizedAabbMin[0] = rightChildNode.m_quantizedAabbMin[0];
				subtree.m_quantizedAabbMin[1] = rightChildNode.m_quantizedAabbMin[1];
				subtree.m_quantizedAabbMin[2] = rightChildNode.m_quantizedAabbMin[2];

				subtree.m_quantizedAabbMax[0] = rightChildNode.m_quantizedAabbMax[0];
				subtree.m_quantizedAabbMax[1] = rightChildNode.m_quantizedAabbMax[1];
				subtree.m_quantizedAabbMax[2] = rightChildNode.m_quantizedAabbMax[2];
				subtree.m_firstNode = rightChildNodeIndex;
				subtree.m_lastNode = rightChildNodeIndex + rightSubTreeSize;

				state.subTreeCount++;
			}
		}

		state.nodes[internalNodeIndex].m_count = 0;
		state.nodes[internalNodeIndex].m_escapeIndexOrTriangleIndex = escapeIndex;
	}
	else
	{
		// assign a leaf node
		auto i = state.curNodeIndex;
		state.nodes[i].m_count = count;
		state.nodes[i].m_escapeIndexOrTriangleIndex = polys[start].idx;

		for (int p = start; p < start + count; p++)
		{
			mergeInternalNodeAabb(i, getAabbMin(p), getAabbMax(p));
		}

		state.curNodeIndex++;
	}
}

inline void ComputeBVH(phBVH* bvh, std::vector<phBVHPolyBounds>& polys, int maxPerNode)
{
	uint32_t numNodes = (2 * polys.size()) + 1;
	std::vector<phBVHNode> nodes(numNodes);

	for (auto& node : nodes)
	{
		node.m_escapeIndexOrTriangleIndex = 1;
		node.m_count = 0;

		node.m_quantizedAabbMax[0] = -32768;
		node.m_quantizedAabbMax[1] = -32768;
		node.m_quantizedAabbMax[2] = -32768;

		node.m_quantizedAabbMin[0] = 32767;
		node.m_quantizedAabbMin[1] = 32767;
		node.m_quantizedAabbMin[2] = 32767;
	}

	std::vector<phBVHSubTree> subTrees(std::max(numNodes >> 1, uint32_t(1)));

	phBVHBuildState state;
	state.curNodeIndex = 0;
	state.subTreeCount = 0;
	state.nodes = std::move(nodes);
	state.subTrees = std::move(subTrees);

	// build BVH
	if (numNodes > 0)
	{
		ComputeBVHInternal(state, polys, 0, polys.size(), maxPerNode, -1);
	}

	if (state.subTreeCount == 0)
	{
		state.subTrees[0].m_firstNode = 0;
		state.subTrees[0].m_lastNode = state.nodes[0].m_escapeIndexOrTriangleIndex;

		for (int i = 0; i < 3; i++)
		{
			state.subTrees[0].m_quantizedAabbMin[i] = state.nodes[0].m_quantizedAabbMin[i];
			state.subTrees[0].m_quantizedAabbMax[i] = state.nodes[0].m_quantizedAabbMax[i];
		}

		state.subTreeCount = 1;
	}

	bvh->SetBVH(state.curNodeIndex, state.nodes.data(), state.subTreeCount, state.subTrees.data());
}

inline void CalculateBVH(phBoundBVH* bound, int maxPerNode = 4)
{
	auto bvh = new(false) phBVH();

	auto aabbMin = bound->GetAABBMin();
	auto aabbMax = bound->GetAABBMax();

	bvh->SetAABB(
		{ aabbMin.x, aabbMin.y, aabbMin.z },
		{ aabbMax.x, aabbMax.y, aabbMax.z }
	);
	
	std::vector<phBVHPolyBounds> polyBounds(bound->GetNumPolygons());

	auto polygons = bound->GetPolygons();
	auto vertices = bound->GetVertices();

	auto boundScale = bound->GetQuantum();
	auto boundOffset = bound->GetVertexOffset();

	auto margin = bound->GetMargin();

	auto getVert = [&](int16_t vertIdx)
	{
		vertIdx = vertIdx & 0x7FFF;

		return Vector3{
			(vertices[vertIdx].x * boundScale.x) + boundOffset.x,
			(vertices[vertIdx].y * boundScale.y) + boundOffset.y,
			(vertices[vertIdx].z * boundScale.z) + boundOffset.z
		};
	};

	for (int polyIdx = 0; polyIdx < bound->GetNumPolygons(); polyIdx++)
	{
		Vector3 min;
		Vector3 max;
		Vector3 center;

		rage::five::phBoundPoly& poly = polygons[polyIdx];
		switch (poly.type)
		{
		case 0:
		{
			Vector3 polyMin = { FLT_MAX, FLT_MAX, FLT_MAX };
			Vector3 polyMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
			Vector3 polyAvg = { 0.0f, 0.0f, 0.0f };

			for (int16_t* vert : { &poly.poly.v1, &poly.poly.v2, &poly.poly.v3 })
			{
				auto& v = getVert(*vert);

				polyMin = { std::min(polyMin.x, v.x), std::min(polyMin.y, v.y), std::min(polyMin.z, v.z) };
				polyMax = { std::max(polyMax.x, v.x), std::max(polyMax.y, v.y), std::max(polyMax.z, v.z) };
				polyAvg = { polyAvg.x + v.x, polyAvg.y + v.y, polyAvg.z + v.z };
			}

			min = { polyMin.x - margin, polyMin.y - margin, polyMin.z - margin };
			max = { polyMax.x + margin, polyMax.y + margin, polyMax.z + margin };
			center = { polyAvg.x / 3.0f, polyAvg.y / 3.0f, polyAvg.z / 3.0f };

			break;
		}
		default:
		{
			printf("warning: unsupported collision poly type for BVH\n");
			break;
		}
		}

		polyBounds[polyIdx].idx = polyIdx;
		bvh->QuantizePolyMin(min, polyBounds[polyIdx].min);
		bvh->QuantizePolyMax(max, polyBounds[polyIdx].max);
		bvh->QuantizePolyCenter(center, polyBounds[polyIdx].center);
	}

	ComputeBVH(bvh, polyBounds, maxPerNode);

	bound->SetBVH(bvh);
}
#endif

#endif

#include <formats-footer.h>
