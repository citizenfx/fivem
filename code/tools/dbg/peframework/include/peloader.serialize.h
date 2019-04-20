// Header that contains all the structures used by PEFramework to serialize
// a PE file. Optional include file for the general runtime.

#ifndef _PELOADER_SERIALIZE_
#define _PELOADER_SERIALIZE_

// We get our own copies of Windows things, to keep a well-reasoned versioning.

// Meant-to-be-serialized file structures.
// Not recommended to deal with directly.
namespace PEStructures
{

// Main PE headers.
#define PEL_IMAGE_DOS_SIGNATURE         0x5A4D

struct IMAGE_DOS_HEADER      // DOS .EXE header
{
    std::uint16_t   e_magic;            // Magic number
    std::uint16_t   e_cblp;             // Bytes on last page of file
    std::uint16_t   e_cp;               // Pages in file
    std::uint16_t   e_crlc;             // Relocations
    std::uint16_t   e_cparhdr;          // Size of header in paragraphs
    std::uint16_t   e_minalloc;         // Minimum extra paragraphs needed
    std::uint16_t   e_maxalloc;         // Maximum extra paragraphs needed
    std::uint16_t   e_ss;               // Initial (relative) SS value
    std::uint16_t   e_sp;               // Initial SP value
    std::uint16_t   e_csum;             // Checksum
    std::uint16_t   e_ip;               // Initial IP value
    std::uint16_t   e_cs;               // Initial (relative) CS value
    std::uint16_t   e_lfarlc;           // File address of relocation table
    std::uint16_t   e_ovno;             // Overlay number
    std::uint16_t   e_res[4];           // Reserved words
    std::uint16_t   e_oemid;            // OEM identifier (for e_oeminfo)
    std::uint16_t   e_oeminfo;          // OEM information; e_oemid specific
    std::uint16_t   e_res2[10];         // Reserved words
    std::int32_t    e_lfanew;           // File address of new exe header
};

#define PEL_IMAGE_FILE_RELOCS_STRIPPED           0x0001  // Relocation info stripped from file.
#define PEL_IMAGE_FILE_EXECUTABLE_IMAGE          0x0002  // File is executable  (i.e. no unresolved external references).
#define PEL_IMAGE_FILE_LINE_NUMS_STRIPPED        0x0004  // Line nunbers stripped from file.
#define PEL_IMAGE_FILE_LOCAL_SYMS_STRIPPED       0x0008  // Local symbols stripped from file.
#define PEL_IMAGE_FILE_AGGRESIVE_WS_TRIM         0x0010  // Aggressively trim working set
#define PEL_IMAGE_FILE_LARGE_ADDRESS_AWARE       0x0020  // App can handle >2gb addresses
#define PEL_IMAGE_FILE_BYTES_REVERSED_LO         0x0080  // Bytes of machine word are reversed.
#define PEL_IMAGE_FILE_32BIT_MACHINE             0x0100  // 32 bit word machine.
#define PEL_IMAGE_FILE_DEBUG_STRIPPED            0x0200  // Debugging info stripped from file in .DBG file
#define PEL_IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP   0x0400  // If Image is on removable media, copy and run from the swap file.
#define PEL_IMAGE_FILE_NET_RUN_FROM_SWAP         0x0800  // If Image is on Net, copy and run from the swap file.
#define PEL_IMAGE_FILE_SYSTEM                    0x1000  // System File.
#define PEL_IMAGE_FILE_DLL                       0x2000  // File is a DLL.
#define PEL_IMAGE_FILE_UP_SYSTEM_ONLY            0x4000  // File should only be run on a UP machine
#define PEL_IMAGE_FILE_BYTES_REVERSED_HI         0x8000  // Bytes of machine word are reversed.

struct IMAGE_FILE_HEADER
{
    std::uint16_t   Machine;
    std::uint16_t   NumberOfSections;
    std::uint32_t   TimeDateStamp;
    std::uint32_t   PointerToSymbolTable;
    std::uint32_t   NumberOfSymbols;
    std::uint16_t   SizeOfOptionalHeader;
    std::uint16_t   Characteristics;
};

// DllCharacteristics Entries

//      PEL_IMAGE_LIBRARY_PROCESS_INIT            0x0001     // Reserved.
//      PEL_IMAGE_LIBRARY_PROCESS_TERM            0x0002     // Reserved.
//      PEL_IMAGE_LIBRARY_THREAD_INIT             0x0004     // Reserved.
//      PEL_IMAGE_LIBRARY_THREAD_TERM             0x0008     // Reserved.
#define PEL_IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA    0x0020  // Image can handle a high entropy 64-bit virtual address space.
#define PEL_IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE       0x0040  // DLL can move.
#define PEL_IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY    0x0080  // Code Integrity Image
#define PEL_IMAGE_DLLCHARACTERISTICS_NX_COMPAT          0x0100  // Image is NX compatible
#define PEL_IMAGE_DLLCHARACTERISTICS_NO_ISOLATION       0x0200  // Image understands isolation and doesn't want it
#define PEL_IMAGE_DLLCHARACTERISTICS_NO_SEH             0x0400  // Image does not use SEH.  No SE handler may reside in this image
#define PEL_IMAGE_DLLCHARACTERISTICS_NO_BIND            0x0800  // Do not bind this image.
#define PEL_IMAGE_DLLCHARACTERISTICS_APPCONTAINER       0x1000  // Image should execute in an AppContainer
#define PEL_IMAGE_DLLCHARACTERISTICS_WDM_DRIVER         0x2000  // Driver uses WDM model
#define PEL_IMAGE_DLLCHARACTERISTICS_GUARD_CF           0x4000  // Image supports Control Flow Guard.
#define PEL_IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE     0x8000

// Section characteristics.
//
//      PEL_IMAGE_SCN_TYPE_REG                   0x00000000  // Reserved.
//      PEL_IMAGE_SCN_TYPE_DSECT                 0x00000001  // Reserved.
//      PEL_IMAGE_SCN_TYPE_NOLOAD                0x00000002  // Reserved.
//      PEL_IMAGE_SCN_TYPE_GROUP                 0x00000004  // Reserved.
#define PEL_IMAGE_SCN_TYPE_NO_PAD                0x00000008  // Reserved.
//      PEL_IMAGE_SCN_TYPE_COPY                  0x00000010  // Reserved.

#define PEL_IMAGE_SCN_CNT_CODE                   0x00000020  // Section contains code.
#define PEL_IMAGE_SCN_CNT_INITIALIZED_DATA       0x00000040  // Section contains initialized data.
#define PEL_IMAGE_SCN_CNT_UNINITIALIZED_DATA     0x00000080  // Section contains uninitialized data.

#define PEL_IMAGE_SCN_LNK_OTHER                  0x00000100  // Reserved.
#define PEL_IMAGE_SCN_LNK_INFO                   0x00000200  // Section contains comments or some other type of information.
//      PEL_IMAGE_SCN_TYPE_OVER                  0x00000400  // Reserved.
#define PEL_IMAGE_SCN_LNK_REMOVE                 0x00000800  // Section contents will not become part of image.
#define PEL_IMAGE_SCN_LNK_COMDAT                 0x00001000  // Section contents comdat.
//                                               0x00002000  // Reserved.
//      PEL_IMAGE_SCN_MEM_PROTECTED - Obsolete   0x00004000
#define PEL_IMAGE_SCN_NO_DEFER_SPEC_EXC          0x00004000  // Reset speculative exceptions handling bits in the TLB entries for this section.
#define PEL_IMAGE_SCN_GPREL                      0x00008000  // Section content can be accessed relative to GP
#define PEL_IMAGE_SCN_MEM_FARDATA                0x00008000
//      PEL_IMAGE_SCN_MEM_SYSHEAP  - Obsolete    0x00010000
#define PEL_IMAGE_SCN_MEM_PURGEABLE              0x00020000
#define PEL_IMAGE_SCN_MEM_16BIT                  0x00020000
#define PEL_IMAGE_SCN_MEM_LOCKED                 0x00040000
#define PEL_IMAGE_SCN_MEM_PRELOAD                0x00080000

#define PEL_IMAGE_SCN_ALIGN_1BYTES               0x00100000  //
#define PEL_IMAGE_SCN_ALIGN_2BYTES               0x00200000  //
#define PEL_IMAGE_SCN_ALIGN_4BYTES               0x00300000  //
#define PEL_IMAGE_SCN_ALIGN_8BYTES               0x00400000  //
#define PEL_IMAGE_SCN_ALIGN_16BYTES              0x00500000  // Default alignment if no others are specified.
#define PEL_IMAGE_SCN_ALIGN_32BYTES              0x00600000  //
#define PEL_IMAGE_SCN_ALIGN_64BYTES              0x00700000  //
#define PEL_IMAGE_SCN_ALIGN_128BYTES             0x00800000  //
#define PEL_IMAGE_SCN_ALIGN_256BYTES             0x00900000  //
#define PEL_IMAGE_SCN_ALIGN_512BYTES             0x00A00000  //
#define PEL_IMAGE_SCN_ALIGN_1024BYTES            0x00B00000  //
#define PEL_IMAGE_SCN_ALIGN_2048BYTES            0x00C00000  //
#define PEL_IMAGE_SCN_ALIGN_4096BYTES            0x00D00000  //
#define PEL_IMAGE_SCN_ALIGN_8192BYTES            0x00E00000  //
// Unused                                        0x00F00000
#define PEL_IMAGE_SCN_ALIGN_MASK                 0x00F00000

#define PEL_IMAGE_SCN_LNK_NRELOC_OVFL            0x01000000  // Section contains extended relocations.
#define PEL_IMAGE_SCN_MEM_DISCARDABLE            0x02000000  // Section can be discarded.
#define PEL_IMAGE_SCN_MEM_NOT_CACHED             0x04000000  // Section is not cachable.
#define PEL_IMAGE_SCN_MEM_NOT_PAGED              0x08000000  // Section is not pageable.
#define PEL_IMAGE_SCN_MEM_SHARED                 0x10000000  // Section is shareable.
#define PEL_IMAGE_SCN_MEM_EXECUTE                0x20000000  // Section is executable.
#define PEL_IMAGE_SCN_MEM_READ                   0x40000000  // Section is readable.
#define PEL_IMAGE_SCN_MEM_WRITE                  0x80000000  // Section is writeable.

#define PEL_IMAGE_PE_HEADER_SIGNATURE       0x4550

struct IMAGE_PE_HEADER
{
    std::uint32_t Signature;
    IMAGE_FILE_HEADER FileHeader;
    // Rest is machine dependent.
};

struct IMAGE_DATA_DIRECTORY
{
    std::uint32_t   VirtualAddress;
    std::uint32_t   Size;
};

#define PEL_IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

#define PEL_IMAGE_NT_OPTIONAL_HDR32_MAGIC       0x010B
#define PEL_IMAGE_NT_OPTIONAL_HDR64_MAGIC       0x020B

#pragma pack(1)
struct IMAGE_OPTIONAL_HEADER32
{
    //
    // Standard fields.
    //

