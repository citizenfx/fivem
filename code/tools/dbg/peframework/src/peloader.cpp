#include "peframework.h"

#include <vector>

#include "peloader.internal.hxx"

PEFile::PEFile( void ) : resourceRoot( false, std::u16string(), 0 ), sections( 0x1000, 0x10000 )
{
    // By default we generate plain PE32+ files.
    // If true then PE32+ files are generated.
    this->isExtendedFormat = false;
}

PEFile::~PEFile( void )
{
    return;
}

std::uint16_t PEFile::GetPENativeFileFlags( void )
{
    std::uint16_t chars = 0;

    // Are relocations stripped?
    if ( this->HasRelocationInfo() == false )
    {
        chars |= PEL_IMAGE_FILE_RELOCS_STRIPPED;
    }

    if ( this->pe_finfo.isExecutableImage )
    {
        chars |= PEL_IMAGE_FILE_EXECUTABLE_IMAGE;
    }

    if ( this->HasLinenumberInfo() == false )
    {
        chars |= PEL_IMAGE_FILE_LINE_NUMS_STRIPPED;
    }

    if ( !this->pe_finfo.hasLocalSymbols )
    {
        chars |= PEL_IMAGE_FILE_LOCAL_SYMS_STRIPPED;
    }

    if ( this->pe_finfo.hasAggressiveTrim )
    {
        chars |= PEL_IMAGE_FILE_AGGRESIVE_WS_TRIM;
    }

    if ( this->pe_finfo.largeAddressAware )
    {
        chars |= PEL_IMAGE_FILE_LARGE_ADDRESS_AWARE;
    }

    if ( this->pe_finfo.bytesReversedLO )
    {
        chars |= PEL_IMAGE_FILE_BYTES_REVERSED_LO;
    }

    // Output if we are made for a 32bit machine.
    // Since the PE file format is extensible we have to preserve this option.
    if ( this->pe_finfo.madeFor32Bit )
    {
        chars |= PEL_IMAGE_FILE_32BIT_MACHINE;
    }

    if ( this->HasDebugInfo() == false )
    {
        chars |= PEL_IMAGE_FILE_DEBUG_STRIPPED;
    }

    if ( this->pe_finfo.removableRunFromSwap )
    {
        chars |= PEL_IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP;
    }

    if ( this->pe_finfo.netRunFromSwap )
    {
        chars |= PEL_IMAGE_FILE_NET_RUN_FROM_SWAP;
    }

    if ( this->pe_finfo.isSystemFile )
    {
        chars |= PEL_IMAGE_FILE_SYSTEM;
    }

    if ( this->pe_finfo.isDLL )
    {
        chars |= PEL_IMAGE_FILE_DLL;
    }

    if ( this->pe_finfo.upSystemOnly )
    {
        chars |= PEL_IMAGE_FILE_UP_SYSTEM_ONLY;
    }

    if ( this->pe_finfo.bytesReversedHI )
    {
        chars |= PEL_IMAGE_FILE_BYTES_REVERSED_HI;
    }

    return chars;
}

std::uint16_t PEFile::GetPENativeDLLOptFlags( void )
{
    std::uint16_t chars = 0;

    if ( this->peOptHeader.dll_supportsHighEntropy )
    {
        chars |= PEL_IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA;
    }

    if ( this->peOptHeader.dll_hasDynamicBase )
    {
        chars |= PEL_IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE;
    }

    if ( this->peOptHeader.dll_forceIntegrity )
    {
        chars |= PEL_IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY;
    }

    if ( this->peOptHeader.dll_nxCompat )
    {
        chars |= PEL_IMAGE_DLLCHARACTERISTICS_NX_COMPAT;
    }

    if ( this->peOptHeader.dll_noIsolation )
    {
        chars |= PEL_IMAGE_DLLCHARACTERISTICS_NO_ISOLATION;
    }

    if ( this->peOptHeader.dll_noSEH )
    {
        chars |= PEL_IMAGE_DLLCHARACTERISTICS_NO_SEH;
    }

    if ( this->peOptHeader.dll_noBind )
    {
        chars |= PEL_IMAGE_DLLCHARACTERISTICS_NO_BIND;
    }

    if ( this->peOptHeader.dll_appContainer )
    {
        chars |= PEL_IMAGE_DLLCHARACTERISTICS_APPCONTAINER;
    }

    if ( this->peOptHeader.dll_wdmDriver )
    {
        chars |= PEL_IMAGE_DLLCHARACTERISTICS_WDM_DRIVER;
    }

    if ( this->peOptHeader.dll_guardCF )
    {
        chars |= PEL_IMAGE_DLLCHARACTERISTICS_GUARD_CF;
    }

    if ( this->peOptHeader.dll_termServAware )
    {
        chars |= PEL_IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE;
    }

    return chars;
}

