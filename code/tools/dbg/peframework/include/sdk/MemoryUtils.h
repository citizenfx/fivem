/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        eirrepo/sdk/MemoryUtils.h
*  PURPOSE:     Memory management templates
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _GLOBAL_MEMORY_UTILITIES_
#define _GLOBAL_MEMORY_UTILITIES_

#include <assert.h>
#include <list>
#include <vector>
#include "rwlist.hpp"
#include "MemoryRaw.h"
#include "DataUtil.h"
#include "MetaHelpers.h"
#include <atomic>
#include <type_traits>

template <typename numberType, typename managerType>
struct FirstPassAllocationSemantics
{
    // This class exposes algorithms-only for allocating chunks of memory
    // in an address-sorted container.
    // REQUIREMENTS FOR managerType CLASS INTERFACE:
    // * REQUIREMENTS sub-type blockIter_t CLASS INTERFACE:
    //   - standard constructor (can do nothing/POD)
    //   - copy-assignable/constructible
    //   - move-assignable/constructible
    //   - void Increment( void )
    //   - memSlice_t GetMemorySlice( void )
    //   - void* GetNativePointer( void )
    // * blockIter_t GetFirstMemoryBlock( void )
    // * blockIter_t GetLastMemoryBlock( void )
    // * bool HasMemoryBlocks( void )
    // * blockIter_t GetRootNode( void )
    // * blockIter_t GetAppendNode( blockIter_t iter )
    // * bool IsEndMemoryBlock( const blockIter_t& iter ) const
    // * bool IsInAllocationRange( const memSlice_t& memRegion )
    // The programmer is encouraged to write their own storage mechanism
    // based on the output. The default storage system is linked list.

    typedef typename managerType::blockIter_t blockIter_t;

    typedef sliceOfData <numberType> memSlice_t;

    struct allocInfo
    {
        memSlice_t slice;
        numberType alignment;
        blockIter_t blockToAppendAt;
    };

    AINLINE static bool FindSpace( managerType& manager, numberType sizeOfBlock, allocInfo& infoOut, const numberType alignmentWidth = sizeof( void* ), numberType allocStart = 0 )
    {
        // Try to allocate memory at the first position we find.
        memSlice_t newAllocSlice( allocStart, sizeOfBlock );

        blockIter_t appendNode = manager.GetRootNode();

        // Make sure we align to the system integer (by default).
        // todo: maybe this is not correct all the time?

        for ( blockIter_t iter( manager.GetFirstMemoryBlock() ); manager.IsEndMemoryBlock( iter ) == false; iter.Increment() )
        {
            // Intersect the current memory slice with the ones on our list.
            memSlice_t blockSlice = iter.GetMemorySlice();

            typename memSlice_t::eIntersectionResult intResult = newAllocSlice.intersectWith( blockSlice );

            if ( !memSlice_t::isFloatingIntersect( intResult ) )
            {
                // Advance the try memory offset.
                numberType tryMemPosition = ( blockSlice.GetSliceEndPoint() + 1 );
                    
                tryMemPosition = ALIGN( tryMemPosition, alignmentWidth, alignmentWidth );

                newAllocSlice.SetSlicePosition( tryMemPosition );
            }
            else if ( intResult == memSlice_t::INTERSECT_FLOATING_END )
            {
                // We kinda encounter this when the alignment makes us skip (!) over
                // data units.
                // For that we skip memory blocks until the floating end is not true anymore
                // or we are out of blocks.
            }
            else
            {
                // This means we are floating before a next-block.
                // Perfect scenario because we choose to "append" at the previous block we met.
                break;
            }

            // Set the append node further.
            appendNode = manager.GetAppendNode( iter );
        }
        
        // The result is - by definition of this algorithm - the earliest allocation spot
        // for our request. Since there cannot be any earlier, if our request happens to be
        // outside of the allowed allocation range, all other prior requests are invalid too.
        if ( !manager.IsInAllocationRange( newAllocSlice ) )
            return false;

        infoOut.slice = std::move( newAllocSlice );
        infoOut.alignment = std::move( alignmentWidth );
        infoOut.blockToAppendAt = std::move( appendNode );
        return true;
    }

    static AINLINE bool ObtainSpaceAt( managerType& manager, numberType offsetAt, numberType sizeOfBlock, allocInfo& infoOut )
    {
        // Skip all blocks that are before us.
        memSlice_t newAllocSlice( offsetAt, sizeOfBlock );

        // Check if we even can fulfill that request.
        if ( !manager.IsInAllocationRange( newAllocSlice ) )
            return false;

        blockIter_t appendNode = manager.GetRootNode();

        typename memSlice_t::eIntersectionResult nextIntResult = memSlice_t::INTERSECT_UNKNOWN;
        bool hasNextIntersection = false;

        for ( blockIter_t iter( manager.GetFirstMemoryBlock() ); manager.IsEndMemoryBlock( iter ) == false; iter.Increment() )
        {
            // We are interested in how many blocks we actually have to skip.
            const memSlice_t& blockSlice = iter.GetMemorySlice();

            typename memSlice_t::eIntersectionResult intResult = blockSlice.intersectWith( newAllocSlice );

            if ( intResult != memSlice_t::INTERSECT_FLOATING_START )
            {
                nextIntResult = intResult;
                hasNextIntersection = true;
                break;
            }

            // We found something that is floating, so we have to check next block.
            appendNode = manager.GetAppendNode( iter );
        }

        // If we have any kind of next node, and it intersects violently, then we have no space :(
        if ( hasNextIntersection && nextIntResult != memSlice_t::INTERSECT_FLOATING_END )
        {
            // There is a collision, meow.
            return false;
        }

        // We are happy. :-)
        infoOut.slice = std::move( newAllocSlice );
        infoOut.alignment = 1;
        infoOut.blockToAppendAt = std::move( appendNode );
        return true;
    }

    static AINLINE bool CheckMemoryBlockExtension( managerType& manager, blockIter_t extItemIter, numberType newSize, memSlice_t& newSliceOut )
    {
        // Since we are sorted in address order, checking for block expansion is a piece of cake.
        memSlice_t newBlockSlice( extItemIter.GetMemorySlice().GetSliceStartPoint(), newSize );

        if ( manager.IsInAllocationRange( newBlockSlice ) )
            return false;

        extItemIter.Increment();

        if ( manager.IsEndMemoryBlock( extItemIter ) == false )
        {
            const memSlice_t& nextBlockSlice = extItemIter.GetMemorySlice();

            typename memSlice_t::eIntersectionResult intResult = newBlockSlice.intersectWith( nextBlockSlice );

            if ( memSlice_t::isFloatingIntersect( intResult ) == false )
                return false;
        }

        newSliceOut = std::move( newBlockSlice );
        return true;
    }

    static AINLINE numberType GetSpanSize( managerType& manager )
    {
        numberType theSpanSize = 0;

        if ( manager.HasMemoryBlocks() )
        {
            blockIter_t lastItem = manager.GetLastMemoryBlock();

            // Thankfully, we depend on the sorting based on memory order.
            // Getting the span size is very easy then, since the last item is automatically at the top most memory offset.
            // Since we always start at position zero, its okay to just take the end point.
            theSpanSize = ( lastItem.GetMemorySlice().GetSliceEndPoint() + 1 );
        }

        return theSpanSize;
    }
};

template <typename numberType, typename managerType, typename collisionConditionalType>
struct ConditionalAllocationProcSemantics
{
    // TODO: we still have to test this system again!

    // Conditional allocation processing semantics.
    // managerType has the same REQUIREMENTS like FirstPassAllocationSemantics
    // ADDITIONAL REQUIREMENTS FOR blockIter_t:
    // - void Decrement( void )

    typedef typename managerType::blockIter_t blockIter_t;

    typedef sliceOfData <numberType> memSlice_t;

    // TODO: we will have to revise this conditional allocation system at some point.
    
    struct conditionalRegionIterator
    {
        const managerType *manager;
        collisionConditionalType *conditional;

    private:
        numberType removalByteCount;

        blockIter_t iter;

    public:
        inline conditionalRegionIterator( const managerType *manager, collisionConditionalType& conditional )
            : manager( manager ), conditional( &conditional ), iter( manager->GetFirstMemoryBlock() )
        {
            this->removalByteCount = 0;
        }

        inline conditionalRegionIterator( const conditionalRegionIterator& right ) = default;
        inline conditionalRegionIterator( conditionalRegionIterator&& right ) = default;

        inline conditionalRegionIterator& operator = ( const conditionalRegionIterator& right ) = default;
        inline conditionalRegionIterator& operator = ( conditionalRegionIterator&& right ) = default;

        inline void Increment( void )
        {
            blockIter_t curItem = this->iter;

            void *curBlock = NULL;

            if ( !this->manager->IsEndMemoryBlock( curItem ) )
            {
                curBlock = curItem.GetNativePointer();
            }

            do
            {
                blockIter_t nextItem = curItem;
                nextItem.Increment();

                bool shouldBreak = false;

                void *nextBlock = NULL;

                if ( !this->manager->IsEndMemoryBlock( nextItem ) )
                {
                    nextBlock = nextItem.GetNativePointer();

                    if ( curBlock == NULL )
                    {
                        this->removalByteCount = 0;
                    }
                    else
                    {
                        if ( this->conditional->DoIgnoreBlock( curBlock ) )
                        {
                            const memSlice_t& nextBlockSlice = nextItem.GetMemorySlice();
                            const memSlice_t& curBlockSlice = curItem.GetMemorySlice();

                            numberType ignoreByteCount = ( nextBlockSlice.GetSliceStartPoint() - curBlockSlice.GetSliceStartPoint() );

                            this->removalByteCount += ignoreByteCount;
                        }
                        else
                        {
                            shouldBreak = true;
                        }
                    }
                }
                else
                {
                    shouldBreak = true;
                }

                curItem = std::move( nextItem );
                curBlock = std::move( nextBlock );

                if ( shouldBreak )
                {
                    break;
                }
            }
            while ( !this->manager->IsEndMemoryBlock( curItem ) );

            this->iter = std::move( curItem );
        }