    std::uint8_t    MajorLinkerVersion;
    std::uint8_t    MinorLinkerVersion;
    std::uint32_t   SizeOfCode;
    std::uint32_t   SizeOfInitializedData;
    std::uint32_t   SizeOfUninitializedData;
    std::uint32_t   AddressOfEntryPoint;
    std::uint32_t   BaseOfCode;
    std::uint32_t   BaseOfData;

    //
    // NT additional fields.
    //

    std::uint32_t   ImageBase;
    std::uint32_t   SectionAlignment;
    std::uint32_t   FileAlignment;
    std::uint16_t   MajorOperatingSystemVersion;
    std::uint16_t   MinorOperatingSystemVersion;
    std::uint16_t   MajorImageVersion;
    std::uint16_t   MinorImageVersion;
    std::uint16_t   MajorSubsystemVersion;
    std::uint16_t   MinorSubsystemVersion;
    std::uint32_t   Win32VersionValue;
    std::uint32_t   SizeOfImage;
    std::uint32_t   SizeOfHeaders;
    std::uint32_t   CheckSum;
    std::uint16_t   Subsystem;
    std::uint16_t   DllCharacteristics;
    std::uint32_t   SizeOfStackReserve;
    std::uint32_t   SizeOfStackCommit;
    std::uint32_t   SizeOfHeapReserve;
    std::uint32_t   SizeOfHeapCommit;
    std::uint32_t   LoaderFlags;
    std::uint32_t   NumberOfRvaAndSizes;
};

struct IMAGE_OPTIONAL_HEADER64
{
    std::uint8_t    MajorLinkerVersion;
    std::uint8_t    MinorLinkerVersion;
    std::uint32_t   SizeOfCode;
    std::uint32_t   SizeOfInitializedData;
    std::uint32_t   SizeOfUninitializedData;
    std::uint32_t   AddressOfEntryPoint;
    std::uint32_t   BaseOfCode;
    std::uint64_t   ImageBase;
    std::uint32_t   SectionAlignment;
    std::uint32_t   FileAlignment;
    std::uint16_t   MajorOperatingSystemVersion;
    std::uint16_t   MinorOperatingSystemVersion;
    std::uint16_t   MajorImageVersion;
    std::uint16_t   MinorImageVersion;
    std::uint16_t   MajorSubsystemVersion;
    std::uint16_t   MinorSubsystemVersion;
    std::uint32_t   Win32VersionValue;
    std::uint32_t   SizeOfImage;
    std::uint32_t   SizeOfHeaders;
    std::uint32_t   CheckSum;
    std::uint16_t   Subsystem;
    std::uint16_t   DllCharacteristics;
    std::uint64_t   SizeOfStackReserve;
    std::uint64_t   SizeOfStackCommit;
    std::uint64_t   SizeOfHeapReserve;
    std::uint64_t   SizeOfHeapCommit;
    std::uint32_t   LoaderFlags;
    std::uint32_t   NumberOfRvaAndSizes;
};
#pragma pack()

struct IMAGE_SECTION_HEADER
{
    std::int8_t     Name[8];
    union
    {
        std::uint32_t   PhysicalAddress;
        std::uint32_t   VirtualSize;
    } Misc;
    std::uint32_t   VirtualAddress;
    std::uint32_t   SizeOfRawData;
    std::uint32_t   PointerToRawData;
    std::uint32_t   PointerToRelocations;
    std::uint32_t   PointerToLinenumbers;
    std::uint16_t   NumberOfRelocations;
    std::uint16_t   NumberOfLinenumbers;
    std::uint32_t   Characteristics;
};

struct IMAGE_RELOCATION
{
    union
    {
        std::uint32_t   VirtualAddress;
        std::uint32_t   RelocCount;     // Set to the real count when IMAGE_SCN_LNK_NRELOC_OVFL is set
    };
    std::uint32_t   SymbolTableIndex;
    std::uint16_t   Type;
};

struct IMAGE_LINENUMBER
{
    union
    {
        std::uint32_t   SymbolTableIndex;   // Symbol table index of function name if Linenumber is 0.
        std::uint32_t   VirtualAddress;     // Virtual address of line number.
    } Type;
    std::uint16_t   Linenumber;             // Line number.
};

// **********************************************
//      PE Data Directories
// **********************************************

#define PEL_IMAGE_DIRECTORY_ENTRY_EXPORT          0   // Export Directory
#define PEL_IMAGE_DIRECTORY_ENTRY_IMPORT          1   // Import Directory
#define PEL_IMAGE_DIRECTORY_ENTRY_RESOURCE        2   // Resource Directory
#define PEL_IMAGE_DIRECTORY_ENTRY_EXCEPTION       3   // Exception Directory
#define PEL_IMAGE_DIRECTORY_ENTRY_SECURITY        4   // Security Directory
#define PEL_IMAGE_DIRECTORY_ENTRY_BASERELOC       5   // Base Relocation Table
#define PEL_IMAGE_DIRECTORY_ENTRY_DEBUG           6   // Debug Directory
//      PEL_IMAGE_DIRECTORY_ENTRY_COPYRIGHT       7   // (X86 usage)
#define PEL_IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    7   // Architecture Specific Data
#define PEL_IMAGE_DIRECTORY_ENTRY_GLOBALPTR       8   // RVA of GP
#define PEL_IMAGE_DIRECTORY_ENTRY_TLS             9   // TLS Directory
#define PEL_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10   // Load Configuration Directory
#define PEL_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11   // Bound Import Directory in headers
#define PEL_IMAGE_DIRECTORY_ENTRY_IAT            12   // Import Address Table
#define PEL_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13   // Delay Load Import Descriptors
#define PEL_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14   // COM Runtime descriptor

struct IMAGE_EXPORT_DIRECTORY
{
    std::uint32_t   Characteristics;
    std::uint32_t   TimeDateStamp;
    std::uint16_t   MajorVersion;
    std::uint16_t   MinorVersion;
    std::uint32_t   Name;
    std::uint32_t   Base;
    std::uint32_t   NumberOfFunctions;
    std::uint32_t   NumberOfNames;
    std::uint32_t   AddressOfFunctions;     // RVA from base of image
    std::uint32_t   AddressOfNames;         // RVA from base of image
    std::uint32_t   AddressOfNameOrdinals;  // RVA from base of image
};

struct IMAGE_IMPORT_DESCRIPTOR
{
    union
    {
        std::uint32_t   Characteristics;        // 0 for terminating null import descriptor
        std::uint32_t   OriginalFirstThunk;     // RVA to original unbound IAT (PIMAGE_THUNK_DATA)
    };
    std::uint32_t   TimeDateStamp;              // 0 if not bound,
                                                // -1 if bound, and real date\time stamp
                                                //     in IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT (new BIND)
                                                // O.W. date/time stamp of DLL bound to (Old BIND)