PEFile::PESection::PESection( void ) : stream( NULL, 0, streamAllocMan )
{
    this->virtualSize = 0;
    this->virtualAddr = 0;
    this->chars.sect_hasNoPadding = false;
    this->chars.sect_containsCode = false;
    this->chars.sect_containsInitData = false;
    this->chars.sect_containsUninitData = false;
    this->chars.sect_link_other = false;
    this->chars.sect_link_info = false;
    this->chars.sect_link_remove = false;
    this->chars.sect_link_comdat = false;
    this->chars.sect_noDeferSpecExcepts = false;
    this->chars.sect_gprel = false;
    this->chars.sect_mem_farData = false;
    this->chars.sect_mem_purgeable = false;
    this->chars.sect_mem_16bit = false;
    this->chars.sect_mem_locked = false;
    this->chars.sect_mem_preload = false;
    this->chars.sect_alignment = eAlignment::BYTES_UNSPECIFIED;
    this->chars.sect_link_nreloc_ovfl = false;
    this->chars.sect_mem_discardable = false;
    this->chars.sect_mem_not_cached = false;
    this->chars.sect_mem_not_paged = false;
    this->chars.sect_mem_shared = false;
    this->chars.sect_mem_execute = false;
    this->chars.sect_mem_read = true;
    this->chars.sect_mem_write = false;
    this->isFinal = false;
    this->ownerImage = NULL;
}

PEFile::PESection::~PESection( void )
{
    // Destruction requires several undo-operations related to PE validity.
    // * all active section allocations have to be invalidated (they can be)
    {
        LIST_FOREACH_BEGIN( PESectionAllocation, this->dataAllocList.root, sectionNode )

            item->theSection = NULL;
            item->sectOffset = 0;
            item->dataSize = 0;

        LIST_FOREACH_END

        this->dataAlloc.Clear();
        LIST_CLEAR( this->dataAllocList.root );
    }
    // * all active section data references
    {
        LIST_FOREACH_BEGIN( PESectionReference, this->dataRefList.root, sectionNode )
        
            // Reset data fields to unlinked status
            // without clearing ties itself.
            item->clearLink();

        LIST_FOREACH_END

        LIST_CLEAR( this->dataRefList.root );
    }
    // * all active placed offsets that refer to this section must be invalidated (write a dead-pointer instead)
    {
        LIST_FOREACH_BEGIN( PEPlacedOffset, this->RVAreferalList.root, targetNode )

            item->targetSect = NULL;
            item->dataOffset = 0;
            item->offsetIntoSect = 0;

        LIST_FOREACH_END

        LIST_CLEAR( this->RVAreferalList.root );
    }

    // Remove us from the PE image, if inside.
    this->unregisterOwnerImage();
}

void PEFile::PESection::SetPlacementInfo( std::uint32_t virtAddr, std::uint32_t virtSize )
{
    // This method is called if the section should be placed into a specific position into
    // a PE binary.
    assert( this->ownerImage == NULL );

    this->virtualAddr = virtAddr;
    this->virtualSize = virtSize;

    // Since this image is about to be placed, we finalize it.
    this->isFinal = true;
}

// Allocation methods of PESection.
std::uint32_t PEFile::PESection::Allocate( PESectionAllocation& allocBlock, std::uint32_t allocSize, std::uint32_t alignment )
{
    // Final sections cannot be allocated on.
    assert( this->isFinal == false );

    // We want to allocate it anywhere.
    sectionSpaceAlloc_t::allocInfo ainfo;

    bool foundSpace = this->dataAlloc.FindSpace( allocSize, ainfo, alignment );

    if ( !foundSpace )
    {
        throw peframework_exception(
            ePEExceptCode::RESOURCE_ERROR,
            "failed to allocate space inside PEFile section"
        );
    }

    this->dataAlloc.PutBlock( &allocBlock.sectionBlock, ainfo );

    // Update meta-data.
    std::uint32_t alloc_off = allocBlock.sectionBlock.slice.GetSliceStartPoint();

    assert( allocBlock.theSection == NULL );

    // We should at least serve the space on the executable section if we allocated there, even if
    // we do not initialize it.
    {
        std::uint32_t sectionDataLength = (std::uint32_t)this->stream.Size();

        std::uint32_t allocOffEnd = alloc_off + allocSize;

        if ( sectionDataLength < allocOffEnd )
        {
            this->stream.Truncate( allocOffEnd );
        }
    }

    allocBlock.theSection = this;
    allocBlock.sectOffset = alloc_off;
    allocBlock.dataSize = allocSize;

    LIST_INSERT( this->dataAllocList.root, allocBlock.sectionNode );

    return alloc_off;
}