        inline void Decrement( void )
        {
            blockIter_t curItem = this->iter;

            void *curBlock = NULL;

            if ( !this->manager->IsEndMemoryBlock( curItem ) )
            {
                curBlock = curItem.GetNativePointer();
            }

            do
            {
                blockIter_t prevItem = curItem;
                prevItem.Decrement();

                bool shouldBreak = false;

                void *prevBlock = NULL;

                if ( !this->manager->IsEndMemoryBlock( prevItem ) )
                {
                    prevBlock = prevItem.GetNativePointer();

                    if ( curBlock == NULL )
                    {
                        // TODO annoying shit.
                        // basically I must restore the state as if the guy went through the list straight.
                        assert( 0 );
                    }
                    else
                    {
                        if ( this->conditional->DoIgnoreBlock( prevBlock ) )
                        {
                            const memSlice_t& curBlockSlice = curItem.GetMemorySlice();
                            const memSlice_t& prevBlockSlice = prevItem.GetMemorySlice();

                            numberType ignoreByteCount = ( curBlockSlice.GetSliceStartPoint() - prevBlockSlice.GetSliceStartPoint() );

                            this->removalByteCount -= ignoreByteCount;
                        }
                        else
                        {
                            shouldBreak = true;
                        }
                    }
                }
                else
                {
                    shouldBreak = true;
                }

                curItem = std::move( prevItem );
                curBlock = std::move( prevBlock );

                if ( shouldBreak )
                {
                    break;
                }
            }
            while ( !this->manager->IsEndMemoryBlock( curItem ) );

            this->iter = std::move( curItem );
        }

        inline bool IsEnd( void ) const
        {
            return ( this->manager->IsEndMemoryBlock( this->iter ) );
        }

        inline bool IsNextEnd( void ) const
        {
            blockIter_t nextItem = this->iter;
            nextItem.Increment();

            return ( this->manager->IsEndMemoryBlock( nextItem ) );
        }

        inline bool IsPrevEnd( void ) const
        {
            blockIter_t prevItem = this->iter;
            prevItem.Decrement();

            return ( this->manager->IsEndMemoryBlock( prevItem ) );
        }

        inline decltype(auto) ResolveBlock( void ) const
        {
            return this->iter.GetNativePointer();
        }

        inline const memSlice_t& ResolveMemorySlice( void ) const
        {
            return this->iter.GetMemorySlice();
        }

        inline numberType ResolveOffset( void ) const
        {
            const memSlice_t& memSlice = this->iter.GetMemorySlice();

            return ( memSlice.GetSliceStartPoint() - this->removalByteCount );
        }

        inline numberType ResolveOffsetAfter( void ) const
        {
            void *curBlock = this->iter.GetNativePointer();

            bool ignoreCurrentBlock = this->conditional->DoIgnoreBlock( curBlock );

            const memSlice_t& memSlice = this->iter.GetMemorySlice();

            if ( ignoreCurrentBlock )
            {
                return ( memSlice.GetSliceStartPoint() - this->removalByteCount );
            }

            return ( ( memSlice.GetSliceEndPoint() + 1 ) - this->removalByteCount );
        }
    };

    // Accelerated conditional look-up routines, so that you can still easily get block offsets and span size when you want to ignore
    // certain blocks.
    static inline numberType GetSpanSizeConditional( const managerType *manager, collisionConditionalType& conditional )
    {
        conditionalRegionIterator iterator( manager, conditional );

        bool hasItem = false;
        
        if ( iterator.IsEnd() == false )
        {
            hasItem = true;

            while ( iterator.IsNextEnd() == false )
            {
                iterator.Increment();
            }
        }

        // If the list of blocks is empty, ignore.
        if ( hasItem == false )
        {
            return 0;
        }

        return iterator.ResolveOffsetAfter();
    }

    static inline bool GetBlockOffsetConditional( const managerType *manager, const void *theBlock, numberType& outOffset, collisionConditionalType& conditional )
    {
        // If the block that we request the offset of should be ignored anyway, we will simply return false.
        if ( conditional.DoIgnoreBlock( theBlock ) )
        {
            return false;
        }

        conditionalRegionIterator iterator( &manager, conditional );

        bool hasFoundTheBlock = false;

        while ( iterator.IsEnd() == false )
        {
            // We terminate if we found our block.
            if ( iterator.ResolveBlock() == theBlock )
            {
                hasFoundTheBlock = true;
                break;
            }

            iterator.Increment();
        }

        if ( hasFoundTheBlock == false )
        {
            // This should never happen.
            return false;
        }

        // Return the actual offset that preserves alignment.
        outOffset = iterator.ResolveOffset();
        return true;
    }

    // This is a very optimized algorithm for turning a static-allocation-offset into its conditional equivalent.
    static inline bool ResolveConditionalBlockOffset( const managerType *manager, numberType staticBlockOffset, numberType& outOffset, collisionConditionalType& conditional )
    {
        // FIXME.
#if 0
        // If the block that we request the offset of should be ignored anyway, we will simply return false.
        if ( conditional.DoIgnoreBlock( theBlock ) )
        {
            return false;
        }
#endif

        conditionalRegionIterator iterator( manager, conditional );

        bool hasFoundTheBlock = false;

        while ( iterator.IsEnd() == false )
        {
            // We terminate if we found our block.
            const memSlice_t& blockSlice = iterator.ResolveMemorySlice();

            numberType thisBlockOffset = blockSlice.GetSliceStartPoint();

            if ( thisBlockOffset == staticBlockOffset )
            {
                hasFoundTheBlock = true;
                break;
            }
            else if ( thisBlockOffset > staticBlockOffset )
            {
                // We have not found it.
                // Terminate early.
                break;
            }

            iterator.Increment();
        }

        if ( hasFoundTheBlock == false )
        {
            // This should never happen.
            return false;
        }

        // Return the actual offset that preserves alignment.
        outOffset = iterator.ResolveOffset();
        return true;
    }
};

template <typename numberType, typename colliderType = void>
class CollisionlessBlockAllocator
{
public:
    typedef sliceOfData <numberType> memSlice_t;

    struct block_t
    {
        RwListEntry <block_t> node;

        memSlice_t slice;
        numberType alignment;
        
        // Use this function ONLY IF block_t is allocated.
        inline void moveFrom( block_t&& right )
        {
            this->slice = std::move( right.slice );
            this->alignment = std::move( right.alignment );

            this->node.moveFrom( std::move( right.node ) );
        }
    };

    RwList <block_t> blockList;

    inline CollisionlessBlockAllocator( colliderType&& collider = colliderType() ) : allocSemMan( std::move( collider ) )
    {
        return;
    }

    inline CollisionlessBlockAllocator( const CollisionlessBlockAllocator& right ) = delete;
    inline CollisionlessBlockAllocator( CollisionlessBlockAllocator&& right ) = default;

    inline CollisionlessBlockAllocator& operator = ( const CollisionlessBlockAllocator& right ) = delete;
    inline CollisionlessBlockAllocator& operator = ( CollisionlessBlockAllocator&& right ) = default;

    // TODO: we do not check whether all blocks deallocated; I guess we implicitly do not care?

    struct allocSemanticsManager
    {
        colliderType collider;

        AINLINE allocSemanticsManager( colliderType&& collider ) : collider( std::move( collider ) )
        {
            return;
        }

        AINLINE CollisionlessBlockAllocator* GetManager( void )
        {
            return (CollisionlessBlockAllocator*)( this - offsetof(CollisionlessBlockAllocator, allocSemMan) );
        }

        AINLINE const CollisionlessBlockAllocator* GetManager( void ) const
        {
            return (const CollisionlessBlockAllocator*)( this - offsetof(CollisionlessBlockAllocator, allocSemMan) );
        }

        struct blockIter_t
        {
            AINLINE blockIter_t( void )
            {
                return;
            }

            AINLINE blockIter_t( RwListEntry <block_t>& node )
            {
                this->iter_node = &node;
            }

        private:
            AINLINE block_t* GetCurrentBlock( void ) const
            {
                return LIST_GETITEM( block_t, this->iter_node, node );
            }

        public:
            AINLINE const memSlice_t& GetMemorySlice( void ) const
            {
                return GetCurrentBlock()->slice;
            }

            AINLINE block_t* GetNativePointer( void ) const
            {
                return GetCurrentBlock();
            }

            AINLINE void Increment( void )
            {
                this->iter_node = this->iter_node->next;
            }

            AINLINE void Decrement( void )
            {
                this->iter_node = this->iter_node->prev;
            }

            RwListEntry <block_t> *iter_node;
        };

        AINLINE blockIter_t GetRootNode( void )
        {
            return ( GetManager()->blockList.root );
        }

        AINLINE blockIter_t GetFirstMemoryBlock( void ) const
        {
            return ( *GetManager()->blockList.root.next );
        }

        AINLINE blockIter_t GetLastMemoryBlock( void ) const
        {
            return ( *GetManager()->blockList.root.prev );
        }

        AINLINE bool HasMemoryBlocks( void )
        {
            return ( LIST_EMPTY( GetManager()->blockList.root ) == false );
        }

        AINLINE bool IsEndMemoryBlock( const blockIter_t& iter ) const
        {
            return ( iter.iter_node == &GetManager()->blockList.root );
        }

        AINLINE bool IsInAllocationRange( const memSlice_t& slice ) const
        {
            return collider.IsInAllocationRange( slice );
        }

