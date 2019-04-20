#ifndef _EIRREPO_DATA_UTILITIES_
#define _EIRREPO_DATA_UTILITIES_

// Some helpers.
namespace FSDataUtil
{
    template <typename dataType>
    static inline void copy_impl( const dataType *srcPtr, const dataType *srcPtrEnd, dataType *dstPtr ) noexcept
    {
        while ( srcPtr != srcPtrEnd )
        {
            *dstPtr++ = *srcPtr++;
        }
    }

    template <typename dataType>
    static inline void copy_backward_impl( const dataType *srcPtr, const dataType *srcPtrEnd, dataType *dstPtr ) noexcept
    {
        while ( srcPtr != srcPtrEnd )
        {
            *(--dstPtr) = *(--srcPtrEnd);
        }
    }
};

#endif //_EIRREPO_DATA_UTILITIES_