void PEFile::PESectionAllocation::WriteToSection( const void *dataPtr, std::uint32_t dataSize, std::int32_t dataOff )
{
    PESection *allocSect = this->theSection;

    if ( !allocSect )
    {
        throw peframework_exception(
            ePEExceptCode::RUNTIME_ERROR,
            "invalid write section call on unallocated construct"
        );
    }

    allocSect->stream.Seek( this->sectOffset + dataOff );
    allocSect->stream.Write( dataPtr, dataSize );
}

void PEFile::PESectionAllocation::RegisterTargetRVA(
    std::uint32_t patchOffset, PESection *targetSect, std::uint32_t targetOff,
    PEFile::PESection::PEPlacedOffset::eOffsetType offsetType
)
{
    this->theSection->RegisterTargetRVA( this->sectOffset + patchOffset, targetSect, targetOff, offsetType );
}

void PEFile::PESectionAllocation::RegisterTargetRVA(
    std::uint32_t patchOffset, const PESectionAllocation& targetInfo, std::uint32_t targetOff,
    PEFile::PESection::PEPlacedOffset::eOffsetType offsetType
)
{
    this->RegisterTargetRVA( patchOffset, targetInfo.theSection, targetInfo.sectOffset + targetOff, offsetType );
}

void PEFile::PESectionAllocation::RegisterTargetRVA(
    std::uint32_t patchOffset, const PESectionDataReference& targetInfo, std::uint32_t targetOff,
    PEFile::PESection::PEPlacedOffset::eOffsetType offsetType
)
{
    this->RegisterTargetRVA( patchOffset, targetInfo.theSect, targetInfo.sectOffset + targetOff, offsetType );
}

void PEFile::PESection::SetPlacedMemory( PESectionAllocation& blockMeta, std::uint32_t allocOff, std::uint32_t allocSize )
{
    assert( allocOff >= this->virtualAddr );

    this->SetPlacedMemoryInline(
        blockMeta,
        ( allocOff - this->virtualAddr ),
        allocSize
    );
}

void PEFile::PESection::SetPlacedMemoryInline( PESectionAllocation& blockMeta, std::uint32_t allocOff, std::uint32_t allocSize )
{
    assert( this->isFinal == true );
    assert( this->ownerImage != NULL );

    // We keep the block allocation structure invalid.
    assert( blockMeta.theSection == NULL );

    // Verify that this allocation really is inside the section.
    {
        typedef sliceOfData <std::uint32_t> streamSlice_t;

        streamSlice_t sectionSlice( 0, this->virtualSize );
        
        streamSlice_t reqSlice( allocOff, std::max( allocSize, 1u ) );

        streamSlice_t::eIntersectionResult intResult = reqSlice.intersectWith( sectionSlice );

        assert( intResult == streamSlice_t::INTERSECT_INSIDE || intResult == streamSlice_t::INTERSECT_EQUAL );
    }

    blockMeta.sectOffset = allocOff;
    blockMeta.dataSize = allocSize;
    blockMeta.theSection = this;

    LIST_INSERT( this->dataAllocList.root, blockMeta.sectionNode );
}

PEFile::PESection::PEPlacedOffset::PEPlacedOffset( std::uint32_t dataOffset, PESection *targetSect, std::uint32_t offsetIntoSect, eOffsetType offType )
{
    this->dataOffset = dataOffset;
    this->targetSect = targetSect;
    this->offsetIntoSect = offsetIntoSect;
    this->offsetType = offType;

    if ( targetSect )
    {
        LIST_INSERT( targetSect->RVAreferalList.root, this->targetNode );
    }
}

void PEFile::PESection::PEPlacedOffset::WriteIntoData( PEFile *peImage, PESection *writeSect, std::uint64_t imageBase ) const
{
    // Parameters to write the offset.
    std::int32_t writeOff = this->dataOffset;

    // Parameters to calculate the offset.
    PESection *targetSect = this->targetSect;
    std::uint32_t targetOff = this->offsetIntoSect;

    // There are several types of offsets we can write, not just RVA.
    eOffsetType offType = this->offsetType;

    if ( offType == eOffsetType::RVA )
    {
        // Calculate target RVA.
        std::uint32_t targetRVA = 0;

        if ( targetSect )
        {
            targetRVA = targetSect->ResolveRVA( targetOff );
        }

        // Write the RVA.
        writeSect->stream.Seek( writeOff );
        writeSect->stream.WriteUInt32( targetRVA );
    }
    else if ( offType == eOffsetType::VA_32BIT )
    {
        // Calculate the absolute VA.
        std::uint32_t targetVA = 0;
        
        if ( targetSect )
        {
            targetVA = ( (std::uint32_t)imageBase + targetSect->ResolveRVA( targetOff ) );
        }

        // Write it prematurely.
        writeSect->stream.Seek( writeOff );
        writeSect->stream.WriteUInt32( targetVA );

        // Notify the runtime.
        peImage->OnWriteAbsoluteVA( writeSect, writeOff, false );
    }
    else if ( offType == eOffsetType::VA_64BIT )
    {
        std::uint64_t targetVA = 0;

        if ( targetSect )
        {
            targetVA = ( imageBase + targetSect->ResolveRVA( targetOff ) );
        }

        writeSect->stream.Seek( writeOff );
        writeSect->stream.WriteUInt64( targetVA );

        peImage->OnWriteAbsoluteVA( writeSect, writeOff, true );
    }
    else
    {
        // Should never happen.
        assert( 0 );
    }
}

