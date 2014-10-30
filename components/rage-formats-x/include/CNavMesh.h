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

struct TAdjPoly
{
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
};

struct TNavMeshPoly
{
	uint32_t flags : 6;
	uint32_t thisCount : 5;
	uint32_t pad : 21;

	uint32_t flags2 : 16;
	uint32_t polyStart : 16;

	uint32_t _f8;
	uint32_t _fC;

	uint16_t aabbMinX;
	uint16_t aabbMaxX;
	uint16_t aabbMinY;
	uint16_t aabbMaxY;
	uint16_t aabbMinZ;
	uint16_t aabbMaxZ;

	uint16_t _f1C;

	uint16_t centerX;
	uint16_t centerY;

	uint8_t _f22;
	uint8_t _f23;

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
		_f23 = 0;
		_f24 = 0;
	}
};

class CNavMeshSector : public pgStreamableBase
{
private:

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{

	}
};

class CNavMesh : public pgStreamableBase
{
private:
	float m_transform[4][4];
	Vector3 m_size;
	uint32_t m_flags;
	uint16_t _f54; // typically 5
	uint16_t _f56; // typically 1
	pgPtr<CNavMeshCompressedVertex> m_vertices;
	pgPtr<void> m_uncompressedVertices; // null
	pgPtr<uint16_t> m_indices;
	pgPtr<TAdjPoly> m_adjPolys; // and not edges, that's too edgy
	uint32_t m_numIndices;
	pgPtr<TNavMeshPoly> m_polys;
	pgPtr<CNavMeshSector> m_sector;
	pgPtr<void> m_portals;
	uint32_t m_numVertices;
	uint32_t m_numPolys;
	uint32_t m_thisSectorId;
	uint32_t m_dataSize;
	uint32_t m_numChildBounds;
	uint32_t m_numPortals;
	uint32_t _f90;

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
		_f54 = 5;
		_f56 = 1;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_vertices.Resolve(blockMap);
		m_indices.Resolve(blockMap);
		m_adjPolys.Resolve(blockMap);
		m_polys.Resolve(blockMap);

		m_sector.Resolve(blockMap);
		m_sector->Resolve(blockMap);

		m_portals.Resolve(blockMap);
	}
};

#endif

#include <formats-footer.h>