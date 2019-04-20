#include "peframework.h"

#include "peloader.internal.hxx"

#include <unordered_map>

using namespace PEloader;

// Writing helpers.
AINLINE void PEWrite( PEStream *peStream, std::uint32_t peOff, std::uint32_t peSize, const void *dataPtr )
{
    // Seek to the right offset.
    {
        bool seekSuccess = peStream->Seek( peOff );

        if ( seekSuccess == false )
        {
            throw peframework_exception(
                ePEExceptCode::RUNTIME_ERROR,
                "failed to seek to PE offset"
            );
        }
    }

    bool hasWritten = peStream->Write( dataPtr, peSize );

    if ( !hasWritten )
    {
        throw peframework_exception(
            ePEExceptCode::RESOURCE_ERROR,
            "failed to write PE data to file"
        );
    }
}

AINLINE void writeContentAt( peFileAlloc& fileSpaceAlloc, PEStream *peStream, peFileAlloc::block_t& allocBlock, std::uint32_t peOff, std::uint32_t peSize, const void *dataPtr )
{
    peFileAlloc::allocInfo alloc_data;

    if ( fileSpaceAlloc.ObtainSpaceAt( peOff, peSize, alloc_data ) == false )
    {
        throw peframework_exception(
            ePEExceptCode::RESOURCE_ERROR,
            "failed to write PE data"
        );
    }
    
    fileSpaceAlloc.PutBlock( &allocBlock, alloc_data );

    // Actually write things.
    PEWrite( peStream, peOff, peSize, dataPtr );
}

AINLINE std::uint32_t allocContentSpace( peFileAlloc& fileSpaceAlloc, peFileAlloc::block_t& allocBlock, std::uint32_t peSize )
{
    peFileAlloc::allocInfo alloc_data;

    if ( !fileSpaceAlloc.FindSpace( peSize, alloc_data, sizeof(std::uint32_t) ) )
    {
        throw peframework_exception(
            ePEExceptCode::RESOURCE_ERROR,
            "failed to find allocation space for PE data"
        );
    }

    fileSpaceAlloc.PutBlock( &allocBlock, alloc_data );

    return alloc_data.slice.GetSliceStartPoint();
}

AINLINE void writeContent( peFileAlloc& fileSpaceAlloc, PEStream *peStream, peFileAlloc::block_t& allocBlock, std::uint32_t peSize, const void *dataPtr )
{
    std::uint32_t dataPos = allocContentSpace( fileSpaceAlloc, allocBlock, peSize );

    // Write things.
    PEWrite( peStream, dataPos, peSize, dataPtr );
}

template <typename keyType, typename mapType>
inline decltype( auto ) FindMapValue( mapType& map, const keyType& key )
{
    const auto& foundIter = map.find( key );

    if ( foundIter == map.end() )
        return (decltype(&foundIter->second))NULL;

    return &foundIter->second;
}

template <typename numberType>
static AINLINE numberType RVA2VA( std::uint32_t rva, std::uint64_t imageBase )
{
    if ( rva == 0 )
        return 0;

    return (numberType)( rva + imageBase );
}

namespace ResourceTools
{

struct item_allocInfo
{
    inline item_allocInfo( void )
    {
        entry_off = 0;
        name_off = 0;
        dataitem_off = 0;
    }
    inline item_allocInfo( const item_allocInfo& right ) = delete;
    inline item_allocInfo( item_allocInfo&& right ) = default;

    inline ~item_allocInfo( void )
    {
        // Delete all possible child entries.
        for ( const auto& pairItem : this->children )
        {
            item_allocInfo *childItem = pairItem.second;

            delete childItem;
        }

        this->children.clear();
    }

    inline item_allocInfo& operator = ( const item_allocInfo& right ) = delete;
    inline item_allocInfo& operator = ( item_allocInfo&& right ) = default;

    // Both entries are relative to the resource directory VA.
    std::uint32_t entry_off;        // Offset to the directory or item entry
    std::uint32_t name_off;         // Offset to the name string (unicode); only valid if child
    std::uint32_t dataitem_off;     // Offset to the resource data item info; only valid if leaf

    std::unordered_map <const PEFile::PEResourceItem*, item_allocInfo*> children;
};

template <typename callbackType>
static AINLINE void ForAllResourceItems( const PEFile::PEResourceDir& resDir, item_allocInfo& allocItem, const callbackType& cb )
{
    // Named children.
    resDir.ForAllChildren(
        [&]( const PEFile::PEResourceItem *childItem, bool hasIdentiferName )
    {
        const auto& childAllocItemNode = allocItem.children.find( childItem );

        assert( childAllocItemNode != allocItem.children.end() );

        item_allocInfo& childAllocItem = *childAllocItemNode->second;

        // Execute for us.
        cb( childItem, childAllocItem );

        if ( childItem->itemType == PEFile::PEResourceItem::eType::DIRECTORY )
        {
            const PEFile::PEResourceDir *childItemDir = (const PEFile::PEResourceDir*)childItem;

            // Now for all children.
            ForAllResourceItems( *childItemDir, childAllocItem, cb );
        }
    });
}

};

// PHASE #1.
void PEFile::PEFileSpaceData::ResolveDataPhaseAllocation( std::uint32_t& rvaOut, std::uint32_t& sizeOut ) const
{
    // If we are section-based data, we can easily register ourselves here.
    // Otherwise we have to wait until all data has been written into stream.
    eStorageType storageType = this->storageType;

    if ( storageType == eStorageType::SECTION )
    {
        // We can write most of it here that matters.
        sizeOut = (std::uint32_t)this->sectRef.GetDataSize();
        // One special optimization here is that data cannot exist on temporary sections here.
        // So we can resolve the offset right away!
        rvaOut = this->sectRef.ResolveOffset( 0 );
    }
    else if ( storageType == eStorageType::FILE )
    {
        // Wait until later.
        sizeOut = (std::uint32_t)this->fileRef.size();
        rvaOut = 0;   // will stay zero.
    }
    else
    {
        // This could be either none or unknown.
        sizeOut = 0;
        rvaOut = 0;
    }
}

// PHASE #2.
std::uint32_t PEFile::PEFileSpaceData::ResolveFinalizationPhase( PEStream *peStream, FileSpaceAllocMan& allocMan, const sect_allocMap_t& sectFileAlloc ) const
{
    std::uint32_t fileDataOff = 0;

    eStorageType storageType = this->storageType;

    if ( storageType == eStorageType::SECTION )
    {
        // Calculate the file pointer from the allocated offsets for the sections.
        PESection *dataSect = this->sectRef.GetSection();

        const auto& fileSectAllocNode = sectFileAlloc.find( dataSect->GetVirtualAddress() );

        assert( fileSectAllocNode != sectFileAlloc.end() );

        const sect_allocInfo& fileSectAllocInfo = fileSectAllocNode->second;

        fileDataOff = ( fileSectAllocInfo.alloc_off + this->sectRef.ResolveInternalOffset( 0 ) );
    }
    else if ( storageType == eStorageType::FILE )
    {
        // For this we need to allocate new space on the executable.
        std::uint32_t dataSize = (std::uint32_t)this->fileRef.size();

        fileDataOff = allocMan.AllocateAny( dataSize, 1 );

        // Also write it.
        PEWrite( peStream, fileDataOff, dataSize, this->fileRef.data() );
    }

    return fileDataOff;
}

bool PEFile::PEFileSpaceData::NeedsFinalizationPhase( void ) const
{
    eStorageType storageType = this->storageType;

    // We need to establish a file pointer for data that is present.
    if ( storageType == eStorageType::SECTION ||
         storageType == eStorageType::FILE )
    {
        return true;
    }

    // We do not need a file pointer.
    return false;
}

void PEFile::PEImportDesc::AllocatePEImportFunctionsData( PESection& writeSect, functions_t& functionList )
{
    size_t numFuncs = functionList.size();

    for ( size_t n = 0; n < numFuncs; n++ )
    {
        PEImportDesc::importFunc& funcInfo = functionList[ n ];

        if ( funcInfo.isOrdinalImport == false )
        {
            // Check if we have our identificator entry allocated yet.
            if ( funcInfo.nameAllocEntry.IsAllocated() == false )
            {
                // Dynamic size of the name entry, since it contains optional ordinal hint.
                std::uint32_t funcNameWriteCount = (std::uint32_t)( funcInfo.name.size() + 1 );
                std::uint32_t nameEntrySize = ( sizeof(std::uint16_t) + funcNameWriteCount );

                // Decide if we have to write a trailing zero byte, as required by the documentation.
                // It is required if this entry size is not a multiple of sizeof(WORD).
                bool requiresTrailZeroByte = false;

                if ( ( nameEntrySize % sizeof(std::uint16_t) ) != 0 )
                {
                    requiresTrailZeroByte = true;

                    nameEntrySize++;
                }

                PESectionAllocation nameAllocEntry;
                writeSect.Allocate( nameAllocEntry, nameEntrySize, sizeof(std::uint16_t) );

                // Ordinal hint.
                nameAllocEntry.WriteToSection( &funcInfo.ordinal_hint, sizeof(funcInfo.ordinal_hint), 0 );

                // Actual name.
                nameAllocEntry.WriteToSection( funcInfo.name.c_str(), funcNameWriteCount, sizeof(std::uint16_t) );

                if ( requiresTrailZeroByte )
                {
                    nameAllocEntry.WriteUInt8( 0, sizeof(std::uint16_t) + funcNameWriteCount );
                }

                funcInfo.nameAllocEntry = std::move( nameAllocEntry );
            }
        }
    }

    // Done writing all sub-data.
}

PEFile::PESectionAllocation PEFile::PEImportDesc::WritePEImportFunctions( PESection& writeSect, const functions_t& functionList, bool isExtendedFormat )
{
    // The size of an entry depends on PE32 or PE32+.
    std::uint32_t entrySize = GetPEPointerSize( isExtendedFormat );

    std::uint32_t numFuncs = (std::uint32_t)functionList.size();

    // We need to end of the array with a zero-entry to describe the end.
    std::uint32_t actualArrayItemCount = ( numFuncs + 1 );

    PESectionAllocation impNameAllocArrayEntry;
    writeSect.Allocate( impNameAllocArrayEntry, actualArrayItemCount * entrySize, entrySize );

    for ( std::uint32_t n = 0; n < numFuncs; n++ )
    {
        const PEImportDesc::importFunc& funcInfo = functionList[ n ];

        std::uint64_t entry = 0;
        std::uint32_t entryWriteOffset = ( entrySize * n );

        if ( funcInfo.isOrdinalImport )
        {
            entry |= funcInfo.ordinal_hint;

            if ( isExtendedFormat )
            {
                entry |= PEL_IMAGE_ORDINAL_FLAG64;
            }
            else
            {
                entry |= PEL_IMAGE_ORDINAL_FLAG32;
            }
        }
        else
        {
            // Because the PE format does not set the flag when it writes a RVA, we
            // can use our delayed RVA writer routine without modifications.
            impNameAllocArrayEntry.RegisterTargetRVA( entryWriteOffset, funcInfo.nameAllocEntry );
        }

        // Write the item.
        if ( isExtendedFormat )
        {
            impNameAllocArrayEntry.WriteUInt64( entry, entryWriteOffset );
        }
        else
        {
            impNameAllocArrayEntry.WriteUInt32( (std::uint32_t)entry, entryWriteOffset );
        }
    }

    // Finish it off with a zero.
    {
        if ( isExtendedFormat )
        {
            impNameAllocArrayEntry.WriteUInt64( 0, entrySize * numFuncs );
        }
        else
        {
            impNameAllocArrayEntry.WriteUInt32( 0, entrySize * numFuncs );
        }
    }

    return impNameAllocArrayEntry;
}