std::uint32_t PEFile::PESection::ResolveRVA( std::uint32_t sectOffset ) const
{
    assert( this->ownerImage != NULL );

    return ( this->virtualAddr + sectOffset );
}

void PEFile::PESection::SetPENativeFlags( std::uint32_t schars )
{
    chars.sect_hasNoPadding             = ( schars & PEL_IMAGE_SCN_TYPE_NO_PAD ) != 0;
    chars.sect_containsCode             = ( schars & PEL_IMAGE_SCN_CNT_CODE ) != 0;
    chars.sect_containsInitData         = ( schars & PEL_IMAGE_SCN_CNT_INITIALIZED_DATA ) != 0;
    chars.sect_containsUninitData       = ( schars & PEL_IMAGE_SCN_CNT_UNINITIALIZED_DATA ) != 0;
    chars.sect_link_other               = ( schars & PEL_IMAGE_SCN_LNK_OTHER ) != 0;
    chars.sect_link_info                = ( schars & PEL_IMAGE_SCN_LNK_INFO ) != 0;
    chars.sect_link_remove              = ( schars & PEL_IMAGE_SCN_LNK_REMOVE ) != 0;
    chars.sect_link_comdat              = ( schars & PEL_IMAGE_SCN_LNK_COMDAT ) != 0;
    chars.sect_noDeferSpecExcepts       = ( schars & PEL_IMAGE_SCN_NO_DEFER_SPEC_EXC ) != 0;
    chars.sect_gprel                    = ( schars & PEL_IMAGE_SCN_GPREL ) != 0;
    chars.sect_mem_farData              = ( schars & PEL_IMAGE_SCN_MEM_FARDATA ) != 0;
    chars.sect_mem_purgeable            = ( schars & PEL_IMAGE_SCN_MEM_PURGEABLE ) != 0;
    chars.sect_mem_16bit                = ( schars & PEL_IMAGE_SCN_MEM_16BIT ) != 0;
    chars.sect_mem_locked               = ( schars & PEL_IMAGE_SCN_MEM_LOCKED ) != 0;
    chars.sect_mem_preload              = ( schars & PEL_IMAGE_SCN_MEM_PRELOAD ) != 0;
        
    // Parse the alignment information out of the chars.
    PESection::eAlignment alignNum = (PESection::eAlignment)( ( schars & 0x00F00000 ) >> 20 );
    chars.sect_alignment = alignNum;

    chars.sect_link_nreloc_ovfl         = ( schars & PEL_IMAGE_SCN_LNK_NRELOC_OVFL ) != 0;
    chars.sect_mem_discardable          = ( schars & PEL_IMAGE_SCN_MEM_DISCARDABLE ) != 0;
    chars.sect_mem_not_cached           = ( schars & PEL_IMAGE_SCN_MEM_NOT_CACHED ) != 0;
    chars.sect_mem_not_paged            = ( schars & PEL_IMAGE_SCN_MEM_NOT_PAGED ) != 0;
    chars.sect_mem_shared               = ( schars & PEL_IMAGE_SCN_MEM_SHARED ) != 0;
    chars.sect_mem_execute              = ( schars & PEL_IMAGE_SCN_MEM_EXECUTE ) != 0;
    chars.sect_mem_read                 = ( schars & PEL_IMAGE_SCN_MEM_READ ) != 0;
    chars.sect_mem_write                = ( schars & PEL_IMAGE_SCN_MEM_WRITE ) != 0;
}

