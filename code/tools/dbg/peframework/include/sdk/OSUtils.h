/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/OSUtils.h
*  PURPOSE:     Implementation dependant routines for native features
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _COMMON_OS_UTILS_
#define _COMMON_OS_UTILS_

#include "MemoryRaw.h"
#include "MemoryUtils.h"

#include <atomic>

// Namespace of abstraction types to be used by any NativePageAllocator implementation.
namespace NativePageAllocation
{
    // Abstraction page handle information.
    struct pageHandleInfo
    {
        void *pAddress;
        size_t memSize;
    };

    // Abstraction memory region information.
    struct pageInfo
    {
        void *pAddress;
        size_t regionSize;
#if 0
        // Examples.
        bool isWritable;
        bool isReadable;
        bool isExecutable;
#endif
    };
};

// Native OS memory allocation manager that marks pages on RAM to be used by the program.
// This implementation should be used if the API exposed by the OS is not enough for you (malloc, etc).
// Version 2.
struct NativePageAllocator
{
    // WARNING: this class is NOT thread-safe!

    // There is still a lot to be done for this class!
    // For the basic purpose it always guarrantees functionality under optimal situations (enough memory, etc).
    // TODO:
    // * exception safety under usage of specialized types

private:
    // Private memory heap to prevent unwanted recursion.
    // Made to not depend on the CRT.
    struct privateAllocatorManager
    {
        inline privateAllocatorManager( void )
        {
            this->_privateHeap = HeapCreate( 0, 0, 0 );

            assert( this->_privateHeap != NULL );
        }

        inline ~privateAllocatorManager( void )
        {
            HeapDestroy( this->_privateHeap );
        }

        inline void* Allocate( size_t memSize, unsigned int )
        {
            return HeapAlloc( this->_privateHeap, 0, memSize );
        }

        inline void* Realloc( void *memPtr, size_t memSize, unsigned int )
        {
            return HeapReAlloc( this->_privateHeap, 0, memPtr, memSize );
        }

        inline void Free( void *memPtr )
        {
            HeapFree( this->_privateHeap, 0, memPtr );
        }

    private:
        HANDLE _privateHeap;
    };

    struct privateAllocator
    {
        inline privateAllocator( privateAllocatorManager *manager )
        {
            this->manager = manager;
        }

        inline privateAllocator( privateAllocator&& right )
        {
            this->manager = right.manager;

            right.manager = NULL;
        }

        inline ~privateAllocator( void )
        {
            return;
        }

        inline void* Allocate( size_t memSize, unsigned int flags )
        {
            return manager->Allocate( memSize, flags );
        }

        inline void* Realloc( void *memPtr, size_t memSize, unsigned int flags )
        {
            return manager->Realloc( memPtr, memSize, flags );
        }

        inline void Free( void *memPtr )
        {
            manager->Free( memPtr );
        }

    private:
        privateAllocatorManager *manager;
    };

    privateAllocatorManager _privateAlloc;

    SYSTEM_INFO _systemInfo;

    // It is good to keep some useful metrics.
    std::atomic <size_t> numAllocatedArenas;
    std::atomic <size_t> numAllocatedPageHandles;

public:
    inline NativePageAllocator( void ) : numAllocatedArenas( 0 ), numAllocatedPageHandles( 0 )
    {
        GetSystemInfo( &_systemInfo );
    }

    inline ~NativePageAllocator( void )
    {
        // Delete all active page handles.
        while ( !LIST_EMPTY( activeHandles.root ) )
        {
            pageHandle *handle = LIST_GETITEM( pageHandle, activeHandles.root.next, managerNode );

            Free( handle );
        }

        // Now delete any active pages.
        while ( !LIST_EMPTY( activeMemoryRanges.root ) )
        {
            pageAllocation *allocation = LIST_GETITEM( pageAllocation, activeMemoryRanges.root.next, managerNode );

            DeletePageAllocation( allocation );
        }
    }
    
    struct pageHandle;

private:
    typedef sliceOfData <SIZE_T> memBlockSlice_t;

    struct pageAllocation;

    typedef iterativeGrowableArrayEx <pageAllocation*, 8, 0, size_t, privateAllocator> residingMemBlocks_t;

public:
    struct pageHandle
    {
        friend struct NativePageAllocator;

        inline pageHandle( privateAllocator manager, memBlockSlice_t spanSlice )
            : requestedMemory( std::move( spanSlice ) ), residingMemBlocks( std::move( manager ) )
        {
            return;
        }

        inline ~pageHandle( void )
        {
            return;
        }

        inline void* GetTargetPointer( void ) const     { return (void*)requestedMemory.GetSliceStartPoint(); }
        inline size_t GetTargetSize( void ) const       { return (size_t)requestedMemory.GetSliceSize(); }

    private:
        memBlockSlice_t requestedMemory;    // slice that represents memory that can be accessed by the application

        residingMemBlocks_t residingMemBlocks;  // the memory arenas we are part of, have to be sorted by address!

        RwListEntry <pageHandle> managerNode;   // entry in the active page handle list
    };

private:
    RwList <pageHandle> activeHandles;

    // An arena that spans multiple pages.
    struct pageAllocation
    {
        inline pageAllocation( NativePageAllocator *manager, LPVOID arenaAddress, size_t numSlots )
            : pageSpan( (SIZE_T)arenaAddress, (SIZE_T)( numSlots * manager->_systemInfo.dwPageSize ) ),
              handleList( privateAllocator( &manager->_privateAlloc ) )
        {
            this->manager = manager;

            SIZE_T arenaSpanSize = (SIZE_T)( numSlots * manager->_systemInfo.dwPageSize );

            this->arenaAddress = arenaAddress;
            this->allocSize = arenaSpanSize;

            this->refCount = 0;

            this->slotCount = numSlots;
        }

        inline ~pageAllocation( void )
        {
            // Make sure nobody uses us anymore.
            assert( this->refCount == 0 );

            // Release the allocated arena.
            BOOL freeSuccess = VirtualFree( this->arenaAddress, 0, MEM_RELEASE );

            assert( freeSuccess == TRUE );
        }

        // DEBUG.
        inline void CheckForCollision( const memBlockSlice_t& memoryRegion )
        {
#ifdef _DEBUG
            // DEBUG: check for intersection, meow.
            {
                for ( size_t n = 0; n < this->handleList.GetCount(); n++ )
                {
                    pageHandle *foreignHandle = this->handleList.GetFast( n );

                    memBlockSlice_t::eIntersectionResult intRes = foreignHandle->requestedMemory.intersectWith( memoryRegion );

                    if ( memBlockSlice_t::isFloatingIntersect( intRes ) == false )
                    {
                        // Oh no :(
                        __debugbreak();
                    }
                }
            }
#endif
        }

