/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <stdint.h>

#include <datBase.h>

#define RAGE_FORMATS_FILE pgBase
#include <formats-header.h>

#ifdef RAGE_FORMATS_OK
#if defined(RAGE_FORMATS_GAME_NY)
#define RAGE_FORMATS_ny_pgBase 1
#endif

struct pgPtrRepresentation
{
#if RAGE_FORMATS_GAME_FIVE_PC
	uint64_t pointer : 60;
	uint64_t blockType : 4;
#else
	uint32_t pointer : 28;
	uint32_t blockType : 4;
#endif
};

struct BlockMap;

class FORMATS_EXPORT pgStreamManager
{
public:
	static void* ResolveFilePointer(pgPtrRepresentation& ptr, BlockMap* blockMap = nullptr);

	static void SetBlockInfo(BlockMap* blockMap);

	static bool IsResolved(const void* ptrAddress);

	static void MarkResolved(const void* ptrAddress);

	static void UnmarkResolved(const void* ptrAddress);

	static BlockMap* BeginPacking();

	static void BeginPacking(BlockMap* blockMap);

	static void MarkToBePacked(pgPtrRepresentation* ptrRepresentation, bool physical, void* tag);

	static void EndPacking();

	static bool IsInBlockMap(void* ptr, BlockMap* blockMap, bool physical);

	static void* Allocate(size_t size, bool isPhysical, BlockMap* blockMap);

	static void* AllocatePlacement(size_t size, void* hintPtr, bool isPhysical, BlockMap* blockMap);

	static inline char* StringDup(const char* str)
	{
		char* outStr = (char*)Allocate(strlen(str) + 1, false, nullptr);

		strcpy(outStr, str);

		return outStr;
	}
};

class FORMATS_EXPORT pgStreamableBase
{
public:
	inline void* operator new(size_t size)
	{
		return malloc(size);
	}

	inline void* operator new(size_t size, bool isPhysical)
	{
		return pgStreamManager::Allocate(size, isPhysical, nullptr);
	}

	inline void* operator new(size_t size, BlockMap* blockMap, bool isPhysical)
	{
		return pgStreamManager::Allocate(size, isPhysical, nullptr);
	}

	inline void* operator new[](size_t size)
	{
		return malloc(size);
	}

	inline void* operator new[](size_t size, bool isPhysical)
	{
		return pgStreamManager::Allocate(size, isPhysical, nullptr);
	}

	inline void* operator new[](size_t size, BlockMap* blockMap, bool isPhysical)
	{
		return pgStreamManager::Allocate(size, isPhysical, nullptr);
	}
};

template<typename T, bool Physical = false>
class pgPtr : public pgStreamableBase
{
private:
union 
{
	pgPtrRepresentation on_disk;

#if (!defined(RAGE_FORMATS_GAME_FIVE_PC) && defined(_M_IX86)) || (defined(RAGE_FORMATS_GAME_FIVE_PC) && defined(_M_AMD64))
	T* pointer;
#elif defined(__i386__) || defined(__amd64__)
	T* pointer;
#else
	// what
	T* pointer;
#endif
};

public:
	pgPtr()
	{
		pointer = nullptr;
	}

	~pgPtr()
	{
		pgStreamManager::UnmarkResolved(this);
	}

	T* operator->() const
	{
		if (pgStreamManager::IsResolved(this))
		{
			return pointer;
		}
		else
		{
			return (T*)pgStreamManager::ResolveFilePointer(const_cast<pgPtrRepresentation&>(on_disk));
		}
	}

	T* operator*() const
	{
		if (pgStreamManager::IsResolved(this))
		{
			return pointer;
		}
		else
		{
			return (T*)pgStreamManager::ResolveFilePointer(const_cast<pgPtrRepresentation&>(on_disk));
		}
	}

	pgPtr operator=(T* other)
	{
		pointer = other;

		pgStreamManager::MarkToBePacked(&on_disk, Physical, _ReturnAddress());
		pgStreamManager::MarkResolved(this);

		return *this;
	}

	pgPtr(const pgPtr& from)
	{
		if (pgStreamManager::IsResolved(&from))
		{
			pgStreamManager::MarkResolved(this);
		}

		on_disk = from.on_disk;
	}

	inline bool IsNull()
	{
		return (pointer == nullptr);
	}

	void Resolve(BlockMap* blockMap = nullptr)
	{
		if (on_disk.blockType == 0 && on_disk.pointer == 0)
		{
			return;
		}

		if (!pgStreamManager::IsResolved(this))
		{
			bool physical = (on_disk.blockType == 6);

			pointer = (T*)pgStreamManager::ResolveFilePointer(on_disk, blockMap);
			pgStreamManager::MarkToBePacked(&on_disk, physical, "ResolveFilePointer");

			pgStreamManager::MarkResolved(this);
		}
	}
};

inline void* datBase::operator new(size_t size)
{
	return malloc(size);
}

inline void* datBase::operator new(size_t size, bool isPhysical)
{
	return pgStreamManager::Allocate(size, isPhysical, nullptr);
}

inline void* datBase::operator new(size_t size, BlockMap* blockMap, bool isPhysical)
{
	return pgStreamManager::Allocate(size, isPhysical, nullptr);
}

struct FORMATS_EXPORT BlockMap : public pgStreamableBase
{
	uint16_t virtualLen;
	uint16_t physicalLen;

	struct BlockInfo
	{
		uint32_t offset;
		void* data;
		uint32_t size;
	} blocks[128];

	bool Save(int version, fwAction<const void*, size_t> writer);
};

/*struct BlockInfo : public pgStreamableBase
{
	uint16_t m_numVirtual;
	uint16_t m_numPhysical;
	uint32_t m_block[128];

	inline BlockInfo()
	{
		m_numPhysical = 0;
		m_numVirtual = 0;
	}
};*/

class FORMATS_EXPORT pgBase : public datBase
{
private:
	pgPtr<BlockMap> m_blockMap;

public:
	inline void SetBlockMap() { m_blockMap = new(false) BlockMap; }

	inline pgBase()
	{
		SetBlockMap();
	}

	virtual ~pgBase() { }
};
#endif

#include <formats-footer.h>