std::uint32_t PEFile::PESection::GetPENativeFlags( void ) const
{
    std::uint32_t chars = 0;

    if ( this->chars.sect_hasNoPadding )
    {
        chars |= PEL_IMAGE_SCN_TYPE_NO_PAD;
    }

    if ( this->chars.sect_containsCode )
    {
        chars |= PEL_IMAGE_SCN_CNT_CODE;
    }

    if ( this->chars.sect_containsInitData )
    {
        chars |= PEL_IMAGE_SCN_CNT_INITIALIZED_DATA;
    }

    if ( this->chars.sect_containsUninitData )
    {
        chars |= PEL_IMAGE_SCN_CNT_UNINITIALIZED_DATA;
    }

    if ( this->chars.sect_link_other )
    {
        chars |= PEL_IMAGE_SCN_LNK_OTHER;
    }

    if ( this->chars.sect_link_info )
    {
        chars |= PEL_IMAGE_SCN_LNK_INFO;
    }

    if ( this->chars.sect_link_remove )
    {
        chars |= PEL_IMAGE_SCN_LNK_REMOVE;
    }

    if ( this->chars.sect_link_comdat )
    {
        chars |= PEL_IMAGE_SCN_LNK_COMDAT;
    }

    if ( this->chars.sect_noDeferSpecExcepts )
    {
        chars |= PEL_IMAGE_SCN_NO_DEFER_SPEC_EXC;
    }

    if ( this->chars.sect_gprel )
    {
        chars |= PEL_IMAGE_SCN_GPREL;
    }

    if ( this->chars.sect_mem_farData )
    {
        chars |= PEL_IMAGE_SCN_MEM_FARDATA;
    }

    if ( this->chars.sect_mem_purgeable )
    {
        chars |= PEL_IMAGE_SCN_MEM_PURGEABLE;
    }

    if ( this->chars.sect_mem_16bit )
    {
        chars |= PEL_IMAGE_SCN_MEM_16BIT;
    }

    if ( this->chars.sect_mem_locked )
    {
        chars |= PEL_IMAGE_SCN_MEM_LOCKED;
    }

    if ( this->chars.sect_mem_preload )
    {
        chars |= PEL_IMAGE_SCN_MEM_PRELOAD;
    }

    switch( this->chars.sect_alignment )
    {
    case eAlignment::BYTES_UNSPECIFIED: break;  // unknown.
    case eAlignment::BYTES_1:           chars |= PEL_IMAGE_SCN_ALIGN_1BYTES; break;
    case eAlignment::BYTES_2:           chars |= PEL_IMAGE_SCN_ALIGN_2BYTES; break;
    case eAlignment::BYTES_4:           chars |= PEL_IMAGE_SCN_ALIGN_4BYTES; break;
    case eAlignment::BYTES_8:           chars |= PEL_IMAGE_SCN_ALIGN_8BYTES; break;
    case eAlignment::BYTES_16:          chars |= PEL_IMAGE_SCN_ALIGN_16BYTES; break;
    case eAlignment::BYTES_32:          chars |= PEL_IMAGE_SCN_ALIGN_32BYTES; break;
    case eAlignment::BYTES_64:          chars |= PEL_IMAGE_SCN_ALIGN_64BYTES; break;
    case eAlignment::BYTES_128:         chars |= PEL_IMAGE_SCN_ALIGN_128BYTES; break;
    case eAlignment::BYTES_256:         chars |= PEL_IMAGE_SCN_ALIGN_256BYTES; break;
    case eAlignment::BYTES_512:         chars |= PEL_IMAGE_SCN_ALIGN_512BYTES; break;
    case eAlignment::BYTES_1024:        chars |= PEL_IMAGE_SCN_ALIGN_1024BYTES; break;
    case eAlignment::BYTES_2048:        chars |= PEL_IMAGE_SCN_ALIGN_2048BYTES; break;
    case eAlignment::BYTES_4096:        chars |= PEL_IMAGE_SCN_ALIGN_4096BYTES; break;
    case eAlignment::BYTES_8192:        chars |= PEL_IMAGE_SCN_ALIGN_8192BYTES; break;
    default:                            break;  // should never happen.
    }

    if ( this->chars.sect_link_nreloc_ovfl )
    {
        chars |= PEL_IMAGE_SCN_LNK_NRELOC_OVFL;
    }

    if ( this->chars.sect_mem_discardable )
    {
        chars |= PEL_IMAGE_SCN_MEM_DISCARDABLE;
    }

    if ( this->chars.sect_mem_not_cached )
    {
        chars |= PEL_IMAGE_SCN_MEM_NOT_CACHED;
    }

    if ( this->chars.sect_mem_not_paged )
    {
        chars |= PEL_IMAGE_SCN_MEM_NOT_PAGED;
    }

    if ( this->chars.sect_mem_shared )
    {
        chars |= PEL_IMAGE_SCN_MEM_SHARED;
    }

    if ( this->chars.sect_mem_execute )
    {
        chars |= PEL_IMAGE_SCN_MEM_EXECUTE;
    }

    if ( this->chars.sect_mem_read )
    {
        chars |= PEL_IMAGE_SCN_MEM_READ;
    }

    if ( this->chars.sect_mem_write )
    {
        chars |= PEL_IMAGE_SCN_MEM_WRITE;
    }

    return chars;
}