        AINLINE blockIter_t GetAppendNode( blockIter_t node )
        {
            return node;
        }
    };

    allocSemanticsManager allocSemMan;

private:
    typedef FirstPassAllocationSemantics <numberType, allocSemanticsManager> allocSemantics;

public:
    typedef typename allocSemantics::allocInfo allocInfo;

    inline bool FindSpace( numberType sizeOfBlock, allocInfo& infoOut, const numberType alignmentWidth = sizeof( void* ) )
    {
        return allocSemantics::FindSpace( allocSemMan, sizeOfBlock, infoOut, alignmentWidth );
    }

    inline bool ObtainSpaceAt( numberType offsetAt, numberType sizeOfBlock, allocInfo& infoOut )
    {
        return allocSemantics::ObtainSpaceAt( allocSemMan, offsetAt, sizeOfBlock, infoOut );
    }

    inline bool SetBlockSize( block_t *allocBlock, numberType newSize )
    {
        // Is there conflict?
        // If not we adjust the block.
        return ( allocSemantics::CheckMemoryBlockExtension( allocSemMan, allocBlock->node, newSize, allocBlock->slice ) );
    }

    inline void PutBlock( block_t *allocatedStruct, allocInfo& info )
    {
        allocatedStruct->slice = std::move( info.slice );
        allocatedStruct->alignment = std::move( info.alignment );

        LIST_INSERT( *info.blockToAppendAt.iter_node, allocatedStruct->node );
    }

    inline void RemoveBlock( block_t *theBlock )
    {
        LIST_REMOVE( theBlock->node );
    }

    inline void Clear( void )
    {
        LIST_CLEAR( blockList.root );
    }

    inline numberType GetSpanSize( void )
    {
        return allocSemantics::GetSpanSize( allocSemMan );
    }
};

// Standard allocator that allows infinite space of allocation.
struct InfiniteCollisionlessAllocProxy
{
    template <typename memSliceType>
    AINLINE bool IsInAllocationRange( const memSliceType& memSlice ) const
    {
        return true;
    }
};

template <typename numberType>
using InfiniteCollisionlessBlockAllocator = CollisionlessBlockAllocator <numberType, InfiniteCollisionlessAllocProxy>;

template <size_t memorySize>
class StaticMemoryAllocator
{
    typedef InfiniteCollisionlessBlockAllocator <size_t> blockAlloc_t;

    typedef blockAlloc_t::memSlice_t memSlice_t;

    blockAlloc_t blockRegions;

public:
    AINLINE StaticMemoryAllocator( void ) : validAllocationRegion( 0, memorySize )
    {
#ifdef _DEBUG
        memset( memData, 0, memorySize );
#endif
    }

    AINLINE ~StaticMemoryAllocator( void )
    {
        return;
    }

    AINLINE bool IsValidPointer( void *ptr )
    {
        return ( ptr >= memData && ptr <= ( memData + sizeof( memData ) ) );
    }

    AINLINE void* Allocate( size_t memSize )
    {
        // We cannot allocate zero size.
        if ( memSize == 0 )
            return NULL;

        // Get the actual mem block size.
        size_t actualMemSize = ( memSize + sizeof( memoryEntry ) );

        blockAlloc_t::allocInfo allocInfo;

        bool hasSpace = blockRegions.FindSpace( actualMemSize, allocInfo );

        // The space allocation could fail if there is not enough space given by the size_t type.
        // This is very unlikely to happen, but must be taken care of.
        if ( hasSpace == false )
            return NULL;

        // Make sure we allocate in the valid region.
        {
            memSlice_t::eIntersectionResult intResult = allocInfo.slice.intersectWith( validAllocationRegion );

            if ( intResult != memSlice_t::INTERSECT_EQUAL &&
                 intResult != memSlice_t::INTERSECT_INSIDE )
            {
                return NULL;
            }
        }

        // Create the allocation information structure and return it.
        memoryEntry *entry = (memoryEntry*)( (char*)memData + allocInfo.slice.GetSliceStartPoint() );

        entry->blockSize = memSize;

        // Insert into the block manager.
        blockRegions.PutBlock( entry, allocInfo );

        return entry->GetData();
    }

    AINLINE void Free( void *ptr )
    {
        // Make sure this structure is a valid pointer from our heap.
        assert( IsValidPointer( ptr ) == true );

        // Remove the structure from existance.
        memoryEntry *entry = (memoryEntry*)ptr - 1;

        blockRegions.RemoveBlock( entry );
    }

private:
    blockAlloc_t::memSlice_t validAllocationRegion;

    struct memoryEntry : public blockAlloc_t::block_t
    {
        inline void* GetData( void )
        {
            return this + 1;
        }

        size_t blockSize;
    };

    char memData[ memorySize ];
};

// Struct registry flavor is used to fine-tune the performance of said registry by extending it with features.
// The idea is that we maximize performance by only giving it the features you really want it to have.
template <typename abstractionType>
struct cachedMinimalStructRegistryFlavor
{
    size_t pluginAllocSize;

    cachedMinimalStructRegistryFlavor( void )
    {
        // Reset to zero as we have no plugins allocated.
        this->pluginAllocSize = 0;
    }

    template <typename managerType>
    inline bool GetPluginStructOffset( managerType *manPtr, size_t handleOffset, size_t& actualOffset ) const
    {
        actualOffset = handleOffset;
        return true;
    }

    template <typename managerType>
    inline bool GetPluginStructOffsetByObject( managerType *manPtr, const abstractionType *object, size_t handleOffset, size_t& actualOffset ) const
    {
        actualOffset = handleOffset;
        return true;
    }

    template <typename managerType>
    inline size_t GetPluginAllocSize( managerType *manPtr ) const
    {
        return this->pluginAllocSize;
    }

    template <typename managerType>
    inline size_t GetPluginAllocSizeByObject( managerType *manPtr, const abstractionType *object ) const
    {
        return this->pluginAllocSize;
    }

    template <typename managerType>
    inline void UpdatePluginRegion( managerType *manPtr )
    {
        // Update the overall class size.
        // It is determined by the end of this plugin struct.
        this->pluginAllocSize = manPtr->pluginRegions.GetSpanSize();    // often it will just be ( useOffset + pluginSize ), but we must not rely on that.
    }

    inline static bool DoesUseUnifiedPluginOffset( void )
    {
        return true;
    }

    template <typename managerType>
    struct pluginInterfaceBase
    {
        // Nothing really.
    };

    template <typename managerType>
    struct regionIterator
    {
        typedef typename managerType::blockAlloc_t::block_t block_t;

    private:
        managerType *manPtr;

        RwListEntry <block_t> *iterator;

    public:
        inline regionIterator( managerType *manPtr )
        {
            this->manPtr = manPtr;

            this->iterator = manPtr->pluginRegions.blockList.root.next;
        }

        inline regionIterator( const regionIterator& right )
        {
            this->manPtr = right.manPtr;
        }

        inline void operator = ( const regionIterator& right )
        {
            this->manPtr = right.manPtr;
        }

        inline void Increment( void )
        {
            this->iterator = this->iterator->next;
        }

        inline void Decrement( void )
        {
            this->iterator = this->iterator->prev;
        }

        inline bool IsEnd( void ) const
        {
            return ( this->iterator == &this->manPtr->pluginRegions.blockList.root );
        }

        inline bool IsNextEnd( void ) const
        {
            return ( this->iterator->next == &this->manPtr->pluginRegions.blockList.root );
        }

        inline bool IsPrevEnd( void ) const
        {
            return ( this->iterator->prev == &this->manPtr->pluginRegions.blockList.root );
        }

        inline block_t* ResolveBlock( void ) const
        {
            return LIST_GETITEM( block_t, this->iterator, node );
        }

        inline size_t ResolveOffset( void ) const
        {
            block_t *curBlock = this->ResolveBlock();

            return ( curBlock->slice.GetSliceStartPoint() );
        }

        inline size_t ResolveOffsetAfter( void ) const
        {
            block_t *curBlock = this->ResolveBlock();

            return ( curBlock->slice.GetSliceEndPoint() + 1 );
        }
    };
};

template <typename abstractionType>
struct conditionalStructRegistryFlavor
{
    conditionalStructRegistryFlavor( void )
    {
        return;
    }

    template <typename managerType>
    struct runtimeBlockConditional
    {
    private:
        const managerType *manPtr;

        typedef typename managerType::allocSemanticsManager allocSemanticsManager;

    public:
        typedef ConditionalAllocationProcSemantics <size_t, allocSemanticsManager, runtimeBlockConditional> condSemantics;

        inline runtimeBlockConditional( const managerType *manPtr )
        {
            this->manPtr = manPtr;
        }

        inline bool DoIgnoreBlock( const void *theBlock ) const
        {
            // The given block is always a plugin registration, so cast it appropriately.
            const typename managerType::registered_plugin *pluginDesc = (const typename managerType::registered_plugin*)theBlock;

            pluginInterfaceBase <managerType> *pluginInterface = (pluginInterfaceBase <managerType>*)pluginDesc->descriptor;

            // Lets just ask the vendor, whether he wants the block.
            bool isAvailable = pluginInterface->IsPluginAvailableDuringRuntime( pluginDesc->pluginId );

            // We ignore a block registration if it is not available.
            return ( isAvailable == false );
        }
    };

    template <typename managerType>
    struct regionIterator
    {
    private:
        typedef runtimeBlockConditional <managerType> usedConditional;

    public:
        typedef typename usedConditional::condSemantics condSemantics;

    private:
        usedConditional conditional;
        typename condSemantics::conditionalRegionIterator iterator;

    public:
        inline regionIterator( const managerType *manPtr ) : conditional( manPtr ), iterator( &manPtr->pluginRegions.allocSemMan, conditional )
        {
            return;
        }