        // You have to add items after another that actually come in memory-order after each other!
        inline void RegisterPageHandleSortedOrder( pageHandle *handle, size_t insertionIndex )
        {
            this->refCount++;

            this->handleList.InsertItem( handle, insertionIndex );

            handle->residingMemBlocks.AddItem( this );
        }

        inline void InsertPageHandle( pageHandle *handle, size_t handleInsertionIndex, size_t residingInsertionIndex )
        {
            this->refCount++;

            this->handleList.InsertItem( handle, handleInsertionIndex );

            handle->residingMemBlocks.InsertItem( this, residingInsertionIndex );
        }

        inline void UnregisterPageHandle( pageHandle *handle )
        {
            handle->residingMemBlocks.RemoveItem( this );

            this->handleList.RemoveItem( handle );

            this->refCount--;
        }

        inline bool IsBlockBeingUsed( void ) const
        {
            return ( refCount != 0 );
        }

        NativePageAllocator *manager;

        LPVOID arenaAddress;        // address of the memory arena
        SIZE_T allocSize;           // number in bytes for the allocation range

        memBlockSlice_t pageSpan;       // slice which spans the allocation range

        unsigned int refCount;          // number of handles using this page

        typedef iterativeGrowableArrayEx <pageHandle*, 8, 0, size_t, privateAllocator> handleList_t;

        handleList_t handleList;    // list of pageHandle's that reference this allocation
        size_t slotCount;           // amount of slots we have space for.

        RwListEntry <pageAllocation> managerNode;   // node in the NativePageAllocator allocation list.
        RwListEntry <pageAllocation> sortedNode;
    };

    inline SIZE_T GetAllocationArenaRange( size_t spanSize ) const
    {
        // Returns a rounded up value that determines region of RESERVE allocation.
        return ALIGN( (SIZE_T)spanSize, (SIZE_T)_systemInfo.dwAllocationGranularity, (SIZE_T)_systemInfo.dwAllocationGranularity );
    }

    inline SIZE_T GetPageAllocationRange( size_t spanSize ) const
    {
        // Returns a rounded up value that determines the actual size of a page allocation.
        return ALIGN( (SIZE_T)spanSize, (SIZE_T)_systemInfo.dwPageSize, (SIZE_T)_systemInfo.dwPageSize );
    }

    RwList <pageAllocation> activeMemoryRanges;
    RwList <pageAllocation> sortedMemoryRanges;

    inline void SortedMemoryBlockInsert( pageAllocation *memBlock )
    {
        RwListEntry <pageAllocation> *insertAfter = &sortedMemoryRanges.root;

        SIZE_T insertMemBlockAddress = (SIZE_T)memBlock->arenaAddress;

        LIST_FOREACH_BEGIN( pageAllocation, sortedMemoryRanges.root, sortedNode )
            // Get the address of the list item as number.
            SIZE_T memBlockAddress = (SIZE_T)item->arenaAddress;

            if ( memBlockAddress > insertMemBlockAddress )
            {
                break;
            }

            insertAfter = iter;
        LIST_FOREACH_END

        LIST_APPEND( *insertAfter, memBlock->sortedNode );
    }

    template <typename callbackType>
    AINLINE static void ProcessInclinedMemoryChunk( pageAllocation *arenaHandle, const memBlockSlice_t& memoryRegion, callbackType& cb )
    {
        memBlockSlice_t sharedSlice;

        bool hasPosition = memoryRegion.getSharedRegion( arenaHandle->pageSpan, sharedSlice );

        if ( hasPosition )
        {
            // Tell the meow.
            cb.Process( sharedSlice );
        }
    }

    template <typename callbackType>
    inline static void SortedProcessMemoryChunks( const residingMemBlocks_t& processArenas, const memBlockSlice_t& memoryRegion, callbackType& cb )
    {
        const size_t numArenas = processArenas.GetCount();

        for ( size_t n = 0; n < numArenas; n++ )
        {
            // Check what this allocation has to say.
            pageAllocation *arenaHandle = processArenas.GetFast( n );

            ProcessInclinedMemoryChunk( arenaHandle, memoryRegion, cb );
        }

        // Done.
    }

    struct arenaCommitOperator
    {
        AINLINE void Process( const memBlockSlice_t& allocRegion )
        {
            LPVOID commitHandle = VirtualAlloc( (void*)allocRegion.GetSliceStartPoint(), allocRegion.GetSliceSize(), MEM_COMMIT, PAGE_READWRITE );

            assert( commitHandle == (void*)allocRegion.GetSliceStartPoint() );
        }
    };

    inline static void CommitMemoryOfPageHandle( pageHandle *theHandle, const memBlockSlice_t& commitRegion )
    {
        arenaCommitOperator commitOp;

        SortedProcessMemoryChunks( theHandle->residingMemBlocks, commitRegion, commitOp );
    }

    struct arenaDecommitOperator
    {
        AINLINE void Process( const memBlockSlice_t& allocRegion )
        {
            BOOL decommitSuccess = VirtualFree( (void*)allocRegion.GetSliceStartPoint(), allocRegion.GetSliceSize(), MEM_DECOMMIT );

            assert( decommitSuccess == TRUE );
        }
    };

    inline static void DecommitMemoryOfPageHandle( pageHandle *theHandle, const memBlockSlice_t& decommitRegion )
    {
        arenaDecommitOperator decommitOp;

        SortedProcessMemoryChunks( theHandle->residingMemBlocks, decommitRegion, decommitOp );
    }

    inline pageAllocation* NewArenaAllocation( LPVOID arenaAddress, size_t numSlots )
    {
        this->numAllocatedArenas++;

        return new (_privateAlloc.Allocate( sizeof( pageAllocation ), 0 )) pageAllocation( this, arenaAddress, numSlots );
    }

    inline void FreeArenaAllocation( pageAllocation *arenaPtr )
    {
        this->numAllocatedArenas--;

        arenaPtr->~pageAllocation();

        _privateAlloc.Free( arenaPtr );
    }

    inline void DeletePageAllocation( pageAllocation *memRange )
    {
        LIST_REMOVE( memRange->sortedNode );
        LIST_REMOVE( memRange->managerNode );

        FreeArenaAllocation( memRange );
    }

    struct memReserveAllocInfo
    {
        size_t hostArenaInsertBeforeIndex;
        pageAllocation *hostArena;
        bool isHostArenaNewlyAllocated;
    };

