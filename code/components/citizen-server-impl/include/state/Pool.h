/*
* Dynamic CPool<>
* Copyright (c) 2014 LINK/2012 <dma_2012@hotmail.com>
* Licensed under the MIT License (http://opensource.org/licenses/MIT)
* Modified for the CitizenFX framework, removing GTA III dependencies.
*/
#pragma once
#include <algorithm>
#include <limits>
#include <cassert>

union tPoolObjectFlags
{
	struct
	{
		unsigned char 	uID : 7;
		bool 			bIsFreeSlot : 1;
	}					a;
	unsigned char		b;
};

/*
*  CDynamicPool
*      Extension for CPool, turning it into a dynamically sizeable pool.
*      It's interface is compatible with the original pool interface, so accessing it from the original native code won't be a problem.
*
*      *** Should be a POD-type
*/
template<typename T, typename U = T>
class CPool
{
public:
	typedef T value_type;
	typedef U super_type;

	// Interface must be "compatible" with CPool's interface
	U* 					_m_Objects;         // Always nullptr
	tPoolObjectFlags* 	m_ByteMap;          // Specifies object state at index
	int					m_Size;             // The current size of the pool
	int 				m_nFirstFree;       // Hint of first free index, might be wrong
	bool 				m_bOwnsAllocations; // Do we own our allocations? Of course we do!
	bool 				m_Field_11;         // ?

											// Our additional members
											// Blocks is our way of dynamically allocating more slots
											// When one block is full and we need more objects we can create another block and go ahead
	int                 m_nBlocks;          // Number of blocks                  
	U**                 m_ppBeginBlock;     // Array of pointers to the beggining of each block
	U**                 m_ppEndBlock;       // Array of pointers to the end of each block

											// Constructor, just does Init
	CPool(int size, const char* name)
	{
		this->Init(size, name);
	}

	// Destructor, just does Flush
	~CPool()
	{
		this->Flush();
	}

	// Initialises the pool
	void Init(int size, const char* name)
	{
		// First we need to initialize those fields, so our private methods work
		this->m_Size = 0;           // Will be set up later on
		this->m_ByteMap = nullptr;     // Will be allocated later on
		this->_m_Objects = nullptr;     // Always null, so when the game deletes it, it'll do nothing.
		this->m_bOwnsAllocations = true;// Sure we do
		this->m_nFirstFree = 0;         //

										// Initialises our custom members
		this->m_nBlocks = 0;
		this->m_ppBeginBlock = m_ppEndBlock = nullptr;

		// Size the pool to have enought slots
		this->MakeSureHasBlocks(size);
	}

	// Finishes the pool, freeing any owned allocation
	void Flush()
	{
		// Frees all object blocks
		for (int i = 0; i < m_nBlocks; ++i)
			operator delete(m_ppBeginBlock[i]);

		// Frees block array and bytemap array
		delete[] m_ppBeginBlock;
		delete[] m_ByteMap;

		// Null out original fields
		_m_Objects = nullptr; m_ByteMap = nullptr;
		m_Size = 0; m_nFirstFree = 0;

		// Null out our fields
		m_ByteMap = nullptr;
		m_ppBeginBlock = m_ppEndBlock = nullptr;
		m_nBlocks = 0;
	}

	// Makes all slots in the pool free to be used
	void Clear()
	{
		for (int i = 0; i < m_Size; i++) SetFreeAt(i, true);
	}

	// Gets object at the specified index
	T* GetAt(int idx)
	{
		return (idx >= 0 && idx < m_Size && !IsFreeSlotAtIndex(idx)) ? GetElementAt(idx) : nullptr;
	}

	// Checks if the slot at the specified index is free
	bool IsFreeSlotAtIndex(int idx)
	{
		return m_ByteMap[idx].a.bIsFreeSlot;
	}

	// Sets the specified index to be free or not
	void SetFreeAt(int idx, bool bIsSlotFree)
	{
		m_ByteMap[idx].a.bIsFreeSlot = bIsSlotFree;
	}

