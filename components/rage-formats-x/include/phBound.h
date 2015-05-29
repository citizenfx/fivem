/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include <stdint.h>

#include <pgBase.h>
#include <pgContainers.h>

#define RAGE_FORMATS_FILE phBound
#include <formats-header.h>

#ifdef RAGE_FORMATS_OK
#if defined(RAGE_FORMATS_GAME_NY)
#define RAGE_FORMATS_ny_phBound 1
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
#ifdef RAGE_FORMATS_GAME_FIVE
	Geometry = 4,
	BVH = 8,
	Composite = 10
#elif defined(RAGE_FORMATS_GAME_NY)
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

#ifdef RAGE_FORMATS_GAME_NY
private:
	float _pad;

public:
#endif

	inline phVector3()
		: x(0.0f), y(0.0f), z(0.0f)
	{

	}

	inline phVector3(float x, float y, float z)
		: x(x), y(y), z(z)
	{

	}
};

class phBound
#ifdef RAGE_FORMATS_GAME_FIVE
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

#ifdef RAGE_FORMATS_GAME_FIVE
	uintptr_t m_pad;
#else
	float m_pad;
#endif

	phVector3 m_aabbMax;

#ifdef RAGE_FORMATS_GAME_FIVE
	float m_margin;
#endif
	phVector3 m_aabbMin;

#ifdef RAGE_FORMATS_GAME_FIVE
	uint32_t m_unkCount;
#endif

	phVector3 m_centroid;

#ifdef RAGE_FORMATS_GAME_FIVE
	uint32_t m_unkInt;
#elif defined(RAGE_FORMATS_GAME_NY)
	phVector3 m_unkVector;
#endif

	phVector3 m_cg; // center of gravity

#ifdef RAGE_FORMATS_GAME_FIVE
	uint32_t m_unkInt2;
#endif

	phVector3 m_unkVector2;

#ifdef RAGE_FORMATS_GAME_NY
	float m_margin[3];
	uint32_t m_unkInt;
#elif defined(RAGE_FORMATS_GAME_FIVE)
	float m_unkFloat;
#endif

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		
	}
};

struct phBoundFlagEntry
{
	uint32_t m_0; // boundflags value?
	uint32_t m_4; // defaults to -1 during import, though other values are also seen
};

struct phBVHNode
{
	uint16_t m_quantizedAabbMin[3];
	uint16_t m_quantizedAabbMax[3];

	uint16_t m_start; // if leaf, the start in phBoundPolyhedron poly array; if node, the amount of child nodes, including this node

#ifdef RAGE_FORMATS_GAME_FIVE
	uint16_t m_count; // 0 if node, >=1 if leaf
#elif defined(RAGE_FORMATS_GAME_NY)
	uint8_t m_count;  // 0 if node, >=1 if leaf
	uint8_t m_pad;
#endif
};

struct phBVHSubTree
{
	uint16_t m_quantizedAabbMin[3];
	uint16_t m_quantizedAabbMax[3];

	uint16_t m_firstNode;
	uint16_t m_lastNode;
};

class phBVH
{
private:
	pgArray<phBVHNode, uint32_t> m_nodes;

#ifdef RAGE_FORMATS_GAME_FIVE
	uint32_t m_pad[4];
#elif defined(RAGE_FORMATS_GAME_NY)
	uint32_t m_pad;
#endif

	Vector3 m_aabbMin;
	Vector3 m_aabbMax;

#ifdef RAGE_FORMATS_GAME_FIVE
	Vector3 m_center;
#endif

	Vector3 m_divisor; // m_scale = (1.0f / m_divisor)
	Vector3 m_scale;

	pgArray<phBVHSubTree> m_subTrees;

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_nodes.Resolve(blockMap);
		m_subTrees.Resolve(blockMap);
	}
};

#ifdef RAGE_FORMATS_GAME_NY
struct phBoundAABB
{
	phVector3 min;
	phVector3 max;
};
#endif

class phBoundComposite : public phBound
{
private:
	pgPtr<pgPtr<phBound>> m_childBounds;