    typedef iterativeGrowableArrayEx <memReserveAllocInfo, 4, 0, size_t, privateAllocator> memReserveAllocList_t;

    DEF_LIST_ITER( arenaSortedIterator_t, pageAllocation, sortedNode );

    // Finds the memory arena that hosts or is prior to a memory range.
    inline pageAllocation* FindHostingArena( const memBlockSlice_t& hostMem, bool& isHostingOut )
    {
        pageAllocation *curRegion = NULL;

        arenaSortedIterator_t iter( this->sortedMemoryRanges );

        memBlockSlice_t::eIntersectionResult intResult;

        while ( !iter.IsEnd() )
        {
            curRegion = iter.Resolve();

            intResult = hostMem.intersectWith( curRegion->pageSpan );

            if ( intResult == memBlockSlice_t::INTERSECT_BORDER_START ||
                 intResult == memBlockSlice_t::INTERSECT_INSIDE ||
                 intResult == memBlockSlice_t::INTERSECT_EQUAL )
            {
                isHostingOut = true;
                return curRegion;
            }
            if ( intResult == memBlockSlice_t::INTERSECT_FLOATING_START ||
                 intResult == memBlockSlice_t::INTERSECT_BORDER_END )
            {
                return NULL;
            }

            iter.Increment();
        }

        if ( curRegion && intResult == memBlockSlice_t::INTERSECT_FLOATING_END )
        {
            isHostingOut = false;
            return curRegion;
        }

        return NULL;
    }

    inline pageAllocation* ReserveNewMemory( void *allocStartAddr, size_t allocSize )
    {
        assert( allocSize == GetAllocationArenaRange( allocSize ) );

        LPVOID allocPtr = VirtualAlloc( (LPVOID)allocStartAddr, allocSize, MEM_RESERVE, PAGE_READWRITE );

        if ( allocPtr == NULL )
            return NULL;

        pageAllocation *newHostArena = NewArenaAllocation( allocPtr, allocSize / _systemInfo.dwPageSize );

        if ( !newHostArena )
        {
            VirtualFree( allocPtr, 0, MEM_RELEASE );
            return NULL;
        }

        return newHostArena;
    }
    
