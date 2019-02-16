// Contains classes and functions that help interact across ABI boundaries.

#ifndef _EIRREPO_ABI_HELPERS_
#define _EIRREPO_ABI_HELPERS_

#include "DataUtil.h"

template <typename charType>
struct abiVirtualString
{
private:
    struct virtualContainer
    {
        virtual ~virtualContainer( void )
        {
            if ( stringBuffer )
            {
                delete [] stringBuffer;
            }
        }

        charType *stringBuffer;
        size_t strLen;
    };

    virtualContainer *container;

    inline void allocateContainer( const charType *str, size_t strLen )
    {
        virtualContainer *container = new virtualContainer();

        container->stringBuffer = new charType[ strLen + 1 ];

        FSDataUtil::copy_impl( str, str + strLen, container->stringBuffer );

        container->stringBuffer[ strLen ] = 0;
        container->strLen = strLen;

        this->container = container;
    }

public:
    inline abiVirtualString( void )
    {
        this->container = new virtualContainer();

        this->container->stringBuffer = NULL;
    }

    inline abiVirtualString( const charType *str, size_t strLen )
    {
        allocateContainer( str, strLen );
    }

    inline abiVirtualString( const std::basic_string <charType>& right ) : abiVirtualString( right.c_str(), right.size() )
    { 
        return;
    }

    inline abiVirtualString( abiVirtualString&& right )
    {
        this->container = right.container;

        right.container = NULL;
    }

    inline abiVirtualString( const abiVirtualString& right )
    {
        const charType *srcStr = right.c_str();

        size_t len = std::char_traits <charType>::length( srcStr );

        allocateContainer( srcStr, len );
    }

    inline ~abiVirtualString( void )
    {
        if ( virtualContainer *container = this->container )
        {
            delete container;
        }
    }

    inline abiVirtualString& operator = ( const abiVirtualString& right )
    {
        if ( this->container )
        {
            delete this->container;

            this->container = NULL;
        }

        if ( virtualContainer *rightCont = right.container )
        {
            const charType *str = rightCont->stringBuffer;

            allocateContainer( str, rightCont->strLen );
        }

        return *this;
    }

private:
    template <typename charType>
    inline static const charType* GetEmptyString( void )
    {
        return NULL;
    }

    template <>
    inline static const char* GetEmptyString <char> ( void )
    {
        return "";
    }

    template <>
    inline static const wchar_t* GetEmptyString <wchar_t> ( void )
    {
        return L"";
    }

public:
    inline const charType* c_str( void ) const
    {
        virtualContainer *container = this->container;

        if ( !container || !container->stringBuffer )
        {
            return GetEmptyString <charType> ();
        }

        return container->stringBuffer;
    }

    inline size_t size( void ) const
    {
        size_t len = 0;

        if ( virtualContainer *container = this->container )
        {
            len = container->strLen;
        }

        return len;
    }

    inline size_t length( void ) const
    { return size(); }

    inline operator std::basic_string <charType> ( void ) const
    {
        return std::basic_string <charType>( this->c_str(), this->length() );
    }
};

typedef abiVirtualString <char> abiString;
typedef abiVirtualString <wchar_t> abiWideString;

#endif //_EIRREPO_ABI_HELPERS_