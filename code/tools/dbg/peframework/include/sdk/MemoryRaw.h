/*****************************************************************************
*
*  PROJECT:     MTA:Eir
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        eirrepo/sdk/MemoryRaw.h
*  PURPOSE:     Base memory management definitions for to-the-metal things
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _MEMORY_RAW_DEFS_
#define _MEMORY_RAW_DEFS_

#include "MacroUtils.h"

// Mathematically correct data slice logic.
// It is one of the most important theorems in computing abstraction.
template <typename numberType>
class sliceOfData
{
public:
    AINLINE sliceOfData( void ) noexcept
    {
        this->startOffset = 0;
        this->endOffset = -1;
    }

    AINLINE sliceOfData( numberType startOffset, numberType dataSize ) noexcept
    {
        this->startOffset = startOffset;
        this->endOffset = startOffset + ( dataSize - 1 );
    }

    static AINLINE sliceOfData fromOffsets( numberType startOffset, numberType endOffset ) noexcept
    {
        sliceOfData slice;
        slice.startOffset = startOffset;
        slice.endOffset = endOffset;
        return slice;
    }

    enum eIntersectionResult
    {
        INTERSECT_EQUAL,
        INTERSECT_INSIDE,
        INTERSECT_BORDER_START,
        INTERSECT_BORDER_END,
        INTERSECT_ENCLOSING,
        INTERSECT_FLOATING_START,
        INTERSECT_FLOATING_END,
        INTERSECT_UNKNOWN   // if something went horribly wrong (like NaNs).
    };

    // Static methods for easier result management.
    static AINLINE bool isBorderIntersect( eIntersectionResult result ) noexcept
    {
        return ( result == INTERSECT_BORDER_START || result == INTERSECT_BORDER_END );
    }

    static AINLINE bool isFloatingIntersect( eIntersectionResult result ) noexcept
    {
        return ( result == INTERSECT_FLOATING_START || result == INTERSECT_FLOATING_END );
    }

    AINLINE numberType GetSliceSize( void ) const noexcept
    {
        return ( this->endOffset - this->startOffset ) + 1;
    }

    AINLINE void SetSlicePosition( numberType val ) noexcept
    {
        const numberType sliceSize = GetSliceSize();

        this->startOffset = val;
        this->endOffset = val + ( sliceSize - 1 );
    }

    AINLINE void OffsetSliceBy( numberType val ) noexcept
    {
        SetSlicePosition( this->startOffset + val );
    }

    AINLINE void SetSliceStartPoint( numberType val ) noexcept
    {
        this->startOffset = val;
    }

    AINLINE void SetSliceEndPoint( numberType val ) noexcept
    {
        this->endOffset = val;
    }

    AINLINE numberType GetSliceStartPoint( void ) const noexcept
    {
        return startOffset;
    }

    AINLINE numberType GetSliceEndPoint( void ) const noexcept
    {
        return endOffset;
    }

    AINLINE eIntersectionResult intersectWith( const sliceOfData& right ) const noexcept
    {
        // Make sure the slice has a valid size.
        if ( this->endOffset >= this->startOffset &&
             right.endOffset >= right.startOffset )
        {
            // Get generic stuff.
            numberType sliceStartA = startOffset, sliceEndA = endOffset;
            numberType sliceStartB = right.startOffset, sliceEndB = right.endOffset;

            // slice A -> this
            // slice B -> right

            // Handle all cases.
            // We only implement the logic with comparisons only, as it is the most transparent for all number types.
            if ( sliceStartA == sliceStartB && sliceEndA == sliceEndB )
            {
                // Slice A is equal to Slice B
                return INTERSECT_EQUAL;
            }

            if ( sliceStartB >= sliceStartA && sliceEndB <= sliceEndA )
            {
                // Slice A is enclosing Slice B
                return INTERSECT_ENCLOSING;
            }

            if ( sliceStartB <= sliceStartA && sliceEndB >= sliceEndA )
            {
                // Slice A is inside Slice B
                return INTERSECT_INSIDE;
            }

            if ( sliceStartB < sliceStartA && ( sliceEndB >= sliceStartA && sliceEndB <= sliceEndA ) )
            {
                // Slice A is being intersected at the starting point.
                return INTERSECT_BORDER_START;
            }

            if ( sliceEndB > sliceEndA && ( sliceStartB >= sliceStartA && sliceStartB <= sliceEndA ) )
            {
                // Slice A is being intersected at the ending point.
                return INTERSECT_BORDER_END;
            }

            if ( sliceStartB < sliceStartA && sliceEndB < sliceStartA )
            {
                // Slice A is after Slice B
                return INTERSECT_FLOATING_END;
            }

            if ( sliceStartB > sliceEndA && sliceEndB > sliceEndA )
            {
                // Slice A is before Slice B
                return INTERSECT_FLOATING_START;
            }
        }

        return INTERSECT_UNKNOWN;
    }

    AINLINE bool getSharedRegion( const sliceOfData& right, sliceOfData& sharedOut ) const
    {
        eIntersectionResult intResult = this->intersectWith( right );

        numberType startPos, endPos;
        bool hasPosition = false;

        if ( intResult == INTERSECT_EQUAL || intResult == INTERSECT_ENCLOSING )
        {
            startPos = right.GetSliceStartPoint();
            endPos = right.GetSliceEndPoint();

            hasPosition = true;
        }
        else if ( intResult == INTERSECT_INSIDE )
        {
            startPos = this->GetSliceStartPoint();
            endPos = this->GetSliceEndPoint();

            hasPosition = true;
        }
        else if ( intResult == INTERSECT_BORDER_START )
        {
            startPos = this->GetSliceStartPoint();
            endPos = right.GetSliceEndPoint();

            hasPosition = true;
        }
        else if ( intResult == INTERSECT_BORDER_END )
        {
            startPos = right.GetSliceStartPoint();
            endPos = this->GetSliceEndPoint();

            hasPosition = true;
        }
        else if ( intResult == INTERSECT_FLOATING_START || intResult == INTERSECT_FLOATING_END )
        {
            // Nothing to do.
        }
        // we could also intersect unknown, in which case we do not care.

        if ( hasPosition )
        {
            sharedOut = fromOffsets( startPos, endPos );
        }

        return hasPosition;
    }

private:
    numberType startOffset;
    numberType endOffset;
};

#include <bitset>

// Macro that defines how alignment works.
//  num: base of the number to be aligned
//  sector: aligned-offset that should be added to num
//  align: number of bytes to align to
// EXAMPLE: ALIGN( 0x1001, 4, 4 ) -> 0x1000 (equivalent of compiler structure padding alignment)
//          ALIGN( 0x1003, 1, 4 ) -> 0x1000
//          ALIGN( 0x1003, 2, 4 ) -> 0x1004
template <typename numberType>
AINLINE numberType _ALIGN_GP( numberType num, numberType sector, numberType align )
{
	// General purpose alignment routine.
    // Not as fast as the bitfield version.
    numberType sectorOffset = ((num) + (sector) - 1);

    return sectorOffset - ( sectorOffset % align );
}

template <typename numberType>
AINLINE numberType _ALIGN_NATIVE( numberType num, numberType sector, numberType align )
{
	const size_t bitCount = sizeof( align ) * 8;

    // assume math based on x86 bits.
    if ( std::bitset <bitCount> ( align ).count() == 1 )
    {
        //bitfield version. not compatible with non-bitfield alignments.
        return (((num) + (sector) - 1) & (~((align) - 1)));
    }
    else
    {
		return _ALIGN_GP( num, sector, align );
    }
}

template <typename numberType>
AINLINE numberType ALIGN( numberType num, numberType sector, numberType align )
{
	return _ALIGN_GP( num, sector, align );
}

// Optimized primitives.
template <> AINLINE char			ALIGN( char num, char sector, char align )								{ return _ALIGN_NATIVE( num, sector, align ); }
template <> AINLINE unsigned char	ALIGN( unsigned char num, unsigned char sector, unsigned char align )	{ return _ALIGN_NATIVE( num, sector, align ); }
template <> AINLINE short			ALIGN( short num, short sector, short align )							{ return _ALIGN_NATIVE( num, sector, align ); }
template <> AINLINE unsigned short	ALIGN( unsigned short num, unsigned short sector, unsigned short align ){ return _ALIGN_NATIVE( num, sector, align ); }
template <> AINLINE int				ALIGN( int num, int sector, int align )									{ return _ALIGN_NATIVE( num, sector, align ); }
template <> AINLINE unsigned int	ALIGN( unsigned int num, unsigned int sector, unsigned int align )
{
	return (unsigned int)_ALIGN_NATIVE( (int)num, (int)sector, (int)align );
}

// Helper macro (equivalent of EXAMPLE 1)
template <typename numberType>
inline numberType ALIGN_SIZE( numberType num, numberType sector )
{
    return ( ALIGN( (num), (sector), (sector) ) );
}

#endif //_MEMORY_RAW_DEFS_