	// Allocates a new object in the pool.
	T* New()
	{
		// !!!! This method is reentrant !!!!

		// If pool is empty, grow and allocate... hell, who makes a empty pool?
		if (m_Size == 0) return GrowAndNew();

		// Make sure first free index is valid
		if (m_nFirstFree < 0 || m_nFirstFree >= m_Size)
			m_nFirstFree = 0;

		//
		bool bReachedTop = false;
		auto nStartFree = m_nFirstFree;

		// Iterate on the pool until we find a free index
		while (!IsFreeSlotAtIndex(m_nFirstFree))
		{
			if (++m_nFirstFree >= m_Size)    // Going out of bounds.. ups..
			{
				// If we reached the top another time, it means there's no free slot, grow the pool.
				if (bReachedTop) return GrowAndNew();

				// First time reaching the top
				bReachedTop = true;
				m_nFirstFree = 0;
			}
			else if (bReachedTop && m_nFirstFree == nStartFree)
			{
				// We're back to the index we started searching, grow the pool
				return GrowAndNew();
			}
		}

		// This slot won't be free anymore, advance first free
		auto index = m_nFirstFree++;
		SetFreeAt(index, false);
		++m_ByteMap[index].a.uID;   // Change unique ID
		return GetElementAt(index);
	}

	// Deletes a previosly allocated object in the pool
	void Delete(T* pObject)
	{
		auto index = GetIndexFromElement(pObject);
		assert(index >= 0);
		SetFreeAt(index, true);
		m_nFirstFree = index < m_nFirstFree ? index : m_nFirstFree;
	}

	// Returns SCM handle for object
	int GetIndex(T* pObject)
	{
		auto i = GetIndexFromElement(pObject);
		return (i << 8) | m_ByteMap[i].b;
	}

	// Returns pointer to an object from a SCM handle
	T* AtHandle(int handle)
	{
		int nSlotIndex = handle >> 8;
		return nSlotIndex >= 0 && nSlotIndex < m_Size && m_ByteMap[nSlotIndex].b == (handle & 0xFF) ? GetElementAt(nSlotIndex) : nullptr;
	}

	// Checks if the object is in the range of this pool
	bool Contains(T* obj)
	{
		return GetBlockIndexFromObjectPtr(obj) != -1;
	}

	// Checks if the object is in a block bound
	bool IsExactlyContained(T* obj)
	{
		if (obj)
		{
			auto i = GetBlockIndexFromObjectPtr(obj);
			if (i != -1)
			{
				// Check if the object is on the block boundaries of elements]
				if ((((uintptr_t)obj - (uintptr_t)m_ppBeginBlock[i]) % sizeof(U)) == 0)
				{
					bool result = !IsFreeSlotAtIndex(GetIndexFromElement(obj));
					return !IsFreeSlotAtIndex(GetIndexFromElement(obj));
				}
			}
		}
		return false;
	}




private:    // Hard work goes here

			// Maximum size of the pool
	int MaxSize()
	{
		return (std::numeric_limits<int>::max)();
	}

	// Gets the number of elements in a specific block
	int GetBlockSize(int i)
	{
		return m_ppEndBlock[i] - m_ppBeginBlock[i];
	}

	// Gets m_ppBeginBlock/m_ppEndBlock index a object is in (doesn't matter if on U boundaries or not)
	int GetBlockIndexFromObjectPtr(T* p)
	{
		if (p)
		{
			// Iterate on each block until object pointer is in range
			for (int i = 0; i < this->m_nBlocks; ++i)
			{
				if (p >= m_ppBeginBlock[i] && p < m_ppEndBlock[i])   // Checks if pointer in range of this block
					return i;
			}
		}
		return -1;
	}


	// Finds out the pointer for the element at the specified index
	T* GetElementAt(int index)
	{
		int blockStart, blockEnd = 0;   // The index the current iterating block begins and ends

										// Iterate on each block until index is in range
		for (int i = 0; i < this->m_nBlocks; ++i)
		{
			blockStart = blockEnd;          // Block begins at this offset
			blockEnd += GetBlockSize(i);    // Block ends at this offset

											// Check if index is in this block range
			if (index >= blockStart && index < blockEnd)
				return &m_ppBeginBlock[i][index - blockStart];
		}
		return nullptr;
	}