    inline bool FlowAllocateAfterRegion(
        memReserveAllocList_t& areaToBeAllocatedAt, arenaSortedIterator_t& reserveArenaIter,
        const memBlockSlice_t& handleAllocRegion, const memBlockSlice_t& hostAllocRegion, pageAllocation *hostStartRegion,
        memReserveAllocList_t& allocOut )
    {
        struct temp_alloc_info_commit
        {
            pageAllocation *arenaToBeCommitted;
            RwListEntry <pageAllocation> *appendAfterNode;
        };

        typedef iterativeGrowableArrayEx <temp_alloc_info_commit, 4, 0, size_t, privateAllocator> temp_alloc_cinfo_array_t;

        temp_alloc_cinfo_array_t temp_alloc_cinfo( privateAllocator( &this->_privateAlloc ) );

        SIZE_T reqAllocEndPoint = hostAllocRegion.GetSliceEndPoint();

        bool prepareSuccess = true;

        pageAllocation *prevArena = hostStartRegion;

        bool isInsideRegion = false;

        // Proceed to ensure there are no gaps of unallocated memory.
        pageAllocation *theEndingBit = NULL;

        while ( reserveArenaIter.IsEnd() == false )
        {
            pageAllocation *nextArena = reserveArenaIter.Resolve();

            // Since each arena is allocated on real linear space, we can check the memory addresses.
            const memBlockSlice_t& primary_slice = prevArena->pageSpan;
            const memBlockSlice_t& secondary_slice = nextArena->pageSpan;

            // Fill any hole.
            SIZE_T reqMemStartPos = ( primary_slice.GetSliceEndPoint() + 1 );
            SIZE_T reqMemEndOffset = ( secondary_slice.GetSliceStartPoint() );

            // We should break if the next memory region is far-off our request.
            if ( reqMemEndOffset > reqAllocEndPoint )
            {
                theEndingBit = nextArena;
                break;
            }

            // We need to check the entire bridge part for collision, if we happen to cross an entire arena over.
            if ( isInsideRegion )
            {
                if ( prevArena->handleList.GetCount() != 0 )
                {
                    // There are allocations obstructing the arena we wanted to allocate at.
                    // So we cannot continue any further.
                    return false;
                }
            }

            bool isGap = ( reqMemStartPos != reqMemEndOffset );

            if ( isGap )
            {
                // We need to allocate a new arena here.
                SIZE_T reqMemSize = ( reqMemEndOffset - reqMemStartPos );

                pageAllocation *newArena = ReserveNewMemory( (void*)reqMemStartPos, reqMemSize );

                if ( !newArena )
                {
                    // We could fail to allocate memory for meta-data, in which case we fail.
                    prepareSuccess = false;
                    break;
                }

                // Also need to allocate at the new arena.
                {
                    memReserveAllocInfo info;
                    info.hostArena = newArena;
                    info.hostArenaInsertBeforeIndex = 0;
                    info.isHostArenaNewlyAllocated = true;

                    areaToBeAllocatedAt.AddItem( std::move( info ) );
                }

                // Remember this success and how to register it into the system.
                temp_alloc_info_commit info;
                info.arenaToBeCommitted = newArena;
                info.appendAfterNode = &prevArena->sortedNode;

                temp_alloc_cinfo.AddItem( std::move( info ) );
            }

            // We definitely have to allocate at this.
            {
                memReserveAllocInfo info;
                info.hostArena = nextArena;
                info.hostArenaInsertBeforeIndex = 0;
                info.isHostArenaNewlyAllocated = false;

                areaToBeAllocatedAt.AddItem( std::move( info ) );
            }

            // Go ahead.
            reserveArenaIter.Increment();

            prevArena = nextArena;
        }

        bool allocSuccess = false;

        if ( prepareSuccess )
        {
            const SIZE_T endAllocStartPoint = ( prevArena->pageSpan.GetSliceEndPoint() + 1 );
            const SIZE_T reqAllocEndOffset = ( reqAllocEndPoint + 1 );

            if ( endAllocStartPoint >= reqAllocEndOffset )
            {
                allocSuccess = true;
            }
            else
            {
                // Continue with allocating the ending bit, if required.
                // First we need the relationship of the ending bit to us.
                // This is to determine if we need to allocate something on an "ending bit" that is the last thing that we could ever allocate on.
                pageAllocation *reqAllocEndBit = NULL;

                if ( theEndingBit )
                {
                    memBlockSlice_t::eIntersectionResult endIntResult = handleAllocRegion.intersectWith( theEndingBit->pageSpan );

                    if ( endIntResult == memBlockSlice_t::INTERSECT_BORDER_END )
                    {
                        reqAllocEndBit = theEndingBit;
                    }
                    else if ( endIntResult == memBlockSlice_t::INTERSECT_FLOATING_START )
                    {
                        // Nothing :)
                    }
                    else
                    {
                        // Must not happen.
                        assert( 0 );
                    }
                }

                bool hasValidEndingBitAlloc = true;

                // If we have an ending bit, we want to verify beforehand if the allocation will succeed on it.
                // This is just an optimization.
                if ( reqAllocEndBit )
                {
                    // Check for collision against already allocated things.
                    // Since we are the ending bit which is intruded from the start, we just have to check the first-in-line element.
                    bool isObstructed = false;

                    if ( prevArena->handleList.GetCount() > 0 )
                    {
                        pageHandle *firstAlloc = reqAllocEndBit->handleList.Get( 0 );

                        memBlockSlice_t::eIntersectionResult intResult = handleAllocRegion.intersectWith( firstAlloc->requestedMemory );

                        isObstructed = ( memBlockSlice_t::isFloatingIntersect( intResult ) == false );
                    }

                    if ( isObstructed )
                    {
                        hasValidEndingBitAlloc = false;
                    }
                }

                if ( hasValidEndingBitAlloc )
                {
                    // Here we actually have to allocate anything that might be between ending bit and previous alloc.
                    // This gap "might exist", alright?
                    bool hasValidGapAllocation = true;

                    SIZE_T gapAllocEndOffset;

                    if ( reqAllocEndBit )
                    {
                        gapAllocEndOffset = reqAllocEndBit->pageSpan.GetSliceStartPoint();
                    }
                    else
                    {
                        gapAllocEndOffset = reqAllocEndOffset;
                    }

                    if ( endAllocStartPoint < gapAllocEndOffset )
                    {
                        hasValidGapAllocation = false;

                        SIZE_T gapAllocReqSize = ( gapAllocEndOffset - endAllocStartPoint );

                        pageAllocation *arenaHandle = ReserveNewMemory( (void*)endAllocStartPoint, gapAllocReqSize );

                        if ( arenaHandle )
                        {
                            // Remember this region as allocate-at.
                            {
                                memReserveAllocInfo info;
                                info.hostArena = arenaHandle;
                                info.hostArenaInsertBeforeIndex = 0;
                                info.isHostArenaNewlyAllocated = true;

                                areaToBeAllocatedAt.AddItem( std::move( info ) );
                            }

                            // Register this.
                            temp_alloc_info_commit info;
                            info.arenaToBeCommitted = arenaHandle;
                            info.appendAfterNode = &prevArena->sortedNode;

                            temp_alloc_cinfo.AddItem( std::move( info ) );

                            hasValidGapAllocation = true;
                        }
                    }

                    if ( hasValidGapAllocation )
                    {
                        // The only thing remaining is the allocation on the ending bit, which we have already verified to work.
                        // This operation of allocating data could also fail, but we dont have that kind of error checking currently.

                        if ( reqAllocEndBit )
                        {
                            memReserveAllocInfo allocInfo;
                            allocInfo.hostArena = reqAllocEndBit;
                            allocInfo.hostArenaInsertBeforeIndex = 0;
                            allocInfo.isHostArenaNewlyAllocated = false;

                            areaToBeAllocatedAt.AddItem( std::move( allocInfo ) );
                        }

                        // Success!
                        allocSuccess = true;
                    }
                }
            }
        }

        if ( !allocSuccess )
        {
            // Clean up after ourselves.
            // Those arenas never accounted to anything anyway.
            for ( size_t n = 0; n < temp_alloc_cinfo.GetCount(); n++ )
            {
                const temp_alloc_info_commit& info = temp_alloc_cinfo.Get( n );

                pageAllocation *arena = info.arenaToBeCommitted;

                FreeArenaAllocation( arena );
            }
        }
        else
        {
            // Commit the change to the system.
            for ( size_t n = 0; n < temp_alloc_cinfo.GetCount(); n++ )
            {
                const temp_alloc_info_commit& info = temp_alloc_cinfo.Get( n );

                pageAllocation *arena = info.arenaToBeCommitted;

                LIST_INSERT( *info.appendAfterNode, info.arenaToBeCommitted->sortedNode );  
                LIST_APPEND( this->activeMemoryRanges.root, arena->managerNode );
            }

            // Return the arena handles where the memory request should take place at.
            allocOut = std::move( areaToBeAllocatedAt );
        }

        return allocSuccess;
    }

    inline bool FlowAllocateRegion(
        const memBlockSlice_t& handleAllocRegion, const memBlockSlice_t& hostAllocRegion, pageAllocation *hostStartRegion, size_t hostStartAllocInsertIndex, bool isHostStartRegionNewlyAllocated,
        memReserveAllocList_t& allocOut )
    {
        // Check for some logical things by parameters.
#ifdef _DEBUG
        {
            // Handle alloc region (page memory space) must be inside or equal to host alloc region (reserve memory space).
            memBlockSlice_t::eIntersectionResult intResult = handleAllocRegion.intersectWith( hostAllocRegion );

            assert( intResult == memBlockSlice_t::INTERSECT_INSIDE || intResult == memBlockSlice_t::INTERSECT_EQUAL );
        }
#endif //_DEBUG

        // We found a valid allocation spot!
        // Thus we should allocate any non-reserved pages inbetween.
        // If even that succeeds, we are set.
        memReserveAllocList_t areaToBeAllocatedAt( privateAllocator( &this->_privateAlloc ) );

        // We know that we start in a valid allocation region.
        arenaSortedIterator_t reserveArenaIter( this->sortedMemoryRanges, &hostStartRegion->sortedNode );

        // Check if the starting region is even relevant.
        memBlockSlice_t::eIntersectionResult intResult = handleAllocRegion.intersectWith( hostStartRegion->pageSpan );

        assert( memBlockSlice_t::isFloatingIntersect( intResult ) == false );
        {
            // Add the starting region as allocate-at.
            memReserveAllocInfo info;
            info.hostArena = hostStartRegion;
            info.hostArenaInsertBeforeIndex = hostStartAllocInsertIndex;
            info.isHostArenaNewlyAllocated = isHostStartRegionNewlyAllocated;

            areaToBeAllocatedAt.AddItem( std::move( info ) );
        }

        reserveArenaIter.Increment();

        return FlowAllocateAfterRegion( areaToBeAllocatedAt, reserveArenaIter, handleAllocRegion, hostAllocRegion, hostStartRegion, allocOut );
    }