void PEFile::CommitDataDirectories( void )
{
    bool isExtendedFormat = this->isExtendedFormat;

    // We might need the image base for some operations.
    std::uint64_t imageBase = this->peOptHeader.imageBase;

    // TODO: ensure that data has been properly committed to data sections which had to be.
    // First allocate a new section that should serve as allocation target.
    {
        PESection rdonlySect;
        rdonlySect.shortName = ".rd_pef";

        PESection dataSect;
        dataSect.shortName = ".d_pef";

        // We need to perform allocations onto directory structures for all meta-data.
        {
            // We first have to allocate everything.

            // * EXPORT DIRECTORY.
            PEExportDir& expDir = this->exportDir;

            if ( expDir.chars != 0 || expDir.name.empty() == false || expDir.functions.empty() == false )
            {
                // Commit all export directory name entries.

                // Determine if we need to allocate a function name mapping.
                std::uint32_t numNamedEntries = (std::uint32_t)expDir.funcNameMap.size();

                for ( auto& nameMapIter : expDir.funcNameMap )
                {
                    const PEExportDir::mappedName& nameMap = nameMapIter.first;

                    // Make sure we wrote the name into PE virtual memory.
                    if ( !nameMap.nameAllocEntry.IsAllocated() )
                    {
                        // Allocate an entry for the name.
                        const std::uint32_t strSize = (std::uint32_t)( nameMap.name.size() + 1 );
                    
                        PESectionAllocation nameAllocEntry;
                        rdonlySect.Allocate( nameAllocEntry, strSize, 1 );

                        nameAllocEntry.WriteToSection( nameMap.name.c_str(), strSize );

                        // Remember the completed data.
                        nameMap.nameAllocEntry = std::move( nameAllocEntry );
                    }
                }

                const std::uint32_t numExportEntries = (std::uint32_t)expDir.functions.size();

                // Commit the export module name.
                if ( !expDir.nameAllocEntry.IsAllocated() )
                {
                    const std::uint32_t moduleNameSize = (std::uint32_t)( expDir.name.size() + 1 );

                    PESectionAllocation moduleNameAlloc;
                    rdonlySect.Allocate( moduleNameAlloc, moduleNameSize, 1 );

                    // Write module name.
                    moduleNameAlloc.WriteToSection( expDir.name.c_str(), moduleNameSize );

                    expDir.nameAllocEntry = std::move( moduleNameAlloc );
                }

                // Do we need a new export directory allocation?
                if ( expDir.funcAddressAllocEntry.IsAllocated() == false ||
                     expDir.funcNamesAllocEntry.IsAllocated() == false ||
                     expDir.funcOrdinalsAllocEntry.IsAllocated() == false ||
                     expDir.allocEntry.IsAllocated() == false )
                {
                    // Allocate each directory with its own allocator.
                    struct expfunc_allocInfo
                    {
                        std::uint32_t forwarder_off;
                    };
            
                    std::unordered_map <size_t, expfunc_allocInfo> allocInfos;

                    // Allocate forwarder RVAs.
                    PESectionAllocation expDirAlloc;
                    {
                        FileSpaceAllocMan expAllocMan;

                        expAllocMan.AllocateAt( 0, sizeof( PEStructures::IMAGE_EXPORT_DIRECTORY ) );

                        for ( size_t n = 0; n < numExportEntries; n++ )
                        {
                            const PEExportDir::func& funcEntry = expDir.functions[ n ];

                            if ( funcEntry.isForwarder )
                            {
                                // Allocate an entry for the forwarder.
                                const std::uint32_t strSize = (std::uint32_t)( funcEntry.forwarder.size() + 1 );

                                std::uint32_t forwOffset = expAllocMan.AllocateAny( strSize, 1 );

                                expfunc_allocInfo& info = allocInfos[ n ];
                                info.forwarder_off = forwOffset;
                            }
                        }

                        // Since all entries inside the alloc directory are indeed allocated,
                        // we can create the allocation in the section!
                        rdonlySect.Allocate( expDirAlloc, expAllocMan.GetSpanSize( 1 ), sizeof(std::uint32_t) );
                    }

                    // Now allocate the necessary arrays for export data.
                    // Data offset, optional name ptr and ordinal maps.
                    const std::uint32_t dataOffTableSize = ( sizeof(std::uint32_t) * numExportEntries );

                    PESectionAllocation dataTabOffAlloc;
                    rdonlySect.Allocate( dataTabOffAlloc, dataOffTableSize );

                    PESectionAllocation namePtrTableAlloc;
                    PESectionAllocation ordMapTableAlloc;

                    if ( numNamedEntries != 0 )
                    {
                        const std::uint32_t namePtrTableSize = ( sizeof(std::uint32_t) * numNamedEntries );

                        rdonlySect.Allocate( namePtrTableAlloc, namePtrTableSize );

                        const std::uint32_t ordMapTableSize = ( sizeof(std::uint32_t) * numNamedEntries );

                        rdonlySect.Allocate( ordMapTableAlloc, ordMapTableSize, sizeof(std::uint16_t) );
                    }

                    // At this point the entire export directory data is allocated.
                    // Let's write it!
                    PEStructures::IMAGE_EXPORT_DIRECTORY header;
                    header.Characteristics = expDir.chars;
                    header.TimeDateStamp = expDir.timeDateStamp;
                    header.MajorVersion = expDir.majorVersion;
                    header.MinorVersion = expDir.minorVersion;
                    header.Name = 0;
                    rdonlySect.RegisterTargetRVA( expDirAlloc.ResolveInternalOffset( offsetof(PEStructures::IMAGE_EXPORT_DIRECTORY, Name) ), expDir.nameAllocEntry );
                    header.Base = expDir.ordinalBase;
                    header.NumberOfFunctions = (std::uint32_t)numExportEntries;
                    header.NumberOfNames = (std::uint32_t)numNamedEntries;
                    header.AddressOfFunctions = 0;
                    expDirAlloc.RegisterTargetRVA( offsetof(PEStructures::IMAGE_EXPORT_DIRECTORY, AddressOfFunctions), dataTabOffAlloc );
                    header.AddressOfNames = 0;
                    expDirAlloc.RegisterTargetRVA( offsetof(PEStructures::IMAGE_EXPORT_DIRECTORY, AddressOfNames), namePtrTableAlloc );
                    header.AddressOfNameOrdinals = 0;
                    expDirAlloc.RegisterTargetRVA( offsetof(PEStructures::IMAGE_EXPORT_DIRECTORY, AddressOfNameOrdinals), ordMapTableAlloc );

                    expDirAlloc.WriteToSection( &header, sizeof(header), 0 );
            
                    // Write export offsets.
                    for ( std::uint32_t n = 0; n < numExportEntries; n++ )
                    {
                        const PEExportDir::func& funcInfo = expDir.functions[ n ];

                        // First shedule the offset for writing.
                        const std::uint32_t dataTabItemOff = ( sizeof(std::uint32_t) * n );

                        const expfunc_allocInfo *finfo = FindMapValue( allocInfos, n );

                        if ( funcInfo.isForwarder )
                        {
                            assert( finfo != NULL );

                            dataTabOffAlloc.RegisterTargetRVA( dataTabItemOff, expDirAlloc, finfo->forwarder_off );
                        }
                        else
                        {
                            dataTabOffAlloc.RegisterTargetRVA( dataTabItemOff, funcInfo.expRef );
                        }
                    }

                    // Maybe write a name ordinal map.
                    if ( numNamedEntries != 0 )
                    {
                        assert( namePtrTableAlloc.IsAllocated() == true && ordMapTableAlloc.IsAllocated() == true );

                        std::uint32_t index = 0;

                        for ( const auto& nameMapIter : expDir.funcNameMap )
                        {
                            size_t funcIndex = nameMapIter.second;

                            // Write the name.
                            std::uint16_t ordinal = (std::uint16_t)funcIndex;

                            // Write this name map entry.
                            const PEExportDir::mappedName& nameMap = nameMapIter.first;

                            const std::uint32_t namePtrOff = ( sizeof(std::uint32_t) * index );
                            const std::uint32_t ordOff = ( sizeof(std::uint16_t) * index );

                            namePtrTableAlloc.RegisterTargetRVA( namePtrOff, nameMap.nameAllocEntry );
                            ordMapTableAlloc.WriteToSection( &ordinal, sizeof(ordinal), ordOff );

                            index++;
                        }
                    }

                    // After write-phase we can remember the new offsets.
                    expDir.funcAddressAllocEntry = std::move( dataTabOffAlloc );
                    expDir.funcNamesAllocEntry = std::move( namePtrTableAlloc );
                    expDir.funcOrdinalsAllocEntry = std::move( ordMapTableAlloc );

                    // Last but not least, our export directory pointer.
                    expDir.allocEntry = std::move( expDirAlloc );
                }
            }

            // * IMPORT DIRECTORY.
            auto& importDescs = this->imports;

            std::uint32_t numImportDescriptors = (std::uint32_t)importDescs.size();

            if ( numImportDescriptors > 0 )
            {
                // Commit all sub-data first.
                for ( std::uint32_t n = 0; n < numImportDescriptors; n++ )
                {
                    PEImportDesc& impDesc = importDescs[ n ];

                    // Each descriptor has a list of import IDs, which is an array of
                    // either ordinal or name entries.
                    auto& funcs = impDesc.funcs;

                    if ( funcs.empty() == false )
                    {
                        // First the sub-data.
                        impDesc.AllocatePEImportFunctionsData( rdonlySect, funcs );

                        if ( impDesc.impNameArrayAllocEntry.IsAllocated() == false )
                        {
                            impDesc.impNameArrayAllocEntry =
                                PEImportDesc::WritePEImportFunctions(
                                    rdonlySect, funcs, isExtendedFormat
                                );
                        }
                    }

                    // Allocate and write the module name that we should import from.
                    if ( impDesc.DLLName_allocEntry.IsAllocated() == false )
                    {
                        impDesc.DLLName_allocEntry = WriteZeroTermString( rdonlySect, impDesc.DLLName );
                    }
                }
                
                // Do we need a new import descriptors array?
                if ( this->importsAllocEntry.IsAllocated() == false )
                {
                    // Remember that we write a finishing NULL descriptor.
                    const std::uint32_t writeNumImportDescs = (std::uint32_t)( numImportDescriptors + 1 );

                    // The import descriptor directory consists of a single array of descriptors.
                    PESectionAllocation impDescsAlloc;
                    rdonlySect.Allocate( impDescsAlloc, sizeof(PEStructures::IMAGE_IMPORT_DESCRIPTOR) * writeNumImportDescs, sizeof(std::uint32_t) );

                    for ( std::uint32_t n = 0; n < numImportDescriptors; n++ )
                    {
                        const PEImportDesc& impDesc = importDescs[ n ];

                        // Since all data is allocated now let us write the descriptor.
                        const std::uint32_t descWriteOffset = ( sizeof(PEStructures::IMAGE_IMPORT_DESCRIPTOR) * n );

                        PEStructures::IMAGE_IMPORT_DESCRIPTOR nativeImpDesc;
                        nativeImpDesc.Characteristics = 0;
                        impDescsAlloc.RegisterTargetRVA( descWriteOffset + offsetof(PEStructures::IMAGE_IMPORT_DESCRIPTOR, Characteristics), impDesc.impNameArrayAllocEntry );
                        nativeImpDesc.TimeDateStamp = 0;
                        nativeImpDesc.ForwarderChain = 0;
                        nativeImpDesc.Name = 0;
                        impDescsAlloc.RegisterTargetRVA( descWriteOffset + offsetof(PEStructures::IMAGE_IMPORT_DESCRIPTOR, Name), impDesc.DLLName_allocEntry );
                        nativeImpDesc.FirstThunk = impDesc.firstThunkRef.GetRVA();

                        impDescsAlloc.WriteToSection( &nativeImpDesc, sizeof(nativeImpDesc), descWriteOffset );
                    }

                    // Write the terminating NULL descriptor.
                    {
                        const std::uint32_t nullDescOff = ( numImportDescriptors * sizeof(PEStructures::IMAGE_IMPORT_DESCRIPTOR) );

                        PEStructures::IMAGE_IMPORT_DESCRIPTOR nullImpDesc = { 0 };

                        impDescsAlloc.WriteToSection( &nullImpDesc, sizeof(nullImpDesc), nullDescOff );
                    }

                    // We have written all import descriptors, so remember this allocation.
                    this->importsAllocEntry = std::move( impDescsAlloc );
                }
            }

            // * Resources.
            {
                // Do we need a new resource data segment?
                if ( this->resAllocEntry.IsAllocated() == false )
                {
                    PEResourceDir& resRootDir = this->resourceRoot;

                    FileSpaceAllocMan resDataAlloc;

                    using namespace ResourceTools;

                    struct auxil
                    {
                        static item_allocInfo AllocateResourceDirectory_dirData( FileSpaceAllocMan& allocMan, const PEResourceItem *item )
                        {
                            item_allocInfo infoOut;

                            // We allocate a structure depending on the type.
                            PEResourceItem::eType itemType = item->itemType;

                            if ( itemType == PEResourceItem::eType::DIRECTORY )
                            {
                                const PEResourceDir *itemDir = (const PEResourceDir*)item;

                                // This is the directory entry...
                                std::uint32_t itemSize = sizeof(PEStructures::IMAGE_RESOURCE_DIRECTORY);

                                // and the items following it.
                                size_t numNamedChildren = itemDir->namedChildren.size();
                                size_t numIDChildren = itemDir->idChildren.size();

                                std::uint32_t numChildren =
                                    (std::uint32_t)numNamedChildren +
                                    (std::uint32_t)numIDChildren;

                                itemSize += numChildren * sizeof(PEStructures::IMAGE_RESOURCE_DIRECTORY_ENTRY);

                                infoOut.entry_off = allocMan.AllocateAny( itemSize, sizeof(std::uint32_t) );

                                // Now allocate all children aswell.
                                // First allocate the named entries.
                                for ( const PEFile::PEResourceItem *childItem : itemDir->namedChildren )
                                {
                                    assert ( childItem->hasIdentifierName == false );

                                    item_allocInfo childAlloc = AllocateResourceDirectory_dirData( allocMan, childItem );

                                    // We allocate name strings later.
                                    childAlloc.name_off = 0;

                                    // Create a memory copy of it.
                                    item_allocInfo *memChildAlloc = new item_allocInfo( std::move( childAlloc ) );

                                    assert( memChildAlloc != NULL );

                                    try
                                    {
                                        // Register this child alloc item.
                                        infoOut.children.insert( std::make_pair( childItem, memChildAlloc ) );
                                    }
                                    catch( ... )
                                    {
                                        delete memChildAlloc;

                                        throw;
                                    }
                                }

                                // Now allocate all ID based entries.
                                for ( const PEFile::PEResourceItem *childItem : itemDir->idChildren )
                                {
                                    assert( childItem->hasIdentifierName == true );

                                    item_allocInfo childAlloc = AllocateResourceDirectory_dirData( allocMan, childItem );

                                    // Not named.
                                    childAlloc.name_off = 0;

                                    // Create a memory item.
                                    item_allocInfo *memChildAlloc = new item_allocInfo( std::move( childAlloc ) );

                                    assert( memChildAlloc != NULL );

                                    try
                                    {
                                        infoOut.children.insert( std::make_pair( childItem, memChildAlloc ) );
                                    }
                                    catch( ... )
                                    {
                                        delete memChildAlloc;

                                        throw;
                                    }
                                }
                            }
                            else if ( itemType == PEResourceItem::eType::DATA )
                            {
                                // We process data items later.
                                infoOut.entry_off = 0;
                            }

                            return infoOut;
                        }

                        static void AllocateResourceDirectory_nameStrings( FileSpaceAllocMan& allocMan, const PEResourceDir& rootDir, item_allocInfo& allocItem )
                        {
                            ForAllResourceItems( rootDir, allocItem,
                                [&]( const PEResourceItem *childItem, item_allocInfo& childAllocItem )
                            {
                                // Any name string to allocate?
                                if ( childItem->hasIdentifierName == false )
                                {
                                    const std::uint32_t nameItemCount = (std::uint32_t)( childItem->name.size() );

                                    std::uint32_t nameDataSize = ( nameItemCount * sizeof(char16_t) );

                                    // Add the size of the header.
                                    nameDataSize += sizeof(std::uint16_t);

                                    childAllocItem.name_off = allocMan.AllocateAny( nameDataSize, sizeof(char16_t) );
                                }
                            });

                            // Processed this node.
                        }

                        static void AllocateResourceDirectory_dataItems( FileSpaceAllocMan& allocMan, const PEResourceDir& rootDir, item_allocInfo& allocItem )
                        {
                            ForAllResourceItems( rootDir, allocItem,
                                [&]( const PEResourceItem *childItem, item_allocInfo& childAllocItem )
                            {
                                if ( childItem->itemType == PEResourceItem::eType::DATA )
                                {
                                    // Single item allocation.
                                    const size_t itemSize = sizeof(PEStructures::IMAGE_RESOURCE_DATA_ENTRY);

                                    childAllocItem.entry_off = allocMan.AllocateAny( itemSize, sizeof(std::uint32_t) );
                                }
                            });
                        }

                        static void AllocateResourceDirectory_dataFiles( FileSpaceAllocMan& allocMan, const PEResourceDir& rootDir, item_allocInfo& allocItem )
                        {
                            ForAllResourceItems( rootDir, allocItem,
                                [&]( const PEResourceItem *childItem, item_allocInfo& childAllocItem )
                            {
                                if ( childItem->itemType == PEResourceItem::eType::DATA )
                                {
                                    // TODO: make sure to update this once we support resource data injection!

                                    const PEResourceInfo *childInfoItem = (const PEResourceInfo*)childItem;

                                    // Allocate space inside of our resource section.
                                    const std::uint32_t resFileSize = childInfoItem->sectRef.GetDataSize();

                                    childAllocItem.dataitem_off = allocMan.AllocateAny( resFileSize, 1 );
                                }
                            });
                        }

                        static void WriteResourceDataItem( const PEResourceInfo *dataItem, const item_allocInfo& dataAllocInfo, PESectionAllocation& writeBuf )
                        {
                            // TODO: once we support injecting data buffers into the resource directory,
                            // we will have to extend this with memory stream reading support.

                            std::uint32_t fileWriteOff = dataAllocInfo.dataitem_off;

                            assert( fileWriteOff != 0 );    // invalid because already taken by root directory info.

                            PEDataStream fileSrcStream = PEDataStream::fromDataRef( dataItem->sectRef );

                            // Write data over.
                            const std::uint32_t fileDataSize = dataItem->sectRef.GetDataSize();
                            {
                                char buffer[ 0x4000 ];

                                std::uint32_t curDataOff = 0;
                                    
                                while ( curDataOff < fileDataSize )
                                {
                                    std::uint32_t actualProcCount = std::min( fileDataSize - curDataOff, (std::uint32_t)sizeof(buffer) );

                                    fileSrcStream.Read( buffer, actualProcCount );

                                    writeBuf.WriteToSection( buffer, actualProcCount, fileWriteOff + curDataOff );

                                    curDataOff += sizeof(buffer);
                                }
                            }

                            std::uint32_t dataEntryOff = dataAllocInfo.entry_off;

                            PEStructures::IMAGE_RESOURCE_DATA_ENTRY nativeDataEntry;
                            // We need to write the RVA later.
                            nativeDataEntry.OffsetToData = 0;
                            writeBuf.RegisterTargetRVA( dataEntryOff + offsetof(PEStructures::IMAGE_RESOURCE_DATA_ENTRY, OffsetToData), writeBuf.GetSection(), writeBuf.ResolveInternalOffset( fileWriteOff ) );
                            nativeDataEntry.Size = fileDataSize;
                            nativeDataEntry.CodePage = dataItem->codePage;
                            nativeDataEntry.Reserved = dataItem->reserved;

                            assert( dataAllocInfo.entry_off != 0 );    // invalid because zero is already taken by root directory.

                            writeBuf.WriteToSection( &nativeDataEntry, sizeof(nativeDataEntry), dataAllocInfo.entry_off );
                        }

                        static void WriteResourceDirectory( const PEResourceDir& writeNode, const item_allocInfo& allocNode, PESectionAllocation& writeBuf )
                        {
                            PEStructures::IMAGE_RESOURCE_DIRECTORY nativeResDir;
                            nativeResDir.Characteristics = writeNode.characteristics;
                            nativeResDir.TimeDateStamp = writeNode.timeDateStamp;
                            nativeResDir.MajorVersion = writeNode.majorVersion;
                            nativeResDir.MinorVersion = writeNode.minorVersion;
                        
                            // Count how many named and how many children we have.
                            size_t numNamedEntries = writeNode.namedChildren.size();
                            size_t numIDEntries = writeNode.idChildren.size();

                            nativeResDir.NumberOfNamedEntries = (std::uint16_t)numNamedEntries;
                            nativeResDir.NumberOfIdEntries = (std::uint16_t)numIDEntries;

                            const std::uint32_t dirWriteOff = allocNode.entry_off;

                            writeBuf.WriteToSection( &nativeResDir, sizeof(nativeResDir), allocNode.entry_off );

                            // Now write all children.
                            const std::uint32_t linkWriteOff = ( dirWriteOff + sizeof(nativeResDir) );

                            // First all named ones.
                            size_t writeIndex = 0;

                            for ( const PEResourceItem *childItem : writeNode.namedChildren )
                            {
                                const auto& childAllocInfoNode = allocNode.children.find( childItem );

                                assert( childAllocInfoNode != allocNode.children.end() );

                                const item_allocInfo& childAllocInfo = *childAllocInfoNode->second;

                                // We write a link entry for this child.
                                PEStructures::IMAGE_RESOURCE_DIRECTORY_ENTRY lnkEntry = { 0 };

                                // Write and register ID information, be it name or number.
                                {
                                    lnkEntry.NameIsString = true;

                                    std::uint32_t nameWriteOff = childAllocInfo.name_off;

                                    assert( nameWriteOff != 0 );    // invalid because zero is already root directory info offset.

                                    // First store the amount of characters.
                                    const std::uint16_t numWriteItems = (std::uint16_t)childItem->name.size();

                                    writeBuf.WriteToSection( &numWriteItems, sizeof(numWriteItems), nameWriteOff );

                                    // Write the name correctly.
                                    writeBuf.WriteToSection( childItem->name.c_str(), numWriteItems * sizeof(char16_t), nameWriteOff + sizeof(std::uint16_t) );

                                    // Give the offset.
                                    lnkEntry.NameOffset = childAllocInfo.name_off;
                                }

                                PEResourceItem::eType itemType = childItem->itemType;

                                // Give information about the child we are going to write.
                                lnkEntry.DataIsDirectory = ( itemType == PEResourceItem::eType::DIRECTORY );
                                lnkEntry.OffsetToDirectory = ( childAllocInfo.entry_off );

                                const std::uint32_t lnkEntryOff = (std::uint32_t)( linkWriteOff + writeIndex * sizeof(lnkEntry) );

                                writeBuf.WriteToSection( &lnkEntry, sizeof(lnkEntry), lnkEntryOff );

                                // Advance the write index.
                                writeIndex++;

                                if ( itemType == PEResourceItem::eType::DIRECTORY )
                                {
                                    const PEResourceDir *childDir = (const PEResourceDir*)childItem;

                                    // Just recurse to write more data.
                                    WriteResourceDirectory( *childDir, childAllocInfo, writeBuf );
                                }
                                else if ( itemType == PEResourceItem::eType::DATA )
                                {
                                    const PEResourceInfo *childData = (const PEResourceInfo*)childItem;

                                    WriteResourceDataItem( childData, childAllocInfo, writeBuf );
                                }
                                else
                                {
                                    assert( 0 );
                                }
                            }

                            // Now all ID ones.
                            for ( const PEResourceItem *childItem : writeNode.idChildren )
                            {
                                const auto& childAllocInfoNode = allocNode.children.find( childItem );

                                assert( childAllocInfoNode != allocNode.children.end() );

                                const item_allocInfo& childAllocInfo = *childAllocInfoNode->second;

                                // We write a link entry for this child.
                                PEStructures::IMAGE_RESOURCE_DIRECTORY_ENTRY lnkEntry = { 0 };

                                // Write and register ID information, be it name or number.
                                {
                                    lnkEntry.NameIsString = false;

                                    // Just write the ID.
                                    lnkEntry.Id = childItem->identifier;
                                }

                                PEResourceItem::eType itemType = childItem->itemType;

                                // Give information about the child we are going to write.
                                lnkEntry.DataIsDirectory = ( itemType == PEResourceItem::eType::DIRECTORY );
                                lnkEntry.OffsetToDirectory = ( childAllocInfo.entry_off );

                                const std::uint32_t lnkEntryOff = (std::uint32_t)( linkWriteOff + writeIndex * sizeof(lnkEntry) );

                                writeBuf.WriteToSection( &lnkEntry, sizeof(lnkEntry), lnkEntryOff );

                                // Advance the write index.
                                writeIndex++;

                                if ( itemType == PEResourceItem::eType::DIRECTORY )
                                {
                                    const PEResourceDir *childDir = (const PEResourceDir*)childItem;

                                    // Just recurse to write more data.
                                    WriteResourceDirectory( *childDir, childAllocInfo, writeBuf );
                                }
                                else if ( itemType == PEResourceItem::eType::DATA )
                                {
                                    const PEResourceInfo *childData = (const PEResourceInfo*)childItem;

                                    WriteResourceDataItem( childData, childAllocInfo, writeBuf );
                                }
                                else
                                {
                                    assert( 0 );
                                }
                            }

                            // Finished writing all children.
                        }
                    };

                    // First allocate all directory entries.
                    item_allocInfo allocInfo = auxil::AllocateResourceDirectory_dirData( resDataAlloc, &resRootDir );

                    // Then come the name strings.
                    auxil::AllocateResourceDirectory_nameStrings( resDataAlloc, resRootDir, allocInfo );

                    // And last but not least the data entries.
                    auxil::AllocateResourceDirectory_dataItems( resDataAlloc, resRootDir, allocInfo );

                    // Resource files must be allocated in the resource section, by documentation.
                    auxil::AllocateResourceDirectory_dataFiles( resDataAlloc, resRootDir, allocInfo );

                    assert( allocInfo.entry_off == 0 );
                    allocInfo.name_off = 0;     // the root directory has no name.

                    // Get a main allocation spot inside of the section now that allocation has finished.
                    PESectionAllocation resDirEntry;
                    rdonlySect.Allocate( resDirEntry, resDataAlloc.GetSpanSize( 1 ), sizeof(std::uint32_t) );

                    // Write the data into the executable memory now.
                    auxil::WriteResourceDirectory( resRootDir, allocInfo, resDirEntry );

                    // Remember the allocation now.
                    this->resAllocEntry = std::move( resDirEntry );
                }
            }

            // * Exception Information.
            const auto& exceptRFs = this->exceptRFs;

            std::uint32_t numExceptEntries = (std::uint32_t)exceptRFs.size();

            if ( numExceptEntries != 0 )
            {
                // TODO: remember that exception data is machine dependent.
                // revisit this if we need multi-architecture support.
                // (currently we specialize on x86/AMD64)

                if ( this->exceptAllocEntry.IsAllocated() == false )
                {
                    const std::uint32_t exceptTableSize = ( sizeof(PEStructures::IMAGE_RUNTIME_FUNCTION_ENTRY_X64) * numExceptEntries );

                    PESectionAllocation exceptTableAlloc;
                    rdonlySect.Allocate( exceptTableAlloc, exceptTableSize, sizeof(std::uint32_t) );

                    // Now write all entries.
                    // TODO: documentation says that these entries should be address sorted.
                    for ( std::uint32_t n = 0; n < numExceptEntries; n++ )
                    {
                        const PERuntimeFunction& rfEntry = this->exceptRFs[ n ];

                        PEStructures::IMAGE_RUNTIME_FUNCTION_ENTRY_X64 funcInfo;
                        funcInfo.BeginAddress = rfEntry.beginAddrRef.GetRVA();
                        funcInfo.EndAddress = rfEntry.endAddrRef.GetRVA();
                        funcInfo.UnwindInfoAddress = rfEntry.unwindInfoRef.GetRVA();

                        const std::uint32_t rfEntryOff = ( n * sizeof(PEStructures::IMAGE_RUNTIME_FUNCTION_ENTRY_X64) );

                        exceptTableAlloc.WriteToSection( &funcInfo, sizeof(funcInfo), rfEntryOff );
                    }

                    // Remember this valid exception table.
                    this->exceptAllocEntry = std::move( exceptTableAlloc );
                }
            }

            // nothing to allocate for security cookie.

            // *** BASE RELOC has to be written last because commit operations can spawn relocations.

            // * DEBUG.
            // NOTE: debug data is a 'special citizen' of the PE file standard.
            //       this means that it can write outside of the allocated section area.
            //       phase two of the debug write phase is after sections are written down.
            const auto& debugDescs = this->debugDescs;

            std::uint32_t numDebugDescs = (std::uint32_t)debugDescs.size();
            
            if ( numDebugDescs > 0 )
            {
                // Do we need a new debug descriptors array?
                if ( this->debugDescsAlloc.IsAllocated() == false )
                {
                    std::uint32_t peNumDebugDescs = (std::uint32_t)numDebugDescs;

                    // The entire debug directory is an array of debug descriptors.
                    std::uint32_t debugArraySize = ( sizeof(PEStructures::IMAGE_DEBUG_DIRECTORY) * peNumDebugDescs );

                    PESectionAllocation debugDescsAlloc;
                    rdonlySect.Allocate( debugDescsAlloc, debugArraySize, sizeof(std::uint32_t) );

                    for ( std::uint32_t n = 0; n < numDebugDescs; n++ )
                    {
                        const PEDebugDesc& debugEntry = debugDescs[ n ];

                        PEStructures::IMAGE_DEBUG_DIRECTORY nativeDebug;
                        nativeDebug.Characteristics = debugEntry.characteristics;
                        nativeDebug.TimeDateStamp = debugEntry.timeDateStamp;
                        nativeDebug.MajorVersion = debugEntry.majorVer;
                        nativeDebug.MinorVersion = debugEntry.minorVer;
                        nativeDebug.Type = debugEntry.type;

                        // Resolve the data reference (PHASE #1).
                        {
                            std::uint32_t rva, dataSize;

                            debugEntry.dataStore.ResolveDataPhaseAllocation( rva, dataSize );

                            nativeDebug.AddressOfRawData = rva;
                            nativeDebug.SizeOfData = dataSize;
                        }

                        // We probably write this later.
                        nativeDebug.PointerToRawData = 0;

                        // Let's write ourselves for now!
                        debugDescsAlloc.WriteToSection( &nativeDebug, sizeof(nativeDebug), n * sizeof(PEStructures::IMAGE_DEBUG_DIRECTORY) );
                    }

                    // Done writing all debug descriptors.

                    this->debugDescsAlloc = std::move( debugDescsAlloc );
                }
            }

            // architecture does not need writing.

            // global pointer does not need writing.

            // * THREAD LOCAL STORAGE.
            PEThreadLocalStorage& tlsInfo = this->tlsInfo;

            if ( tlsInfo.NeedsWriting() )
            {
                // Do we need a new Thread Local Storage entry?
                if ( tlsInfo.allocEntry.IsAllocated() == false )
                {
                    // We want to support PE32 and PE32+. So determine the size of the TLS.
                    std::uint32_t tlsDirSize;

                    if ( isExtendedFormat )
                    {
                        tlsDirSize = sizeof(PEStructures::IMAGE_TLS_DIRECTORY64);
                    }
                    else
                    {
                        tlsDirSize = sizeof(PEStructures::IMAGE_TLS_DIRECTORY32);
                    }

                    PESectionAllocation tlsDirAlloc;
                    rdonlySect.Allocate( tlsDirAlloc, tlsDirSize, sizeof(std::uint32_t) );
                
                    // Now write the Thread Local Storage.
                    if ( isExtendedFormat )
                    {
                        PEStructures::IMAGE_TLS_DIRECTORY64 nativeTLS;
                        // We write the VAs later when we actually can resolve all.
                        nativeTLS.StartAddressOfRawData = 0;
                        tlsDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeTLS), StartAddressOfRawData), tlsInfo.startOfRawDataRef, 0,
                            PEPlacedOffset::eOffsetType::VA_64BIT
                        );
                        nativeTLS.EndAddressOfRawData = 0;
                        tlsDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeTLS), EndAddressOfRawData), tlsInfo.endOfRawDataRef, 0,
                            PEPlacedOffset::eOffsetType::VA_64BIT
                        );
                        nativeTLS.AddressOfIndex = 0;
                        tlsDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeTLS), AddressOfIndex), tlsInfo.addressOfIndexRef, 0,
                            PEPlacedOffset::eOffsetType::VA_64BIT
                        );
                        nativeTLS.AddressOfCallBacks = 0;
                        tlsDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeTLS), AddressOfCallBacks), tlsInfo.addressOfCallbacksRef, 0,
                            PEPlacedOffset::eOffsetType::VA_64BIT
                        );
                        nativeTLS.SizeOfZeroFill = tlsInfo.sizeOfZeroFill;
                        nativeTLS.Characteristics = tlsInfo.characteristics;

                        tlsDirAlloc.WriteToSection( &nativeTLS, sizeof(nativeTLS) );
                    }
                    else
                    {
                        PEStructures::IMAGE_TLS_DIRECTORY32 nativeTLS;
                        nativeTLS.StartAddressOfRawData = 0;
                        tlsDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeTLS), StartAddressOfRawData), tlsInfo.startOfRawDataRef, 0,
                            PEPlacedOffset::eOffsetType::VA_32BIT
                        );
                        nativeTLS.EndAddressOfRawData = 0;
                        tlsDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeTLS), EndAddressOfRawData), tlsInfo.endOfRawDataRef, 0,
                            PEPlacedOffset::eOffsetType::VA_32BIT
                        );
                        nativeTLS.AddressOfIndex = 0;
                        tlsDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeTLS), AddressOfIndex), tlsInfo.addressOfIndexRef, 0,
                            PEPlacedOffset::eOffsetType::VA_32BIT
                        );
                        nativeTLS.AddressOfCallBacks = 0;
                        tlsDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeTLS), AddressOfCallBacks), tlsInfo.addressOfCallbacksRef, 0,
                            PEPlacedOffset::eOffsetType::VA_32BIT
                        );
                        nativeTLS.SizeOfZeroFill = tlsInfo.sizeOfZeroFill;
                        nativeTLS.Characteristics = tlsInfo.characteristics;

                        tlsDirAlloc.WriteToSection( &nativeTLS, sizeof(nativeTLS) );
                    }

                    // Remember the valid object.
                    tlsInfo.allocEntry = std::move( tlsDirAlloc );
                }
            }

            // * LOAD CONFIG.
            PELoadConfig& loadConfig = this->loadConfig;

            if ( loadConfig.isNeeded )
            {
                // Do we need a new Load Config object?
                if ( loadConfig.allocEntry.IsAllocated() == false )
                {
                    // Detemine the size of the load configuration directory.
                    std::uint32_t lcfgDirSize;

                    if ( isExtendedFormat )
                    {
                        lcfgDirSize = sizeof(PEStructures::IMAGE_LOAD_CONFIG_DIRECTORY64);
                    }
                    else
                    {
                        lcfgDirSize = sizeof(PEStructures::IMAGE_LOAD_CONFIG_DIRECTORY32);
                    }

                    PESectionAllocation lcfgDirAlloc;
                    rdonlySect.Allocate( lcfgDirAlloc, lcfgDirSize, sizeof(std::uint32_t) );

                    // Write the load configuration directory.
                    if ( isExtendedFormat )
                    {
                        PEStructures::IMAGE_LOAD_CONFIG_DIRECTORY64 nativeConfig;
                        nativeConfig.Size = sizeof(nativeConfig);
                        nativeConfig.TimeDateStamp = loadConfig.timeDateStamp;
                        nativeConfig.MajorVersion = loadConfig.majorVersion;
                        nativeConfig.MinorVersion = loadConfig.minorVersion;
                        nativeConfig.GlobalFlagsClear = loadConfig.globFlagsClear;
                        nativeConfig.GlobalFlagsSet = loadConfig.globFlagsSet;
                        nativeConfig.CriticalSectionDefaultTimeout = loadConfig.critSecDefTimeOut;
                        nativeConfig.DeCommitFreeBlockThreshold = loadConfig.deCommitFreeBlockThreshold;
                        nativeConfig.DeCommitTotalFreeThreshold = loadConfig.deCommitTotalFreeThreshold;
                        nativeConfig.LockPrefixTable = 0;
                        lcfgDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeConfig), LockPrefixTable), loadConfig.lockPrefixTableRef, 0,
                            PEPlacedOffset::eOffsetType::VA_64BIT
                        );
                        nativeConfig.MaximumAllocationSize = loadConfig.maxAllocSize;
                        nativeConfig.VirtualMemoryThreshold = loadConfig.virtualMemoryThreshold;
                        nativeConfig.ProcessAffinityMask = loadConfig.processAffinityMask;
                        nativeConfig.ProcessHeapFlags = loadConfig.processHeapFlags;
                        nativeConfig.CSDVersion = loadConfig.CSDVersion;
                        nativeConfig.Reserved1 = loadConfig.reserved1;
                        nativeConfig.EditList = 0;
                        lcfgDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeConfig), EditList), loadConfig.editListRef, 0,
                            PEPlacedOffset::eOffsetType::VA_64BIT
                        );
                        nativeConfig.SecurityCookie = 0;
                        lcfgDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeConfig), SecurityCookie), loadConfig.securityCookieRef, 0,
                            PEPlacedOffset::eOffsetType::VA_64BIT
                        );
                        nativeConfig.SEHandlerTable = 0;
                        lcfgDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeConfig), SEHandlerTable), loadConfig.SEHandlerTableRef, 0,
                            PEPlacedOffset::eOffsetType::VA_64BIT
                        );
                        nativeConfig.SEHandlerCount = loadConfig.SEHandlerCount;
                        nativeConfig.GuardCFCheckFunctionPointer = 0;
                        lcfgDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeConfig), GuardCFCheckFunctionPointer), loadConfig.guardCFCheckFunctionPtrRef, 0,
                            PEPlacedOffset::eOffsetType::VA_64BIT
                        );
                        nativeConfig.Reserved2 = loadConfig.reserved2;
                        nativeConfig.GuardCFFunctionTable = 0;
                        lcfgDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeConfig), GuardCFFunctionTable), loadConfig.guardCFFunctionTableRef, 0,
                            PEPlacedOffset::eOffsetType::VA_64BIT
                        );
                        nativeConfig.GuardCFFunctionCount = loadConfig.guardCFFunctionCount;
                        nativeConfig.GuardFlags = loadConfig.guardFlags;

                        // Write it.
                        lcfgDirAlloc.WriteToSection( &nativeConfig, sizeof(nativeConfig) );
                    }
                    else
                    {
                        PEStructures::IMAGE_LOAD_CONFIG_DIRECTORY32 nativeConfig;
                        nativeConfig.Size = sizeof(nativeConfig);
                        nativeConfig.TimeDateStamp = loadConfig.timeDateStamp;
                        nativeConfig.MajorVersion = loadConfig.majorVersion;
                        nativeConfig.MinorVersion = loadConfig.minorVersion;
                        nativeConfig.GlobalFlagsClear = loadConfig.globFlagsClear;
                        nativeConfig.GlobalFlagsSet = loadConfig.globFlagsSet;
                        nativeConfig.CriticalSectionDefaultTimeout = loadConfig.critSecDefTimeOut;
                        nativeConfig.DeCommitFreeBlockThreshold = (std::uint32_t)loadConfig.deCommitFreeBlockThreshold;
                        nativeConfig.DeCommitTotalFreeThreshold = (std::uint32_t)loadConfig.deCommitTotalFreeThreshold;
                        nativeConfig.LockPrefixTable = 0;
                        lcfgDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeConfig), LockPrefixTable), loadConfig.lockPrefixTableRef, 0,
                            PEPlacedOffset::eOffsetType::VA_32BIT
                        );
                        nativeConfig.MaximumAllocationSize = (std::uint32_t)loadConfig.maxAllocSize;
                        nativeConfig.VirtualMemoryThreshold = (std::uint32_t)loadConfig.virtualMemoryThreshold;
                        nativeConfig.ProcessHeapFlags = loadConfig.processHeapFlags;
                        nativeConfig.ProcessAffinityMask = (std::uint32_t)loadConfig.processAffinityMask;
                        nativeConfig.CSDVersion = loadConfig.CSDVersion;
                        nativeConfig.Reserved1 = loadConfig.reserved1;
                        nativeConfig.EditList = 0;
                        lcfgDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeConfig), EditList), loadConfig.editListRef, 0,
                            PEPlacedOffset::eOffsetType::VA_32BIT
                        );
                        nativeConfig.SecurityCookie = 0;
                        lcfgDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeConfig), SecurityCookie), loadConfig.securityCookieRef, 0,
                            PEPlacedOffset::eOffsetType::VA_32BIT
                        );
                        nativeConfig.SEHandlerTable = 0;
                        lcfgDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeConfig), SEHandlerTable), loadConfig.SEHandlerTableRef, 0,
                            PEPlacedOffset::eOffsetType::VA_32BIT
                        );
                        nativeConfig.SEHandlerCount = (std::uint32_t)loadConfig.SEHandlerCount;
                        nativeConfig.GuardCFCheckFunctionPointer = 0;
                        lcfgDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeConfig), GuardCFCheckFunctionPointer), loadConfig.guardCFCheckFunctionPtrRef, 0,
                            PEPlacedOffset::eOffsetType::VA_32BIT
                        );
                        nativeConfig.Reserved2 = (std::uint32_t)loadConfig.reserved2;
                        nativeConfig.GuardCFFunctionTable = 0;
                        lcfgDirAlloc.RegisterTargetRVA(
                            offsetof(decltype(nativeConfig), GuardCFFunctionTable), loadConfig.guardCFFunctionTableRef, 0,
                            PEPlacedOffset::eOffsetType::VA_32BIT
                        );
                        nativeConfig.GuardCFFunctionCount = (std::uint32_t)loadConfig.guardCFFunctionCount;
                        nativeConfig.GuardFlags = loadConfig.guardFlags;

                        // Write it.
                        lcfgDirAlloc.WriteToSection( &nativeConfig, sizeof(nativeConfig) );
                    }

                    // Remember the valid object.
                    loadConfig.allocEntry = std::move( lcfgDirAlloc );
                }
            }

            // nothing to do for IAT.
            // it is maintained by compilers.

            // * DELAY LOAD IMPORTS.
            auto& delayLoads = this->delayLoads;

            std::uint32_t numDelayLoads = (std::uint32_t)delayLoads.size();

            if ( numDelayLoads > 0 )
            {
                // Commit the sub-data first.
                for ( size_t n = 0; n < numDelayLoads; n++ )
                {
                    PEDelayLoadDesc& delayDesc = delayLoads[ n ];

                    // Write the DLL name.
                    if ( delayDesc.DLLName_allocEntry.IsAllocated() == false )
                    {
                        delayDesc.DLLName_allocEntry = WriteZeroTermString( rdonlySect, delayDesc.DLLName );
                    }

                    // Check if allocation for the DLL handle is required.
                    if ( delayDesc.DLLHandleAlloc.IsAllocated() == false )
                    {
                        std::uint32_t entrySize = GetPEPointerSize( isExtendedFormat );

                        dataSect.Allocate( delayDesc.DLLHandleAlloc, entrySize, entrySize );
                    }

                    // Write the import names.
                    auto& funcs = delayDesc.importNames;

                    if ( funcs.empty() == false )
                    {
                        // First the sub-data.
                        PEImportDesc::AllocatePEImportFunctionsData( rdonlySect, funcs );

                        if ( delayDesc.importNamesAllocEntry.IsAllocated() == false )
                        {
                            delayDesc.importNamesAllocEntry =
                                PEImportDesc::WritePEImportFunctions(
                                    rdonlySect, funcs, isExtendedFormat
                                );
                        }
                    }
                }

                // Do we need a new delay load descriptors array?
                if ( this->delayLoadsAllocEntry.IsAllocated() == false )
                {
                    // Just like the regular import descriptors, delay loads are stored in a simple
                    // array of descriptors. There is a NULL descriptor at the end.
                    std::uint32_t writeNumDescriptors = ( numDelayLoads + 1 );

                    PESectionAllocation delayLoadsAlloc;
                    rdonlySect.Allocate( delayLoadsAlloc, writeNumDescriptors * sizeof(PEStructures::IMAGE_DELAYLOAD_DESCRIPTOR), sizeof(std::uint32_t) );

                    // Write all descriptors.
                    for ( std::uint32_t n = 0; n < numDelayLoads; n++ )
                    {
                        const PEDelayLoadDesc& delayDesc = delayLoads[ n ];

                        // Calculate the offset we write this descriptor at.
                        std::uint32_t descWriteOff = ( n * sizeof(PEStructures::IMAGE_DELAYLOAD_DESCRIPTOR) );

                        PEStructures::IMAGE_DELAYLOAD_DESCRIPTOR nativeDesc;
                        nativeDesc.Attributes.AllAttributes = delayDesc.attrib;
                        nativeDesc.DllNameRVA = 0;
                        delayLoadsAlloc.RegisterTargetRVA( descWriteOff + offsetof(PEStructures::IMAGE_DELAYLOAD_DESCRIPTOR, DllNameRVA), delayDesc.DLLName_allocEntry );
                        nativeDesc.ModuleHandleRVA = 0;
                        delayLoadsAlloc.RegisterTargetRVA( descWriteOff + offsetof(PEStructures::IMAGE_DELAYLOAD_DESCRIPTOR, ModuleHandleRVA), delayDesc.DLLHandleAlloc );
                        nativeDesc.ImportAddressTableRVA = delayDesc.IATRef.GetRVA();
                        nativeDesc.ImportNameTableRVA = 0;
                        delayLoadsAlloc.RegisterTargetRVA( descWriteOff + offsetof(PEStructures::IMAGE_DELAYLOAD_DESCRIPTOR, ImportNameTableRVA), delayDesc.importNamesAllocEntry );
                        nativeDesc.BoundImportAddressTableRVA = delayDesc.boundImportAddrTableRef.GetRVA();
                        nativeDesc.UnloadInformationTableRVA = delayDesc.unloadInfoTableRef.GetRVA();
                        nativeDesc.TimeDateStamp = delayDesc.timeDateStamp;

                        // Write away!
                        delayLoadsAlloc.WriteToSection( &nativeDesc, sizeof(nativeDesc), descWriteOff );
                    }

                    // At the end, write the NULL termination.
                    {
                        std::uint32_t nullWriteOff = ( numDelayLoads * sizeof(PEStructures::IMAGE_DELAYLOAD_DESCRIPTOR) );

                        PEStructures::IMAGE_DELAYLOAD_DESCRIPTOR nullDesc = { 0 };

                        delayLoadsAlloc.WriteToSection( &nullDesc, sizeof(nullDesc), nullWriteOff );
                    }

                    // Remember the valid data.
                    this->delayLoadsAllocEntry = std::move( delayLoadsAlloc );
                }
            }
        }

        // SECTION-ALLOC PHASE.
        // Put all sections that we added into virtualAddress space.
        // (by the way, pretty retarded that Microsoft does not allow __forceinline on lambdas.)

        if ( rdonlySect.IsEmpty() == false )
        {
            rdonlySect.Finalize();

            this->AddSection( std::move( rdonlySect ) );
        }
        if ( dataSect.IsEmpty() == false )
        {
            dataSect.Finalize();

            this->AddSection( std::move( dataSect ) );
        }
    }

    // After writing and storing all allocation information we should write the RVAs
    // that we previously sheduled. This is possible because now every section has been
    // registered in the image and placed somewhere on virtual memory.
    
    LIST_FOREACH_BEGIN( PESection, this->sections.sectionList.root, sectionNode )

        for ( const PESection::PEPlacedOffset& placedOff : item->placedOffsets )
        {
            placedOff.WriteIntoData( this, item, imageBase );
        }

        // Since we have committed the RVAs into binary memory, no need for the meta-data anymore.
        item->placedOffsets.clear();

    LIST_FOREACH_END

    // For practical reasons we are writing the BASE RELOC information in its own section.
    // Main consensus is that relocation entries are written by placed offsets and that
    // creating another layer to abstract on that is too complicated.
    {
        PESection relocSect;
        relocSect.shortName = ".relsect";

        // * BASE RELOC.
        // Has to be written last because commit-phase may create new relocations!
        const auto& baseRelocs = this->baseRelocs;

        if ( baseRelocs.empty() == false )
        {
            // Do we even need a new base reloc data directory?
            if ( this->baseRelocAllocEntry.IsAllocated() == false )
            {
                // We first calculate how big a directory we need.
                std::uint32_t baseRelocDirSize = 0;

                for ( const auto& relocNode : baseRelocs )
                {
                    const PEBaseReloc& relocInfo = relocNode.second;

                    std::uint32_t numEntries = (std::uint32_t)relocInfo.items.size();

                    // This is the header, and the same-sized reloc entries.
                    std::uint32_t chunkSize = sizeof(PEStructures::IMAGE_BASE_RELOCATION) + numEntries * sizeof(PEStructures::IMAGE_BASE_RELOC_TYPE_ITEM);

                    baseRelocDirSize += chunkSize;
                }

                PESectionAllocation baseRelocAlloc;
                relocSect.Allocate( baseRelocAlloc, baseRelocDirSize, sizeof(std::uint32_t) );

                // Since we are iterating a std::map container, the output is
                // guarranteed to be sorted-by-address!
                std::uint32_t curWriteOff = 0;

                for ( const auto& relocNode : baseRelocs )
                {
                    const PEBaseReloc& relocInfo = relocNode.second;

                    std::uint32_t numEntries = (std::uint32_t)relocInfo.items.size();

                    // Calculate the size of this block.
                    // We kind of did above already.
                    std::uint32_t chunkSize = sizeof(PEStructures::IMAGE_BASE_RELOCATION) + numEntries * sizeof(PEStructures::IMAGE_BASE_RELOC_TYPE_ITEM);

                    // Write header.
                    {
                        PEStructures::IMAGE_BASE_RELOCATION nativeRelocInfo;
                        nativeRelocInfo.VirtualAddress = relocInfo.offsetOfReloc;
                        nativeRelocInfo.SizeOfBlock = chunkSize;

                        baseRelocAlloc.WriteToSection( &nativeRelocInfo, sizeof(nativeRelocInfo), curWriteOff );

                        curWriteOff += sizeof(nativeRelocInfo);
                    }

                    // Write all reloc items now.
                    for ( size_t n = 0; n < numEntries; n++ )
                    {
                        const PEBaseReloc::item& rebaseEntry = relocInfo.items[ n ];

                        PEStructures::IMAGE_BASE_RELOC_TYPE_ITEM nativeEntry;
                        nativeEntry.type = (std::uint16_t)rebaseEntry.type;
                        nativeEntry.offset = rebaseEntry.offset;

                        baseRelocAlloc.WriteToSection( &nativeEntry, sizeof(nativeEntry), curWriteOff );

                        curWriteOff += sizeof(nativeEntry);
                    }
                }

                // Remember it.
                this->baseRelocAllocEntry = std::move( baseRelocAlloc );
            }
        }

        // Place the relocation section.
        if ( relocSect.IsEmpty() == false )
        {
            relocSect.Finalize();

            this->AddSection( std::move( relocSect ) );
        }
    }
}