	// Gets the index for the specified element
	int GetIndexFromElement(T* p)
	{
		if (p)
		{
			// Iterate on each block until object pointer is in range
			for (int i = 0, count = 0; i < this->m_nBlocks; count += GetBlockSize(i), ++i)
			{
				if (p >= m_ppBeginBlock[i] && p < m_ppEndBlock[i])   // Checks if pointer in range of this block
					return count + ((U*)(p)-m_ppBeginBlock[i]);   // <- (real starting index for this block) + (object index on block)
			}
		}
		return -1;
	}



	// Grows the pool, so it can allocate more objects
	bool Grow()
	{
		try
		{
			// Try to grow the pool, the following function might throw bad alloc on failure
			auto old_size = m_Size;
			MakeSureHasBlocks(Growt(m_Size + 1));
			m_nFirstFree = old_size;
			return true;
		}
		catch (const std::bad_alloc&)
		{
			return false;
		}
	}

	// Grows the pool then allocate a new object
	T* GrowAndNew()
	{
		if (Grow()) return New();
		return nullptr;
	}

	// Returns the new size for a grow operation (see Grow())
	int Growt(int requires_)
	{
		if (this->m_Size < requires_)
		{
			// Normally the pool should grow by 50%
			auto half = this->m_Size / 2;
			auto size = (MaxSize() - half < this->m_Size ? 0 :
				this->m_Size + half);

			// Make sure the grow was enought to have 'requires' size
			return (size < requires_ ? requires_ : size);
		}
		return this->m_Size;
	}



	// Makes sure the pool has sufficient blocks to store the size elements
	// This function might throw std::bad_alloc on failure
	void MakeSureHasBlocks(int size)
	{
		if (this->m_Size < size && size)
		{
			this->ReallocByteMap(size);     // Must realloc this before block array because the consequences of this being successful
			this->ReallocBlockArray(size);  // and then this being a failure aren't catastrophical while the other way around is.
			this->m_Size = size;            // Successful, update m_Size
		}
	}

	// Reallocates the bytemap array to hold size elements
	void ReallocByteMap(int size)
	{
		// Allocation first, so in bad_alloc it returns immediately
		tPoolObjectFlags* bytemap = new tPoolObjectFlags[size];

		// Move current bytemap to the new bytemap
		std::copy(m_ByteMap, m_ByteMap + m_Size, bytemap);
		delete[] m_ByteMap;
		this->m_ByteMap = bytemap;

		// Setup new bytemap fields
		for (int i = this->m_Size; i < size; ++i)
		{
			m_ByteMap[i].a.bIsFreeSlot = true;
			m_ByteMap[i].a.uID = 0;
		}
	}

	// Reallocates the block array map, and allocatea  new block, so the pool is able to hold size elements
	void ReallocBlockArray(int size)
	{
		auto nBlockSize = size - this->m_Size;  // Size of the new block
		auto nBlocksAft = this->m_nBlocks + 1;  // Number of blocks after relocation
		U** ppNewArray = nullptr;
		U* pNewBlock = nullptr;

		// Try to block allocations
		try
		{
			ppNewArray = new U*[nBlocksAft * 2];                    // Space for m_ppBeginBlock and m_ppEndBlock
			pNewBlock = (U*) operator new(nBlockSize * sizeof(U));  // Space for one big block
		}
		catch (const std::bad_alloc&)
		{
			delete[] ppNewArray;
			operator delete(pNewBlock);
			throw;
		};

		// Move content pointers of ppBeginBlock and ppEndBlock
		std::copy(m_ppBeginBlock, m_ppBeginBlock + m_nBlocks, ppNewArray + 0);
		std::copy(m_ppEndBlock, m_ppEndBlock + m_nBlocks, ppNewArray + nBlocksAft);
		delete[] m_ppBeginBlock;                        // Free old array of pointers
		m_ppBeginBlock = ppNewArray;                    // Assign the new array of pointers
		m_ppEndBlock = ppNewArray + nBlocksAft;        // Assign the new array of pointers

													   // Assign the new block pointers into the blocks pointer array
		m_ppBeginBlock[m_nBlocks] = pNewBlock;
		m_ppEndBlock[m_nBlocks] = pNewBlock + nBlockSize;

		// Successful, update number of blocks
		this->m_nBlocks = nBlocksAft;
	}
};