    template <typename numberType>
    AINLINE numberType SCALE_DOWN( numberType value, numberType modval )
    {
        numberType divfact = ( value / modval );

        return ( modval * divfact );
    }

    inline bool FindSortedMemoryHandleInsertionIndex( pageAllocation *arenaHandle, const memBlockSlice_t& memRegion, size_t& handleIndexOut )
    {
        // In order to even allocate, the memory region must intersect with the arena's.
        // We assume that this is always the case.
#ifdef _DEBUG
        {
            memBlockSlice_t::eIntersectionResult intResult = memRegion.intersectWith( arenaHandle->pageSpan );

            assert( memBlockSlice_t::isFloatingIntersect( intResult ) == false );
        }
#endif //_DEBUG

        const size_t numAllocations = arenaHandle->handleList.GetCount();

        for ( size_t n = 0; n < numAllocations; n++ )
        {
            pageHandle *alloc = arenaHandle->handleList.Get( n );

            // Check what to make of this.
            memBlockSlice_t::eIntersectionResult intResult = memRegion.intersectWith( alloc->requestedMemory );

            if ( intResult == memBlockSlice_t::INTERSECT_FLOATING_START )
            {
                // Our requested memory does not conflict with anything and is prior to the current thing.
                handleIndexOut = n;
                return true;
            }
            if ( intResult == memBlockSlice_t::INTERSECT_FLOATING_END )
            {
                // We can continue ahead.
            }
            else
            {
                // There was some sort of collision, which is bad.
                return false;
            }
        }

        // We did not collide, so we are good.
        handleIndexOut = numAllocations;
        return true;
    }

    inline static bool IsAllocationObstructed( const memBlockSlice_t& handleAllocSlice, pageHandle *obstructAlloc )
    {
        // Check if we are obstructed by the (next) resident memory.
        // This does not guarrentee allocability on its own, but it gives us a good idea.

        memBlockSlice_t::eIntersectionResult intResult = handleAllocSlice.intersectWith( obstructAlloc->requestedMemory );

        return ( memBlockSlice_t::isFloatingIntersect( intResult ) == false );
    }

    inline static pageAllocation* NormalizeArenaAllocSpot( arenaSortedIterator_t& sortedIter, size_t& currentArenaBlockIndex )
    {
        if ( sortedIter.IsEnd() )
            return NULL;

        pageAllocation *currentArena = sortedIter.Resolve();

        // Find the next-in-line allocation that we could collide against.
        while ( true )
        {
            if ( currentArenaBlockIndex < currentArena->handleList.GetCount() )
            {
                break;
            }

            sortedIter.Increment();
            currentArenaBlockIndex = 0;

            if ( sortedIter.IsEnd() )
            {
                return NULL;
            }

            currentArena = sortedIter.Resolve();
        }

        return currentArena;
    }

    inline static bool IsAllocationObstructedByNext( arenaSortedIterator_t& sortedIter, size_t& currentArenaBlockIndex, const memBlockSlice_t& handleAllocSlice )
    {
        pageAllocation *currentArena = NormalizeArenaAllocSpot( sortedIter, currentArenaBlockIndex );

        if ( !currentArena )
            return false;

        pageHandle *obstructAlloc = currentArena->handleList.Get( currentArenaBlockIndex );

        // Is the selected allocation spot available?
        return IsAllocationObstructed( handleAllocSlice, obstructAlloc );
    }

    // Try placement of memory allocation on a specific memory address.
    inline bool PlaceMemoryRequest( memBlockSlice_t handleMemSlice, memReserveAllocList_t& allocOut )
    {
        // Search for the position of the starting address.
        SIZE_T numMemAddr;
        SIZE_T numMemSize;
        {
            // We have to convert this request into valid unmistakeable parameters.
            SIZE_T realMemAddrEnd = ALIGN_SIZE( handleMemSlice.GetSliceEndPoint() + 1, (SIZE_T)_systemInfo.dwAllocationGranularity );
            SIZE_T realMemAddrStart = SCALE_DOWN( handleMemSlice.GetSliceStartPoint(), (SIZE_T)_systemInfo.dwAllocationGranularity );

            numMemAddr = realMemAddrStart;
            numMemSize = ( realMemAddrEnd - realMemAddrStart );
        }

        const memBlockSlice_t searchMemoryRegion( numMemAddr, numMemSize );

        // See if we have a hosting arena.
        bool hostArenaIsHosting;
        pageAllocation *hostArena = FindHostingArena( searchMemoryRegion, hostArenaIsHosting );

        // If we dont, then we have to allocate one.
        bool isHostArenaAllocated = false;

        if ( !hostArena || !hostArenaIsHosting )
        {
            // Allocation has to happen until we are at the next arena.
            // If there is no next arena, we can allocate everything in one go.
            pageAllocation *nextArena = NULL;

            if ( !hostArena )
            {
                if ( !LIST_EMPTY( this->sortedMemoryRanges.root ) )
                {
                    nextArena = LIST_GETITEM( pageAllocation, this->sortedMemoryRanges.root.next, sortedNode );
                }
            }
            // If ( hostArena != NULL ) then we dont have a next due to FindHostingArena logic.

            SIZE_T allocStartAddr = numMemAddr;
            SIZE_T allocEndOffset;

            if ( nextArena )
            {
                allocEndOffset = ( nextArena->pageSpan.GetSliceEndPoint() + 1 );
            }
            else
            {
                allocEndOffset = ( numMemAddr + numMemSize );
            }

            SIZE_T allocSize = ( allocEndOffset - allocStartAddr );

            hostArena = ReserveNewMemory( (void*)allocStartAddr, allocSize );

            if ( !hostArena )
            {
                // The Operating System refused our request.
                return false;
            }

            // Register this arena.
            LIST_INSERT( this->activeMemoryRanges.root, hostArena->managerNode );

            if ( nextArena )
            {
                LIST_APPEND( nextArena->sortedNode, hostArena->sortedNode );
            }
            else
            {
                LIST_APPEND( this->sortedMemoryRanges.root, hostArena->sortedNode );
            }
            
            isHostArenaAllocated = true;
        }

        // Find allocation index on the host arena.
        size_t handleAllocIndex = 0;

        bool canAllocateOnArena = FindSortedMemoryHandleInsertionIndex( hostArena, handleMemSlice, handleAllocIndex );

        if ( !canAllocateOnArena )
        {
            if ( isHostArenaAllocated )
            {
                DeletePageAllocation( hostArena );
            }

            return false;
        }

        // Continue allocation.
        bool wasAllocSuccess = FlowAllocateRegion( handleMemSlice, searchMemoryRegion, hostArena, handleAllocIndex, isHostArenaAllocated, allocOut );

        if ( !wasAllocSuccess )
        {
            if ( isHostArenaAllocated )
            {
                DeletePageAllocation( hostArena );
            }
        }

        return wasAllocSuccess;
    }

