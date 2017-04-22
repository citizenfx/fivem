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

#if defined(RAGE_FORMATS_GAME_FIVE)
#define RAGE_FORMATS_five_pgBase 1
#endif

struct pgPtrRepresentation
{
#ifndef RAGE_FORMATS_GAME_FIVE
	uint32_t pointer : 28;
	uint32_t blockType : 4;
#else
	uint64_t pointer : 28;
	uint64_t blockType : 36;
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

	static BlockMap* CreateBlockMap();

	static void DeleteBlockMap(BlockMap* blockMap);

	static void BeginPacking(BlockMap* blockMap);

	static void MarkToBePacked(pgPtrRepresentation* ptrRepresentation, bool physical, void* tag);

	static void EndPacking();

	static void FinalizeAllocations(BlockMap* blockMap);

	static bool IsInBlockMap(void* ptr, BlockMap* blockMap, bool physical);

	static void* Allocate(size_t size, bool isPhysical, BlockMap* blockMap);

	static void* AllocatePlacement(size_t size, void* hintPtr, bool isPhysical, BlockMap* blockMap);

	static BlockMap* GetBlockMap();

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
		void* data = pgStreamManager::Allocate(size, isPhysical, nullptr);
		memset(data, 0, size);

		return data;
	}

	inline void* operator new(size_t size, BlockMap* blockMap, bool isPhysical)
	{
		void* data = pgStreamManager::Allocate(size, isPhysical, blockMap);
		memset(data, 0, size);

		return data;
	}

	inline void* operator new[](size_t size)
	{
		return malloc(size);
	}

	inline void* operator new[](size_t size, bool isPhysical)
	{
		void* data = pgStreamManager::Allocate(size, isPhysical, nullptr);
		memset(data, 0, size);

		return data;
	}

	inline void* operator new[](size_t size, BlockMap* blockMap, bool isPhysical)
	{
		void* data = pgStreamManager::Allocate(size, isPhysical, blockMap);
		memset(data, 0, size);

		return data;
	}
};

template<typename T, bool Physical = false>
class pgPtr : public pgStreamableBase
{
private:
union 
{
	pgPtrRepresentation on_disk;

#if (!defined(RAGE_FORMATS_GAME_FIVE) && defined(_M_IX86)) || (defined(RAGE_FORMATS_GAME_FIVE) && defined(_M_AMD64))
	T* pointer;
#elif defined(__i386__) || defined(__amd64__)
	T* pointer;
#elif (defined(RAGE_FORMATS_GAME_FIVE) && defined(_M_IX86)) || (!defined(RAGE_FORMATS_GAME_FIVE) && defined(_M_AMD64))
	// what
	uint32_t pointer;
#else
	T* pointer;
#endif
};

public:
	pgPtr()
	{
		pointer = 0;
	}

	~pgPtr()
	{
#ifdef RAGE_FORMATS_IN_GAME
		return;
#else
		pgStreamManager::UnmarkResolved(this);
#endif
	}

	T* operator->() const
	{
#ifdef RAGE_FORMATS_IN_GAME
		return (T*)pointer;
#else
		if (pgStreamManager::IsResolved(this))
		{
			return (T*)pointer;
		}
		else
		{
			return (T*)pgStreamManager::ResolveFilePointer(const_cast<pgPtrRepresentation&>(on_disk));
		}
#endif
	}

	T* operator*() const
	{
#ifdef RAGE_FORMATS_IN_GAME
		return (T*)pointer;
#else
		if (pgStreamManager::IsResolved(this))
		{
			return (T*)pointer;
		}
		else
		{
			return (T*)pgStreamManager::ResolveFilePointer(const_cast<pgPtrRepresentation&>(on_disk));
		}
#endif
	}

	pgPtr operator=(T* other)
	{
#ifdef RAGE_FORMATS_IN_GAME
		pointer = other;
#else
#if RAGE_NATIVE_ARCHITECTURE
		pointer = other;

		pgStreamManager::MarkToBePacked(&on_disk, Physical, _ReturnAddress());
		pgStreamManager::MarkResolved(this);
#else
		pointer = (decltype(pointer))other;

		pgStreamManager::MarkToBePacked(&on_disk, Physical, _ReturnAddress());
		pgStreamManager::MarkResolved(this);
#endif
#endif

		return *this;
	}

	pgPtr(pgPtrRepresentation fromRep)
	{
		on_disk = fromRep;
	}

	pgPtr(const pgPtr& from)
	{
#ifndef RAGE_FORMATS_IN_GAME
		if (pgStreamManager::IsResolved(&from))
		{
			pgStreamManager::MarkResolved(this);
		}
#endif

		on_disk = from.on_disk;
	}

	pgPtr& operator=(const pgPtr& arg)
	{
#ifndef RAGE_FORMATS_IN_GAME
		if (pgStreamManager::IsResolved(&arg))
		{
			pgStreamManager::MarkResolved(this);
		}
#endif

		pointer = arg.pointer;

		return *this;
	}

	inline bool IsNull()
	{
		return (pointer == 0);
	}

	void Resolve(BlockMap* blockMap = nullptr)
	{
		if (on_disk.blockType == 0 && on_disk.pointer == 0)
		{
			return;
		}

#ifndef RAGE_FORMATS_IN_GAME
		if (!pgStreamManager::IsResolved(this))
		{
			bool physical = (on_disk.blockType == 6);

#if RAGE_NATIVE_ARCHITECTURE
			pointer = (T*)pgStreamManager::ResolveFilePointer(on_disk, blockMap);
#else
			pointer = (decltype(pointer))pgStreamManager::ResolveFilePointer(on_disk, blockMap);
#endif
			pgStreamManager::MarkToBePacked(&on_disk, physical, "ResolveFilePointer");

			pgStreamManager::MarkResolved(this);
		}
#endif
	}
};

inline void* datBase::operator new(size_t size)
{
	return malloc(size);
}

inline void* datBase::operator new(size_t size, bool isPhysical)
{
	//return pgStreamManager::Allocate(size, isPhysical, nullptr);
	void* data = pgStreamManager::Allocate(size, isPhysical, nullptr);
	memset(data, 0, size);

	return data;
}

inline void* datBase::operator new(size_t size, BlockMap* blockMap, bool isPhysical)
{
	//return pgStreamManager::Allocate(size, isPhysical, nullptr);
	void* data = pgStreamManager::Allocate(size, isPhysical, blockMap);
	memset(data, 0, size);

	return data;
}

struct ResourceFlags;

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

	size_t baseAllocationSize[2];

	inline BlockMap()
	{
		virtualLen = 0;
		physicalLen = 0;

		memset(blocks, 0, sizeof(blocks));

#ifdef RAGE_FORMATS_GAME_FIVE
		*(uint16_t*)((char*)this + 8) = 0x407;
#endif
	}

	bool Save(int version, fwAction<const void*, size_t> writer, ResourceFlags* flags = nullptr);
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
#ifndef RAGE_FORMATS_GAME_FIVE
		SetBlockMap();
#endif
	}
};

template<typename TChild>
class datOwner : public pgBase
{
private:
	pgPtr<TChild> m_child;

	uint32_t m_pad[5];

public:
	inline TChild* GetChild()
	{
		return *m_child;
	}

	inline void SetChild(TChild* child)
	{
		m_child = child;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_child.Resolve(blockMap);
		m_child->Resolve(blockMap);
	}
};

struct ResourceFlags
{
	uint32_t physicalFlag;
	uint32_t virtualFlag;
	int version;
};
#endif

#include <formats-footer.h>