        inline regionIterator( const regionIterator& right ) : conditional( right.conditional ), iterator( right.iterator )
        {
            return;
        }

        inline void operator = ( const regionIterator& right )
        {
            this->conditional = right.conditional;
            this->iterator = right.iterator;
        }

        inline void Increment( void )
        {
            this->iterator.Increment();
        }

        inline void Decrement( void )
        {
            this->iterator.Decrement();
        }

        inline bool IsEnd( void ) const
        {
            return this->iterator.IsEnd();
        }

        inline bool IsNextEnd( void ) const
        {
            return this->iterator.IsNextEnd();
        }

        inline bool IsPrevEnd( void ) const
        {
            return this->iterator.IsPrevEnd();
        }

        inline decltype(auto) ResolveBlock( void ) const
        {
            return this->iterator.ResolveBlock();
        }

        inline size_t ResolveOffset( void ) const
        {
            return this->iterator.ResolveOffset();
        }

        inline size_t ResolveOffsetAfter( void ) const
        {
            return this->iterator.ResolveOffsetAfter();
        }
    };

    template <typename managerType>
    inline size_t GetPluginAllocSize( managerType *manPtr ) const
    {
        typedef runtimeBlockConditional <managerType> usedConditional;

        usedConditional conditional( manPtr );

        return usedConditional::condSemantics::GetSpanSizeConditional( &manPtr->pluginRegions.allocSemMan, conditional );
    }

    template <typename managerType>
    struct objectBasedBlockConditional
    {
    private:
        managerType *manPtr;
        const abstractionType *aliveObject;

        typedef typename managerType::allocSemanticsManager allocSemanticsManager;

    public:
        typedef ConditionalAllocationProcSemantics <size_t, allocSemanticsManager, objectBasedBlockConditional> condSemantics;

        inline objectBasedBlockConditional( managerType *manPtr, const abstractionType *aliveObject )
        {
            this->manPtr = manPtr;
            this->aliveObject = aliveObject;
        }

        inline bool DoIgnoreBlock( const void *theBlock ) const
        {
            // The given block is always a plugin registration, so cast it appropriately.
            const typename managerType::registered_plugin *pluginDesc = (const typename managerType::registered_plugin*)theBlock;

            pluginInterfaceBase <managerType> *pluginInterface = (pluginInterfaceBase <managerType>*)pluginDesc->descriptor;

            // Lets just ask the vendor, whether he wants the block.
            bool isAvailable = pluginInterface->IsPluginAvailableAtObject( this->aliveObject, pluginDesc->pluginId );

            // We ignore a block registration if it is not available.
            return ( isAvailable == false );
        }
    };

    template <typename managerType>
    inline size_t GetPluginAllocSizeByObject( managerType *manPtr, const abstractionType *object ) const
    {
        typedef objectBasedBlockConditional <managerType> usedConditional;

        usedConditional conditional( manPtr, object );

        return usedConditional::condSemantics::GetSpanSizeConditional( &manPtr->pluginRegions.allocSemMan, conditional );
    }

    template <typename managerType>
    inline bool GetPluginStructOffset( managerType *manPtr, size_t handleOffset, size_t& actualOffset ) const
    {
        typedef runtimeBlockConditional <managerType> usedConditional;

        usedConditional conditional( manPtr );

        return usedConditional::condSemantics::ResolveConditionalBlockOffset( &manPtr->pluginRegions.allocSemMan, handleOffset, actualOffset, conditional );
    }

    template <typename managerType>
    inline bool GetPluginStructOffsetByObject( managerType *manPtr, const abstractionType *object, size_t handleOffset, size_t& actualOffset ) const
    {
        typedef objectBasedBlockConditional <managerType> usedConditional;

        usedConditional conditional( manPtr, object );

        return usedConditional::condSemantics::ResolveConditionalBlockOffset( &manPtr->pluginRegions.allocSemMan, handleOffset, actualOffset, conditional );
    }

    template <typename managerType>
    inline void UpdatePluginRegion( managerType *manPtr )
    {
        // Whatever.
    }

    inline static bool DoesUseUnifiedPluginOffset( void )
    {
        return false;
    }

    template <typename managerType>
    struct pluginInterfaceBase
    {
        typedef typename managerType::pluginDescriptorType pluginDescriptorType;

        // Conditional interface. This is based on the state of the runtime.
        virtual bool IsPluginAvailableDuringRuntime( pluginDescriptorType pluginId ) const
        {
            return true;
        }

        virtual bool IsPluginAvailableAtObject( const abstractionType *object, pluginDescriptorType pluginId ) const
        {
            // Make sure that the functionality of IsPluginAvailableDuringRuntime and IsPluginAvailableAtObject logically match.
            // Basically, object must store a snapshot of the runtime state and keep it immutable, while the runtime state
            // (and by that, the type layout) may change at any time. This is best stored by some kind of bool variable.
            return true;
        }
    };
};

// Class used to register anonymous structs that can be placed on top of a C++ type.
template <typename abstractionType, typename pluginDescriptorType, typename pluginAvailabilityDispatchType, typename... AdditionalArgs>
struct AnonymousPluginStructRegistry
{
    // A plugin struct registry is based on an infinite range of memory that can be allocated on, like a heap.
    // The structure of this memory heap is then applied onto the underlying type.
    typedef InfiniteCollisionlessBlockAllocator <size_t> blockAlloc_t;

    // TODO: actually make our own allocation semantics manager so we get rid of blockAlloc_t
    typedef blockAlloc_t::allocSemanticsManager allocSemanticsManager;

    blockAlloc_t pluginRegions;

    typedef typename pluginDescriptorType::pluginOffset_t pluginOffset_t;

    pluginOffset_t preferredAlignment;

    inline AnonymousPluginStructRegistry( void )
    {
        this->preferredAlignment = (pluginOffset_t)4;
    }

    inline ~AnonymousPluginStructRegistry( void )
    {
        // TODO: allow custom memory allocators.
        return;
    }

    // Oh my fucking god. "template" can be used other than declaring a templated type?
    // Why does C++ not make it a job for the compiler to determine things... I mean it would be possible!
    // You cannot know what a pain in the head it is to find the solution to add the "template" keyword.
    // Bjarne, seriously.
    typedef typename pluginAvailabilityDispatchType::template pluginInterfaceBase <AnonymousPluginStructRegistry> pluginInterfaceBase;

    // Virtual interface used to describe plugin classes.
    // The construction process must be immutable across the runtime.
    struct pluginInterface : public pluginInterfaceBase
    {
        virtual ~pluginInterface( void )            {}

        virtual bool OnPluginConstruct( abstractionType *object, pluginOffset_t pluginOffset, pluginDescriptorType pluginId, AdditionalArgs... )
        {
            // By default, construction of plugins should succeed.
            return true;
        }

        virtual void OnPluginDestruct( abstractionType *object, pluginOffset_t pluginOffset, pluginDescriptorType pluginId, AdditionalArgs... )
        {
            return;
        }

        virtual bool OnPluginAssign( abstractionType *dstObject, const abstractionType *srcObject, pluginOffset_t pluginOffset, pluginDescriptorType pluginId, AdditionalArgs... )
        {
            // Assignment of data to another plugin struct is optional.
            return false;
        }

        virtual void DeleteOnUnregister( void )
        {
            // Overwrite this method if unregistering should delete this class.
            return;
        }
    };

    // Struct that holds information about registered plugins.
    struct registered_plugin : public blockAlloc_t::block_t
    {
        inline registered_plugin( void )
        {
            this->pluginSize = 0;
        }
        inline registered_plugin( registered_plugin&& right ) :
            pluginId( std::move( right.pluginId ) ), pluginOffset( std::move( right.pluginOffset ) ),
            descriptor( std::move( right.descriptor ) )
        {
            // NOTE THAT EVERY registered_plugin INSTANCE MUST BE ALLOCATED.

            // We know that invalid registered_plugin objects have a pluginSize of zero.
            size_t pluginSize = right.pluginSize;

            if ( pluginSize != 0 )
            {
                this->moveFrom( std::move( right ) );
            }

            this->pluginSize = pluginSize;

            right.pluginSize = 0;       // this means invalidation.
            right.pluginId = pluginDescriptorType();    // TODO: probably set this to invalid id.
            right.pluginOffset = 0;
            right.descriptor = NULL;
        }
        inline registered_plugin( const registered_plugin& right ) = delete;

        // When move assignment is happening, the object is expected to be properly constructed.
        inline registered_plugin& operator =( registered_plugin&& right )
        {
            // Is there even anything to deallocate?
            this->~registered_plugin();

            return *new (this) registered_plugin( std::move( right ) );
        }
        inline registered_plugin& operator =( const registered_plugin& right ) = delete;

        size_t pluginSize;
        pluginDescriptorType pluginId;
        pluginOffset_t pluginOffset;
        pluginInterface *descriptor;
    };

    // Container that holds plugin information.
    // TODO: Using STL types is crap (?). Use a custom type instead!
    typedef std::vector <registered_plugin> registeredPlugins_t;

    registeredPlugins_t regPlugins;

    // Holds things like the way to determine the size of all plugins.
    pluginAvailabilityDispatchType regBoundFlavor;

    inline size_t GetPluginSizeByRuntime( void ) const
    {
        return this->regBoundFlavor.GetPluginAllocSize( this );
    }

    inline size_t GetPluginSizeByObject( const abstractionType *object ) const
    {
        return this->regBoundFlavor.GetPluginAllocSizeByObject( this, object );
    }

