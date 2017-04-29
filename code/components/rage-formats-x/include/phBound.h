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
	float m_worldRadius;
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
	uint32_t m_unkCount;
#elif defined(RAGE_FORMATS_GAME_FIVE)
	float m_unkFloat;
#endif

public:
	inline phBound()
	{
		m_unk1 = 0;
		m_unk2 = 0;

		m_radius = 0;

		m_unkCount = 1;

#ifdef RAGE_FORMATS_GAME_FIVE
		m_unkInt = 0;
		m_unkInt2 = 0;

		m_pad = 0;

		m_unkFloat = 1.0f;

		m_unkVector2 = phVector3(1.0f, 1.0f, 1.0f);
#else
		m_unk1 = 1;
		m_unk2 = 1;

		m_worldRadius = sqrt(3000 * 3000 + 3000 * 3000); // why not (just temp stuff)
#endif
	}

	inline void SetUnkVector(const phVector3& vector)
	{
#ifdef RAGE_FORMATS_GAME_FIVE
		m_unkVector2 = vector;
#else
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
#ifdef RAGE_FORMATS_GAME_NY
		return m_margin[0];
#else
		return m_margin;
#endif
	}

	inline void SetMargin(float value)
	{
#ifdef RAGE_FORMATS_GAME_NY
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

#ifdef RAGE_FORMATS_GAME_FIVE
	pgPtr<phBoundAABB> m_childAABBs;

	pgPtr<phBoundFlagEntry> m_boundFlags; // not set by V import function; might be finalbuild cut, Payne does set it but doesn't do much else with it; still set in V data files

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
#ifdef RAGE_FORMATS_GAME_FIVE
		return *m_childAABBs;
#else
		return &m_childArray.Get(0);
#endif
	}

	inline void SetChildAABBs(uint16_t count, phBoundAABB* data)
	{
#ifdef RAGE_FORMATS_GAME_FIVE
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

	pgPtr<phBoundVertex> m_vertices;

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

struct phBoundMaterial
{
	uint8_t materialIdx;
	uint8_t pad[3];

#ifdef RAGE_FORMATS_GAME_FIVE
	uint32_t pad2;
#endif
};

class phBoundGeometry : public phBoundPolyhedron
{
private:
	pgPtr<phBoundMaterial> m_materials;

#ifdef RAGE_FORMATS_GAME_FIVE
	pgPtr<uint32_t> m_materialColors;

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

#ifdef RAGE_FORMATS_GAME_FIVE
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
};

#endif

#include <formats-footer.h>