    // Find and allocate required memory, if possible.
    inline bool SearchForReservedMemory( size_t memSize, memBlockSlice_t& handleAllocSliceOut, memReserveAllocList_t& allocOut )
    {
        // We have to scan all reserved and/or committed memory for space that we can use.
        // This is so that we can reuse as most memory as possible.
        // If this fails we go ahead and ask Windows itself for new memory arenas.

        memBlockSlice_t handleAllocSlice( (SIZE_T)0, (SIZE_T)memSize );

        size_t alignedMemSize = GetAllocationArenaRange( memSize );

        pageAllocation *allocSelectArena = NULL;
        size_t curArenaPassAllocIndex = 0;      // the index we could insert a new allocation into if it does not obstruct things.

        arenaSortedIterator_t sortedIter( this->sortedMemoryRanges );
        
        if ( !sortedIter.IsEnd() )
        {
            pageAllocation *currentArena = sortedIter.Resolve();
            size_t currentArenaBlockIndex = 0;

            handleAllocSlice.SetSlicePosition( (SIZE_T)currentArena->arenaAddress );

            do
            {
                // Note that an optimization behavior in this function is that we allocate at maximum free space.
                // When we tried and failed at maximum free space, we skip the entire space!
                // This is perfectly valid under the fact that memory allocation establishes one block of contiguous memory.

                allocSelectArena = currentArena;
                curArenaPassAllocIndex = currentArenaBlockIndex;

                // Check if there is an obstruction in the next-in-line item.
                // Because we are address-sorted, this is a very fast operation.
                bool isCurrentAllocationSpotObstructed = false;
                {
                    currentArena = NormalizeArenaAllocSpot( sortedIter, currentArenaBlockIndex );

                    if ( currentArena )
                    {
                        pageHandle *obstructAlloc = currentArena->handleList.Get( currentArenaBlockIndex );

                        // Is the selected allocation spot available?
                        isCurrentAllocationSpotObstructed = IsAllocationObstructed( handleAllocSlice, obstructAlloc );
                    }
                }

                // If we are obstructing, then we must go on.
                if ( !isCurrentAllocationSpotObstructed )
                {
                    // Get the real allocation slice for the arena region.
                    const SIZE_T arenaAllocStart = SCALE_DOWN( (SIZE_T)handleAllocSlice.GetSliceStartPoint(), (SIZE_T)_systemInfo.dwAllocationGranularity );
                    const SIZE_T arenaAllocEnd = ALIGN_SIZE( (SIZE_T)( handleAllocSlice.GetSliceEndPoint() + 1 ), (SIZE_T)_systemInfo.dwAllocationGranularity );

                    SIZE_T arenaAllocSize = ( arenaAllocEnd - arenaAllocStart );

                    const memBlockSlice_t arenaAllocSlice( arenaAllocStart, arenaAllocSize );

                    bool couldAllocate =
                        FlowAllocateRegion( handleAllocSlice, arenaAllocSlice, allocSelectArena, curArenaPassAllocIndex, false, allocOut );

                    if ( couldAllocate )
                    {
                        // We are successful, so return the allocation place.
                        handleAllocSliceOut = std::move( handleAllocSlice );
                        return true;
                    }
                }

                // Advance the current allocation attempt.
                // For that we have to check if there is a next memory location to try.
                // IMPORTANT: the next location _must_ be valid!
goAgain:
                if ( sortedIter.IsEnd() )
                {
                    // Premature end.
                    return false;
                }

                bool hasReachedEnd = ( currentArenaBlockIndex >= currentArena->handleList.GetCount() );

                currentArenaBlockIndex++;

                if ( hasReachedEnd )
                {
                    // Advance.
                    currentArenaBlockIndex = 0;

                    sortedIter.Increment();

                    if ( sortedIter.IsEnd() )
                    {
                        // If there is no more location to try for allocation, we simply fail out of
                        // our search for shared memory allocations. We will directly ask the OS for
                        // memory next.
                        return false;
                    }

                    currentArena = sortedIter.Resolve();
                }

                // On this new allocation slot, which we definitely have, we need to advance the memory request slice 
                // until we are inside of a new valid spot.
                SIZE_T newSliceStart;

                if ( currentArenaBlockIndex > 0 )
                {
                    pageHandle *prevPage = currentArena->handleList.Get( currentArenaBlockIndex - 1 );

                    newSliceStart = ( prevPage->requestedMemory.GetSliceEndPoint() + 1 );
                }
                else
                {
                    newSliceStart = (SIZE_T)currentArena->arenaAddress;
                }

                handleAllocSlice.SetSlicePosition( newSliceStart );

                // Check if said slice even starts in the current arena.
                // If it does not, then we need to check next spot.
                {
                    memBlockSlice_t::eIntersectionResult intResult = handleAllocSlice.intersectWith( currentArena->pageSpan );

                    bool isNotIntersecting = memBlockSlice_t::isFloatingIntersect( intResult );

                    if ( isNotIntersecting )
                    {
                        // We need a valid position that actually intersects the current arena!
                        goto goAgain;
                    }
                }
            }
            while ( true );
        }

        // We just failed.
        return false;
    }

