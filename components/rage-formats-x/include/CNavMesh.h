/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

// actually GTA, not RAGE, but who cares
#define RAGE_FORMATS_FILE CNavMesh

#include <pgBase.h>

#include <math.h>
#include <formats-header.h>

#ifdef RAGE_FORMATS_OK

#if defined(RAGE_FORMATS_GAME_NY)
#define RAGE_FORMATS_ny_CNavMesh 1
#endif

struct CNavMeshCompressedVertex
{
	uint16_t x;
	uint16_t y;
	uint16_t z;

	inline CNavMeshCompressedVertex(uint16_t x, uint16_t y, uint16_t z)
		: x(x), y(y), z(z)
	{

	}
};

union TAdjPolyEntry
{
	struct
	{
		uint32_t sector : 4;
		uint32_t unk : 1;
		uint32_t poly : 15;
	};

	uint32_t flags;
};

struct TAdjPoly
{
#ifdef RAGE_FORMATS_GAME_NY
	uint32_t poly1 : 16;
	uint32_t sector1 : 16;

	uint32_t flags1 : 2; // usually 3
	uint32_t flags2 : 2; // either 0 or 1

	uint32_t poly2 : 16;

	uint32_t sector2 : 12;

	TAdjPoly()
	{
		flags1 = 3;
		flags2 = 0;
	}
#elif defined(RAGE_FORMATS_GAME_FIVE)
	TAdjPolyEntry a;
	TAdjPolyEntry b;
#endif
};

struct TNavMeshPoly
{
#ifdef RAGE_FORMATS_GAME_NY
	uint32_t flags : 6;
	uint32_t thisCount : 5;
	uint32_t pad : 21;

	uint32_t flags2 : 16;
	uint32_t polyStart : 16;
#elif defined(RAGE_FORMATS_GAME_FIVE)
	uint32_t pad : 21;
	uint32_t thisCount : 5;
	uint32_t flags : 6;

	uint32_t polyStart : 16;
	uint32_t flags2 : 16;
#endif

	pgPtr<void> _f8; // usually 0 on-disk
	pgPtr<void> _fC; // ^

	int16_t aabbMinX;
	int16_t aabbMaxX;
	int16_t aabbMinY;
	int16_t aabbMaxY;
	int16_t aabbMinZ;
	int16_t aabbMaxZ;

#ifdef RAGE_FORMATS_GAME_NY
	uint16_t _f1C;
#elif defined(RAGE_FORMATS_GAME_FIVE)
	uint32_t _f1C;
#endif

	uint16_t centerX;
	uint16_t centerY;

#ifdef RAGE_FORMATS_GAME_NY
	uint8_t _f22;
	uint8_t _f23;
#endif

	uint32_t _f24;

	inline TNavMeshPoly()
	{
		flags = 0xCC;
		flags2 = 0x7BF4;

		aabbMinX = 0;
		aabbMinY = 0;
		aabbMinZ = 0;

		aabbMaxX = 0;
		aabbMaxY = 0;
		aabbMaxZ = 0;

		_f1C = 0;
#ifdef RAGE_FORMATS_GAME_NY
		_f23 = 0;
#endif
		_f24 = 0;
	}
};

class CNavMeshSectorData : public pgStreamableBase
{
private:
	pgPtr<void> m_unkPtr; // runtime?
	pgPtr<uint16_t> m_polyIndices;
	pgPtr<void> m_bounds;

	uint16_t m_numPolyIndices;
	uint16_t m_numBounds;

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_polyIndices.Resolve(blockMap);
		m_bounds.Resolve(blockMap);
	}
};

#pragma pack(push, 1)
class CNavMeshSector : public pgStreamableBase
{
private:
#ifdef RAGE_FORMATS_GAME_FIVE
	Vector3 m_bbMin;
	Vector3 m_bbMax;
	
	int16_t m_aabbMinX; // quantized
	int16_t m_aabbMaxX;
	int16_t m_aabbMinY;
	int16_t m_aabbMaxY;
	int16_t m_aabbMinZ;
	int16_t m_aabbMaxZ;

	pgPtr<CNavMeshSectorData> m_sectorData;

	pgPtr<CNavMeshSector> m_subTrees[4];
#endif

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_sectorData.Resolve(blockMap);

		if (*m_sectorData)
		{
			(*m_sectorData)->Resolve(blockMap);
		}

		for (int i = 0; i < 4; i++)
		{
			m_subTrees[i].Resolve(blockMap);

			if (*m_subTrees[i])
			{
				(*m_subTrees[i])->Resolve(blockMap);
			}
		}
	}
};
#pragma pack(pop)

