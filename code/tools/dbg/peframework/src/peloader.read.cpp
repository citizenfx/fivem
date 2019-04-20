// I am (more or less) thankful of certain other people that have done research on PE loading aswell,
// such as...
//  http://python-pefile.sourcearchive.com/
//  https://code.google.com/archive/p/corkami/wikis/PE.wiki

// Deserialization of PE files.
#include "peframework.h"

#include "peloader.internal.hxx"

void PEFile::PEFileSpaceData::ReadFromFile( PEStream *peStream, const PESectionMan& sections, std::uint32_t rva, std::uint32_t filePtr, std::uint32_t dataSize )
{
    // Determine the storage type of this debug information.
    eStorageType storageType;

    if ( rva != 0 )
    {
        // Since we are allocated on address space, we assume its entirely allocated.
        // There is no special storage required.
        storageType = eStorageType::SECTION;
    }
    else if ( filePtr != 0 )
    {
        // Being placed out-of-band is very interesting because you essentially are an
        // attachment appended to the PE file, basically being not a part of it at all.
        // This storage type is assumingly legacy.
        storageType = eStorageType::FILE;
    }
    else
    {
        // Otherwise we can have no storage at all, which is fine.
        storageType = eStorageType::NONE;
    }

    this->storageType = storageType;

    // Register data presence depending on the storage type.
    if ( storageType == eStorageType::SECTION )
    {
        PESection *fileDataSect;

        bool gotLocation = sections.GetPEDataLocation( rva, NULL, &fileDataSect );

        if ( !gotLocation )
        {
            throw peframework_exception(
                ePEExceptCode::ACCESS_OUT_OF_BOUNDS,
                "failed to get section location of PE file-space data"
            );
        }

        fileDataSect->SetPlacedMemory( this->sectRef, rva, dataSize );
    }
    else if ( storageType == eStorageType::FILE )
    {
        // In this case we have to read the data out of the file manually.
        // After all, debug information is a 'special citizen' of the PE standard.
        peStream->Seek( filePtr );

        this->fileRef.resize( dataSize );

        size_t readCount = peStream->Read( this->fileRef.data(), dataSize );

        if ( readCount != dataSize )
        {
            throw peframework_exception(
                ePEExceptCode::ACCESS_OUT_OF_BOUNDS,
                "truncated PE file-space data error"
            );
        }
    }
    // Having no storage is perfectly fine.
}

static AINLINE std::uint32_t VA2RVA( std::uint64_t va, std::uint64_t imageBase )
{
    if ( va == 0 )
        return 0;

    return (std::uint32_t)( va - imageBase );
}

PEFile::PEImportDesc::functions_t PEFile::PEImportDesc::ReadPEImportFunctions( PESectionMan& sections, std::uint32_t rva, PESectionAllocation& allocEntry, bool isExtendedFormat )
{
    PESection *importNameArraySect;
    PEDataStream importNameArrayStream;
    {
        bool hasStream = sections.GetPEDataStream( rva, importNameArrayStream, &importNameArraySect );

        if ( !hasStream )
        {
            throw peframework_exception(
                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                "failed to read PE import function name array"
            );
        }
    }
                    
    importNameArraySect->SetPlacedMemory( allocEntry, rva );

    // The array goes on until a terminating NULL.
    functions_t funcs;

    while ( true )
    {
        // Read the entry properly.
        std::uint64_t importNameRVA;

        if ( isExtendedFormat )
        {
            std::uint64_t importNameRVA_read;
            importNameArrayStream.Read( &importNameRVA_read, sizeof( importNameRVA_read ) );

            importNameRVA = importNameRVA_read;
        }
        else
        {
            std::uint32_t importNameRVA_read;
            importNameArrayStream.Read( &importNameRVA_read, sizeof( importNameRVA_read ) );

            importNameRVA = importNameRVA_read;
        }

        if ( !importNameRVA )
            break;

        PEImportDesc::importFunc funcInfo;

        // Check if this is an ordinal import or a named import.
        bool isOrdinalImport;

        if ( isExtendedFormat )
        {
            isOrdinalImport = ( importNameRVA & PEL_IMAGE_ORDINAL_FLAG64 ) != 0;
        }
        else
        {
            isOrdinalImport = ( importNameRVA & PEL_IMAGE_ORDINAL_FLAG32 ) != 0;
        }

        if ( isOrdinalImport )
        {
            // According to the documentation ordinals are 16bit numbers.

            // The documentation says that even for PE32+ the number stays 31bit.
            // It is really weird that this was made a 64bit number tho.
            funcInfo.ordinal_hint = (std::uint16_t)importNameRVA;
        }
        else
        {
            PESection *importNameSect;
            PEDataStream importNameStream;
            {
                bool gotStream = sections.GetPEDataStream( (std::uint32_t)importNameRVA, importNameStream, &importNameSect );

                if ( !gotStream )
                {
                    throw peframework_exception(
                        ePEExceptCode::CORRUPT_PE_STRUCTURE,
                        "failed to read PE import function name entry"
                    );
                }
            }

            importNameSect->SetPlacedMemory( funcInfo.nameAllocEntry, (std::uint32_t)importNameRVA );

            // Read stuff.
            std::uint16_t ordinal_hint;
            importNameStream.Read( &ordinal_hint, sizeof(ordinal_hint) );

            funcInfo.ordinal_hint = ordinal_hint;

            ReadPEString( importNameStream, funcInfo.name );
        }
        funcInfo.isOrdinalImport = isOrdinalImport;
                        
        funcs.push_back( std::move( funcInfo ) );
    }

    return funcs;
}