    // Function used to register a new plugin struct into the class.
    inline pluginOffset_t RegisterPlugin( size_t pluginSize, const pluginDescriptorType& pluginId, pluginInterface *plugInterface )
    {
        // Make sure we have got valid parameters passed.
        if ( pluginSize == 0 || plugInterface == NULL )
            return 0;   // TODO: fix this crap, 0 is ambivalent since its a valid index!

        // Determine the plugin offset that should be used for allocation.
        blockAlloc_t::allocInfo blockAllocInfo;

        bool hasSpace = pluginRegions.FindSpace( pluginSize, blockAllocInfo, this->preferredAlignment );

        // Handle obscure errors.
        if ( hasSpace == false )
            return 0;

        // The beginning of the free space is where our plugin should be placed at.
        pluginOffset_t useOffset = blockAllocInfo.slice.GetSliceStartPoint();

        // Register ourselves.
        registered_plugin info;
        info.pluginSize = pluginSize;
        info.pluginId = pluginId;
        info.pluginOffset = useOffset;
        info.descriptor = plugInterface;

        // Register the pointer to the registered plugin.
        pluginRegions.PutBlock( &info, blockAllocInfo );

        regPlugins.push_back( std::move( info ) );

        // Notify the flavor to update.
        this->regBoundFlavor.UpdatePluginRegion( this );

        return useOffset;
    }

    inline bool UnregisterPlugin( pluginOffset_t pluginOffset )
    {
        bool hasDeleted = false;

        // Get the plugin information that represents this plugin offset.
        for ( typename registeredPlugins_t::iterator iter = regPlugins.begin(); iter != regPlugins.end(); iter++ )
        {
            registered_plugin& thePlugin = *iter;

            if ( thePlugin.pluginOffset == pluginOffset )
            {
                // We found it!
                // Now remove it and (probably) delete it.
                thePlugin.descriptor->DeleteOnUnregister();

                pluginRegions.RemoveBlock( &thePlugin );

                regPlugins.erase( iter );

                hasDeleted = true;
                break;  // there can be only one.
            }
        }

        if ( hasDeleted )
        {
            // Since we really have deleted, we need to readjust our struct memory size.
            // This is done by getting the span size of the plugin allocation region, which is a very fast operation!
            this->regBoundFlavor.UpdatePluginRegion( this );
        }

        return hasDeleted;
    }

private:
    typedef typename pluginAvailabilityDispatchType::template regionIterator <AnonymousPluginStructRegistry> regionIterator_t;

    inline void DestroyPluginBlockInternal( abstractionType *pluginObj, regionIterator_t& iter, AdditionalArgs... addArgs )
    {
        try
        {
            while ( true )
            {
                iter.Decrement();

                if ( iter.IsEnd() )
                    break;

                const blockAlloc_t::block_t *blockItem = iter.ResolveBlock();

                const registered_plugin *constructedInfo = (const registered_plugin*)blockItem;

                // Destroy that plugin again.
                constructedInfo->descriptor->OnPluginDestruct(
                    pluginObj,
                    iter.ResolveOffset(),
                    constructedInfo->pluginId,
                    addArgs...
                );
            }
        }
        catch( ... )
        {
            // We must not fail destruction, in any way.
            assert( 0 );
        }
    }

public:
    inline bool ConstructPluginBlock( abstractionType *pluginObj, AdditionalArgs... addArgs )
    {
        // Construct all plugins.
        bool pluginConstructionSuccessful = true;

        regionIterator_t iter( this );

        try
        {
            while ( !iter.IsEnd() )
            {
                const blockAlloc_t::block_t *blockItem = iter.ResolveBlock();

                const registered_plugin *pluginInfo = (const registered_plugin*)blockItem;

                // TODO: add dispatch based on the reg bound flavor.
                // it should say whether this plugin exists and where it is ultimatively located.
                // in the cached handler, this is an O(1) operation, in the conditional it is rather funky.
                // this is why you really should not use the conditional handler too often.

                bool success =
                    pluginInfo->descriptor->OnPluginConstruct(
                        pluginObj,
                        iter.ResolveOffset(),
                        pluginInfo->pluginId,
                        addArgs...
                    );

                if ( !success )
                {
                    pluginConstructionSuccessful = false;
                    break;
                }

                iter.Increment();
            }
        }
        catch( ... )
        {
            // There was an exception while trying to construct a plugin.
            // We do not let it pass and terminate here.
            pluginConstructionSuccessful = false;
        }

        if ( !pluginConstructionSuccessful )
        {
            // The plugin failed to construct, so destroy all plugins that
            // constructed up until that point.
            DestroyPluginBlockInternal( pluginObj, iter, addArgs... );
        }

        return pluginConstructionSuccessful;
    }

    inline bool AssignPluginBlock( abstractionType *dstPluginObj, const abstractionType *srcPluginObj, AdditionalArgs... addArgs )
    {
        // Call all assignment operators.
        bool cloneSuccess = true;

        regionIterator_t iter( this );

        try
        {
            for ( ; !iter.IsEnd(); iter.Increment() )
            {
                const blockAlloc_t::block_t *blockInfo = iter.ResolveBlock();

                const registered_plugin *pluginInfo = (const registered_plugin*)blockInfo;

                bool assignSuccess = pluginInfo->descriptor->OnPluginAssign(
                    dstPluginObj, srcPluginObj,
                    iter.ResolveOffset(),
                    pluginInfo->pluginId,
                    addArgs...
                );

                if ( !assignSuccess )
                {
                    cloneSuccess = false;
                    break;
                }
            }
        }
        catch( ... )
        {
            // There was an exception while cloning plugin data.
            // We do not let it pass and terminate here.
            cloneSuccess = false;
        }

        return cloneSuccess;
    }

    inline void DestroyPluginBlock( abstractionType *pluginObj, AdditionalArgs... addArgs )
    {
        // Call destructors of all registered plugins.
        regionIterator_t endIter( this );

#if 0
        // By decrementing this double-linked-list iterator by one, we get to the end.
        endIter.Decrement();
#else
        // We want to iterate to the end.
        while ( endIter.IsEnd() == false )
        {
            endIter.Increment();
        }
#endif

        DestroyPluginBlockInternal( pluginObj, endIter, addArgs... );
    }

    // Use this function whenever you receive a handle offset to a plugin struct.
    // It is optimized so that you cannot go wrong.
    inline pluginOffset_t ResolvePluginStructOffsetByRuntime( pluginOffset_t handleOffset )
    {
        size_t theActualOffset;

        bool gotOffset = this->regBoundFlavor.GetPluginStructOffset( this, handleOffset, theActualOffset );

        return ( gotOffset ? theActualOffset : 0 );
    }

    inline pluginOffset_t ResolvePluginStructOffsetByObject( const abstractionType *obj, pluginOffset_t handleOffset )
    {
        size_t theActualOffset;

        bool gotOffset = this->regBoundFlavor.GetPluginStructOffsetByObject( this, obj, handleOffset, theActualOffset );

        return ( gotOffset ? theActualOffset : 0 );
    }
};

// Helper struct for common plugin system functions.
// THREAD-SAFE because this class itself is immutable and the systemType is THREAD-SAFE.
template <typename classType, typename systemType, typename pluginDescriptorType, typename... AdditionalArgs>
struct CommonPluginSystemDispatch
{
    systemType& sysType;

    typedef typename systemType::pluginOffset_t pluginOffset_t;

    inline CommonPluginSystemDispatch( systemType& sysType ) : sysType( sysType )
    {
        return;
    }

    template <typename interfaceType>
    inline pluginOffset_t RegisterCommonPluginInterface( interfaceType *plugInterface, size_t structSize, const pluginDescriptorType& pluginId )
    {
        pluginOffset_t pluginOffset = 0;

        if ( plugInterface )
        {
            // Register our plugin!
            pluginOffset = sysType.RegisterPlugin(
                structSize, pluginId,
                plugInterface
            );

            // Delete our interface again if the plugin offset is invalid.
            if ( !systemType::IsOffsetValid( pluginOffset ) )
            {
                delete plugInterface;
            }
        }
        return pluginOffset;
    }

    // Helper functions used to create common plugin templates.
    template <typename structType>
    inline pluginOffset_t RegisterStructPlugin( const pluginDescriptorType& pluginId )
    {
        struct structPluginInterface : systemType::pluginInterface
        {
            bool OnPluginConstruct( classType *obj, pluginOffset_t pluginOffset, pluginDescriptorType pluginId, AdditionalArgs... addArgs ) override
            {
                void *structMem = pluginId.template RESOLVE_STRUCT <structType> ( obj, pluginOffset, addArgs... );

                if ( structMem == NULL )
                    return false;

                // Construct the struct!
                structType *theStruct = new (structMem) structType;

                return ( theStruct != NULL );
            }

            void OnPluginDestruct( classType *obj, pluginOffset_t pluginOffset, pluginDescriptorType pluginId, AdditionalArgs... addArgs ) override
            {
                structType *theStruct = pluginId.template RESOLVE_STRUCT <structType> ( obj, pluginOffset, addArgs... );

                // Destruct the struct!
                theStruct->~structType();
            }

            bool OnPluginAssign( classType *dstObject, const classType *srcObject, pluginOffset_t pluginOffset, pluginDescriptorType pluginId, AdditionalArgs... addArgs ) override
            {
                // To an assignment operation.
                structType *dstStruct = pluginId.template RESOLVE_STRUCT <structType> ( dstObject, pluginOffset, addArgs... );
                const structType *srcStruct = pluginId.template RESOLVE_STRUCT <structType> ( srcObject, pluginOffset, addArgs... );

                *dstStruct = *srcStruct;
                return true;
            }

            void DeleteOnUnregister( void ) override
            {
                delete this;
            }
        };

        // Create the interface that should handle our plugin.
        structPluginInterface *plugInterface = new structPluginInterface();

        return RegisterCommonPluginInterface( plugInterface, sizeof( structType ), pluginId );
    }