template<typename T>
class aiSplitSegment : public pgStreamableBase
{
private:
	pgPtr<T> m_data;
	uint32_t m_count;

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_data.Resolve(blockMap);
	}
};

template<typename T, int SplitCount>
class aiSplitArray : public datBase
{
private:
	uint32_t m_count;
	pgPtr<aiSplitSegment<T>> m_segments;

public:
	inline int GetNumSegments()
	{
		int segments = m_count / SplitCount;

		if ((m_count % SplitCount) != 0)
		{
			segments++;
		}

		return segments;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		int num = GetNumSegments();

		m_segments.Resolve(blockMap);

		for (int i = 0; i < num; i++)
		{
			(*m_segments)[i].Resolve(blockMap);
		}
	}
};

using TNavIndex = uint16_t;

#ifdef RAGE_FORMATS_GAME_NY
using CNavMeshCompressedVertexArray = CNavMeshCompressedVertex;
using TNavIndexArray = TNavIndex;
using TAdjPolyArray = TAdjPoly;
using TNavMeshPolyArray = TNavMeshPoly;
#else
using CNavMeshCompressedVertexArray = aiSplitArray<CNavMeshCompressedVertex, 2730>;
using TNavIndexArray = aiSplitArray<TNavIndex, 8192>;
using TAdjPolyArray = aiSplitArray<TAdjPoly, 2048>;
using TNavMeshPolyArray = aiSplitArray<TNavMeshPoly, 409>;
#endif

class CNavMesh
#ifdef RAGE_FORMATS_GAME_NY
	: public pgStreamableBase
#else
	: public pgBase
#endif
{
private:
#ifdef RAGE_FORMATS_GAME_NY
	float m_transform[4][4];
	Vector3 m_size;
#endif
	uint32_t m_flags; // 9 in Five?
	uint16_t m_num1; // typically 5 in NY, 0x11 in Five?
	uint16_t m_num2; // typically 1 in NY, 1 in Five too
#ifdef RAGE_FORMATS_GAME_FIVE
	uint8_t m_align[8]; // to align the matrix to 16b
	float m_transform[4][4];
	Vector3 m_size;
#endif
	pgPtr<CNavMeshCompressedVertexArray> m_vertices;
	pgPtr<void> m_uncompressedVertices; // null
	pgPtr<TNavIndexArray> m_indices;
	pgPtr<TAdjPolyArray> m_adjPolys; // and not edges, that's too edgy
	uint32_t m_numIndices;
#ifdef RAGE_FORMATS_GAME_FIVE
	uint32_t m_numAdjSectors;
	uint32_t m_adjSectors[32];
#endif
	pgPtr<TNavMeshPolyArray> m_polys;
	pgPtr<CNavMeshSector> m_sector;
	pgPtr<void> m_portals;
#ifdef RAGE_FORMATS_GAME_FIVE
	pgPtr<void> m_unks;
#endif
	uint32_t m_numVertices;
	uint32_t m_numPolys;
	uint32_t m_thisSectorId;
	uint32_t m_dataSize;
	uint32_t m_numChildBounds;
	uint32_t m_numPortals;
	uint32_t _f90;

#ifdef RAGE_FORMATS_GAME_FIVE
	uint8_t m_pad[12];
	uint32_t m_unkHash;
	uint8_t m_pad2[12];
#endif

public:
	inline CNavMesh()
	{
		memset(m_transform, 0, sizeof(m_transform));

		m_transform[0][0] = 1;
		m_transform[1][1] = 1;
		m_transform[2][2] = 1;

		m_transform[0][3] = NAN;
		m_transform[1][3] = NAN;
		m_transform[2][3] = NAN;
		m_transform[3][3] = NAN;

		m_flags = 0;
		m_num1 = 5;
		m_num2 = 1;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_vertices.Resolve(blockMap);
		(*m_vertices)->Resolve(blockMap);

		m_indices.Resolve(blockMap);
		(*m_indices)->Resolve(blockMap);

		m_adjPolys.Resolve(blockMap);
		(*m_adjPolys)->Resolve(blockMap);

		m_polys.Resolve(blockMap);
		(*m_polys)->Resolve(blockMap);

		m_sector.Resolve(blockMap);
		m_sector->Resolve(blockMap);

		m_portals.Resolve(blockMap);
	}
};

#ifdef RAGE_FORMATS_GAME_FIVE
template<typename T, size_t Size, size_t ActualSize = sizeof(T)>
inline void* ValidateSizeNv()
{
	static_assert(Size == ActualSize, "Invalid size.");
	return nullptr;
}

static void* n = ValidateSizeNv<CNavMesh, 368>();
static void* m = ValidateSizeNv<TNavMeshPoly, 48>();
#endif

#endif

#include <formats-footer.h>