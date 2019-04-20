// API specific to ease management of relocation entries in PE files.

#include "peframework.h"

#include "peloader.internal.hxx"

void PEFile::AddRelocation( std::uint32_t rva, PEBaseReloc::eRelocType relocType )
{
    // We only support particular types of items here.
    if ( relocType != PEBaseReloc::eRelocType::ABSOLUTE &&
         relocType != PEBaseReloc::eRelocType::HIGH &&
         relocType != PEBaseReloc::eRelocType::LOW &&
         relocType != PEBaseReloc::eRelocType::HIGHLOW &&
         relocType != PEBaseReloc::eRelocType::DIR64 )
    {
        throw peframework_exception(
            ePEExceptCode::RUNTIME_ERROR,
            "invalid relocation type registration attempt"
        );
    }

    // Since we divided base relocation RVA by chunk size (default 4K) we can
    // simple divide to get the index by RVA aswell. Pretty nice!

    std::uint32_t dictIndex = ( rva / baserelocChunkSize );

    PEBaseReloc& relocDict = this->baseRelocs[ dictIndex ];

    // Items inside of a base relocation chunk are not structured particularily.
    // At least this is my assumption, based on eRelocType::HIGHADJ.

    std::uint32_t insideChunkOff = ( rva % baserelocChunkSize );

    PEBaseReloc::item newItem;
    newItem.type = (std::uint16_t)relocType;
    newItem.offset = insideChunkOff;

    relocDict.items.push_back( std::move( newItem ) );

    // We need a new base relocations array.
    this->baseRelocAllocEntry = PESectionAllocation();
}

void PEFile::RemoveRelocations( std::uint32_t rva, std::uint32_t regionSize )
{
    if ( regionSize == 0 )
        return;

    // We remove all relocations inside of the given region.

    std::uint32_t baserelocRemoveIndex = ( rva / baserelocChunkSize );

    typedef sliceOfData <std::uint32_t> rvaSlice_t;

    rvaSlice_t requestSlice( rva, regionSize );

    auto iter = this->baseRelocs.lower_bound( baserelocRemoveIndex );

    while ( iter != this->baseRelocs.end() )
    {
        bool doRemove = false;
        {
            PEBaseReloc& relocDict = iter->second;

            // Check the relationship to this item.
            rvaSlice_t dictSlice( relocDict.offsetOfReloc, baserelocChunkSize );

            rvaSlice_t::eIntersectionResult intResult = requestSlice.intersectWith( dictSlice );

            if ( rvaSlice_t::isFloatingIntersect( intResult ) )
            {
                // We are finished.
                break;
            }
            else if ( intResult == rvaSlice_t::INTERSECT_ENCLOSING ||
                      intResult == rvaSlice_t::INTERSECT_EQUAL )
            {
                // The request is enclosing the base relocation block.
                // We must get rid of it entirely.
                doRemove = true;
            }
            else if ( intResult == rvaSlice_t::INTERSECT_INSIDE ||
                      intResult == rvaSlice_t::INTERSECT_BORDER_END ||
                      intResult == rvaSlice_t::INTERSECT_BORDER_START )
            {
                // We remove single items from this base relocation entry.
                {
                    auto item_iter = relocDict.items.begin();

                    while ( item_iter != relocDict.items.end() )
                    {
                        PEBaseReloc::item& dictItem = *item_iter;

                        // TODO: maybe make dict items size after what memory they actually take.

                        rvaSlice_t itemSlice( dictSlice.GetSliceStartPoint() + dictItem.offset, 1 );

                        rvaSlice_t::eIntersectionResult itemIntResult = requestSlice.intersectWith( itemSlice );

                        bool shouldRemove = ( rvaSlice_t::isFloatingIntersect( itemIntResult ) == false );

                        if ( shouldRemove )
                        {
                            item_iter = relocDict.items.erase( item_iter );
                        }
                        else
                        {
                            item_iter++;
                        }
                    }
                }

                // Now see if we can remove an empty dict.
                if ( relocDict.items.empty() )
                {
                    doRemove = true;
                }
            }
            else
            {
                assert( 0 );
            }
        }

        if ( doRemove )
        {
            iter = this->baseRelocs.erase( iter );
        }
        else
        {
            iter++;
        }
    }
    
    // Finished.
}

void PEFile::OnWriteAbsoluteVA( PESection *writeSect, std::uint32_t sectOff, bool is64Bit )
{
    // Check if we need to write a relocation entry.
    bool needsRelocation = false;

    if ( this->peOptHeader.dll_hasDynamicBase )
    {
        needsRelocation = true;
    }

    if ( !needsRelocation )
    {
        if ( this->baseRelocs.empty() == false )
        {
            needsRelocation = true;
        }
    }

    if ( needsRelocation )
    {
        // We either write a 32bit or 64bit relocation entry.
        PEBaseReloc::eRelocType relocType;

        if ( is64Bit )
        {
            relocType = PEBaseReloc::eRelocType::DIR64;
        }
        else
        {
            relocType = PEBaseReloc::eRelocType::HIGHLOW;
        }

        // Calculate the RVA.
        std::uint32_t rva = writeSect->ResolveRVA( sectOff );

        this->AddRelocation( rva, relocType );
    }
}