void PEFile::PESection::RegisterTargetRVA(
    std::uint32_t patchOffset, PESection *targetSect, std::uint32_t targetOffset,
    PEPlacedOffset::eOffsetType offsetType
)
{
    // Make sure our section has space at that point.
    {
        assert( this->isFinal == false );

        std::int32_t sectSize = this->stream.Size();

        const std::int32_t reqSectSize = (std::int32_t)( patchOffset + sizeof(std::uint32_t) );

        if ( sectSize < reqSectSize )
        {
            this->stream.Truncate( reqSectSize );
        }
    }

    this->placedOffsets.emplace_back( patchOffset, targetSect, targetOffset, offsetType );
}

void PEFile::PESection::RegisterTargetRVA(
    std::uint32_t patchOffset, const PESectionAllocation& targetInfo,
    PEPlacedOffset::eOffsetType offsetType
)
{
    RegisterTargetRVA( patchOffset, targetInfo.theSection, targetInfo.sectOffset, offsetType );
}

void PEFile::PESection::Finalize( void )
{
    if ( this->isFinal )
        return;

    // The image does not have a virtualSize parameter yet.
    assert( this->virtualSize == 0 );
    
    // It is created by taking the rawdata size.
    // The image will later round it to section alignment.
    this->virtualSize = ( (decltype(virtualSize))stream.Size() );

    // Final images are considered not allocatable anymore
    // so lets get rid of allocation information.
    this->dataAlloc.Clear();

    this->isFinal = true;
}

void PEFile::PESection::FinalizeProfound( std::uint32_t virtSize )
{
    if ( this->isFinal )
        return;

    // The image does not have a virtualSize parameter yet.
    assert( this->virtualSize == 0 );
    
    // Ensure that the guy set a proper virtual size.
    // If not then we are in trouble.
    std::uint32_t atLeastVirtSize = ( (std::uint32_t)stream.Size() );

    if ( atLeastVirtSize > virtSize )
    {
        throw peframework_exception(
            ePEExceptCode::RUNTIME_ERROR,
            "invalid virtual size in profound section finalization"
        );
    }
    
    // Store the user-given value.
    // We assume that it is aligned properly.
    this->virtualSize = virtSize;

    // Final images are considered not allocatable anymore
    // so lets get rid of allocation information.
    this->dataAlloc.Clear();

    this->isFinal = true;
}

PEFile::PESectionMan::PESectionMan( std::uint32_t sectionAlignment, std::uint32_t imageBase )
{
    this->numSections = 0;
    this->sectionAlignment = sectionAlignment;
    this->imageBase = imageBase;
}

PEFile::PESectionMan::PESectionMan( PESectionMan&& right )
    : sectionAlignment( std::move( right.sectionAlignment ) ),
      imageBase( std::move( right.imageBase ) ),
      sectVirtualAllocMan( std::move( right.sectVirtualAllocMan ) ),
      numSections( std::move( right.numSections ) ),
      sectionList( std::move( right.sectionList ) )
{
    LIST_FOREACH_BEGIN( PESection, this->sectionList.root, sectionNode )

        // Need to update this.
        item->ownerImage = this;

    LIST_FOREACH_END
}

PEFile::PESectionMan::~PESectionMan( void )
{
    // Destroy all sections that still reside in us.
    LIST_FOREACH_BEGIN( PESection, this->sectionList.root, sectionNode )

        item->ownerImage = NULL;

        delete item;

    LIST_FOREACH_END

    LIST_CLEAR( this->sectionList.root );

    this->numSections = 0;
}

PEFile::PESectionMan& PEFile::PESectionMan::operator = ( PESectionMan&& right )
{
    // We do the default way of move-assignment.
    this->~PESectionMan();

    return *new (this) PESectionMan( std::move( right ) );
}

PEFile::PESection* PEFile::PESectionMan::AddSection( PESection&& theSection )
{
    assert( theSection.ownerImage == NULL );

    // Before proceeding we must have finalized the section.
    // A final section must have a valid virtualSize region of all its allocations.
    assert( theSection.isFinal == true );

    // Images have a base address to start allocations from that is decided from the
    // very beginning.
    const std::uint32_t imageBase = this->imageBase;

    // When the section is bound to our image, we will give it an aligned size
    // based on sectionAlignment.
    const std::uint32_t sectionAlignment = this->sectionAlignment;

    std::uint32_t alignedSectionSize = ALIGN_SIZE( theSection.virtualSize, sectionAlignment );

    // We allocate space for this section inside of our executable.
    sectAllocSemantics::allocInfo allocInfo;

    bool foundSpace = sectAllocSemantics::FindSpace( sectVirtualAllocMan, alignedSectionSize, allocInfo, sectionAlignment, imageBase );

    if ( !foundSpace )
    {
        // In very critical scenarios the executable may be full!
        return NULL;
    }

    // We need to move the section into memory we control.
    PESection *ourSect = new PESection( std::move( theSection ) );

    // Since we did find some space lets register the new section candidate.
    ourSect->virtualAddr = allocInfo.slice.GetSliceStartPoint();
    ourSect->virtualSize = std::move( alignedSectionSize );

    // Put after correct block.
    LIST_INSERT( *allocInfo.blockToAppendAt.node_iter, ourSect->sectionNode );

    ourSect->ownerImage = this;

    this->numSections++;

    return ourSect;
}