void PEFile::LoadFromDisk( PEStream *peStream )
{
    // We read the DOS stub.
    DOSStub dos;

    // Cache some properties.
    std::int32_t peFileStartOffset;
    {
        // It's data is expected to have no complicated things
        PEStructures::IMAGE_DOS_HEADER dosHeader;

        bool couldReadDOS = peStream->ReadStruct( dosHeader );

        if ( !couldReadDOS )
        {
            throw peframework_exception(
                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                "cannot read MSDOS header"
            );
        }

        // Verify DOS header (light).
        bool isValidDOSHeader =
            ( dosHeader.e_magic == PEL_IMAGE_DOS_SIGNATURE );

        if ( !isValidDOSHeader )
        {
            throw peframework_exception(
                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                "invalid MSDOS checksum"
            );
        }

        // Save all information about the DOS stub.
        dos.cblp = dosHeader.e_cblp;
        dos.cp = dosHeader.e_cp;
        dos.crlc = dosHeader.e_crlc;
        dos.cparhdr = dosHeader.e_cparhdr;
        dos.minalloc = dosHeader.e_minalloc;
        dos.maxalloc = dosHeader.e_maxalloc;
        dos.ss = dosHeader.e_ss;
        dos.sp = dosHeader.e_sp;
        dos.csum = dosHeader.e_csum;
        dos.ip = dosHeader.e_ip;
        dos.cs = dosHeader.e_cs;
        dos.lfarlc = dosHeader.e_lfarlc;
        dos.ovno = dosHeader.e_ovno;
        memcpy( dos.reserved1, dosHeader.e_res, sizeof( dos.reserved1 ) );
        dos.oemid = dosHeader.e_oemid;
        dos.oeminfo = dosHeader.e_oeminfo;
        memcpy( dos.reserved2, dosHeader.e_res2, sizeof( dos.reserved2 ) );

        // We need the program data aswell.
        // Assumption is that the data directly follows the header and ends in the new data ptr.
        {
            std::int32_t newDataOffset = dosHeader.e_lfanew;

            std::int32_t sizeOfStubData = ( newDataOffset - sizeof( dosHeader ) );

            assert( sizeOfStubData >= 0 );

            std::vector <unsigned char> progData( sizeOfStubData );
            {
                size_t progReadCount = peStream->Read( progData.data(), sizeOfStubData );

                if ( progReadCount != sizeOfStubData )
                {
                    throw peframework_exception(
                        ePEExceptCode::CORRUPT_PE_STRUCTURE,
                        "invalid MSDOS stub"
                    );
                }
            }

            dos.progData = std::move( progData );
        }

        peFileStartOffset = dosHeader.e_lfanew;
    }

    // Go on to the PE header.
    PEFileInfo peInfo;

    // The loader runtime needs to know if we are PE32 or PE32+.
    bool isExtendedFormat;

    // Cache some properties.
    std::uint16_t numSections;
    std::uint16_t peOptHeaderSize;
    {
        bool seekSuccess = peStream->Seek( peFileStartOffset );

        assert( seekSuccess == true );

        // Read PE information.
        PEStructures::IMAGE_PE_HEADER peHeader;

        bool couldReadPE = peStream->ReadStruct( peHeader );

        if ( couldReadPE == false )
        {
            throw peframework_exception(
                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                "failed to read PE NT headers"
            );
        }

        // Validate some things.
        if ( peHeader.Signature != PEL_IMAGE_PE_HEADER_SIGNATURE )
        {
            throw peframework_exception(
                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                "invalid PE header signature"
            );
        }

        // Read the machine type.
        std::uint16_t machineType = peHeader.FileHeader.Machine;

        // Store stuff.
        peInfo.machine_id = machineType;
        peInfo.timeDateStamp = peHeader.FileHeader.TimeDateStamp;
    
        // Flags that matter.
        std::uint16_t chars = peHeader.FileHeader.Characteristics;

        peInfo.isExecutableImage        = ( chars & PEL_IMAGE_FILE_EXECUTABLE_IMAGE ) != 0;
        peInfo.hasLocalSymbols          = ( chars & PEL_IMAGE_FILE_LOCAL_SYMS_STRIPPED ) == 0;
        peInfo.hasAggressiveTrim        = ( chars & PEL_IMAGE_FILE_AGGRESIVE_WS_TRIM ) != 0;
        peInfo.largeAddressAware        = ( chars & PEL_IMAGE_FILE_LARGE_ADDRESS_AWARE ) != 0;
        peInfo.bytesReversedLO          = ( chars & PEL_IMAGE_FILE_BYTES_REVERSED_LO ) != 0;
        peInfo.madeFor32Bit             = ( chars & PEL_IMAGE_FILE_32BIT_MACHINE ) != 0;
        peInfo.removableRunFromSwap     = ( chars & PEL_IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP ) != 0;
        peInfo.netRunFromSwap           = ( chars & PEL_IMAGE_FILE_NET_RUN_FROM_SWAP ) != 0;
        peInfo.isSystemFile             = ( chars & PEL_IMAGE_FILE_SYSTEM ) != 0;
        peInfo.isDLL                    = ( chars & PEL_IMAGE_FILE_DLL ) != 0;
        peInfo.upSystemOnly             = ( chars & PEL_IMAGE_FILE_UP_SYSTEM_ONLY ) != 0;
        peInfo.bytesReversedHI          = ( chars & PEL_IMAGE_FILE_BYTES_REVERSED_HI ) != 0;

        // Other properties should be respected during parsing.
        bool hasRelocsStripped          = ( chars & PEL_IMAGE_FILE_RELOCS_STRIPPED ) != 0;
        bool hasLineNumsStripped        = ( chars & PEL_IMAGE_FILE_LINE_NUMS_STRIPPED ) != 0;
        bool hasLocalSymsStripped       = ( chars & PEL_IMAGE_FILE_LOCAL_SYMS_STRIPPED ) != 0;
        bool hasDebugStripped           = ( chars & PEL_IMAGE_FILE_DEBUG_STRIPPED ) != 0;

        // Remember that we were here.
        pe_file_ptr_t optionalHeaderOffset = peStream->Tell();

        // We should definately try reading symbol information.
        std::uint32_t symbolOffset = peHeader.FileHeader.PointerToSymbolTable;
        std::uint32_t numOfSymbols = peHeader.FileHeader.NumberOfSymbols;

        if ( symbolOffset != 0 && numOfSymbols != 0 )
        {
            // Try locating the symbols and read them!
            peStream->Seek( symbolOffset );

            // Do it meow.
            throw peframework_exception(
                ePEExceptCode::UNSUPPORTED,
                "unsupported COFF debug information format"
            );

            // Move back to the optional header we should read next.
            peStream->Seek( optionalHeaderOffset );
        }

        numSections = peHeader.FileHeader.NumberOfSections;

        // Verify that we have a proper optional header size.
        peOptHeaderSize = peHeader.FileHeader.SizeOfOptionalHeader;
    }

    // Let's read our optional header!
    PEOptHeader peOpt;

    // We have to extract this.
    PEStructures::IMAGE_DATA_DIRECTORY dataDirs[PEL_IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
    std::uint32_t sectionAlignment;

    // Also some important references we must resolve later.
    std::uint32_t rvaAddressOfEntryPoint = 0;
    {
        // Do we even have a magic number according to the optional header size?
        if ( peOptHeaderSize < sizeof(std::uint16_t) )
        {
            throw peframework_exception(
                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                "no PE optional header space for magic number"
            );
        }

        // Need to know if we are PE32+ or PE32 plain.
        std::uint16_t headerMagic;

        bool readMagic = peStream->ReadStruct( headerMagic );

        if ( !readMagic )
        {
            throw peframework_exception(
                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                "failed to read PE executable optional header magic"
            );
        }

        // We should provide a size of the optional header without the magic number.
        size_t optHeaderSizeRemain = ( peOptHeaderSize - sizeof(std::uint16_t) );

        std::uint16_t dllChars;

        std::uint32_t numberOfDataDirs;

        if ( headerMagic == PEL_IMAGE_NT_OPTIONAL_HDR64_MAGIC )
        {
            // We are a PE32+ file.
            isExtendedFormat = true;

            // Verify size.
            typedef PEStructures::IMAGE_OPTIONAL_HEADER64 optHeaderType;

            if ( optHeaderSizeRemain < sizeof(optHeaderType) )
            {
                throw peframework_exception(
                    ePEExceptCode::CORRUPT_PE_STRUCTURE,
                    "invalid PE optional header size"
                );
            }

            optHeaderType optHeader;

            bool readOptHeader = peStream->ReadStruct( optHeader );

            if ( !readOptHeader )
            {
                throw peframework_exception(
                    ePEExceptCode::CORRUPT_PE_STRUCTURE,
                    "reading optional header failed"
                );
            }

            // Fetch the information.
            // We will store the pointers in 64bit format regardless of the machine type.
            // This is to keep a simple data layout.
            peOpt.majorLinkerVersion = optHeader.MajorLinkerVersion;
            peOpt.minorLinkerVersion = optHeader.MinorLinkerVersion;
            peOpt.sizeOfCode = optHeader.SizeOfCode;
            peOpt.sizeOfInitializedData = optHeader.SizeOfInitializedData;
            peOpt.sizeOfUninitializedData = optHeader.SizeOfUninitializedData;
            rvaAddressOfEntryPoint = optHeader.AddressOfEntryPoint;
            peOpt.baseOfCode = optHeader.BaseOfCode;
            peOpt.baseOfData = 0;   // not available.
            peOpt.imageBase = optHeader.ImageBase;
            peOpt.fileAlignment = optHeader.FileAlignment;
            peOpt.majorOSVersion = optHeader.MajorOperatingSystemVersion;
            peOpt.minorOSVersion = optHeader.MinorOperatingSystemVersion;
            peOpt.majorImageVersion = optHeader.MajorImageVersion;
            peOpt.minorImageVersion = optHeader.MinorImageVersion;
            peOpt.majorSubsysVersion = optHeader.MajorSubsystemVersion;
            peOpt.minorSubsysVersion = optHeader.MinorSubsystemVersion;
            peOpt.win32VersionValue = optHeader.Win32VersionValue;
            peOpt.sizeOfImage = optHeader.SizeOfImage;
            peOpt.sizeOfHeaders = optHeader.SizeOfHeaders;
            peOpt.checkSum = optHeader.CheckSum;
            peOpt.subsys = optHeader.Subsystem;
            dllChars = optHeader.DllCharacteristics;
            peOpt.sizeOfStackReserve = optHeader.SizeOfStackReserve;
            peOpt.sizeOfStackCommit = optHeader.SizeOfStackCommit;
            peOpt.sizeOfHeapReserve = optHeader.SizeOfHeapReserve;
            peOpt.sizeOfHeapCommit = optHeader.SizeOfHeapCommit;
            peOpt.loaderFlags = optHeader.LoaderFlags;

            // Extract the section alignment.
            sectionAlignment = optHeader.SectionAlignment;

            // Extract the data directory information.
            numberOfDataDirs = optHeader.NumberOfRvaAndSizes;
            
            // Decrease remaining size.
            optHeaderSizeRemain -= sizeof(optHeaderType);
        }
        else if ( headerMagic == PEL_IMAGE_NT_OPTIONAL_HDR32_MAGIC )
        {
            // We are a plain PE32 file.
            isExtendedFormat = false;

            // Verify size.
            typedef PEStructures::IMAGE_OPTIONAL_HEADER32 optHeaderType;

            if ( optHeaderSizeRemain < sizeof(optHeaderType) )
            {
                throw peframework_exception(
                    ePEExceptCode::CORRUPT_PE_STRUCTURE,
                    "invalid PE optional header size"
                );
            }

            optHeaderType optHeader;

            bool readOptHeader = peStream->ReadStruct( optHeader );

            if ( !readOptHeader )
            {
                throw peframework_exception(
                    ePEExceptCode::CORRUPT_PE_STRUCTURE,
                    "reading optional header failed"
                );
            }

            // Fetch the information.
            // We will store the pointers in 64bit format regardless of the machine type.
            // This is to keep a simple data layout.
            peOpt.majorLinkerVersion = optHeader.MajorLinkerVersion;
            peOpt.minorLinkerVersion = optHeader.MinorLinkerVersion;
            peOpt.sizeOfCode = optHeader.SizeOfCode;
            peOpt.sizeOfInitializedData = optHeader.SizeOfInitializedData;
            peOpt.sizeOfUninitializedData = optHeader.SizeOfUninitializedData;
            rvaAddressOfEntryPoint = optHeader.AddressOfEntryPoint;
            peOpt.baseOfCode = optHeader.BaseOfCode;
            peOpt.baseOfData = optHeader.BaseOfData;
            peOpt.imageBase = optHeader.ImageBase;
            peOpt.fileAlignment = optHeader.FileAlignment;
            peOpt.majorOSVersion = optHeader.MajorOperatingSystemVersion;
            peOpt.minorOSVersion = optHeader.MinorOperatingSystemVersion;
            peOpt.majorImageVersion = optHeader.MajorImageVersion;
            peOpt.minorImageVersion = optHeader.MinorImageVersion;
            peOpt.majorSubsysVersion = optHeader.MajorSubsystemVersion;
            peOpt.minorSubsysVersion = optHeader.MinorSubsystemVersion;
            peOpt.win32VersionValue = optHeader.Win32VersionValue;
            peOpt.sizeOfImage = optHeader.SizeOfImage;
            peOpt.sizeOfHeaders = optHeader.SizeOfHeaders;
            peOpt.checkSum = optHeader.CheckSum;
            peOpt.subsys = optHeader.Subsystem;
            dllChars = optHeader.DllCharacteristics;
            peOpt.sizeOfStackReserve = optHeader.SizeOfStackReserve;
            peOpt.sizeOfStackCommit = optHeader.SizeOfStackCommit;
            peOpt.sizeOfHeapReserve = optHeader.SizeOfHeapReserve;
            peOpt.sizeOfHeapCommit = optHeader.SizeOfHeapCommit;
            peOpt.loaderFlags = optHeader.LoaderFlags;

            // Extract the section alignment.
            sectionAlignment = optHeader.SectionAlignment;

            // Extract the data directory information.
            numberOfDataDirs = optHeader.NumberOfRvaAndSizes;

            // Decrease the remaining optional header data size.
            optHeaderSizeRemain -= sizeof(optHeaderType);
        }
        else
        {
            throw peframework_exception(
                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                "unknown PE executable optional header magic value"
            );
        }

        // Process the DLL flags and store them sensibly.
        peOpt.dll_supportsHighEntropy =     ( dllChars & PEL_IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA ) != 0;
        peOpt.dll_hasDynamicBase =          ( dllChars & PEL_IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE ) != 0;
        peOpt.dll_forceIntegrity =          ( dllChars & PEL_IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY ) != 0;
        peOpt.dll_nxCompat =                ( dllChars & PEL_IMAGE_DLLCHARACTERISTICS_NX_COMPAT ) != 0;
        peOpt.dll_noIsolation =             ( dllChars & PEL_IMAGE_DLLCHARACTERISTICS_NO_ISOLATION ) != 0;
        peOpt.dll_noSEH =                   ( dllChars & PEL_IMAGE_DLLCHARACTERISTICS_NO_ISOLATION ) != 0;
        peOpt.dll_noBind =                  ( dllChars & PEL_IMAGE_DLLCHARACTERISTICS_NO_BIND ) != 0;
        peOpt.dll_appContainer =            ( dllChars & PEL_IMAGE_DLLCHARACTERISTICS_APPCONTAINER ) != 0;
        peOpt.dll_wdmDriver =               ( dllChars & PEL_IMAGE_DLLCHARACTERISTICS_WDM_DRIVER ) != 0;
        peOpt.dll_guardCF =                 ( dllChars & PEL_IMAGE_DLLCHARACTERISTICS_GUARD_CF ) != 0;
        peOpt.dll_termServAware =           ( dllChars & PEL_IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE ) != 0;

        // After the optional header main information we find the data directories.
        // Just read them and zero out the remaining ones which we do not have available.
        std::uint32_t canReadEntries = std::min( (std::uint32_t)countof(dataDirs), numberOfDataDirs );

        if ( canReadEntries > 0 )
        {
            // Check if we even have such available.
            const size_t dataDirReadCount = ( canReadEntries * sizeof(PEStructures::IMAGE_DATA_DIRECTORY) );

            if ( optHeaderSizeRemain < dataDirReadCount )
            {
                throw peframework_exception(
                    ePEExceptCode::CORRUPT_PE_STRUCTURE,
                    "not enough PE optional header size bytes for data directories"
                );
            }

            peStream->Read( &dataDirs, dataDirReadCount );

            // We have processed probably the last of the data.
            optHeaderSizeRemain -= dataDirReadCount;
        }

        // Skip any entries we cannot read/do not support.
        std::uint32_t skipEntryCount = ( numberOfDataDirs - canReadEntries );

        if ( skipEntryCount > 0 )
        {
            peStream->Seek(
                peStream->Tell() + skipEntryCount * sizeof(PEStructures::IMAGE_DATA_DIRECTORY)
            );
        }

        // If we read less entries than we support then we have to zero out the remaining ones.
        std::uint32_t remEntryCount = ( countof(dataDirs) - canReadEntries );

        if ( remEntryCount > 0 )
        {
            memset( dataDirs + canReadEntries, 0, remEntryCount * sizeof(PEStructures::IMAGE_DATA_DIRECTORY) );
        }
    }

    // Should handle data sections first because data directories depend on them.
    PESectionMan sections( sectionAlignment, peOpt.baseOfCode );

    // For some reason we need to remember the file-space section offsets.
    // Those will come in handy for certain PE files that still come with bound imports.
    bool requiresSectionFileOffsets = ( dataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].VirtualAddress != 0 );

    struct pesect_file_off_info
    {
        std::uint32_t virt_addr;
        std::uint32_t file_off;
    };

    std::vector <pesect_file_off_info> pesect_file_off;

    if ( requiresSectionFileOffsets )
    {
        // We only allocate this thing on demand, because modern PE files do not
        // come with it anymore.
        pesect_file_off.resize( numSections );
    }

    for ( size_t n = 0; n < numSections; n++ )
    {
        PEStructures::IMAGE_SECTION_HEADER sectHeader;

        bool readSection = peStream->ReadStruct( sectHeader );

        if ( !readSection )
        {
            throw peframework_exception(
                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                "failed to read PE section header"
            );
        }

        pe_file_ptr_t sectHeaderOff = peStream->Tell();

        PESection section;
        section.shortName = std::string( (const char*)sectHeader.Name, strnlen( (const char*)sectHeader.Name, countof(sectHeader.Name) ) );
        section.SetPlacementInfo( sectHeader.VirtualAddress, sectHeader.Misc.VirtualSize );
        
        // Save characteristics flags.
        std::uint32_t schars = sectHeader.Characteristics;

        section.SetPENativeFlags( schars );

        // Read raw data.
        std::uint32_t ptrToRawData = sectHeader.PointerToRawData;
        {
            peStream->Seek( ptrToRawData );

            section.stream.Truncate( (std::uint32_t)sectHeader.SizeOfRawData );

            size_t actualReadCount = peStream->Read( section.stream.Data(), sectHeader.SizeOfRawData );

            if ( actualReadCount != sectHeader.SizeOfRawData )
            {
                throw peframework_exception(
                    ePEExceptCode::CORRUPT_PE_STRUCTURE,
                    "failed to read PE section raw data"
                );
            }
        }

        // Remember the file pointer if we have to.
        if ( requiresSectionFileOffsets )
        {
            pesect_file_off_info& offInfo = pesect_file_off[ n ];

            // Really crusty legacy dumptasty needs this.
            offInfo.virt_addr = sectHeader.VirtualAddress;
            offInfo.file_off = ptrToRawData;
        }

        // Read relocation information.
        {
            peStream->Seek( sectHeader.PointerToRelocations );

            std::vector <PERelocation> relocs;
            relocs.reserve( sectHeader.NumberOfRelocations );

            for ( std::uint32_t n = 0; n < sectHeader.NumberOfRelocations; n++ )
            {
                PEStructures::IMAGE_RELOCATION relocEntry;

                bool readReloc = peStream->ReadStruct( relocEntry );

                if ( !readReloc )
                {
                    throw peframework_exception(
                        ePEExceptCode::CORRUPT_PE_STRUCTURE,
                        "failed to read PE section relocation information"
                    );
                }

                // Store it.
                PERelocation data;
                data.virtAddr = relocEntry.VirtualAddress;
                data.symbolTableIndex = relocEntry.SymbolTableIndex;
                data.type = relocEntry.Type;

                relocs.push_back( std::move( data ) );
            }

            section.relocations = std::move( relocs );
        }

        // Read linenumber information.
        {
            peStream->Seek( sectHeader.PointerToLinenumbers );

            std::vector <PELinenumber> linenums;
            linenums.reserve( sectHeader.NumberOfLinenumbers );

            for ( size_t n = 0; n < sectHeader.NumberOfLinenumbers; n++ )
            {
                PEStructures::IMAGE_LINENUMBER lineInfo;

                bool gotLinenum = peStream->ReadStruct( lineInfo );

                if ( !gotLinenum )
                {
                    throw peframework_exception(
                        ePEExceptCode::CORRUPT_PE_STRUCTURE,
                        "failed to read PE linenumber info"
                    );
                }

                PELinenumber line;
                line.symTableIndex = lineInfo.Type.SymbolTableIndex;
                line.number = lineInfo.Linenumber;

                linenums.push_back( std::move( line ) );
            }

            section.linenumbers = std::move( linenums );
        }

        // We need to set our stream back on track.
        peStream->Seek( sectHeaderOff );

        // Remember this section.
        bool regSuccess = ( sections.PlaceSection( std::move( section ) ) != NULL );

        if ( !regSuccess )
        {
            throw peframework_exception(
                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                "invalid PE section configuration"
            );
        }
    }

    // This is the (official) end of the executable data reading.
    // Now we dispatch onto the data directories, which base on things found inside the sections.

    // NOTE: the DEBUG DIRECTORY probably needs to use peStream again.

    // Catch up on optional header dependencies to PE sections such as addressOfEntryPoint.
    peOpt.addressOfEntryPointRef = sections.ResolveRVAToRef( rvaAddressOfEntryPoint );

    // If we ever encounter an absolute VA during deserialization, we need to subtract the image base.
    const std::uint64_t imageBase = peOpt.imageBase;

    // Load the directory information now.
    // We decide to create meta-data structs out of them.
    // If possible, delete the section that contains the meta-data.
    // * EXPORT INFORMATION.
    PEExportDir expInfo;
    {
        const PEStructures::IMAGE_DATA_DIRECTORY& expDirEntry = dataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_EXPORT ];

        if ( expDirEntry.VirtualAddress != 0 )
        {
            PESection *expDirSect;

            PEStructures::IMAGE_EXPORT_DIRECTORY expEntry;
            {
                bool gotData = sections.ReadPEData( expDirEntry.VirtualAddress, sizeof(expEntry), &expEntry, &expDirSect );

                if ( !gotData )
                {
                    throw peframework_exception(
                        ePEExceptCode::CORRUPT_PE_STRUCTURE,
                        "invalid PE export directory"
                    );
                }
            }

            expDirSect->SetPlacedMemory( expInfo.allocEntry, expDirEntry.VirtualAddress, expDirEntry.Size );

            // Store the usual tidbits.
            expInfo.chars = expEntry.Characteristics;
            expInfo.timeDateStamp = expEntry.TimeDateStamp;
            expInfo.majorVersion = expEntry.MajorVersion;
            expInfo.minorVersion = expEntry.MinorVersion;
            expInfo.ordinalBase = expEntry.Base;

            // Read the name.
            PESection *sectOfName;
            {
                bool gotName = sections.ReadPEString( expEntry.Name, expInfo.name, &sectOfName );

                if ( !gotName )
                {
                    throw peframework_exception(
                        ePEExceptCode::CORRUPT_PE_STRUCTURE,
                        "failed to read PE export directory name"
                    );
                }
            }

            sectOfName->SetPlacedMemory( expInfo.nameAllocEntry, expEntry.Name );

            // Allocate functions.
            if ( expEntry.AddressOfFunctions != 0 )
            {
                std::vector <PEExportDir::func> funcs;
                funcs.reserve( expEntry.NumberOfFunctions );

                std::uint32_t archPointerSize = GetPEPointerSize( isExtendedFormat );
                std::uint64_t tabSize = ( archPointerSize * expEntry.NumberOfFunctions );
                
                PESection *addrPtrSect;
                PEDataStream addrPtrStream;
                {
                    bool gotStream = sections.GetPEDataStream(
                        expEntry.AddressOfFunctions, addrPtrStream,
                        &addrPtrSect
                    );

                    if ( !gotStream )
                    {
                        throw peframework_exception(
                            ePEExceptCode::CORRUPT_PE_STRUCTURE,
                            "failed to get PE export info function entries"
                        );
                    }
                }

                addrPtrSect->SetPlacedMemory( expInfo.funcAddressAllocEntry, expEntry.AddressOfFunctions );

                for ( std::uint32_t n = 0; n < expEntry.NumberOfFunctions; n++ )
                {
                    PEExportDir::func fentry;
                    // by default no export is named.

                    bool isForwarder;
                    {
                        std::uint32_t ptr;
                        addrPtrStream.Read( &ptr, sizeof(ptr) );

                        // We could be an empty entry.
                        // (encountered in system DLLs)
                        if ( ptr != 0 )
                        {
                            // Determine if we are a forwarder or an export.
                            {
                                typedef sliceOfData <std::uint32_t> rvaSlice_t;

                                rvaSlice_t requestSlice( ptr, 1 );

                                rvaSlice_t expDirSlice( expDirEntry.VirtualAddress, expDirEntry.Size );

                                rvaSlice_t::eIntersectionResult intResult = requestSlice.intersectWith( expDirSlice );

                                isForwarder = ( rvaSlice_t::isFloatingIntersect( intResult ) == false );
                            }

                            // Store properties according to the type.
                            PESection *exportOffPtrSect;
                            PEDataStream expOffStream;
                            {
                                bool gotStream = sections.GetPEDataStream( ptr, expOffStream, &exportOffPtrSect );

                                if ( !gotStream )
                                {
                                    throw peframework_exception(
                                        ePEExceptCode::CORRUPT_PE_STRUCTURE,
                                        "failed to get PE export offset pointer"
                                    );
                                }
                            }

                            // We store the location of the data entry, but NOTE that
                            // this behavior NEVER is an allocation!
                            if ( !isForwarder ) // we need to know nothing about forwarders.
                            {
                                std::uint32_t offStore = ( ptr - exportOffPtrSect->GetVirtualAddress() );

                                fentry.expRef = PESectionDataReference( exportOffPtrSect, offStore );
                            }
                            // Otherwise read the forwarder entry.
                            else
                            {
                                ReadPEString( expOffStream, fentry.forwarder );
                            }
                        }
                        else
                        {
                            // Empty settings.
                            isForwarder = false;
                        }
                    }
                    fentry.isForwarder = isForwarder;

                    funcs.push_back( std::move( fentry ) );
                }

                // Read names and ordinals, if available.
                if ( expEntry.AddressOfNames != 0 && expEntry.AddressOfNameOrdinals != 0 )
                {
                    // Establish name ptr array.
                    PESection *addrNamesSect;
                    PEDataStream addrNamesStream;
                    {
                        bool gotStream = sections.GetPEDataStream( expEntry.AddressOfNames, addrNamesStream, &addrNamesSect );

                        if ( !gotStream )
                        {
                            throw peframework_exception(
                                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                                "failed to get PE export directory function name list"
                            );
                        }
                    }

                    addrNamesSect->SetPlacedMemory( expInfo.funcNamesAllocEntry, expEntry.AddressOfNames );

                    // Establish ordinal mapping array.
                    PESection *addrNameOrdSect;
                    PEDataStream addrNameOrdStream;
                    {
                        bool gotStream = sections.GetPEDataStream( expEntry.AddressOfNameOrdinals, addrNameOrdStream, &addrNameOrdSect );
                        
                        if ( !gotStream )
                        {
                            throw peframework_exception(
                                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                                "failed to get PE export directory function ordinals"
                            );
                        }
                    }

                    addrNameOrdSect->SetPlacedMemory( expInfo.funcOrdinalsAllocEntry, expEntry.AddressOfNameOrdinals );

                    // Map names to functions.
                    for ( std::uint32_t n = 0; n < expEntry.NumberOfNames; n++ )
                    {
                        std::uint16_t ordinal;
                        addrNameOrdStream.Read( &ordinal, sizeof(ordinal) );

                        // Get the index to map the function name to (== ordinal).
                        size_t mapIndex = ( ordinal );

                        if ( mapIndex >= funcs.size() )
                        {
                            // Invalid mapping.
                            throw peframework_exception(
                                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                                "PE binary has broken export mapping (ordinal out of bounds)"
                            );
                        }

                        // Get the name we should map to.
                        PESection *realNamePtrSect;

                        std::uint32_t namePtrRVA;
                        addrNamesStream.Read( &namePtrRVA, sizeof(namePtrRVA) );

                        // Read the actual name.
                        std::string realName;
                        {
                            bool gotString = sections.ReadPEString( namePtrRVA, realName, &realNamePtrSect );

                            if ( !gotString )
                            {
                                throw peframework_exception(
                                    ePEExceptCode::CORRUPT_PE_STRUCTURE,
                                    "failed to get PE export directory function name ptr"
                                );
                            }
                        }

                        // Check some kind of sense behind the name.
                        if ( realName.empty() )
                        {
                            // Kind of invalid.
                            throw peframework_exception(
                                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                                "invalid PE export name: empty string"
                            );
                        }

                        // Store this link.
                        PEExportDir::mappedName nameMap;
                        nameMap.name = std::move( realName );
                        
                        realNamePtrSect->SetPlacedMemory( nameMap.nameAllocEntry, namePtrRVA );

                        expInfo.funcNameMap.insert( std::make_pair( std::move( nameMap ), std::move( mapIndex ) ) );
                    }
                }

                expInfo.functions = std::move( funcs );
            }

            // We got the export directory! :)
        }
    }

    // * IMPORT directory.
    std::vector <PEImportDesc> impDescs;
    {
        const PEStructures::IMAGE_DATA_DIRECTORY& impDir = dataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_IMPORT ];

        if ( impDir.VirtualAddress != 0 )
        {
            PESection *impDirSect;
            PEDataStream importDescsStream;
            {
                bool gotStream = sections.GetPEDataStream( impDir.VirtualAddress, importDescsStream, &impDirSect );

                if ( !gotStream )
                {
                    throw peframework_exception(
                        ePEExceptCode::CORRUPT_PE_STRUCTURE,
                        "failed to read PE import descriptors"
                    );
                }
            }

            impDirSect->SetPlacedMemory( this->importsAllocEntry, impDir.VirtualAddress, impDir.Size );

            // Read all the descriptors.
            const std::uint32_t potentialNumDescriptors = ( impDir.Size / sizeof( PEStructures::IMAGE_IMPORT_DESCRIPTOR ) );

            impDescs.reserve( potentialNumDescriptors );

            std::uint32_t n = 0;

            while ( n < potentialNumDescriptors )
            {
                PEStructures::IMAGE_IMPORT_DESCRIPTOR importInfo;
                importDescsStream.Read( &importInfo, sizeof(importInfo) );

                // TODO: allow secure bounded parsing of PE files, so we check for
                // violations of PE rules and reject those files.

                // By definition, an IMAGE_IMPORT_DESCRIPTOR with all entries zero
                // is the end of the table.
                if ( importInfo.Characteristics == 0 &&
                     importInfo.TimeDateStamp == 0 &&
                     importInfo.ForwarderChain == 0 &&
                     importInfo.Name == 0 && 
                     importInfo.FirstThunk == 0 )
                {
                    break;
                }

                PEImportDesc impDesc;

                // Get the function names (with their ordinals).
                if ( importInfo.Characteristics != 0 )
                {
                    impDesc.funcs =
                        PEImportDesc::ReadPEImportFunctions(
                            sections, importInfo.Characteristics, impDesc.impNameArrayAllocEntry, isExtendedFormat
                        );
                }

                // Store the DLL name we import from.
                {
                    PESection *dllNameSect;
                    {
                        bool gotName = sections.ReadPEString( importInfo.Name, impDesc.DLLName, &dllNameSect );

                        if ( !gotName )
                        {
                            throw peframework_exception(
                                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                                "failed to read PE import desc DLL name"
                            );
                        }
                    }

                    dllNameSect->SetPlacedMemory( impDesc.DLLName_allocEntry, importInfo.Name );
                }

                impDesc.firstThunkRef = sections.ResolveRVAToRef( importInfo.FirstThunk );

                // Store this import desc.
                impDescs.push_back( std::move( impDesc ) );

                // Next iteration.
                n++;

                // Done with this import desc!
            }

            // Done with all imports.
        }
    }

    // * Resources.
    PEResourceDir resourceRoot( false, std::u16string(), 0 );
    {
        struct helpers
        {
            inline static PEResourceDir LoadResourceDirectory(
                PESectionMan& sections, PEDataStream& rootStream,
                bool isIdentifierName, std::u16string nameOfDir, std::uint16_t identifier,
                const PEStructures::IMAGE_RESOURCE_DIRECTORY& serResDir )
            {
                PEResourceDir curDir( std::move( isIdentifierName ), std::move( nameOfDir ), std::move( identifier ) );

                // Store general details.
                curDir.characteristics = serResDir.Characteristics;
                curDir.timeDateStamp = serResDir.TimeDateStamp;
                curDir.majorVersion = serResDir.MajorVersion;
                curDir.minorVersion = serResDir.MinorVersion;

                // Read sub entries.
                // Those are planted directly after the directory.
                std::uint16_t numNamedEntries = serResDir.NumberOfNamedEntries;
                std::uint16_t numIDEntries = serResDir.NumberOfIdEntries;

                // Function to read the data behind a resource directory entry.
                auto resDataParser = [&]( bool isIdentifierName, std::u16string nameOfItem, std::uint16_t identifier, const PEStructures::IMAGE_RESOURCE_DIRECTORY_ENTRY& entry ) -> PEResourceItem*
                {
                    // Seek to this data entry.
                    rootStream.Seek( entry.OffsetToData );

                    // Are we a sub-directory or an actual data leaf?
                    if ( entry.DataIsDirectory )
                    {
                        // Get the sub-directory structure.
                        PEStructures::IMAGE_RESOURCE_DIRECTORY subDirData;
                        rootStream.Read( &subDirData, sizeof(subDirData) );

                        PEResourceDir subDir = LoadResourceDirectory(
                            sections, rootStream,
                            std::move( isIdentifierName ), std::move( nameOfItem ), std::move( identifier ),
                            subDirData
                        );

                        return new PEResourceDir( std::move( subDir ) );
                    }
                    else
                    {
                        // Get the data leaf.
                        PEStructures::IMAGE_RESOURCE_DATA_ENTRY itemData;
                        rootStream.Read( &itemData, sizeof(itemData) );

                        // The data pointer can reside in any section.
                        // We want to resolve it properly into a PESectionAllocation-like
                        // inline construct.
                        PESection *dataSect;
                        std::uint32_t sectOff;

                        bool gotLocation = sections.GetPEDataLocation( itemData.OffsetToData, &sectOff, &dataSect );

                        if ( !gotLocation )
                        {
                            throw peframework_exception(
                                ePEExceptCode::ACCESS_OUT_OF_BOUNDS,
                                "invalid PE resource item data pointer (could not find section location)"
                            );
                        }

                        // We dont have to recurse anymore.
                        PEResourceInfo resItem(
                            std::move( isIdentifierName ), std::move( nameOfItem ), std::move( identifier ),
                            PESectionDataReference( dataSect, std::move( sectOff ), itemData.Size )
                        );
                        resItem.codePage = itemData.CodePage;
                        resItem.reserved = itemData.Reserved;

                        return new PEResourceInfo( std::move( resItem ) );
                    }
                };

                // Due to us using only one PEDataStream we need to seek to all our entries properly.
                std::uint32_t subDirStartOff = rootStream.Tell();

                for ( std::uint32_t n = 0; n < numNamedEntries; n++ )
                {
                    rootStream.Seek( subDirStartOff + n * sizeof(PEStructures::IMAGE_RESOURCE_DIRECTORY_ENTRY) );

                    PEStructures::IMAGE_RESOURCE_DIRECTORY_ENTRY namedEntry;
                    rootStream.Read( &namedEntry, sizeof(namedEntry) );

                    if ( namedEntry.NameIsString == false )
                    {
                        throw peframework_exception(
                            ePEExceptCode::CORRUPT_PE_STRUCTURE,
                            "invalid PE resource directory entry: expected named entry"
                        );
                    }

                    // Load the name.
                    std::u16string nameOfItem;
                    {
                        rootStream.Seek( namedEntry.NameOffset );

                        std::uint16_t nameCharCount;
                        rootStream.Read( &nameCharCount, sizeof(nameCharCount) );

                        nameOfItem.resize( nameCharCount );
                
                        rootStream.Read( (char16_t*)nameOfItem.c_str(), nameCharCount );
                    }

                    // Create a resource item.
                    PEResourceItem *resItem = resDataParser( false, std::move( nameOfItem ), 0, namedEntry );

                    // Store ourselves.
                    try
                    {
                        curDir.namedChildren.insert( resItem );
                    }
                    catch( ... )
                    {
                        delete resItem;

                        throw;
                    }
                }

                for ( std::uint32_t n = 0; n < numIDEntries; n++ )
                {
                    rootStream.Seek( subDirStartOff + ( n + numNamedEntries ) * sizeof(PEStructures::IMAGE_RESOURCE_DIRECTORY_ENTRY) );
            
                    PEStructures::IMAGE_RESOURCE_DIRECTORY_ENTRY idEntry;
                    rootStream.Read( &idEntry, sizeof(idEntry) );

                    if ( idEntry.NameIsString == true )
                    {
                        throw peframework_exception(
                            ePEExceptCode::CORRUPT_PE_STRUCTURE,
                            "invalid PE resource directory ID entry"
                        );
                    }

                    // Create a resource item.
                    PEResourceItem *resItem = resDataParser( true, std::u16string(), idEntry.Id, idEntry );

                    // Store it.
                    try
                    {
                        curDir.idChildren.insert( resItem );
                    }
                    catch( ... )
                    {
                        delete resItem;

                        throw;
                    }
                }

                return curDir;
            }
        };

        const PEStructures::IMAGE_DATA_DIRECTORY& resDir = dataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_RESOURCE ];

        if ( resDir.VirtualAddress != 0 )
        {
            PESection *resDataSect;
            PEDataStream resDataStream;
            {
                bool gotStream = sections.GetPEDataStream( resDir.VirtualAddress, resDataStream, &resDataSect );

                if ( !gotStream )
                {
                    throw peframework_exception(
                        ePEExceptCode::CORRUPT_PE_STRUCTURE,
                        "invalid PE resource root"
                    );
                }
            }

            resDataSect->SetPlacedMemory( this->resAllocEntry, resDir.VirtualAddress, resDir.Size );

            PEStructures::IMAGE_RESOURCE_DIRECTORY resDir;
            resDataStream.Read( &resDir, sizeof(resDir) );

            resourceRoot = helpers::LoadResourceDirectory(
                sections, resDataStream,
                false, std::u16string(), 0,
                resDir
            );
        }
    }

    // * Exception Information.
    std::vector <PERuntimeFunction> exceptRFs;
    {
        const PEStructures::IMAGE_DATA_DIRECTORY& rtDir = dataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_EXCEPTION ];

        if ( rtDir.VirtualAddress != 0 )
        {
            // TODO: apparently exception data is machine dependent, so we should
            // deserialize this in a special way depending on machine_id.
            // (currently we specialize on x86/AMD64)

            PESection *rtFuncsSect;
            PEDataStream rtFuncsStream;
            {
                bool gotStream = sections.GetPEDataStream( rtDir.VirtualAddress, rtFuncsStream, &rtFuncsSect );

                if ( !gotStream )
                {
                    throw peframework_exception(
                        ePEExceptCode::CORRUPT_PE_STRUCTURE,
                        "invalid PE exception directory"
                    );
                }
            }

            rtFuncsSect->SetPlacedMemory( this->exceptAllocEntry, rtDir.VirtualAddress, rtDir.Size );

            const std::uint32_t numFuncs = ( rtDir.Size / sizeof( PEStructures::IMAGE_RUNTIME_FUNCTION_ENTRY_X64 ) );

            exceptRFs.reserve( numFuncs );

            for ( size_t n = 0; n < numFuncs; n++ )
            {
                PEStructures::IMAGE_RUNTIME_FUNCTION_ENTRY_X64 func;
                rtFuncsStream.Read( &func, sizeof(func) );

                // Since the runtime function entry stores RVAs, we want to remember them
                // relocation independent.
                PESection *beginAddrSect = NULL;
                std::uint32_t beginAddrSectOff = 0;

                if ( std::uint32_t BeginAddress = func.BeginAddress )
                {
                    bool gotLocation = sections.GetPEDataLocation( BeginAddress, &beginAddrSectOff, &beginAddrSect );

                    if ( !gotLocation )
                    {
                        throw peframework_exception(
                            ePEExceptCode::CORRUPT_PE_STRUCTURE,
                            "invalid PE runtime function begin address"
                        );
                    }
                }
                PESection *endAddrSect = NULL;
                std::uint32_t endAddrSectOff = 0;

                if ( std::uint32_t EndAddress = func.EndAddress )
                {
                    bool gotLocation = sections.GetPEDataLocation( EndAddress, &endAddrSectOff, &endAddrSect );

                    if ( !gotLocation )
                    {
                        throw peframework_exception(
                            ePEExceptCode::CORRUPT_PE_STRUCTURE,
                            "invalid PE runtime function end address"
                        );
                    }
                }
                PESection *unwindInfoSect = NULL;
                std::uint32_t unwindInfoSectOff = 0;

                if ( std::uint32_t UnwindInfoAddress = func.UnwindInfoAddress )
                {
                    bool gotLocation = sections.GetPEDataLocation( UnwindInfoAddress, &unwindInfoSectOff, &unwindInfoSect );

                    if ( !gotLocation )
                    {
                        throw peframework_exception(
                            ePEExceptCode::CORRUPT_PE_STRUCTURE,
                            "invalid PE runtime function unwind info address"
                        );
                    }
                }

                PERuntimeFunction funcInfo;
                funcInfo.beginAddrRef = PESectionDataReference( beginAddrSect, beginAddrSectOff );
                funcInfo.endAddrRef = PESectionDataReference( endAddrSect, endAddrSectOff );
                funcInfo.unwindInfoRef = PESectionDataReference( unwindInfoSect, unwindInfoSectOff );

                exceptRFs.push_back( std::move( funcInfo ) );
            }
        }
    }

    // * ATTRIBUTE CERTIFICATES.
    PESecurity securityCookie;
    {
        const PEStructures::IMAGE_DATA_DIRECTORY& certDir = dataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_SECURITY ];

        // VirtualAddress in this data directory is a file pointer.
        std::uint32_t certFilePtr = certDir.VirtualAddress;
        std::uint32_t certBufSize = certDir.Size;

        securityCookie.certStore.ReadFromFile( peStream, sections, 0, certFilePtr, certBufSize );
    }

    // * BASE RELOC.
    std::map <std::uint32_t, PEBaseReloc> baseRelocs;
    {
        const PEStructures::IMAGE_DATA_DIRECTORY& baserelocDir = dataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_BASERELOC ];

        if ( baserelocDir.VirtualAddress != 0 )
        {
            const std::uint32_t sizeRelocations = baserelocDir.Size;

            PESection *baseRelocDescsSect;
            PEDataStream baseRelocDescsStream;
            {
                bool gotStream = sections.GetPEDataStream( baserelocDir.VirtualAddress, baseRelocDescsStream, &baseRelocDescsSect );

                if ( !gotStream )
                {
                    throw peframework_exception(
                        ePEExceptCode::CORRUPT_PE_STRUCTURE,
                        "invalid PE base relocation directory"
                    );
                }
            }

            baseRelocDescsSect->SetPlacedMemory( this->baseRelocAllocEntry, baserelocDir.VirtualAddress, baserelocDir.Size );

            // We read relocation data until we are at the end of the directory.
            while ( true )
            {
                // Remember our current offset.
                std::uint32_t curOffset = baseRelocDescsStream.Tell();

                if ( curOffset >= sizeRelocations )
                    break;

                // Get current relocation.
                PEStructures::IMAGE_BASE_RELOCATION baseReloc;
                baseRelocDescsStream.Read( &baseReloc, sizeof(baseReloc) );

                // Store it.
                const std::uint32_t blockSize = baseReloc.SizeOfBlock;

                // Validate the blockSize.
                if ( blockSize < sizeof(PEStructures::IMAGE_BASE_RELOCATION) )
                {
                    throw peframework_exception(
                        ePEExceptCode::CORRUPT_PE_STRUCTURE,
                        "malformed PE base relocation sub block"
                    );
                }

                // Subtract the meta-data size.
                const std::uint32_t entryBlockSize = ( blockSize - sizeof(PEStructures::IMAGE_BASE_RELOCATION) );
                {
                    std::uint32_t relVirtAddr = baseReloc.VirtualAddress;

                    // Verify that it is a valid block address.
                    if ( ( relVirtAddr % baserelocChunkSize ) != 0 )
                    {
                        throw peframework_exception(
                            ePEExceptCode::CORRUPT_PE_STRUCTURE,
                            "invalid PE base relocation block virtual address (must be aligned on 4K boundaries)"
                        );
                    }

                    PEBaseReloc info;
                    info.offsetOfReloc = relVirtAddr;

                    // Read all relocations.
                    const std::uint32_t numRelocItems = ( entryBlockSize / sizeof( PEStructures::IMAGE_BASE_RELOC_TYPE_ITEM ) );

                    info.items.reserve( numRelocItems );

                    // Base relocation are stored in a stream-like array. Some entries form tuples,
                    // so that two entries have to be next to each other.
                    size_t reloc_index = 0;

                    while ( reloc_index < numRelocItems )
                    {
                        PEStructures::IMAGE_BASE_RELOC_TYPE_ITEM reloc;
                        baseRelocDescsStream.Read( &reloc, sizeof(reloc) );

                        PEBaseReloc::item itemInfo;
                        itemInfo.offset = reloc.offset;
                        itemInfo.type = reloc.type;

                        info.items.push_back( std::move( itemInfo ) );

                        reloc_index++;
                    }

                    // Remember us.
                    // We take advantage of the alignedness and divide by that number.
                    std::uint32_t baseRelocIndex = ( relVirtAddr / baserelocChunkSize );

                    baseRelocs.insert( std::make_pair( baseRelocIndex, std::move( info ) ) );
                }

                // Done reading this descriptor.
            }

            // Done reading all base relocations.
        }
    }

    // * DEBUG.
    decltype(this->debugDescs) debugDescs;
    {
        const PEStructures::IMAGE_DATA_DIRECTORY& debugDir = dataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_DEBUG ];

        if ( debugDir.VirtualAddress != 0 )
        {
            PESection *debugEntrySection;
            PEDataStream debugEntryStream;
            {
                bool gotStream = sections.GetPEDataStream( debugDir.VirtualAddress, debugEntryStream, &debugEntrySection );

                if ( !gotStream )
                {
                    throw peframework_exception(
                        ePEExceptCode::CORRUPT_PE_STRUCTURE,
                        "invalid PE debug directory"
                    );
                }
            }

            std::uint32_t debugDirSize = debugDir.Size;

            debugEntrySection->SetPlacedMemory( this->debugDescsAlloc, debugDir.VirtualAddress, debugDirSize );

            // Get the amount of debug descriptors available.
            const std::uint32_t numDescriptors = ( debugDirSize / sizeof(PEStructures::IMAGE_DEBUG_DIRECTORY) );

            for ( size_t n = 0; n < numDescriptors; n++ )
            {
                PEStructures::IMAGE_DEBUG_DIRECTORY debugEntry;
                debugEntryStream.Read( &debugEntry, sizeof(debugEntry) );

                // We store this debug information entry.
                // Debug information can be of many types and we cannot ever handle all of them!
                PEDebugDesc debugInfo;
                debugInfo.characteristics = debugEntry.Characteristics;
                debugInfo.timeDateStamp = debugEntry.TimeDateStamp;
                debugInfo.majorVer = debugEntry.MajorVersion;
                debugInfo.minorVer = debugEntry.MinorVersion;
                debugInfo.type = debugEntry.Type;

                // Fetch the debug data from the file, so that we do not depend on "file pointers"
                // for PE structures.
                debugInfo.dataStore.ReadFromFile(
                    peStream, sections,
                    debugEntry.AddressOfRawData, debugEntry.PointerToRawData, debugEntry.SizeOfData
                );

                // Store our information.
                debugDescs.push_back( std::move( debugInfo ) );
            }
        }
    }

    // * ARCHITECTURE.
    {
        // Reserved. Must be zero.
    }

    // * GLOBAL PTR.
    PEGlobalPtr globalPtr;
    {
        const PEStructures::IMAGE_DATA_DIRECTORY& globptrData = dataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_GLOBALPTR ];

        globalPtr.ptrOffset = globptrData.VirtualAddress;
    }

    // * THREAD LOCAL STORAGE.
    PEThreadLocalStorage tlsInfo;
    {
        const PEStructures::IMAGE_DATA_DIRECTORY& tlsDataDir = dataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_TLS ];

        if ( tlsDataDir.VirtualAddress != 0 )
        {
            PESection *tlsDirSect;
            PEDataStream tlsDirStream;
            {
                bool gotStream = sections.GetPEDataStream( tlsDataDir.VirtualAddress, tlsDirStream, &tlsDirSect );

                if ( !gotStream )
                {
                    throw peframework_exception( 
                        ePEExceptCode::CORRUPT_PE_STRUCTURE,
                        "invalid PE thread-local-storage directory"
                    );
                }
            }

            // It depends on the architecture what directory type we encounter here.
            uint32_t startOfRawDataRVA = 0;
            uint32_t endOfRawDataRVA = 0;
            uint32_t addressOfIndexRVA = 0;
            uint32_t addressOfCallbacksRVA = 0;

            if ( isExtendedFormat )
            {
                PEStructures::IMAGE_TLS_DIRECTORY64 tlsDir;
                tlsDirStream.Read( &tlsDir, sizeof(tlsDir) );

                // Convert the VAs into RVAs.
                startOfRawDataRVA = VA2RVA( tlsDir.StartAddressOfRawData, imageBase );
                endOfRawDataRVA = VA2RVA( tlsDir.EndAddressOfRawData, imageBase );
                addressOfIndexRVA = VA2RVA( tlsDir.AddressOfIndex, imageBase );
                addressOfCallbacksRVA = VA2RVA( tlsDir.AddressOfCallBacks, imageBase );

                // Write some things.
                tlsInfo.sizeOfZeroFill = tlsDir.SizeOfZeroFill;
                tlsInfo.characteristics = tlsDir.Characteristics;
            }
            else
            {
                PEStructures::IMAGE_TLS_DIRECTORY32 tlsDir;
                tlsDirStream.Read( &tlsDir, sizeof(tlsDir) );

                // Convert the VAs into RVAs.
                startOfRawDataRVA = VA2RVA( tlsDir.StartAddressOfRawData, imageBase );
                endOfRawDataRVA = VA2RVA( tlsDir.EndAddressOfRawData, imageBase );
                addressOfIndexRVA = VA2RVA( tlsDir.AddressOfIndex, imageBase );
                addressOfCallbacksRVA = VA2RVA( tlsDir.AddressOfCallBacks, imageBase );

                // Write some things.
                tlsInfo.sizeOfZeroFill = tlsDir.SizeOfZeroFill;
                tlsInfo.characteristics = tlsDir.Characteristics;
            }

            // Get the references.
            tlsInfo.startOfRawDataRef = sections.ResolveRVAToRef( startOfRawDataRVA );
            tlsInfo.endOfRawDataRef = sections.ResolveRVAToRef( endOfRawDataRVA );
            tlsInfo.addressOfIndexRef = sections.ResolveRVAToRef( addressOfIndexRVA );
            tlsInfo.addressOfCallbacksRef = sections.ResolveRVAToRef( addressOfCallbacksRVA );

            tlsDirSect->SetPlacedMemory( tlsInfo.allocEntry, tlsDataDir.VirtualAddress, tlsDataDir.Size );
        }
    }

    // * LOAD CONFIG.
    PELoadConfig loadConfig;
    {
        const PEStructures::IMAGE_DATA_DIRECTORY& lcfgDataDir = dataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG ];

        if ( lcfgDataDir.VirtualAddress != 0 )
        {
            PESection *lcfgDirSect;
            PEDataStream lcfgDirStream;
            {
                bool gotStream = sections.GetPEDataStream( lcfgDataDir.VirtualAddress, lcfgDirStream, &lcfgDirSect );

                if ( !gotStream )
                {
                    throw peframework_exception(
                        ePEExceptCode::CORRUPT_PE_STRUCTURE,
                        "invalid PE load config directory"
                    );
                }
            }

            // RVAs need special handling.
            std::uint32_t lockPrefixTableRVA = 0;
            std::uint32_t editListRVA = 0;
            std::uint32_t securityCookieRVA = 0;
            std::uint32_t SEHandlerTableRVA = 0;
            std::uint32_t guardCFCheckFunctionPointerRVA = 0;
            std::uint32_t guardCFFunctionTableRVA = 0;

            if ( isExtendedFormat )
            {
                PEStructures::IMAGE_LOAD_CONFIG_DIRECTORY64 lcfgDir;
                lcfgDirStream.Read( &lcfgDir, sizeof(lcfgDir) );

                if ( lcfgDir.Size >= offsetof(decltype(lcfgDir), SEHandlerCount) )
                {
                    loadConfig.timeDateStamp = lcfgDir.TimeDateStamp;
                    loadConfig.majorVersion = lcfgDir.MajorVersion;
                    loadConfig.minorVersion = lcfgDir.MinorVersion;
                    loadConfig.globFlagsClear = lcfgDir.GlobalFlagsClear;
                    loadConfig.globFlagsSet = lcfgDir.GlobalFlagsSet;
                    loadConfig.critSecDefTimeOut = lcfgDir.CriticalSectionDefaultTimeout;
                    loadConfig.deCommitFreeBlockThreshold = lcfgDir.DeCommitFreeBlockThreshold;
                    loadConfig.deCommitTotalFreeThreshold = lcfgDir.DeCommitTotalFreeThreshold;
                    lockPrefixTableRVA = VA2RVA( lcfgDir.LockPrefixTable, imageBase );
                    loadConfig.maxAllocSize = lcfgDir.MaximumAllocationSize;
                    loadConfig.virtualMemoryThreshold = lcfgDir.VirtualMemoryThreshold;
                    loadConfig.processAffinityMask = lcfgDir.ProcessAffinityMask;
                    loadConfig.processHeapFlags = lcfgDir.ProcessHeapFlags;
                    loadConfig.CSDVersion = lcfgDir.CSDVersion;
                    loadConfig.reserved1 = lcfgDir.Reserved1;
                    editListRVA = VA2RVA( lcfgDir.EditList, imageBase );
                    securityCookieRVA = VA2RVA( lcfgDir.SecurityCookie, imageBase );
                    SEHandlerTableRVA = VA2RVA( lcfgDir.SEHandlerTable, imageBase );
                    loadConfig.SEHandlerCount = lcfgDir.SEHandlerCount;
                }

                if ( lcfgDir.Size >= offsetof(decltype(lcfgDir), GuardCFFunctionCount) )
                {
                    guardCFCheckFunctionPointerRVA = VA2RVA( lcfgDir.GuardCFCheckFunctionPointer, imageBase );
                    loadConfig.reserved2 = lcfgDir.Reserved2;
                    guardCFFunctionTableRVA = VA2RVA( lcfgDir.GuardCFFunctionTable, imageBase );
                    loadConfig.guardCFFunctionCount = lcfgDir.GuardCFFunctionCount;
                    loadConfig.guardFlags = lcfgDir.GuardFlags;
                }
            }
            else
            {
                PEStructures::IMAGE_LOAD_CONFIG_DIRECTORY32 lcfgDir;
                lcfgDirStream.Read( &lcfgDir, sizeof(lcfgDir) );

                if ( lcfgDir.Size >= offsetof(decltype(lcfgDir), SEHandlerCount) )
                {
                    loadConfig.timeDateStamp = lcfgDir.TimeDateStamp;
                    loadConfig.majorVersion = lcfgDir.MajorVersion;
                    loadConfig.minorVersion = lcfgDir.MinorVersion;
                    loadConfig.globFlagsClear = lcfgDir.GlobalFlagsClear;
                    loadConfig.globFlagsSet = lcfgDir.GlobalFlagsSet;
                    loadConfig.critSecDefTimeOut = lcfgDir.CriticalSectionDefaultTimeout;
                    loadConfig.deCommitFreeBlockThreshold = lcfgDir.DeCommitFreeBlockThreshold;
                    loadConfig.deCommitTotalFreeThreshold = lcfgDir.DeCommitTotalFreeThreshold;
                    lockPrefixTableRVA = VA2RVA( lcfgDir.LockPrefixTable, imageBase );
                    loadConfig.maxAllocSize = lcfgDir.MaximumAllocationSize;
                    loadConfig.virtualMemoryThreshold = lcfgDir.VirtualMemoryThreshold;
                    loadConfig.processAffinityMask = lcfgDir.ProcessAffinityMask;
                    loadConfig.processHeapFlags = lcfgDir.ProcessHeapFlags;
                    loadConfig.CSDVersion = lcfgDir.CSDVersion;
                    loadConfig.reserved1 = lcfgDir.Reserved1;
                    editListRVA = VA2RVA( lcfgDir.EditList, imageBase );
                    securityCookieRVA = VA2RVA( lcfgDir.SecurityCookie, imageBase );
                    SEHandlerTableRVA = VA2RVA( lcfgDir.SEHandlerTable, imageBase );
                    loadConfig.SEHandlerCount = lcfgDir.SEHandlerCount;
                }

                if ( lcfgDir.Size >= offsetof(decltype(lcfgDir), GuardCFFunctionCount) )
                {
                    guardCFCheckFunctionPointerRVA = VA2RVA( lcfgDir.GuardCFCheckFunctionPointer, imageBase );
                    loadConfig.reserved2 = lcfgDir.Reserved2;
                    guardCFFunctionTableRVA = VA2RVA( lcfgDir.GuardCFFunctionTable, imageBase );
                    loadConfig.guardCFFunctionCount = lcfgDir.GuardCFFunctionCount;
                    loadConfig.guardFlags = lcfgDir.GuardFlags;
                }
            }

            // Process the VA registrations.
            loadConfig.lockPrefixTableRef = sections.ResolveRVAToRef( lockPrefixTableRVA );
            loadConfig.editListRef = sections.ResolveRVAToRef( editListRVA );
            loadConfig.securityCookieRef = sections.ResolveRVAToRef( securityCookieRVA );
            loadConfig.SEHandlerTableRef = sections.ResolveRVAToRef( SEHandlerTableRVA );
            loadConfig.guardCFCheckFunctionPtrRef = sections.ResolveRVAToRef( guardCFCheckFunctionPointerRVA );
            loadConfig.guardCFFunctionTableRef = sections.ResolveRVAToRef( guardCFFunctionTableRVA );

            // We kinda need the load config.
            loadConfig.isNeeded = true;

            lcfgDirSect->SetPlacedMemory( loadConfig.allocEntry, lcfgDataDir.VirtualAddress, lcfgDataDir.Size );
        }
    }

    // * BOUND IMPORT DIR.
    decltype( this->boundImports ) boundImports;
    {
        const PEStructures::IMAGE_DATA_DIRECTORY& boundDataDir = dataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ];

        if ( boundDataDir.VirtualAddress != 0 )
        {
            // The bound imports directory appears to be stored in old file-offset format.
            // Despite that we have to read it. The data is explicitly written outside
            // PE sections so we need to read it custom.

            std::uint32_t boundImpFileOff = boundDataDir.VirtualAddress;
            std::uint32_t boundImpStoreSize = boundDataDir.Size;

            struct helpers
            {
                // A huge thank-you to...
                // http://www.karmany.net/ingenieria-inversa/35-peheader/72-boundimportdirectory
                // for debunking the bound import directory!

                // Bound import directories.
                inline static PEBoundImport LoadBoundImportDirectory( PEStream *peStream, std::uint32_t dirRootOff, const PEStructures::IMAGE_BOUND_IMPORT_DESCRIPTOR& boundDesc )
                {
                    pe_file_ptr_t saved_fileOff = peStream->Tell();

                    // Fetch the DLL name.
                    std::string DLLName;
                    {
                        peStream->Seek( dirRootOff + boundDesc.OffsetModuleName );

                        while ( true )
                        {
                            char c;
                
                            bool gotChar = peStream->ReadStruct( c );

                            if ( !gotChar )
                            {
                                throw peframework_exception(
                                    ePEExceptCode::CORRUPT_PE_STRUCTURE,
                                    "failed to read PE bound import descriptor module name"
                                );
                            }

                            if ( c == '\0' )
                            {
                                break;
                            }

                            DLLName += c;
                        }
                    }

                    // Restore the usual file position.
                    peStream->Seek( saved_fileOff );

                    // Create our structure.
                    PEBoundImport desc;
                    desc.timeDateStamp = boundDesc.TimeDateStamp;
                    desc.DLLName = std::move( DLLName );
        
                    // Read child structures.
                    const std::uint32_t numChildren = boundDesc.NumberOfModuleForwarderRefs;

                    for ( std::uint32_t n = 0; n < numChildren; n++ )
                    {
                        PEStructures::IMAGE_BOUND_IMPORT_DESCRIPTOR boundDesc;
                        {
                            bool gotDesc = peStream->ReadStruct( boundDesc );

                            if ( !gotDesc )
                            {
                                throw peframework_exception(
                                    ePEExceptCode::CORRUPT_PE_STRUCTURE,
                                    "failed to read PE bound import descriptor (forward)"
                                );
                            }
                        }

                        PEBoundImport childBoundImport = LoadBoundImportDirectory( peStream, dirRootOff, boundDesc );

                        desc.forw_bindings.push_back( std::move( childBoundImport ) );
                    }

                    return desc;
                }
            };

            // Read all bound import descriptors.
            // They are not fixed-size, so we need to read till end.
            peStream->Seek( boundImpFileOff );
            
            while ( true )
            {
                // Are we past the descriptors?
                pe_file_ptr_t curFileOff = peStream->Tell();

                const pe_file_ptr_t off_into_desc = ( curFileOff - boundImpFileOff );

                if ( off_into_desc >= boundDataDir.Size )
                    break;

                // We end with a zero-valued descriptor.
                PEStructures::IMAGE_BOUND_IMPORT_DESCRIPTOR boundDesc;
                {
                    bool gotDesc = peStream->ReadStruct( boundDesc );

                    if ( !gotDesc )
                    {
                        throw peframework_exception(
                            ePEExceptCode::CORRUPT_PE_STRUCTURE,
                            "failed to read PE bound import descriptor (root)"
                        );
                    }
                }

                if ( boundDesc.TimeDateStamp == 0 )
                {
                    // It is invalid to have a checksum of zero.
                    break;
                }

                PEBoundImport boundImport = helpers::LoadBoundImportDirectory( peStream, boundImpFileOff, boundDesc );

                boundImports.push_back( std::move( boundImport ) );
            }

            // OK.
        }
    }

    // * IMPORT ADDRESS TABLE.
    // This is a pointer to the entire THUNK IAT array that is used in the IMPORTS DIRECTORY.
    // All thunks have to be contiguously allocated inside of this directory.
    PEThunkIATStore thunkIAT;
    {
        const PEStructures::IMAGE_DATA_DIRECTORY& iatDataDir = dataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_IAT ];

        thunkIAT.thunkDataStart = iatDataDir.VirtualAddress;
        thunkIAT.thunkDataSize = iatDataDir.Size;
    }

    // * DELAY LOAD IMPORTS.
    std::vector <PEDelayLoadDesc> delayLoads;
    {
        const PEStructures::IMAGE_DATA_DIRECTORY& delayDataDir = dataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT ];

        if ( delayDataDir.VirtualAddress != 0 )
        {
            PESection *delayLoadDescsSect;
            PEDataStream delayLoadDescsStream;
            {
                bool gotStream = sections.GetPEDataStream( delayDataDir.VirtualAddress, delayLoadDescsStream, &delayLoadDescsSect );

                if ( !gotStream )
                {
                    throw peframework_exception(
                        ePEExceptCode::CORRUPT_PE_STRUCTURE,
                        "invalid PE delay loads directory"
                    );
                }
            }

            delayLoadDescsSect->SetPlacedMemory( this->delayLoadsAllocEntry, delayDataDir.VirtualAddress, delayDataDir.Size );

            const std::uint32_t numDelayLoads = ( delayDataDir.Size / sizeof(PEStructures::IMAGE_DELAYLOAD_DESCRIPTOR) );

            delayLoads.reserve( numDelayLoads );

            // Store all of the details.
            std::uint32_t n = 0;

            while ( n < numDelayLoads )
            {
                // Seek to this descriptor.
                delayLoadDescsStream.Seek( n * sizeof(PEStructures::IMAGE_DELAYLOAD_DESCRIPTOR) );

                PEStructures::IMAGE_DELAYLOAD_DESCRIPTOR delayLoad;
                delayLoadDescsStream.Read( &delayLoad, sizeof(delayLoad) );

                // If we found a NULL descriptor, terminate.
                if ( delayLoad.Attributes.AllAttributes == 0 &&
                     delayLoad.DllNameRVA == 0 &&
                     delayLoad.ModuleHandleRVA == 0 &&
                     delayLoad.ImportAddressTableRVA == 0 &&
                     delayLoad.BoundImportAddressTableRVA == 0 &&
                     delayLoad.UnloadInformationTableRVA == 0 &&
                     delayLoad.TimeDateStamp == 0 )
                {
                    // Encountered terminating NULL descriptor.
                    break;
                }

                PEDelayLoadDesc desc;
                desc.attrib = delayLoad.Attributes.AllAttributes;
                
                // Read DLL name.
                if ( std::uint32_t DllNameRVA = delayLoad.DllNameRVA )
                {
                    PESection *dllNamePtrSect;
                    {
                        bool gotName = sections.ReadPEString( DllNameRVA, desc.DLLName, &dllNamePtrSect );

                        if ( !gotName )
                        {
                            throw peframework_exception(
                                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                                "failed to read PE delay load desc DLL name"
                            );
                        }
                    }

                    dllNamePtrSect->SetPlacedMemory( desc.DLLName_allocEntry, DllNameRVA );
                }

                // Take over the memory location of the DLL handle.
                {
                    std::uint32_t archPointerSize = GetPEPointerSize( isExtendedFormat );

                    std::uint32_t sectOff;
                    PESection *handleSect;
                    {
                        bool gotLocation = sections.GetPEDataLocationEx( delayLoad.ModuleHandleRVA, archPointerSize, &sectOff, &handleSect );

                        if ( !gotLocation )
                        {
                            throw peframework_exception(
                                ePEExceptCode::CORRUPT_PE_STRUCTURE,
                                "failed to read valid PE delay-load module handle allocation"
                            );
                        }
                    }

                    handleSect->SetPlacedMemoryInline( desc.DLLHandleAlloc, sectOff, archPointerSize );
                }

                desc.IATRef = sections.ResolveRVAToRef( delayLoad.ImportAddressTableRVA );
                
                if ( std::uint32_t importNamesRVA = delayLoad.ImportNameTableRVA )
                {
                    desc.importNames =
                        PEImportDesc::ReadPEImportFunctions(
                            sections, importNamesRVA, desc.importNamesAllocEntry, isExtendedFormat
                        );
                }

                desc.boundImportAddrTableRef = sections.ResolveRVAToRef( delayLoad.BoundImportAddressTableRVA );
                desc.unloadInfoTableRef = sections.ResolveRVAToRef( delayLoad.UnloadInformationTableRVA );
                desc.timeDateStamp = delayLoad.TimeDateStamp;

                // Store it.
                delayLoads.push_back( std::move( desc ) );

                // Advance to the next.
                n++;
            }
        }
    }

    // * COMMON LANGUAGE RUNTIME INFO.
    PECommonLanguageRuntimeInfo clrInfo;
    {
        const PEStructures::IMAGE_DATA_DIRECTORY& clrDataDir = dataDirs[ PEL_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR ];

        clrInfo.dataOffset = clrDataDir.VirtualAddress;
        clrInfo.dataSize = clrDataDir.Size;
    }

    // TODO: maybe validate all structures more explicitly in context now.
    
    // Successfully loaded!
    // Store everything inside ourselves.
    this->dos_data = std::move( dos );
    this->pe_finfo = std::move( peInfo );
    this->peOptHeader = std::move( peOpt );
    this->sections = std::move( sections );
    
    // Data directories.
    this->exportDir = std::move( expInfo );
    this->imports = std::move( impDescs );
    this->resourceRoot = std::move( resourceRoot );
    this->exceptRFs = std::move( exceptRFs );
    this->securityCookie = std::move( securityCookie );
    this->baseRelocs = std::move( baseRelocs );
    this->debugDescs = std::move( debugDescs );
    this->globalPtr = std::move( globalPtr );
    this->tlsInfo = std::move( tlsInfo );
    this->loadConfig = std::move( loadConfig );
    this->boundImports = std::move( boundImports );
    this->iatThunkAll = std::move( thunkIAT );
    this->delayLoads = std::move( delayLoads );
    this->clrInfo = std::move( clrInfo );

    // Store some meta-data.
    this->isExtendedFormat = isExtendedFormat;        // important for casting certain offsets.

    // Next thing we would need is writing support.
}