    static inline bool IsValidAllocation( void *desiredAddress, size_t spanSize )
    {
        bool isValid = true;

        if ( desiredAddress != NULL )
        {
            // Check that there is no number overflow.
            SIZE_T memDesiredAddress = (SIZE_T)desiredAddress;
            SIZE_T memSpanSize = (SIZE_T)spanSize;

            SIZE_T memAddressBorder = ( memDesiredAddress + memSpanSize );

            // The condition I check here is that if I add those two numbers,
            // the result must be bigger than the source operand I added to.
            isValid = ( memAddressBorder > memDesiredAddress );
        }
        return isValid;
    }

public:
    inline pageHandle* Allocate( void *desiredAddress, size_t spanSize )
    {
        pageHandle *theHandle = NULL;

        // Only proceed if the requested allocation is valid.
        if ( IsValidAllocation( desiredAddress, spanSize ) )
        {
            // Properly align the allocation request on page boundaries.
            SIZE_T pageDesiredAddressStart = SCALE_DOWN( (SIZE_T)desiredAddress, (SIZE_T)_systemInfo.dwPageSize );
            SIZE_T pageDesiredAddressEnd = ALIGN_SIZE( (SIZE_T)desiredAddress + (SIZE_T)spanSize, (SIZE_T)_systemInfo.dwPageSize );

            SIZE_T pageSpanSize = ( pageDesiredAddressEnd - pageDesiredAddressStart );

            // Determine the pages that should host the requested memory region.
            memReserveAllocList_t hostPages( privateAllocator( &this->_privateAlloc ) );
            bool validAllocation = false;

            // The actual allocation slice.
            memBlockSlice_t pageDesiredMemSlice( pageDesiredAddressStart, pageSpanSize );

            // We first have to find pages that can host our memory.
            {
                // If we know the address we should allocate on, we attempt to find regions that have already been allocated
                // so they can host our memory.
                if ( pageDesiredAddressStart != NULL )
                {
                    validAllocation = PlaceMemoryRequest( pageDesiredMemSlice, hostPages );
                }
                else
                {
                    // Otherwise we have to search for a new spot.
                    validAllocation = SearchForReservedMemory( pageSpanSize, pageDesiredMemSlice, hostPages );

                    if ( !validAllocation )
                    {
                        // As a last resort, request memory from the OS.
                        size_t arenaSpanSize = GetAllocationArenaRange( pageSpanSize );

                        pageAllocation *newArena = ReserveNewMemory( NULL, arenaSpanSize );

                        if ( newArena )
                        {
                            // We allocate at the start of the new arena.
                            pageDesiredMemSlice.SetSlicePosition( (SIZE_T)newArena->arenaAddress );

                            memReserveAllocInfo allocInfo;
                            allocInfo.hostArena = newArena;
                            allocInfo.hostArenaInsertBeforeIndex = 0;
                            allocInfo.isHostArenaNewlyAllocated = true;

                            hostPages.AddItem( std::move( allocInfo ) );

                            // Register this new reserved memory.
                            LIST_INSERT( this->activeMemoryRanges.root, newArena->managerNode );
                            SortedMemoryBlockInsert( newArena );

                            validAllocation = true;
                        }
                    }
                }
            }

            if ( validAllocation )
            {
                // Create a pageHandle to it.
                pageHandle *newHandle = new (_privateAlloc.Allocate( sizeof( pageHandle ), 0 )) pageHandle( privateAllocator( &this->_privateAlloc ), pageDesiredMemSlice );

                if ( newHandle )
                {
                    // Register it inside the host pages.
                    size_t count = hostPages.GetCount();

                    // IMPORTANT: hostPages is sorted in memory address order!
                    for ( size_t n = 0; n < count; n++ )
                    {
                        const memReserveAllocInfo& info = hostPages.Get( n );

                        pageAllocation *allocation = info.hostArena;

#ifdef _PARANOID_MEMTESTS_
                        // DEBUG.
                        allocation->CheckForCollision( newHandle->requestedMemory );
#endif //_PARANOID_MEMTESTS_

                        allocation->RegisterPageHandleSortedOrder( newHandle, info.hostArenaInsertBeforeIndex );
                    }

                    // Register the handle.
                    LIST_INSERT( activeHandles.root, newHandle->managerNode );

                    this->numAllocatedPageHandles++;

                    // Put the memory active in the OS.
                    CommitMemoryOfPageHandle( newHandle, pageDesiredMemSlice );

                    theHandle = newHandle;
                }
            }

            if ( theHandle == NULL )
            {
                // Delete all allocated pages.
                const size_t count = hostPages.GetCount();

                for ( unsigned int n = 0; n < count; n++ )
                {
                    const memReserveAllocInfo& info = hostPages.Get( n );

                    if ( info.isHostArenaNewlyAllocated )
                    {
                        pageAllocation *thePage = info.hostArena;

                        DeletePageAllocation( thePage );
                    }
                }
            }
        }

        return theHandle;
    }