PEFile::PESection* PEFile::PESectionMan::PlaceSection( PESection&& theSection )
{
    assert( theSection.ownerImage == NULL );

    // The section must be final because it requires a given offset and size.
    assert( theSection.isFinal == true );

    assert( theSection.virtualSize != 0 );

    // In this routine we place a section at it's requested aligned offset.
    const std::uint32_t sectionAlignment = this->sectionAlignment;

    std::uint32_t alignSectOffset = ALIGN( theSection.virtualAddr, 1u, sectionAlignment );
    std::uint32_t alignSectSize = ALIGN_SIZE( theSection.virtualSize, sectionAlignment );

    sectAllocSemantics::allocInfo allocInfo;

    bool obtSpace = sectAllocSemantics::ObtainSpaceAt( sectVirtualAllocMan, alignSectOffset, alignSectSize, allocInfo );

    if ( !obtSpace )
    {
        // If this is triggered then most likely there is an invalid PE section configuration.
        return NULL;
    }

    // Now put the section into our space.
    PESection *ourSect = new PESection( std::move( theSection ) );

    ourSect->virtualAddr = std::move( alignSectOffset );
    ourSect->virtualSize = std::move( alignSectSize );

    // Put after correct block.
    LIST_INSERT( *allocInfo.blockToAppendAt.node_iter, ourSect->sectionNode );

    ourSect->ownerImage = this;

    this->numSections++;

    return ourSect;
}

bool PEFile::PESectionMan::RemoveSection( PESection *section )
{
    if ( section->ownerImage != this )
        return false;

    LIST_REMOVE( section->sectionNode );

    section->ownerImage = NULL;

    this->numSections--;

    return true;
}

bool PEFile::PESectionMan::FindSectionSpace( std::uint32_t spanSize, std::uint32_t& addrOut )
{
    // Images have a base address to start allocations from that is decided from the
    // very beginning.
    const std::uint32_t imageBase = this->imageBase;

    // When the section is bound to our image, we will give it an aligned size
    // based on sectionAlignment.
    const std::uint32_t sectionAlignment = this->sectionAlignment;

    std::uint32_t alignedSectionSize = ALIGN_SIZE( spanSize, sectionAlignment );

    // We allocate space for this section inside of our executable.
    sectAllocSemantics::allocInfo allocInfo;

    bool foundSpace = sectAllocSemantics::FindSpace( sectVirtualAllocMan, alignedSectionSize, allocInfo, sectionAlignment, imageBase );
    
    if ( !foundSpace )
    {
        return false;
    }

    addrOut = allocInfo.slice.GetSliceStartPoint();
    return true;
}

void PEFile::PEFileSpaceData::ClearData( void )
{
    if ( this->storageType == eStorageType::FILE )
    {
        this->fileRef.clear();
    }
    else if ( this->storageType == eStorageType::SECTION )
    {
        this->sectRef = PESectionAllocation();
    }
    else
    {
        assert( this->storageType == eStorageType::NONE );
    }
        
    this->storageType = eStorageType::NONE;
}

void PEFile::PEFileSpaceData::fileSpaceStreamBufferManager::EstablishBufferView( void*& memPtr, std::int32_t& streamSize, std::int32_t reqSize )
{
    PEFileSpaceData *fileSpaceMan = this->GetManager();

    if ( reqSize == 0 )
    {
        // Release all resources.
        fileSpaceMan->ClearData();

        // Reset buffer variables.
        memPtr = NULL;
        streamSize = 0;
    }
    else
    {
        // If the data is section-based, we must release the section reference.
        if ( fileSpaceMan->storageType == eStorageType::SECTION )
        {
            // Copy all data into a buffer.
            std::vector <char> dataBuf;

            std::uint32_t dataSize = fileSpaceMan->sectRef.GetDataSize();

            dataBuf.resize( dataSize );

            PEDataStream dataStream( fileSpaceMan->sectRef.GetSection(), fileSpaceMan->sectRef.ResolveInternalOffset( 0 ) );

            dataStream.Read( dataBuf.data(), dataSize );

            // Release the section reference.
            fileSpaceMan->sectRef = PESectionAllocation();

            // Acquire a new file-data status.
            fileSpaceMan->fileRef = std::move( dataBuf );

            fileSpaceMan->storageType = eStorageType::FILE;
        }
        // By now, we must not be section based.

        // Simply resize ourselves.
        try
        {
            fileSpaceMan->fileRef.resize( reqSize );
        }
        catch( std::exception& )
        {
            // Some error occurred... :(
            return;
        }

        // Success!
        memPtr = fileSpaceMan->fileRef.data();
        streamSize = reqSize;

        fileSpaceMan->storageType = eStorageType::FILE;
    }
}