    template <typename structType>
    struct dependantStructPluginInterface : systemType::pluginInterface
    {
        bool OnPluginConstruct( classType *obj, pluginOffset_t pluginOffset, pluginDescriptorType pluginId, AdditionalArgs... addArgs ) override
        {
            void *structMem = pluginId.template RESOLVE_STRUCT <structType> ( obj, pluginOffset, addArgs... );

            if ( structMem == NULL )
                return false;

            // Construct the struct!
            structType *theStruct = new (structMem) structType;

            if ( theStruct )
            {
                try
                {
                    // Initialize the manager.
                    theStruct->Initialize( obj );
                }
                catch( ... )
                {
                    // We have to destroy our struct again.
                    theStruct->~structType();

                    throw;
                }
            }

            return ( theStruct != NULL );
        }

        void OnPluginDestruct( classType *obj, pluginOffset_t pluginOffset, pluginDescriptorType pluginId, AdditionalArgs... addArgs ) override
        {
            structType *theStruct = pluginId.template RESOLVE_STRUCT <structType> ( obj, pluginOffset, addArgs... );

            // Deinitialize the manager.
            theStruct->Shutdown( obj );

            // Destruct the struct!
            theStruct->~structType();
        }

        // Since a lot of functionality could not support the copy assignment, we help the user out by throwing a common exception.
        template <typename subStructType>
        AINLINE typename std::enable_if <std::is_copy_assignable <subStructType>::value>::type copy_assign( subStructType& left, const subStructType& right )
        {
            left = right;
        }

        template <typename subStructType>
        AINLINE typename std::enable_if <!std::is_copy_assignable <subStructType>::value>::type copy_assign( subStructType& left, const subStructType& right )
        {
            throw std::exception( "failed copy assignment due to missing plugin copy assignment operator overload" );
        }

        bool OnPluginAssign( classType *dstObject, const classType *srcObject, pluginOffset_t pluginOffset, pluginDescriptorType pluginId, AdditionalArgs... addArgs ) override
        {
            // To an assignment operation.
            structType *dstStruct = pluginId.template RESOLVE_STRUCT <structType> ( dstObject, pluginOffset, addArgs... );
            const structType *srcStruct = pluginId.template RESOLVE_STRUCT <structType> ( srcObject, pluginOffset, addArgs... );

            copy_assign( *dstStruct, *srcStruct );
            return true;
        }

        void DeleteOnUnregister( void ) override
        {
            delete this;
        }
    };

    template <typename structType>
    inline pluginOffset_t RegisterDependantStructPlugin( const pluginDescriptorType& pluginId, size_t structSize = sizeof(structType) )
    {
        typedef dependantStructPluginInterface <structType> structPluginInterface;

        // Create the interface that should handle our plugin.
        structPluginInterface *plugInterface = new structPluginInterface();

        return RegisterCommonPluginInterface( plugInterface, structSize, pluginId );
    }

    struct conditionalPluginStructInterface abstract
    {
        // Conditional interface. This is based on the state of the runtime.
        // Both functions here have to match logically.
        virtual bool IsPluginAvailableDuringRuntime( pluginDescriptorType pluginId ) const = 0;
        virtual bool IsPluginAvailableAtObject( const classType *object, pluginDescriptorType pluginId ) const = 0;
    };

    // Helper to register a conditional type that acts as dependant struct.
    template <typename structType>
    inline pluginOffset_t RegisterDependantConditionalStructPlugin( const pluginDescriptorType& pluginId, conditionalPluginStructInterface *conditional, size_t structSize = sizeof(structType) )
    {
        struct structPluginInterface : public dependantStructPluginInterface <structType>
        {
            bool IsPluginAvailableDuringRuntime( pluginDescriptorType pluginId ) const override
            {
                return conditional->IsPluginAvailableDuringRuntime( pluginId );
            }

            bool IsPluginAvailableAtObject( const classType *object, pluginDescriptorType pluginId ) const override
            {
                return conditional->IsPluginAvailableAtObject( object, pluginId );
            }

            void DeleteOnUnregister( void )
            {
                // Delete our data.
                //TODO

                dependantStructPluginInterface <structType>::DeleteOnUnregister();
            }

            conditionalPluginStructInterface *conditional;
        };

        // Create the interface that should handle our plugin.
        structPluginInterface *plugInterface = new structPluginInterface();

        if ( plugInterface )
        {
            plugInterface->conditional = conditional;

            return RegisterCommonPluginInterface( plugInterface, structSize, pluginId );
        }

        return 0;
    }
};

// Static plugin system that constructs classes that can be extended at runtime.
// This one is inspired by the RenderWare plugin system.
// This container is NOT MULTI-THREAD SAFE.
// All operations are expected to be ATOMIC.
template <typename classType, typename flavorType = cachedMinimalStructRegistryFlavor <classType>>
struct StaticPluginClassFactory
{
    typedef classType hostType_t;

    static const unsigned int ANONYMOUS_PLUGIN_ID = 0xFFFFFFFF;

    std::atomic <unsigned int> aliveClasses;

    inline StaticPluginClassFactory( void )
    {
        aliveClasses = 0;
    }

    inline ~StaticPluginClassFactory( void )
    {
        assert( aliveClasses == 0 );
    }

    inline unsigned int GetNumberOfAliveClasses( void ) const
    {
        return this->aliveClasses;
    }

    // Number type used to store the plugin offset.
    typedef ptrdiff_t pluginOffset_t;

    // Helper functoid.
    struct pluginDescriptor
    {
        typedef typename StaticPluginClassFactory::pluginOffset_t pluginOffset_t;

        inline pluginDescriptor( void )
        {
            this->pluginId = StaticPluginClassFactory::ANONYMOUS_PLUGIN_ID;
        }

        inline pluginDescriptor( unsigned int pluginId )
        {
            this->pluginId = pluginId;
        }

        operator const unsigned int& ( void ) const
        {
            return this->pluginId;
        }

        template <typename pluginStructType>
        AINLINE static pluginStructType* RESOLVE_STRUCT( classType *object, pluginOffset_t offset )
        {
            return StaticPluginClassFactory::RESOLVE_STRUCT <pluginStructType> ( object, offset );
        }

        template <typename pluginStructType>
        AINLINE static const pluginStructType* RESOLVE_STRUCT( const classType *object, pluginOffset_t offset )
        {
            return StaticPluginClassFactory::RESOLVE_STRUCT <const pluginStructType> ( object, offset );
        }

        unsigned int pluginId;
    };

    typedef AnonymousPluginStructRegistry <classType, pluginDescriptor, flavorType> structRegistry_t;

    structRegistry_t structRegistry;

    // Localize certain plugin registry types.
    typedef typename structRegistry_t::pluginInterface pluginInterface;

    static const pluginOffset_t INVALID_PLUGIN_OFFSET = (pluginOffset_t)-1;

    AINLINE static bool IsOffsetValid( pluginOffset_t offset )
    {
        return ( offset != INVALID_PLUGIN_OFFSET );
    }

    template <typename pluginStructType>
    AINLINE static pluginStructType* RESOLVE_STRUCT( classType *object, pluginOffset_t offset )
    {
        if ( IsOffsetValid( offset ) == false )
            return NULL;

        return (pluginStructType*)( (char*)object + sizeof( classType ) + offset );
    }

    template <typename pluginStructType>
    AINLINE static const pluginStructType* RESOLVE_STRUCT( const classType *object, pluginOffset_t offset )
    {
        if ( IsOffsetValid( offset ) == false )
            return NULL;

        return (const pluginStructType*)( (const char*)object + sizeof( classType ) + offset );
    }

    // Just helpers, no requirement.
    AINLINE static classType* BACK_RESOLVE_STRUCT( void *pluginObj, pluginOffset_t offset )
    {
        if ( IsOffsetValid( offset ) == false )
            return NULL;

        return (classType*)( (char*)pluginObj - ( sizeof( classType ) + offset ) );
    }

    AINLINE static const classType* BACK_RESOLVE_STRUCT( const void *pluginObj, pluginOffset_t offset )
    {
        if ( IsOffsetValid( offset ) == false )
            return NULL;

        return (const classType*)( (const char*)pluginObj - ( sizeof( classType ) + offset ) );
    }

    // Function used to register a new plugin struct into the class.
    inline pluginOffset_t RegisterPlugin( size_t pluginSize, unsigned int pluginId, pluginInterface *plugInterface )
    {
        assert( this->aliveClasses == 0 );

        return structRegistry.RegisterPlugin( pluginSize, pluginId, plugInterface );
    }

    inline void UnregisterPlugin( pluginOffset_t pluginOffset )
    {
        assert( this->aliveClasses == 0 );

        structRegistry.UnregisterPlugin( pluginOffset );
    }

    typedef CommonPluginSystemDispatch <classType, StaticPluginClassFactory, pluginDescriptor> functoidHelper_t;

    // Helper functions used to create common plugin templates.
    template <typename structType>
    inline pluginOffset_t RegisterStructPlugin( unsigned int pluginId = ANONYMOUS_PLUGIN_ID )
    {
        return functoidHelper_t( *this ).RegisterStructPlugin <structType> ( pluginId );
    }

    template <typename structType>
    inline pluginOffset_t RegisterDependantStructPlugin( unsigned int pluginId = ANONYMOUS_PLUGIN_ID, size_t structSize = sizeof( structType ) )
    {
        return functoidHelper_t( *this ).RegisterDependantStructPlugin <structType> ( pluginId, structSize );
    }