    std::uint32_t   ForwarderChain;             // -1 if no forwarders
    std::uint32_t   Name;
    std::uint32_t   FirstThunk;                 // RVA to IAT (if bound this IAT has actual addresses)
};

#define PEL_IMAGE_ORDINAL_FLAG64 0x8000000000000000
#define PEL_IMAGE_ORDINAL_FLAG32 0x80000000

struct IMAGE_RUNTIME_FUNCTION_ENTRY_X64
{
    std::uint32_t BeginAddress;
    std::uint32_t EndAddress;
    union
    {
        std::uint32_t UnwindInfoAddress;
        std::uint32_t UnwindData;
    };
};

struct IMAGE_DEBUG_DIRECTORY
{
    std::uint32_t   Characteristics;
    std::uint32_t   TimeDateStamp;
    std::uint16_t   MajorVersion;
    std::uint16_t   MinorVersion;
    std::uint32_t   Type;
    std::uint32_t   SizeOfData;
    std::uint32_t   AddressOfRawData;
    std::uint32_t   PointerToRawData;
};

template <typename vaNumberType>
struct IMAGE_TLS_DIRECTORY_TEMPLATE
{
    vaNumberType StartAddressOfRawData;
    vaNumberType EndAddressOfRawData;
    vaNumberType AddressOfIndex;          // PDWORD
    vaNumberType AddressOfCallBacks;      // PIMAGE_TLS_CALLBACK *;
    std::uint32_t SizeOfZeroFill;
    union {
        std::uint32_t Characteristics;
        struct {
            std::uint32_t Reserved0 : 20;
            std::uint32_t Alignment : 4;
            std::uint32_t Reserved1 : 8;
        };
    };
};

typedef IMAGE_TLS_DIRECTORY_TEMPLATE <std::uint32_t> IMAGE_TLS_DIRECTORY32;
typedef IMAGE_TLS_DIRECTORY_TEMPLATE <std::uint64_t> IMAGE_TLS_DIRECTORY64;

struct IMAGE_LOAD_CONFIG_DIRECTORY32
{
    std::uint32_t   Size;
    std::uint32_t   TimeDateStamp;
    std::uint16_t   MajorVersion;
    std::uint16_t   MinorVersion;
    std::uint32_t   GlobalFlagsClear;
    std::uint32_t   GlobalFlagsSet;
    std::uint32_t   CriticalSectionDefaultTimeout;
    std::uint32_t   DeCommitFreeBlockThreshold;
    std::uint32_t   DeCommitTotalFreeThreshold;
    std::uint32_t   LockPrefixTable;                // VA
    std::uint32_t   MaximumAllocationSize;
    std::uint32_t   VirtualMemoryThreshold;
    std::uint32_t   ProcessHeapFlags;
    std::uint32_t   ProcessAffinityMask;
    std::uint16_t   CSDVersion;
    std::uint16_t   Reserved1;
    std::uint32_t   EditList;                       // VA
    std::uint32_t   SecurityCookie;                 // VA
    std::uint32_t   SEHandlerTable;                 // VA
    std::uint32_t   SEHandlerCount;
    std::uint32_t   GuardCFCheckFunctionPointer;    // VA
    std::uint32_t   Reserved2;
    std::uint32_t   GuardCFFunctionTable;           // VA
    std::uint32_t   GuardCFFunctionCount;
    std::uint32_t   GuardFlags;
};

struct IMAGE_LOAD_CONFIG_DIRECTORY64
{
    std::uint32_t   Size;
    std::uint32_t   TimeDateStamp;
    std::uint16_t   MajorVersion;
    std::uint16_t   MinorVersion;
    std::uint32_t   GlobalFlagsClear;
    std::uint32_t   GlobalFlagsSet;
    std::uint32_t   CriticalSectionDefaultTimeout;
    std::uint64_t   DeCommitFreeBlockThreshold;
    std::uint64_t   DeCommitTotalFreeThreshold;
    std::uint64_t   LockPrefixTable;             // VA
    std::uint64_t   MaximumAllocationSize;
    std::uint64_t   VirtualMemoryThreshold;
    std::uint64_t   ProcessAffinityMask;
    std::uint32_t   ProcessHeapFlags;
    std::uint16_t   CSDVersion;
    std::uint16_t   Reserved1;
    std::uint64_t   EditList;                    // VA
    std::uint64_t   SecurityCookie;              // VA
    std::uint64_t   SEHandlerTable;              // VA
    std::uint64_t   SEHandlerCount;
    std::uint64_t   GuardCFCheckFunctionPointer; // VA
    std::uint64_t   Reserved2;
    std::uint64_t   GuardCFFunctionTable;        // VA
    std::uint64_t   GuardCFFunctionCount;
    std::uint32_t   GuardFlags;
};

struct IMAGE_BASE_RELOCATION
{
    std::uint32_t   VirtualAddress;
    std::uint32_t   SizeOfBlock;
//  std::uint16_t   TypeOffset[1];
};

struct IMAGE_DELAYLOAD_DESCRIPTOR
{
    union
    {
        std::uint32_t AllAttributes;
        struct
        {
            std::uint32_t RvaBased : 1;         // Delay load version 2
            std::uint32_t ReservedAttributes : 31;
        };
    } Attributes;

