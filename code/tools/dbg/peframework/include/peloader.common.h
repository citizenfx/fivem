// PEloader common utilities that are globally included inside of the module.

#ifndef _PELOADER_COMMON_HEADER_
#define _PELOADER_COMMON_HEADER_

#include "peexcept.h"

#include <sdk/MemoryUtils.h>

// Machine types.

#define PEL_IMAGE_FILE_MACHINE_UNKNOWN           0
#define PEL_IMAGE_FILE_MACHINE_I386              0x014c  // Intel 386.
#define PEL_IMAGE_FILE_MACHINE_R3000             0x0162  // MIPS little-endian, 0x160 big-endian
#define PEL_IMAGE_FILE_MACHINE_R4000             0x0166  // MIPS little-endian
#define PEL_IMAGE_FILE_MACHINE_R10000            0x0168  // MIPS little-endian
#define PEL_IMAGE_FILE_MACHINE_WCEMIPSV2         0x0169  // MIPS little-endian WCE v2
#define PEL_IMAGE_FILE_MACHINE_ALPHA             0x0184  // Alpha_AXP
#define PEL_IMAGE_FILE_MACHINE_SH3               0x01a2  // SH3 little-endian
#define PEL_IMAGE_FILE_MACHINE_SH3DSP            0x01a3
#define PEL_IMAGE_FILE_MACHINE_SH3E              0x01a4  // SH3E little-endian
#define PEL_IMAGE_FILE_MACHINE_SH4               0x01a6  // SH4 little-endian
#define PEL_IMAGE_FILE_MACHINE_SH5               0x01a8  // SH5
#define PEL_IMAGE_FILE_MACHINE_ARM               0x01c0  // ARM Little-Endian
#define PEL_IMAGE_FILE_MACHINE_THUMB             0x01c2  // ARM Thumb/Thumb-2 Little-Endian
#define PEL_IMAGE_FILE_MACHINE_ARMNT             0x01c4  // ARM Thumb-2 Little-Endian
#define PEL_IMAGE_FILE_MACHINE_AM33              0x01d3
#define PEL_IMAGE_FILE_MACHINE_POWERPC           0x01F0  // IBM PowerPC Little-Endian
#define PEL_IMAGE_FILE_MACHINE_POWERPCFP         0x01f1
#define PEL_IMAGE_FILE_MACHINE_IA64              0x0200  // Intel 64
#define PEL_IMAGE_FILE_MACHINE_MIPS16            0x0266  // MIPS
#define PEL_IMAGE_FILE_MACHINE_ALPHA64           0x0284  // ALPHA64
#define PEL_IMAGE_FILE_MACHINE_MIPSFPU           0x0366  // MIPS
#define PEL_IMAGE_FILE_MACHINE_MIPSFPU16         0x0466  // MIPS
#define PEL_IMAGE_FILE_MACHINE_AXP64             IMAGE_FILE_MACHINE_ALPHA64
#define PEL_IMAGE_FILE_MACHINE_TRICORE           0x0520  // Infineon
#define PEL_IMAGE_FILE_MACHINE_CEF               0x0CEF
#define PEL_IMAGE_FILE_MACHINE_EBC               0x0EBC  // EFI Byte Code
#define PEL_IMAGE_FILE_MACHINE_AMD64             0x8664  // AMD64 (K8)
#define PEL_IMAGE_FILE_MACHINE_M32R              0x9041  // M32R little-endian
#define PEL_IMAGE_FILE_MACHINE_CEE               0xC0EE

// Subsystem Values

#define PEL_IMAGE_SUBSYSTEM_UNKNOWN              0   // Unknown subsystem.
#define PEL_IMAGE_SUBSYSTEM_NATIVE               1   // Image doesn't require a subsystem.
#define PEL_IMAGE_SUBSYSTEM_WINDOWS_GUI          2   // Image runs in the Windows GUI subsystem.
#define PEL_IMAGE_SUBSYSTEM_WINDOWS_CUI          3   // Image runs in the Windows character subsystem.
#define PEL_IMAGE_SUBSYSTEM_OS2_CUI              5   // image runs in the OS/2 character subsystem.
#define PEL_IMAGE_SUBSYSTEM_POSIX_CUI            7   // image runs in the Posix character subsystem.
#define PEL_IMAGE_SUBSYSTEM_NATIVE_WINDOWS       8   // image is a native Win9x driver.
#define PEL_IMAGE_SUBSYSTEM_WINDOWS_CE_GUI       9   // Image runs in the Windows CE subsystem.
#define PEL_IMAGE_SUBSYSTEM_EFI_APPLICATION      10  //
#define PEL_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER  11   //
#define PEL_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER   12  //
#define PEL_IMAGE_SUBSYSTEM_EFI_ROM              13
#define PEL_IMAGE_SUBSYSTEM_XBOX                 14
#define PEL_IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION 16

namespace PEloader
{

struct PEAllocFileAllocProxy
{
    template <typename sliceType>
    AINLINE bool IsInAllocationRange( const sliceType& slice )
    {
        // TODO: add limit checking for 32bit allocatibility here (if required).
        return true;
    }
};

typedef InfiniteCollisionlessBlockAllocator <std::uint32_t> peFileAlloc;

struct FileSpaceAllocMan
{
    inline FileSpaceAllocMan( void )
    {
        return;
    }

    inline ~FileSpaceAllocMan( void )
    {
        // Free all allocations that have not yet been freed (which is every alloc).
        while ( !LIST_EMPTY( this->internalAlloc.blockList.root ) )
        {
            peFileAlloc::block_t *item = LIST_GETITEM( peFileAlloc::block_t, this->internalAlloc.blockList.root.next, node );

            alloc_block_t *allocBlock = LIST_GETITEM( alloc_block_t, item, allocatorEntry );

            // Remove us from registration.
            this->internalAlloc.RemoveBlock( item );

            // Delete us.
            delete allocBlock;
        }
    }

    inline std::uint32_t AllocateAny( std::uint32_t peSize, std::uint32_t peAlignment = sizeof(std::uint32_t) )
    {
        peFileAlloc::allocInfo alloc_data;

        if ( internalAlloc.FindSpace( peSize, alloc_data, peAlignment ) == false )
        {
            throw peframework_exception(
                ePEExceptCode::RESOURCE_ERROR,
                "failed to find PE file space for allocation"
            );
        }

        alloc_block_t *alloc_savior = new alloc_block_t();
        
        internalAlloc.PutBlock( &alloc_savior->allocatorEntry, alloc_data );

        return alloc_savior->allocatorEntry.slice.GetSliceStartPoint();
    }

    inline void AllocateAt( std::uint32_t peOff, std::uint32_t peSize )
    {
        peFileAlloc::allocInfo alloc_data;

        if ( internalAlloc.ObtainSpaceAt( peOff, peSize, alloc_data ) == false )
        {
            throw peframework_exception(
                ePEExceptCode::RESOURCE_ERROR,
                "failed to obtain PE file space at presignated offset"
            );
        }

        alloc_block_t *alloc_savior = new alloc_block_t();

        internalAlloc.PutBlock( &alloc_savior->allocatorEntry, alloc_data );
    }

    inline std::uint32_t GetSpanSize( std::uint32_t alignment )
    {
        return ALIGN_SIZE( internalAlloc.GetSpanSize(), alignment );
    }

private:
    peFileAlloc internalAlloc;

    struct alloc_block_t
    {
        peFileAlloc::block_t allocatorEntry;
    };
};

}; //PEloader

#endif //_PELOADER_COMMON_HEADER_