PEFile::PEFileSpaceData::fileSpaceStream_t PEFile::PEFileSpaceData::OpenStream( bool createNew )
{
    // This means to clear previous data.
    if ( createNew )
    {
        this->ClearData();
    }

    // Get the buffer properties.
    void *streamBuf = NULL;
    std::uint32_t streamSize = 0;

    eStorageType storageType = this->storageType;

    if ( storageType == eStorageType::SECTION )
    {
        // We do something really dangerous, which is getting a direct pointer
        // into section data. This is a totally valid operation as long as we
        // control it internally.

        PESection *accSect = this->sectRef.GetSection();

        assert( accSect != NULL );

        streamBuf = ( (char*)accSect->stream.Data() + this->sectRef.ResolveInternalOffset( 0 ) );
        streamSize = this->sectRef.GetDataSize();
    }
    else if ( storageType == eStorageType::FILE )
    {
        streamBuf = this->fileRef.data();
        streamSize = (std::uint32_t)this->fileRef.size();
    }

    return fileSpaceStream_t( streamBuf, streamSize, streamMan );
}

PEFile::PESection* PEFile::AddSection( PESection&& theSection )
{
    return this->sections.AddSection( std::move( theSection ) );
}

PEFile::PESection* PEFile::PlaceSection( PESection&& theSection )
{
    return this->sections.PlaceSection( std::move( theSection ) );
}

PEFile::PESection* PEFile::FindFirstSectionByName( const char *name )
{
    LIST_FOREACH_BEGIN( PESection, this->sections.sectionList.root, sectionNode )

        if ( item->shortName == name )
            return item;
    
    LIST_FOREACH_END

    return NULL;
}

PEFile::PESection* PEFile::FindFirstAllocatableSection( void )
{
    LIST_FOREACH_BEGIN( PESection, this->sections.sectionList.root, sectionNode )
    
        if ( item->IsFinal() == false )
            return item;
    
    LIST_FOREACH_END

    return NULL;
}

PEFile::PESection* PEFile::FindSectionByRVA( std::uint32_t rva, std::uint32_t *sectIndexOut, std::uint32_t *sectOffOut )
{
    PESection *rvaSect;

    bool gotLocation = sections.GetPEDataLocation( rva, sectOffOut, &rvaSect, sectIndexOut );

    if ( gotLocation )
    {
        // Got it.
        return rvaSect;
    }

    // Nothing found.
    return NULL;
}

bool PEFile::RemoveSection( PESection *section )
{
    return sections.RemoveSection( section );
}

bool PEFile::FindSectionSpace( std::uint32_t spanSize, std::uint32_t& addrOut )
{
    return this->sections.FindSectionSpace( spanSize, addrOut );
}

void PEFile::ForAllSections( std::function <void ( PESection* )> cb )
{
    LIST_FOREACH_BEGIN( PESection, this->sections.sectionList.root, sectionNode )

        cb( item );

    LIST_FOREACH_END
}

void PEFile::ForAllSections( std::function <void ( const PESection* )> cb ) const
{
    LIST_FOREACH_BEGIN( PESection, this->sections.sectionList.root, sectionNode )

        const PESection *constSect = item;

        cb( constSect );

    LIST_FOREACH_END
}

bool PEFile::HasRelocationInfo( void ) const
{
    // Check any sections.
    LIST_FOREACH_BEGIN( PESection, this->sections.sectionList.root, sectionNode )
    
        if ( item->relocations.size() != 0 )
            return true;
    
    LIST_FOREACH_END

    // Check the base relocation data.
    if ( this->baseRelocs.size() != 0 )
        return true;

    // Nothing found.
    return false;
}

bool PEFile::HasLinenumberInfo( void ) const
{
    // Check sections.
    LIST_FOREACH_BEGIN( PESection, this->sections.sectionList.root, sectionNode )
    
        if ( item->linenumbers.size() != 0 )
            return true;
    
    LIST_FOREACH_END

    // Has no embedded line number info.
    return false;
}

bool PEFile::HasDebugInfo( void ) const
{
    // We check if we have debug directory data.
    if ( this->debugDescs.size() != 0 )
    {
        return true;
    }

    return false;
}

bool PEFile::IsDynamicLinkLibrary( void ) const
{
    return ( this->pe_finfo.isDLL );
}