    std::uint32_t DllNameRVA;                   // RVA to the name of the target library (NULL-terminate ASCII string)
    std::uint32_t ModuleHandleRVA;              // RVA to the HMODULE caching location (PHMODULE)
    std::uint32_t ImportAddressTableRVA;        // RVA to the start of the IAT (PIMAGE_THUNK_DATA)
    std::uint32_t ImportNameTableRVA;           // RVA to the start of the name table (PIMAGE_THUNK_DATA::AddressOfData)
    std::uint32_t BoundImportAddressTableRVA;   // RVA to an optional bound IAT
    std::uint32_t UnloadInformationTableRVA;    // RVA to an optional unload info table
    std::uint32_t TimeDateStamp;                // 0 if not bound,
                                                // Otherwise, date/time of the target DLL
};

// Legacy bound imports directory; probably not used by OS loader anymore.
struct IMAGE_BOUND_IMPORT_DESCRIPTOR
{
    std::uint32_t   TimeDateStamp;
    std::uint16_t   OffsetModuleName;
    std::uint16_t   NumberOfModuleForwarderRefs;
// Array of zero or more IMAGE_BOUND_FORWARDER_REF follows
};

struct IMAGE_BOUND_FORWARDER_REF
{
    std::uint32_t   TimeDateStamp;
    std::uint16_t   OffsetModuleName;
    std::uint16_t   Reserved;
};

struct IMAGE_BASE_RELOC_TYPE_ITEM
{
    std::uint16_t offset : 12;
    std::uint16_t type : 4;
};

struct IMAGE_RESOURCE_DIRECTORY {
    std::uint32_t Characteristics;
    std::uint32_t TimeDateStamp;
    std::uint16_t MajorVersion;
    std::uint16_t MinorVersion;
    std::uint16_t NumberOfNamedEntries;
    std::uint16_t NumberOfIdEntries;
//  IMAGE_RESOURCE_DIRECTORY_ENTRY DirectoryEntries[];
};

struct IMAGE_RESOURCE_DIRECTORY_ENTRY
{
    union {
        struct {
            std::uint32_t NameOffset:31;
            std::uint32_t NameIsString:1;
        };
        std::uint32_t     Name;
        std::uint16_t     Id;
    };
    union {
        std::uint32_t     OffsetToData:31;
        struct {
            std::uint32_t OffsetToDirectory:31;
            std::uint32_t DataIsDirectory:1;
        };
    };
};

struct IMAGE_RESOURCE_DIRECTORY_STRING
{
    std::uint16_t       Length;
    char                NameString[ 1 ];
};

struct IMAGE_RESOURCE_DIR_STRING_U
{
    std::uint16_t       Length;
    wchar_t             NameString[ 1 ];
};

struct IMAGE_RESOURCE_DATA_ENTRY
{
    std::uint32_t OffsetToData;
    std::uint32_t Size;
    std::uint32_t CodePage;
    std::uint32_t Reserved;
};

struct IMAGE_ATTRIB_CERT_DESC
{
    std::uint32_t Size;
    std::uint16_t Revision;
    std::uint16_t CertificateType;
    // Followed by the actual certificate.
};

};

#endif //_PELOADER_SERIALIZE_