    // Note that this function only guarrantees to return an object size that is correct at this point in time.
    inline size_t GetClassSize( void ) const
    {
        return ( sizeof( classType ) + this->structRegistry.GetPluginSizeByRuntime() );
    }

private:
    inline void DestroyBaseObject( classType *toBeDestroyed )
    {
        try
        {
            toBeDestroyed->~classType();
        }
        catch( ... )
        {
            // Throwing exceptions from destructors is lethal.
            // We have to notify the developer about this.
            assert( 0 );
        }
    }

public:
    template <typename constructorType>
    inline classType* ConstructPlacementEx( void *classMem, const constructorType& constructor )
    {
        classType *resultObject = NULL;
        {
            classType *intermediateClassObject = NULL;

            try
            {
                intermediateClassObject = constructor.Construct( classMem );
            }
            catch( ... )
            {
                // The base object failed to construct, so terminate here.
                intermediateClassObject = NULL;
            }

            if ( intermediateClassObject )
            {
                bool pluginConstructionSuccessful = structRegistry.ConstructPluginBlock( intermediateClassObject );

                if ( pluginConstructionSuccessful )
                {
                    // We succeeded, so return our pointer.
                    // We promote it to a real class object.
                    resultObject = intermediateClassObject;
                }
                else
                {
                    // Else we cannot keep the intermediate class object anymore.
                    DestroyBaseObject( intermediateClassObject );
                }
            }
        }

        if ( resultObject )
        {
            this->aliveClasses++;
        }

        return resultObject;
    }

    template <typename allocatorType, typename constructorType>
    inline classType* ConstructTemplate( allocatorType& memAllocator, const constructorType& constructor )
    {
        // Attempt to allocate the necessary memory.
        const size_t baseClassSize = sizeof( classType );
        const size_t wholeClassSize = this->GetClassSize();

        void *classMem = memAllocator.Allocate( wholeClassSize );

        if ( !classMem )
            return NULL;

        classType *resultObj = ConstructPlacementEx( classMem, constructor );

        if ( !resultObj )
        {
            // Clean up.
            memAllocator.Free( classMem, wholeClassSize );
        }

        return resultObj;
    }

    template <typename constructorType>
    inline classType* ClonePlacementEx( void *classMem, const classType *srcObject, const constructorType& constructor )
    {
        classType *clonedObject = NULL;
        {
            // Construct a basic class where we assign stuff to.
            classType *dstObject = NULL;

            try
            {
                dstObject = constructor.CopyConstruct( classMem, srcObject );
            }
            catch( ... )
            {
                dstObject = NULL;
            }

            if ( dstObject )
            {
                bool pluginConstructionSuccessful = structRegistry.ConstructPluginBlock( dstObject );

                if ( pluginConstructionSuccessful )
                {
                    bool cloneSuccess = structRegistry.AssignPluginBlock( dstObject, srcObject );

                    if ( cloneSuccess )
                    {
                        clonedObject = dstObject;
                    }

                    if ( clonedObject == NULL )
                    {
                        structRegistry.DestroyPluginBlock( dstObject );
                    }
                }
                
                if ( clonedObject == NULL )
                {
                    // Since cloning plugin data has not succeeded, we have to destroy the constructed base object again.
                    // Make sure that we do not throw exceptions.
                    DestroyBaseObject( dstObject );

                    dstObject = NULL;
                }
            }
        }

        if ( clonedObject )
        {
            this->aliveClasses++;
        }

        return clonedObject;
    }

    template <typename allocatorType, typename constructorType>
    inline classType* CloneTemplate( allocatorType& memAllocator, const classType *srcObject, const constructorType& constructor )
    {
        // Attempt to allocate the necessary memory.
        const size_t baseClassSize = sizeof( classType );
        const size_t wholeClassSize = this->GetClassSize();

        void *classMem = memAllocator.Allocate( wholeClassSize );

        if ( !classMem )
            return NULL;

        classType *clonedObject = ClonePlacementEx( classMem, srcObject, constructor );

        if ( clonedObject == NULL )
        {
            memAllocator.Free( classMem );
        }

        return clonedObject;
    }

    struct basicClassConstructor
    {
        inline classType* Construct( void *mem ) const
        {
            return new (mem) classType;
        }

        inline classType* CopyConstruct( void *mem, const classType *srcMem ) const
        {
            return new (mem) classType( *srcMem );
        }
    };

    template <typename allocatorType>
    inline classType* Construct( allocatorType& memAllocator )
    {
        basicClassConstructor constructor;

        return ConstructTemplate( memAllocator, constructor );
    }

    inline classType* ConstructPlacement( void *memPtr )
    {
        basicClassConstructor constructor;

        return ConstructPlacementEx( memPtr, constructor );
    }

    template <typename allocatorType>
    inline classType* Clone( allocatorType& memAllocator, const classType *srcObject )
    {
        basicClassConstructor constructor;

        return CloneTemplate( memAllocator, srcObject, constructor );
    }

    inline classType* ClonePlacement( void *memPtr, const classType *srcObject )
    {
        basicClassConstructor constructor;

        return ClonePlacementEx( memPtr, srcObject, constructor );
    }

    // Assignment is good.
    inline bool Assign( classType *dstObj, const classType *srcObj )
    {
        // First we assign the language object.
        // Not that hard.
        try
        {
            *dstObj = *srcObj;
        }
        catch( ... )
        {
            return false;
        }

        // Next we should assign the plugin blocks.
        return structRegistry.AssignPluginBlock( dstObj, srcObj );
    }

    inline void DestroyPlacement( classType *classObject )
    {
        // Destroy plugin data first.
        structRegistry.DestroyPluginBlock( classObject );

        try
        {
            // Destroy the base class object.
            classObject->~classType();
        }
        catch( ... )
        {
            // There was an exception while destroying the base class.
            // This must not happen either; we have to notify the guys!
            assert( 0 );
        }

        // Decrease the number of alive classes.
        this->aliveClasses--;
    }

    template <typename allocatorType>
    inline void Destroy( allocatorType& memAllocator, classType *classObject )
    {
        if ( classObject == NULL )
            return;

        // Invalidate the memory that is in "classObject".
        DestroyPlacement( classObject );

        // Free our memory.
        void *classMem = classObject;

        memAllocator.Free( classMem, this->GetClassSize() );
    }

    template <typename allocatorType>
    struct DeferredConstructor
    {
        StaticPluginClassFactory *pluginRegistry;
        allocatorType& memAllocator;

        inline DeferredConstructor( StaticPluginClassFactory *pluginRegistry, allocatorType& memAllocator ) : memAllocator( memAllocator )
        {
            this->pluginRegistry = pluginRegistry;
        }

        inline allocatorType& GetAllocator( void )
        {
            return memAllocator;
        }

        inline classType* Construct( void )
        {
            return pluginRegistry->Construct( memAllocator );
        }

        template <typename constructorType>
        inline classType* ConstructTemplate( constructorType& constructor )
        {
            return pluginRegistry->ConstructTemplate( memAllocator, constructor );
        }

        inline classType* Clone( const classType *srcObject )
        {
            return pluginRegistry->Clone( memAllocator, srcObject );
        }

        inline void Destroy( classType *object )
        {
            return pluginRegistry->Destroy( memAllocator, object );
        }
    };

    template <typename allocatorType>
    inline DeferredConstructor <allocatorType>* CreateConstructor( allocatorType& memAllocator )
    {
        typedef DeferredConstructor <allocatorType> Constructor;

        Constructor *result = NULL;

        {
            void *constructorMem = memAllocator.Allocate( sizeof( Constructor ) );

            if ( constructorMem )
            {
                result = new (constructorMem) Constructor( this, memAllocator );
            }
        }
        
        return result;
    }

    template <typename allocatorType>
    inline void DeleteConstructor( DeferredConstructor <allocatorType> *handle )
    {
        typedef DeferredConstructor <allocatorType> Constructor;

        allocatorType& memAlloc = handle->GetAllocator();
        {
            handle->~Constructor();
        }

        void *constructorMem = handle;

        memAlloc.Free( constructorMem, sizeof( Constructor ) );
    }
};

// Array implementation that extends on concepts found inside GTA:SA
// NOTE: This array type is a 'trusted type'.
// -> Use it whenever necessary.
// WARNING (2016, The_GTA): this type is in need of a major overhaul due to violating modern C++ principles!
template <typename dataType, unsigned int pulseCount, unsigned int allocFlags, typename arrayMan, typename countType, typename allocatorType>
struct growableArrayEx
{
    typedef dataType dataType_t;

    allocatorType _memAllocator;

    AINLINE void* _memAlloc( size_t memSize, unsigned int flags )
    {
        return _memAllocator.Allocate( memSize, flags );
    }

    AINLINE void* _memRealloc( void *memPtr, size_t memSize, unsigned int flags )
    {
        return _memAllocator.Realloc( memPtr, memSize, flags );
    }

    AINLINE void _memFree( void *memPtr )
    {
        _memAllocator.Free( memPtr );
    }

    AINLINE growableArrayEx( void )
    {
        data = NULL;
        numActiveEntries = 0;
        sizeCount = 0;
    }

    AINLINE growableArrayEx( allocatorType allocNode ) : _memAllocator( std::move( allocNode ) )
    {
        data = NULL;
        numActiveEntries = 0;
        sizeCount = 0;
    }

    AINLINE growableArrayEx( growableArrayEx&& right )
    {
        this->data = right.data;
        this->numActiveEntries = right.numActiveEntries;
        this->sizeCount = right.sizeCount;

        right.data = NULL;
        right.numActiveEntries = 0;
        right.sizeCount = 0;
    }

    AINLINE growableArrayEx( const growableArrayEx& right )
    {
        this->data = NULL;
        this->numActiveEntries = 0;
        this->sizeCount = 0;

        operator = ( right );
    }

    AINLINE void operator = ( const growableArrayEx& right )
    {
        SetSizeCount( right.GetSizeCount() );

        // Copy all data over.
        if ( sizeCount != 0 )
        {
            std::copy( right.data, right.data + sizeCount, data );
        }

        // Set the number of active entries.
        numActiveEntries = right.numActiveEntries;
    }

