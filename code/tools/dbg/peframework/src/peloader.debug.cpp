// PEloader helpers for dealing with the debug directory.

#include "peloader.h"

#include "peloader.internal.hxx"

#include <time.h>

PEFile::PEDebugDesc& PEFile::AddDebugData( std::uint32_t debugType )
{
    // Create a new descriptor.
    {
        PEDebugDesc newDesc;
        newDesc.characteristics = 0;
        newDesc.timeDateStamp = (std::uint32_t)time( NULL );
        newDesc.majorVer = 0;
        newDesc.minorVer = 0;
        newDesc.type = debugType;
    
        this->debugDescs.push_back( std::move( newDesc ) );

        // Invalidate the PE native array.
        this->debugDescsAlloc = PESectionAllocation();
    }

    // Return it.
    return this->debugDescs.back();
}

bool PEFile::ClearDebugDataOfType( std::uint32_t debugType )
{
    bool hasChanged = false;

    auto iter = this->debugDescs.begin();

    while ( iter != this->debugDescs.end() )
    {
        PEDebugDesc& debugInfo = *iter;

        if ( debugInfo.type == debugType )
        {
            iter = this->debugDescs.erase( iter );

            hasChanged = true;
        }
        else
        {
            iter++;
        }
    }

    if ( hasChanged )
    {
        this->debugDescsAlloc = PESectionAllocation();
    }

    return hasChanged;
}