	pgPtr<void> m_childMatrices;
	pgPtr<void> m_childMatricesInternal; // copied from child matrices, only if 'allowinternalmotion:' is set

#ifdef RAGE_FORMATS_GAME_FIVE
	pgPtr<void> m_childAABBs;

	pgPtr<phBoundFlagEntry> m_boundFlags; // not set by V import function; might be finalbuild cut, Payne does set it but doesn't do much else with it; still set in V data files

	pgArray<phBoundFlagEntry> m_childArray; // contains child count/size, and pointer points to the same as m_boundFlags

	pgPtr<phBVH> m_bvh; // only set if > 5 child bounds - yes phBoundComposite has its very own BVH nowadays
#else
	union
	{
		pgPtr<phBoundAABB> m_childAABBs;

		pgArray<phBoundAABB> m_childArray;
	};

	uint32_t m_compositePad[3];
#endif

public:
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
};

#ifndef RAGE_FORMATS_GAME_NY
// RDR+ allegedly, confirmed in Payne/Five
struct phBoundPoly
{
public:
	union
	{
		struct  
		{
			uint32_t type : 3; // 0: triangle, 1: sphere, 2: capsule, 3: box, 4: cylinder
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
			float polyRadius;

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
struct phBoundPoly
{
	float vertexUnks[4];
	int16_t indices[4];
	int16_t edges[4];
};
#endif

class phBoundPolyhedron : public phBound
{
private:
	uint32_t m_unk1; // 112

	pgPtr<void> m_unkPtr1;

#ifdef RAGE_FORMATS_GAME_FIVE
	uint16_t m_unkShort1;
	uint8_t m_numUnksPerVertex; // can be 0 without any ill effect, it seems
	uint16_t m_numVerticesShort;
#elif defined(RAGE_FORMATS_GAME_NY)
	pgPtr<void> m_unkPtr2;
#endif

	pgPtr<phBoundPoly> m_polyEntries;

	Vector4 m_vertexScale;
	Vector4 m_vertexOffset;

	pgPtr<void> m_vertices;

	pgPtr<void> m_vertexUnks;

#ifdef RAGE_FORMATS_GAME_FIVE
	pgPtr<void> m_unkPtr3;

	pgPtr<pgPtr<void>> m_unkPtr4;
#elif defined(RAGE_FORMATS_GAME_NY)
	uint8_t m_val;
	uint8_t m_polyPad[3];

	uint32_t m_polyPad2[3];
#endif

	uint32_t m_numVertices;
	uint32_t m_numPolys;

#ifdef RAGE_FORMATS_GAME_FIVE
	uint64_t m_unkVal1;
	uint64_t m_unkVal2;
	uint64_t m_unkVal3;
#endif

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		phBound::Resolve(blockMap);

		m_unkPtr1.Resolve(blockMap);
		m_polyEntries.Resolve(blockMap);
		m_vertices.Resolve(blockMap);
		m_vertexUnks.Resolve(blockMap);

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
	pgPtr<void> m_materials;

#ifdef RAGE_FORMATS_GAME_FIVE
	pgPtr<void> m_materialColors;

	uintptr_t m_pad1;

	pgPtr<void> m_secondSurfaceData;

	uint32_t m_numSecondSurfaceVertices;

	pgPtr<uint8_t> m_polysToMaterials;
#elif defined(RAGE_FORMATS_GAME_NY)
	pgPtr<void> m_geomUnkPtr;
#endif

	uint8_t m_numMaterials;

#ifdef RAGE_FORMATS_GAME_FIVE
	uint8_t m_numMaterialColors;

	uint32_t m_pad2;

	uint16_t m_geomUnkShort1;
	uint16_t m_geomUnkShort2;
	uint16_t m_geomUnkShort3;
#elif defined(RAGE_FORMATS_GAME_NY)
	uint16_t m_geomPad[3];
#endif

public:
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

#ifdef RAGE_FORMATS_GAME_FIVE
	uint64_t m_unkBvhPtr1;

	uint16_t m_unkBvhShort1;
	
	char m_bvhPad[14];
#elif defined(RAGE_FORMATS_GAME_NY)
	uint32_t m_bvhPad[3];
#endif

public:
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
};

#endif

#include <formats-footer.h>