void PEFile::WriteToStream( PEStream *peStream )
{
    // Write data that requires writing.
    this->CommitDataDirectories();
    
    // Prepare the data directories.
    PEStructures::IMAGE_DATA_DIRECTORY peDataDirs[ PEL_IMAGE_NUMBEROF_DIRECTORY_ENTRIES ];
    {
        // Reset everything we do not use.
        memset( peDataDirs, 0, sizeof( peDataDirs ) );

        auto dirRegHelper = []( PEStructures::IMAGE_DATA_DIRECTORY& dataDir, const PESectionAllocation& allocEntry )
        {
            if ( allocEntry.IsAllocated() )
            {
                dataDir.VirtualAddress = allocEntry.ResolveOffset( 0 );
                dataDir.Size = allocEntry.GetDataSize();
            }
            else
            {
                dataDir.VirtualAddress = 0;
                dataDir.Size = 0;
            }
        };

        dirRegHelper( peDataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_EXPORT ], this->exportDir.allocEntry );
        dirRegHelper( peDataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_IMPORT ], this->importsAllocEntry );
        dirRegHelper( peDataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_RESOURCE ], this->resAllocEntry );
        dirRegHelper( peDataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_EXCEPTION ], this->exceptAllocEntry );
        
        // Attribute certificate table needs to be written after the sections!

        dirRegHelper( peDataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_BASERELOC ], this->baseRelocAllocEntry );
        dirRegHelper( peDataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_DEBUG ], this->debugDescsAlloc );
        
        // Architecture.
        {
            PEStructures::IMAGE_DATA_DIRECTORY& archDataDir = peDataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_ARCHITECTURE ];

            archDataDir.VirtualAddress = 0;
            archDataDir.Size = 0;
        }

        // Global pointer.
        {
            PEStructures::IMAGE_DATA_DIRECTORY& gptrDataDir = peDataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_GLOBALPTR ];

            gptrDataDir.VirtualAddress = this->globalPtr.ptrOffset;
            gptrDataDir.Size = 0;
        }

        dirRegHelper( peDataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_TLS ], this->tlsInfo.allocEntry );
        dirRegHelper( peDataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG ], this->loadConfig.allocEntry );

        // Bound Import Directory is written after sections!
        
        // IAT.
        {
            PEStructures::IMAGE_DATA_DIRECTORY& iatDataDir = peDataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_IAT ];

            iatDataDir.VirtualAddress = this->iatThunkAll.thunkDataStart;
            iatDataDir.Size = this->iatThunkAll.thunkDataSize;
        }

        dirRegHelper( peDataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT ], this->delayLoadsAllocEntry );

        // COM descriptor.
        {
            PEStructures::IMAGE_DATA_DIRECTORY& comDescDataDir = peDataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR ];

            comDescDataDir.VirtualAddress = this->clrInfo.dataOffset;
            comDescDataDir.Size = this->clrInfo.dataSize;
        }
    }

    // TODO: properly write the PE file onto disk.
    
    FileSpaceAllocMan allocMan;

    // Allocate and write the DOS header.
    allocMan.AllocateAt( 0, sizeof( PEStructures::IMAGE_DOS_HEADER ) + (std::uint32_t)this->dos_data.progData.size() );

    PEStructures::IMAGE_DOS_HEADER dos_header;
    dos_header.e_magic = PEL_IMAGE_DOS_SIGNATURE;
    dos_header.e_cblp = this->dos_data.cblp;
    dos_header.e_cp = this->dos_data.cp;
    dos_header.e_crlc = this->dos_data.crlc;
    dos_header.e_cparhdr = this->dos_data.cparhdr;
    dos_header.e_minalloc = this->dos_data.minalloc;
    dos_header.e_maxalloc = this->dos_data.maxalloc;
    dos_header.e_ss = this->dos_data.ss;
    dos_header.e_sp = this->dos_data.sp;
    dos_header.e_csum = this->dos_data.csum;
    dos_header.e_ip = this->dos_data.ip;
    dos_header.e_cs = this->dos_data.cs;
    dos_header.e_lfarlc = this->dos_data.lfarlc;
    dos_header.e_ovno = this->dos_data.ovno;
    memcpy( dos_header.e_res, this->dos_data.reserved1, sizeof( dos_header.e_res ) );
    dos_header.e_oemid = this->dos_data.oemid;
    dos_header.e_oeminfo = this->dos_data.oeminfo;
    memcpy( dos_header.e_res2, this->dos_data.reserved2, sizeof( dos_header.e_res2 ) );

    // Allocate and write PE information next.
    // This has to be the PE header, the optional header (32bit or 64bit), the data directory info
    // and the section info.
    {
        bool isExtendedFormat = this->isExtendedFormat;

        // Determine the size of data to-be-written.
        std::uint32_t peDataSize = sizeof( PEStructures::IMAGE_PE_HEADER );

        // The optional header.
        std::uint32_t peOptHeaderSize = sizeof( std::uint16_t );    // start with the magic number.

        if ( isExtendedFormat )
        {
            // TODO: if directory entries support turns dynamic we need to adjust this.
            peOptHeaderSize += sizeof( PEStructures::IMAGE_OPTIONAL_HEADER64 );
        }
        else
        {
            peOptHeaderSize += sizeof( PEStructures::IMAGE_OPTIONAL_HEADER32 );
        }

        // We must include how many data directories we are willing to write.
        peOptHeaderSize += sizeof(peDataDirs);

        peDataSize += peOptHeaderSize;

        // Add the size of section headers.
        peDataSize += ( this->sections.numSections * sizeof( PEStructures::IMAGE_SECTION_HEADER ) );

        // TODO: there is "deprecated" information like lineinfo and native relocation
        // info allowed. should we add support? this would mean adding even more size to
        // peDataSize.

        std::uint32_t peDataPos = allocMan.AllocateAny( peDataSize );

        // Remember that anything before us counts as MSDOS space.
        const std::uint32_t dosAllocSize = ( peDataPos );

        PEStructures::IMAGE_PE_HEADER pe_data;
        pe_data.Signature = PEL_IMAGE_PE_HEADER_SIGNATURE;
        pe_data.FileHeader.Machine = this->pe_finfo.machine_id;
        pe_data.FileHeader.NumberOfSections = (std::uint16_t)this->sections.numSections;
        pe_data.FileHeader.TimeDateStamp = this->pe_finfo.timeDateStamp;
        pe_data.FileHeader.PointerToSymbolTable = 0;        // not supported yet.
        pe_data.FileHeader.NumberOfSymbols = 0;
        pe_data.FileHeader.SizeOfOptionalHeader = peOptHeaderSize;
        
        // Set up the flags.
        pe_data.FileHeader.Characteristics = GetPENativeFileFlags();

        // Time for the optional header.
        // Once again a complicated construct that depends on data before and after.
        // For that reason we allocate here and fill out the structure afterward.
        std::uint32_t peOptHeaderOffset = ( peDataPos + sizeof(PEStructures::IMAGE_PE_HEADER) );

        // Write the data directories.
        // Remember that we must not do allocations about the data directories here anymore.

        // Write the section headers with all the meta-data surrounding them.
        // Offset of section data.
        const std::uint32_t sectHeadOffset = ( peOptHeaderOffset + pe_data.FileHeader.SizeOfOptionalHeader );

        std::uint32_t sectionAlignment = this->sections.GetSectionAlignment();

        // Remember file-space allocation data of every section.
        // We will (probably) need it for the debug 'special citizen'.
        sect_allocMap_t sect_allocMap;

        // Allocate and write section data.
        {
            std::uint32_t sectIndex = 0;

            LIST_FOREACH_BEGIN( PESection, this->sections.sectionList.root, sectionNode )
            
                // Allocate this section.
                const std::uint32_t allocVirtualSize = ALIGN_SIZE( item->GetVirtualSize(), sectionAlignment );
                const std::uint32_t rawDataSize = (std::uint32_t)item->stream.Size();

                std::uint32_t sectOffset = allocMan.AllocateAny( rawDataSize, this->peOptHeader.fileAlignment );

                // Remember meta-data about the allocation.
                std::uint32_t sectVirtAddr = item->GetVirtualAddress();
                {
                    sect_allocInfo allocInfo;
                    allocInfo.alloc_off = sectOffset;

                    // For the storage we assume that the virtual address cannot change here.
                    sect_allocMap.insert( std::make_pair( sectVirtAddr, std::move( allocInfo ) ) );
                }

                PEStructures::IMAGE_SECTION_HEADER header;
                strncpy( (char*)header.Name, item->shortName.c_str(), countof(header.Name) );
                header.VirtualAddress = sectVirtAddr;
                header.Misc.VirtualSize = allocVirtualSize;
                header.SizeOfRawData = rawDataSize;
                header.PointerToRawData = sectOffset;
                header.PointerToRelocations = 0;    // TODO: change this if native relocations become a thing.
                header.PointerToLinenumbers = 0;    // TODO: change this if linenumber data becomes a thing
                header.NumberOfRelocations = 0;
                header.NumberOfLinenumbers = 0;
                header.Characteristics = item->GetPENativeFlags();

                // Write it.
                {
                    // TODO: remember to update this logic if we support relocations or linenumbers.
                    const std::uint32_t sectHeadFileOff = ( sectHeadOffset + sizeof(header) * sectIndex );

                    PEWrite( peStream, sectHeadFileOff, sizeof(header), &header );
                }

                // Also write the PE data.
                PEWrite( peStream, sectOffset, rawDataSize, item->stream.Data() );

                sectIndex++;
            
            LIST_FOREACH_END
        }
        // Do note that the serialized section headers are ordered parallel to the section meta-data in PEFile.
        // So that the indices match for serialized and runtime data.

        // Now that section info has been written we need to return to the debug 'special citizen'.
        {
            PESectionAllocation& debugDescsAlloc = this->debugDescsAlloc;

            if ( PESection *debugDescsSection = debugDescsAlloc.GetSection() )
            {
                // Get the allocation info.
                const auto& allocInfoNode = sect_allocMap.find( debugDescsSection->GetVirtualAddress() );

                assert( allocInfoNode != sect_allocMap.end() );

                const sect_allocInfo& sectAllocInfo = allocInfoNode->second;

                // Get the written offset to the debug descriptors array.
                std::uint32_t fileDebugArrayOff = ( sectAllocInfo.alloc_off + debugDescsAlloc.ResolveInternalOffset( 0 ) );

                // Process all debug descriptors.
                auto& debugDescs = this->debugDescs;

                const std::uint32_t numDebugDescs = (std::uint32_t)debugDescs.size();

                for ( std::uint32_t n = 0; n < numDebugDescs; n++ )
                {
                    PEDebugDesc& debugEntry = debugDescs[ n ];

                    // We need to establish a file pointer for debug data that is present.
                    if ( debugEntry.dataStore.NeedsFinalizationPhase() )
                    {
                        // Get the offset to the written debug descriptor.
                        std::uint32_t writtenOffset = ( fileDebugArrayOff + n * sizeof(PEStructures::IMAGE_DEBUG_DIRECTORY) );

                        std::uint32_t fileDataOff =
                            debugEntry.dataStore.ResolveFinalizationPhase( peStream, allocMan, sect_allocMap );

                        // Write the file offset.
                        PEWrite( peStream, writtenOffset + offsetof(PEStructures::IMAGE_DEBUG_DIRECTORY, PointerToRawData), sizeof(fileDataOff), &fileDataOff );
                    }
                }
            }
        }

        // Write the Bound Import Directory.
        {
            PEStructures::IMAGE_DATA_DIRECTORY& boundImpDir = peDataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ];

            size_t numDesc = boundImports.size();

            std::uint32_t boundImp_peSize = 0;
            std::uint32_t boundImp_peOff = 0;

            if ( numDesc != 0 )
            {
                const auto& boundImports = this->boundImports;

                struct boundImp_allocInfo
                {
                    std::uint16_t DLLName_allocOff;

                    std::vector <boundImp_allocInfo> forw_infos;
                };

                std::vector <boundImp_allocInfo> allocInfo_boundImports( numDesc );

                // We have a nasty recursive scheme.
                struct helpers
                {
                    static inline std::uint32_t AllocateBoundImportDirectory_array( const decltype( PEBoundImport::forw_bindings )& boundImps )
                    {
                        // We return the size necessary for the array of descriptors.
                        std::uint32_t currentSize = sizeof( PEStructures::IMAGE_BOUND_IMPORT_DESCRIPTOR );

                        for ( const PEBoundImport& boundImp : boundImps )
                        {
                            currentSize += AllocateBoundImportDirectory_array( boundImp.forw_bindings );
                        }

                        return currentSize;
                    }

                    static inline void AllocateBoundImportDirectory_names(
                        FileSpaceAllocMan& allocMan, const PEBoundImport& boundImp,
                        boundImp_allocInfo& ainfoOut
                    )
                    {
                        size_t forw_entryCount = boundImp.forw_bindings.size();

                        // Actually allocate the DLLName strings.
                        {
                            std::uint32_t DLLName_allocSize = (std::uint32_t)( boundImp.DLLName.size() + 1 );

                            ainfoOut.DLLName_allocOff = allocMan.AllocateAny( DLLName_allocSize, 1 );
                        }

                        // Go for all forwardings.
                        ainfoOut.forw_infos.resize( forw_entryCount );

                        for ( size_t n = 0; n < forw_entryCount; n++ )
                        {
                            const PEBoundImport& forw_bind = boundImp.forw_bindings[ n ];
                            boundImp_allocInfo& forw_bind_allocInfo = ainfoOut.forw_infos[ n ];

                            AllocateBoundImportDirectory_names( allocMan, forw_bind, forw_bind_allocInfo );
                        }
                    }

                    static inline void WriteBoundImportDirectory( PEStream *streamOut, std::uint32_t offDirRoot, const PEBoundImport& boundImp, const boundImp_allocInfo& ainfo )
                    {
                        std::uint16_t offModuleName = ainfo.DLLName_allocOff;
                        size_t numForwRefs = boundImp.forw_bindings.size();

                        // Write the descriptor at the current position.
                        PEStructures::IMAGE_BOUND_IMPORT_DESCRIPTOR nativeDesc;
                        nativeDesc.TimeDateStamp = boundImp.timeDateStamp;  // checksum.
                        nativeDesc.OffsetModuleName = offModuleName;
                        nativeDesc.NumberOfModuleForwarderRefs = (std::uint16_t)numForwRefs;

                        streamOut->WriteStruct( nativeDesc );

                        pe_file_ptr_t saved_fileOff = streamOut->Tell();

                        // Write the DLLName.
                        {
                            const std::string& DLLName = boundImp.DLLName;

                            size_t DLLName_writeSize = ( DLLName.size() + 1 );

                            streamOut->Seek( offDirRoot + ainfo.DLLName_allocOff );
                            streamOut->Write( DLLName.c_str(), DLLName_writeSize );
                        }

                        streamOut->Seek( saved_fileOff );

                        for ( size_t n = 0; n < numForwRefs; n++ )
                        {
                            const PEBoundImport& forw_boundImp = boundImp.forw_bindings[ n ];
                            const boundImp_allocInfo& forw_boundImp_allocInfo = ainfo.forw_infos[ n ];

                            WriteBoundImportDirectory( streamOut, offDirRoot, forw_boundImp, forw_boundImp_allocInfo );
                        }
                    }
                };

                // We first have to allocate the space.
                FileSpaceAllocMan boundImpAllocMan;
            
                // First the array of descriptors.
                std::uint32_t arraySize = 0;

                for ( size_t n = 0; n < numDesc; n++ )
                {
                    const PEBoundImport& curImp = boundImports[ n ];

                    arraySize += helpers::AllocateBoundImportDirectory_array( curImp.forw_bindings );
                }

                // We must include a NULL descriptor as termination.
                arraySize += sizeof( PEStructures::IMAGE_BOUND_IMPORT_DESCRIPTOR );

                boundImpAllocMan.AllocateAt( 0, arraySize );

                // Now the name strings.
                for ( size_t n = 0; n < numDesc; n++ )
                {
                    const PEBoundImport& curImp = boundImports[ n ];
                    boundImp_allocInfo& allocInfo = allocInfo_boundImports[ n ];

                    helpers::AllocateBoundImportDirectory_names( boundImpAllocMan, curImp, allocInfo );
                }

                // Allocate the entire chunk on file-space.
                boundImp_peSize = boundImpAllocMan.GetSpanSize( sizeof( std::uint32_t ) );
                boundImp_peOff = allocMan.AllocateAny( boundImp_peSize );

                // Write the things.
                peStream->Seek( boundImp_peOff );

                for ( size_t n = 0; n < numDesc; n++ )
                {
                    const PEBoundImport& curImp = boundImports[ n ];
                    const boundImp_allocInfo& allocInfo = allocInfo_boundImports[ n ];

                    helpers::WriteBoundImportDirectory( peStream, boundImp_peOff, curImp, allocInfo );
                }

                // Terminate with a NULL descriptor.
                {
                    PEStructures::IMAGE_BOUND_IMPORT_DESCRIPTOR nullDesc;
                    nullDesc.TimeDateStamp = 0;
                    nullDesc.OffsetModuleName = 0;
                    nullDesc.NumberOfModuleForwarderRefs = 0;

                    peStream->WriteStruct( nullDesc );
                }
            }

            // Store details in the data directory.
            boundImpDir.VirtualAddress = boundImp_peOff;
            boundImpDir.Size = boundImp_peSize;
        }

        // Write the Attribute Certificate Table.
        {
            PEStructures::IMAGE_DATA_DIRECTORY& certDataDir = peDataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_SECURITY ];

            std::uint32_t _rva, dataSize;
            this->securityCookie.certStore.ResolveDataPhaseAllocation( _rva, dataSize );

            // We actually cannot store as RVA.
            assert( _rva == 0 );

            std::uint32_t filePtr = this->securityCookie.certStore.ResolveFinalizationPhase( peStream, allocMan, sect_allocMap );

            certDataDir.VirtualAddress = filePtr;
            certDataDir.Size = dataSize;
        }

        // Calculate the required image size in memory.
        // Since sections are address sorted, this is pretty easy.
        std::uint32_t memImageSize = sections.GetImageSize();

        // Write PE data.
        // First the header
        PEWrite( peStream, peDataPos, sizeof( pe_data ), &pe_data );

        // Calculate the size of headers, which also includes MSDOS stub.
        std::uint32_t sizeOfHeaders = ( peDataSize + dosAllocSize );

        // Now we need to write the optional header.
        if ( isExtendedFormat )
        {
#pragma pack(1)
            struct
            {
                std::uint16_t Magic;
                PEStructures::IMAGE_OPTIONAL_HEADER64 optHeader;
                decltype( peDataDirs ) dataDirs;
            } headerData;
#pragma pack()

            headerData.Magic = 0x020B;

            PEStructures::IMAGE_OPTIONAL_HEADER64& optHeader = headerData.optHeader;
            optHeader.MajorLinkerVersion = this->peOptHeader.majorLinkerVersion;
            optHeader.MinorLinkerVersion = this->peOptHeader.minorLinkerVersion;
            optHeader.SizeOfCode = this->peOptHeader.sizeOfCode;
            optHeader.SizeOfInitializedData = this->peOptHeader.sizeOfInitializedData;
            optHeader.SizeOfUninitializedData = this->peOptHeader.sizeOfUninitializedData;
            optHeader.AddressOfEntryPoint = this->peOptHeader.addressOfEntryPointRef.GetRVA();
            optHeader.BaseOfCode = this->peOptHeader.baseOfCode;
            optHeader.ImageBase = this->peOptHeader.imageBase;
            optHeader.SectionAlignment = sectionAlignment;
            optHeader.FileAlignment = this->peOptHeader.fileAlignment;
            optHeader.MajorOperatingSystemVersion = this->peOptHeader.majorOSVersion;
            optHeader.MinorOperatingSystemVersion = this->peOptHeader.minorOSVersion;
            optHeader.MajorImageVersion = this->peOptHeader.majorImageVersion;
            optHeader.MinorImageVersion = this->peOptHeader.minorImageVersion;
            optHeader.MajorSubsystemVersion = this->peOptHeader.majorSubsysVersion;
            optHeader.MinorSubsystemVersion = this->peOptHeader.minorSubsysVersion;
            optHeader.Win32VersionValue = this->peOptHeader.win32VersionValue;
            optHeader.SizeOfImage = memImageSize;
            optHeader.SizeOfHeaders = ALIGN_SIZE( sizeOfHeaders, this->peOptHeader.fileAlignment );
            optHeader.CheckSum = this->peOptHeader.checkSum;    // TODO: Windows Critical components need to update this.
            optHeader.Subsystem = this->peOptHeader.subsys;
            optHeader.DllCharacteristics = this->GetPENativeDLLOptFlags();
            optHeader.SizeOfStackReserve = this->peOptHeader.sizeOfStackReserve;
            optHeader.SizeOfStackCommit = this->peOptHeader.sizeOfStackCommit;
            optHeader.SizeOfHeapReserve = this->peOptHeader.sizeOfHeapReserve;
            optHeader.SizeOfHeapCommit = this->peOptHeader.sizeOfHeapCommit;
            optHeader.LoaderFlags = this->peOptHeader.loaderFlags;
            optHeader.NumberOfRvaAndSizes = countof(peDataDirs);
            memcpy( headerData.dataDirs, peDataDirs, sizeof( peDataDirs ) );

            PEWrite( peStream, peOptHeaderOffset, sizeof(headerData), &headerData );
        }
        else
        {
#pragma pack(1)
            struct
            {
                std::uint16_t Magic;
                PEStructures::IMAGE_OPTIONAL_HEADER32 optHeader;
                decltype( peDataDirs ) dataDirs;
            } headerData;
#pragma pack()

            headerData.Magic = 0x010B;

            PEStructures::IMAGE_OPTIONAL_HEADER32& optHeader = headerData.optHeader;
            optHeader.MajorLinkerVersion = this->peOptHeader.majorLinkerVersion;
            optHeader.MinorLinkerVersion = this->peOptHeader.minorLinkerVersion;
            optHeader.SizeOfCode = this->peOptHeader.sizeOfCode;
            optHeader.SizeOfInitializedData = this->peOptHeader.sizeOfInitializedData;
            optHeader.SizeOfUninitializedData = this->peOptHeader.sizeOfUninitializedData;
            optHeader.AddressOfEntryPoint = this->peOptHeader.addressOfEntryPointRef.GetRVA();
            optHeader.BaseOfCode = this->peOptHeader.baseOfCode;
            optHeader.BaseOfData = this->peOptHeader.baseOfData;    // TODO: maybe this needs updating if we change from 32bit to 64bit.
            optHeader.ImageBase = (std::uint32_t)this->peOptHeader.imageBase;
            optHeader.SectionAlignment = sectionAlignment;
            optHeader.FileAlignment = this->peOptHeader.fileAlignment;
            optHeader.MajorOperatingSystemVersion = this->peOptHeader.majorOSVersion;
            optHeader.MinorOperatingSystemVersion = this->peOptHeader.minorOSVersion;
            optHeader.MajorImageVersion = this->peOptHeader.majorImageVersion;
            optHeader.MinorImageVersion = this->peOptHeader.minorImageVersion;
            optHeader.MajorSubsystemVersion = this->peOptHeader.majorSubsysVersion;
            optHeader.MinorSubsystemVersion = this->peOptHeader.minorSubsysVersion;
            optHeader.Win32VersionValue = this->peOptHeader.win32VersionValue;
            optHeader.SizeOfImage = memImageSize;
            optHeader.SizeOfHeaders = ALIGN_SIZE( sizeOfHeaders, this->peOptHeader.fileAlignment );
            optHeader.CheckSum = this->peOptHeader.checkSum;    // TODO: Windows Critical components need to update this.
            optHeader.Subsystem = this->peOptHeader.subsys;
            optHeader.DllCharacteristics = this->GetPENativeDLLOptFlags();
            optHeader.SizeOfStackReserve = (std::uint32_t)this->peOptHeader.sizeOfStackReserve;
            optHeader.SizeOfStackCommit = (std::uint32_t)this->peOptHeader.sizeOfStackCommit;
            optHeader.SizeOfHeapReserve = (std::uint32_t)this->peOptHeader.sizeOfHeapReserve;
            optHeader.SizeOfHeapCommit = (std::uint32_t)this->peOptHeader.sizeOfHeapCommit;
            optHeader.LoaderFlags = this->peOptHeader.loaderFlags;
            optHeader.NumberOfRvaAndSizes = countof(peDataDirs);
            memcpy( headerData.dataDirs, peDataDirs, sizeof( peDataDirs ) );

            PEWrite( peStream, peOptHeaderOffset, sizeof(headerData), &headerData );
        }

        // TODO: update section headers and stuff with offsets of sections and other data.

        // We need to know where PE data starts at.
        dos_header.e_lfanew = (std::int32_t)peDataPos;
    }

    peStream->Seek( 0 );
    peStream->Write( &dos_header,sizeof( dos_header ) );
    peStream->Write( this->dos_data.progData.data(), this->dos_data.progData.size() );
}
