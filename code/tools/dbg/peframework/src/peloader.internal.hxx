#ifndef _PELOADER_INTERNAL_
#define _PELOADER_INTERNAL_

// Forward to the global header, because it is sometimes necessary.
#include "peloader.serialize.h"

// Helper function for pointer size.
inline std::uint32_t GetPEPointerSize( bool isExtendedFormat )
{
    if ( isExtendedFormat )
    {
        return sizeof(std::uint64_t);
    }

    return sizeof(std::uint32_t);
}

#endif //_PELOADER_INTERNAL_