    AINLINE void operator = ( growableArrayEx&& right )
    {
        SetSizeCount( 0 );

        this->data = right.data;
        this->numActiveEntries = right.numActiveEntries;
        this->sizeCount = right.sizeCount;

        right.data = NULL;
        right.numActiveEntries = 0;
        right.sizeCount = 0;
    }

    AINLINE void SetArrayCachedTo( growableArrayEx& target )
    {
        countType targetSizeCount = GetSizeCount();
        countType oldTargetSizeCount = target.GetSizeCount();

        target.AllocateToIndex( targetSizeCount );

        if ( targetSizeCount != 0 )
        {
            std::copy( data, data + targetSizeCount, target.data );

            // Anything that is above the new target size count must be reset.
            for ( countType n = targetSizeCount; n < oldTargetSizeCount; n++ )
            {
                dataType *theField = &target.data[ n ];

                // Reset it.
                theField->~dataType();

                new (theField) dataType;

                // Tell it to the manager.
                manager.InitField( *theField );
            }
        }

        // Set the number of active entries.
        target.numActiveEntries = numActiveEntries;
    }

    AINLINE ~growableArrayEx( void )
    {
        Shutdown();
    }

    AINLINE void Init( void )
    { }

    AINLINE void Shutdown( void )
    {
        if ( data )
            SetSizeCount( 0 );

        numActiveEntries = 0;
        sizeCount = 0;
    }

    AINLINE void SetSizeCount( countType index )
    {
        if ( index != sizeCount )
        {
            countType oldCount = sizeCount;

            sizeCount = index;

            if ( data )
            {
                // Destroy any structures that got removed.
                for ( countType n = index; n < oldCount; n++ )
                {
                    data[n].~dataType();
                }
            }

            if ( index == 0 )
            {
                // Handle clearance requests.
                if ( data )
                {
                    _memFree( data );

                    data = NULL;
                }
            }
            else
            {
                size_t newArraySize = sizeCount * sizeof( dataType );

                if ( !data )
                    data = (dataType*)_memAlloc( newArraySize, allocFlags );
                else
                    data = (dataType*)_memRealloc( data, newArraySize, allocFlags );
            }

            if ( data )
            {
                // FIXME: here is a FATAL ERROR.
                // Pointers to items inside of growableArray cannot be PRESERVED.
                // Hence growableArray is NO LONGER C++ SAFE since the introduction
                // of move semantics!

                // Fill the gap.
                for ( countType n = oldCount; n < index; n++ )
                {
                    new (&data[n]) dataType;

                    manager.InitField( data[n] );
                }
            }
            else
                sizeCount = 0;
        }
    }

    AINLINE void AllocateToIndex( countType index )
    {
        if ( index >= sizeCount )
        {
            SetSizeCount( index + ( pulseCount + 1 ) );
        }
    }

    AINLINE void SetItem( const dataType& dataField, countType index )
    {
        AllocateToIndex( index );

        data[index] = dataField;
    }

    AINLINE void SetItem( dataType&& dataField, countType index )
    {
        AllocateToIndex( index );

        data[index] = std::move( dataField );
    }

    AINLINE void SetFast( const dataType& dataField, countType index )
    {
        // God mercy the coder knows why and how he is using this.
        // We might introduce a hyper-paranoid assertion that even checks this...
        data[index] = dataField;
    }

    AINLINE void SetFast( dataType&& dataField, countType index )
    {
        // :(
        data[index] = std::move( dataField );
    }

    AINLINE dataType& GetFast( countType index ) const
    {
        // and that.
        return data[index];
    }

    AINLINE void AddItem( const dataType& data )
    {
        SetItem( data, numActiveEntries );

        numActiveEntries++;
    }

    AINLINE void AddItem( dataType&& data )
    {
        SetItem( std::move( data ), numActiveEntries );

        numActiveEntries++;
    }

private:
    template <typename insertCB>
    AINLINE void InsertItemPrepare( insertCB& cb, size_t insertIndex )
    {
        // Make sure we have enough space allocated.
        size_t numItems = this->numActiveEntries;

        // Input argument has to be in range.
        insertIndex = std::min( numItems, insertIndex );

        this->AllocateToIndex( numItems );

        // Move items one up.
        size_t moveUpStart = numItems;

        while ( moveUpStart > insertIndex )
        {
            this->data[ moveUpStart ] = std::move( this->data[ moveUpStart - 1 ] );

            moveUpStart--;
        }

        // Put in the new item.
        cb.Put( this->data[ moveUpStart ] );

        this->numActiveEntries++;
    }

public:
    AINLINE void InsertItem( dataType&& data, size_t insertIndex )
    {
        struct itemMovePutCB
        {
            AINLINE itemMovePutCB( dataType&& data ) : data( std::move( data ) )
            {
                return;
            }

            AINLINE void Put( dataType& dataOut )
            {
                dataOut = std::move( data );
            }

        private:
            dataType&& data;
        };

        itemMovePutCB cb( std::move( data ) );

        InsertItemPrepare( cb, insertIndex );
    }

    AINLINE void InsertItem( const dataType& data, size_t insertIndex )
    {
        struct insertCopyPutCB
        {
            AINLINE insertCopyPutCB( const dataType& data ) : data( data )
            {
                return;
            }

            AINLINE void Put( dataType& data )
            {
                data = this->data;
            }

        private:
            const dataType& data;
        };

        insertCopyPutCB cb( data );

        InsertItemPrepare( cb, insertIndex );
    }

    AINLINE dataType& ObtainItem( countType obtainIndex )
    {
        AllocateToIndex( obtainIndex );

        return data[obtainIndex];
    }

    AINLINE dataType& ObtainItem( void )
    {
        return ObtainItem( numActiveEntries++ );
    }

    AINLINE countType GetCount( void ) const
    {
        return numActiveEntries;
    }

    AINLINE countType GetSizeCount( void ) const
    {
        return sizeCount;
    }

    AINLINE dataType& Get( countType index )
    {
        assert( index < sizeCount );

        return data[index];
    }

    AINLINE const dataType& Get( countType index ) const
    {
        assert( index < sizeCount );

        return data[index];
    }

    AINLINE bool Front( dataType& outVal ) const
    {
        bool success = ( GetCount() != 0 );

        if ( success )
        {
            outVal = data[ 0 ];
        }

        return success;
    }

    AINLINE bool Tail( dataType& outVal ) const
    {
        countType count = GetCount();

        bool success = ( count != 0 );

        if ( success )
        {
            outVal = data[ count - 1 ];
        }

        return success;
    }

    AINLINE bool Pop( dataType& item )
    {
        if ( numActiveEntries != 0 )
        {
            item = data[--numActiveEntries];
            return true;
        }

        return false;
    }

    AINLINE bool Pop( void )
    {
        if ( numActiveEntries != 0 )
        {
            --numActiveEntries;
            return true;
        }

        return false;
    }

    AINLINE void RemoveItem( countType foundSlot )
    {
        assert( foundSlot >= 0 && foundSlot < numActiveEntries );
        assert( numActiveEntries != 0 );

        countType moveCount = numActiveEntries - ( foundSlot + 1 );

        if ( moveCount != 0 )
        {
            FSDataUtil::copy_impl( data + foundSlot + 1, data + numActiveEntries, data + foundSlot );
        }

        numActiveEntries--;
    }

    AINLINE bool RemoveItem( const dataType& item )
    {
        countType foundSlot = -1;
        
        if ( !Find( item, foundSlot ) )
            return false;

        RemoveItem( foundSlot );
        return true;
    }

    AINLINE bool Find( const dataType& inst, countType& indexOut ) const
    {
        for ( countType n = 0; n < numActiveEntries; n++ )
        {
            if ( data[n] == inst )
            {
                indexOut = n;
                return true;
            }
        }

        return false;
    }

    AINLINE bool Find( const dataType& inst ) const
    {
        countType trashIndex;

        return Find( inst, trashIndex );
    }

    AINLINE unsigned int Count( const dataType& inst ) const
    {
        unsigned int count = 0;

        for ( countType n = 0; n < numActiveEntries; n++ )
        {
            if ( data[n] == inst )
                count++;
        }

        return count;
    }

    AINLINE void Clear( void )
    {
        numActiveEntries = 0;
    }

    AINLINE void TrimTo( countType indexTo )
    {
        if ( numActiveEntries > indexTo )
            numActiveEntries = indexTo;
    }

    AINLINE void SwapContents( growableArrayEx& right )
    {
        dataType *myData = this->data;
        dataType *swapData = right.data;

        this->data = swapData;
        right.data = myData;

        countType myActiveCount = this->numActiveEntries;
        countType swapActiveCount = right.numActiveEntries;

        this->numActiveEntries = swapActiveCount;
        right.numActiveEntries = myActiveCount;

        countType mySizeCount = this->sizeCount;
        countType swapSizeCount = right.sizeCount;

        this->sizeCount = swapSizeCount;
        right.sizeCount = mySizeCount;
    }
    
    AINLINE void SetContents( growableArrayEx& right )
    {
        right.SetSizeCount( numActiveEntries );

        for ( countType n = 0; n < numActiveEntries; n++ )
            right.data[n] = data[n];

        right.numActiveEntries = numActiveEntries;
    }

    dataType* data;
    countType numActiveEntries;
    countType sizeCount;
    arrayMan manager;
};

template <typename dataType>
struct iterativeGrowableArrayExManager
{
    AINLINE void InitField( dataType& theField )
    {
        return;
    }
};

template <typename dataType, unsigned int pulseCount, unsigned int allocFlags, typename countType, typename allocatorType>
using iterativeGrowableArrayEx = growableArrayEx <dataType, pulseCount, allocFlags, iterativeGrowableArrayExManager <dataType>, countType, allocatorType>;

#endif //_GLOBAL_MEMORY_UTILITIES_