    inline pageHandle* FindHandleByAddress( void *pAddress )
    {
        // Just compare addresses of every alive handle and return
        // the one that matches the query.
        LIST_FOREACH_BEGIN( pageHandle, activeHandles.root, managerNode )
            if ( item->GetTargetPointer() == pAddress )
            {
                return item;
            }
        LIST_FOREACH_END

        return NULL;
    }

private:
    // Helper function to get a signed difference between two unsigned numbers.
    template <typename numberType>
    static inline numberType GetSignedDifference( const numberType& left, const numberType& right, bool& isSigned )
    {
        bool _isSigned = ( left < right );

        numberType result;

        if ( _isSigned )
        {
            result = ( right - left );
        }
        else
        {
            result = ( left - right );
        }

        isSigned = _isSigned;

        return result;
    }

public:
    // Attempts to update the handle size so that either more or less memory
    // can be used.
    inline bool SetHandleSize( pageHandle *theHandle, size_t _unaligned_newReserveSize )
    {
        // Properly align the request size.
        // This is important because we represent real memory pages.
        size_t newReserveSize = GetPageAllocationRange( _unaligned_newReserveSize );

        // Do nothing if the handle size has not changed.
        size_t oldSize = theHandle->GetTargetSize();

        if ( newReserveSize == oldSize )
            return true;

        if ( newReserveSize == 0 )
            return false;

        bool isSigned;
        size_t memSizeDifference = GetSignedDifference( newReserveSize, oldSize, isSigned );

        bool success = false;

        if ( !isSigned )
        {
            // Make sure that this allocation is valid.
            // It can only turn invalid if the memory size is greater than before.
            if ( IsValidAllocation( theHandle->GetTargetPointer(), newReserveSize ) )
            {
                // If the new memory size is greater than the old,
                // allocate additional memory pages, on demand of course.
                memBlockSlice_t requiredRegion( (SIZE_T)theHandle->GetTargetPointer() + (SIZE_T)oldSize, (SIZE_T)memSizeDifference );
                
                size_t countOfArenas = theHandle->residingMemBlocks.GetCount();

                assert( countOfArenas > 0 );

                pageAllocation *lastArenaOfSpace = theHandle->residingMemBlocks.Get( countOfArenas - 1 );

                // Check if we collide against something on expansion request.
                // If we do, we basically cannot expand anyway.
                bool isCollidingAgainstStuff = false;
                {
                    size_t indexOfItemOnArena = 0;

                    bool hasFoundItem = lastArenaOfSpace->handleList.Find( theHandle, indexOfItemOnArena );

                    assert( hasFoundItem == true );

                    // We try accessing the next item.
                    indexOfItemOnArena++;

                    arenaSortedIterator_t sortedIter( this->sortedMemoryRanges, &lastArenaOfSpace->sortedNode );

                    isCollidingAgainstStuff = IsAllocationObstructedByNext( sortedIter, indexOfItemOnArena, requiredRegion );
                }

                if ( !isCollidingAgainstStuff )
                {
                    // Now we simply allocate the region(s) after the memory and
                    // merge the two (or more) allocation regions into one.
                    memReserveAllocList_t listOfExpandedArenas_input( privateAllocator( &this->_privateAlloc ) );

                    arenaSortedIterator_t sortedIter( this->sortedMemoryRanges, &lastArenaOfSpace->sortedNode );

                    sortedIter.Increment();

                    // Create the arena-aligned region around the required region.
                    SIZE_T arenaAllocStart = SCALE_DOWN( requiredRegion.GetSliceStartPoint(), (SIZE_T)_systemInfo.dwAllocationGranularity );
                    SIZE_T arenaAllocEnd = ALIGN_SIZE( requiredRegion.GetSliceEndPoint() + 1, (SIZE_T)_systemInfo.dwAllocationGranularity );

                    SIZE_T arenaAllocSize = ( arenaAllocEnd - arenaAllocStart );

                    memBlockSlice_t arenaAllocSlice( arenaAllocStart, arenaAllocSize );

                    memReserveAllocList_t expansionOut( privateAllocator( &this->_privateAlloc ) );

                    bool flowAllocExpandSuccess =
                        FlowAllocateAfterRegion( listOfExpandedArenas_input, sortedIter, requiredRegion, arenaAllocSlice, lastArenaOfSpace, expansionOut );

                    // Have we succeeded in reserving the requested memory pages?
                    if ( flowAllocExpandSuccess )
                    {
                        // Add the things together, merge them.
                        const size_t numExpansions = expansionOut.GetCount();

                        for ( size_t n = 0; n < numExpansions; n++ )
                        {
                            const memReserveAllocInfo& info = expansionOut.Get( n );

                            pageAllocation *hostArena = info.hostArena;

#ifdef _PARANOID_MEMTESTS_
                            // DEBUG.
                            hostArena->CheckForCollision( theHandle->requestedMemory );
                            hostArena->CheckForCollision( requiredRegion );
#endif //_PARANOID_MEMTESTS_

                            hostArena->RegisterPageHandleSortedOrder( theHandle, info.hostArenaInsertBeforeIndex );
                        }

                        // Now update the OS.
                        CommitMemoryOfPageHandle( theHandle, requiredRegion );

                        success = true;
                    }
                }
            }
        }
        else
        {
            // Otherwise the new memory size is smaller than the old.
            // We potentially have to remove pages from the residency list.

            memBlockSlice_t requiredRegion( (SIZE_T)theHandle->GetTargetPointer() + (SIZE_T)newReserveSize, (SIZE_T)memSizeDifference );
            
            // Update the OS.
            DecommitMemoryOfPageHandle( theHandle, requiredRegion );

            pageAllocation *lastArenaReside = NULL;
            
            size_t numResidingArenas = theHandle->residingMemBlocks.GetCount();

            // Determine the amount of arenas that should be dereferenced in the course of this influence area shrinking.
            size_t iter = numResidingArenas;

            while ( iter > 0 )
            {
                iter--;

                pageAllocation *oneFromTail = theHandle->residingMemBlocks.GetFast( iter );

                bool doRemove = false;
                bool doBreak = false;

                // Check what relationship this arena has to the shrinking.
                memBlockSlice_t::eIntersectionResult intResult = requiredRegion.intersectWith( oneFromTail->pageSpan );

                if ( intResult == memBlockSlice_t::INTERSECT_EQUAL )
                {
                    doRemove = true;
                    doBreak = true;
                }
                else if ( intResult == memBlockSlice_t::INTERSECT_ENCLOSING )
                {
                    doRemove = true;
                }
                else if ( intResult == memBlockSlice_t::INTERSECT_INSIDE )
                {
                    doBreak = true;
                }
                else if ( intResult == memBlockSlice_t::INTERSECT_BORDER_START )
                {
                    doBreak = true;
                }
                else if ( intResult == memBlockSlice_t::INTERSECT_BORDER_END )
                {
                    doRemove = true;
                }
                else if ( intResult == memBlockSlice_t::INTERSECT_FLOATING_START )
                {
                    doRemove = true;
                }
                else if ( intResult == memBlockSlice_t::INTERSECT_FLOATING_END )
                {
                    doBreak = true;
                }
                else
                {
                    assert( 0 );
                }

                if ( doRemove )
                {
                    oneFromTail->handleList.RemoveItem( theHandle );
                        
                    oneFromTail->refCount--;

                    theHandle->residingMemBlocks.RemoveItem( iter );

                    MemBlockGarbageCollection( oneFromTail );

                    numResidingArenas--;
                }

                if ( doBreak )
                {
                    break;
                }
            }

            success = true;
        }

        if ( success )
        {
            // Set the new handle region.
            theHandle->requestedMemory.SetSliceEndPoint( (SIZE_T)theHandle->GetTargetPointer() + (SIZE_T)newReserveSize - (SIZE_T)1 );
        }

        return success;
    }

private:
    inline void MemBlockGarbageCollection( pageAllocation *memBlock )
    {
        // If the page is not being used anymore, release it.
        if ( !memBlock->IsBlockBeingUsed() )
        {
            DeletePageAllocation( memBlock );
        }
    }

public:
    inline void Free( pageHandle *memRange )
    {
        // Release the contents of the memory to the OS.
        DecommitMemoryOfPageHandle( memRange, memRange->requestedMemory );

        // Free the link to the allocated OS memory regions.
        while ( memRange->residingMemBlocks.GetCount() != 0 )
        {
            pageAllocation *memBlock = memRange->residingMemBlocks.Get( 0 );

            memBlock->UnregisterPageHandle( memRange );

            // Clean up memory blocks that are not used anymore.
            MemBlockGarbageCollection( memBlock );
        }

        this->numAllocatedPageHandles--;

        // Delete and unregister our pageHandle.
        LIST_REMOVE( memRange->managerNode );

        memRange->~pageHandle();

        _privateAlloc.Free( memRange );
    }

    inline bool FreeByAddress( void *pAddress )
    {
        pageHandle *theHandle = FindHandleByAddress( pAddress );

        if ( !theHandle )
            return false;

        Free( theHandle );
        return true;
    }
};

#endif //_COMMON_OS_UTILS_