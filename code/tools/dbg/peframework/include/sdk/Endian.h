#ifndef _ENDIAN_COMPAT_HEADER_
#define _ENDIAN_COMPAT_HEADER_

#include "MacroUtils.h"

// Endianness compatibility definitions.
namespace endian
{
    AINLINE static bool is_little_endian( void )
    {
        // SHOULD be evaluating without runtime code.
        return ( *(unsigned short*)"\xFF\x00" == 0x00FF );
    }

    template <typename numberType>
    AINLINE numberType byte_swap_fast( const char *data )
    {
        char swapped_data[ sizeof(numberType) ];

        for ( unsigned int n = 0; n < sizeof(numberType); n++ )
        {
            swapped_data[ n ] = data[ ( sizeof(numberType) - 1 ) - n ];
        }

        return *(numberType*)swapped_data;
    }

#ifdef _MSC_VER
    template <> AINLINE short byte_swap_fast( const char *data )                { return _byteswap_ushort( *(unsigned short*)data ); }
    template <> AINLINE unsigned short byte_swap_fast( const char *data )       { return _byteswap_ushort( *(unsigned short*)data ); }
    template <> AINLINE int byte_swap_fast( const char *data )                  { return _byteswap_ulong( *(unsigned int*)data ); }
    template <> AINLINE unsigned int byte_swap_fast( const char *data )         { return _byteswap_ulong( *(unsigned int*)data ); }
    template <> AINLINE long byte_swap_fast( const char *data )                 { return _byteswap_ulong( *(unsigned long*)data ); }
    template <> AINLINE unsigned long byte_swap_fast( const char *data )        { return _byteswap_ulong( *(unsigned long*)data ); }
    template <> AINLINE long long byte_swap_fast( const char *data )            { return _byteswap_uint64( *(unsigned long long*)data ); }
    template <> AINLINE unsigned long long byte_swap_fast( const char *data )   { return _byteswap_uint64( *(unsigned long long*)data ); }
#elif defined(__linux__)
    template <> AINLINE short byte_swap_fast( const char *data )                { return __builtin_bswap16( *(unsigned short*)data ); }
    template <> AINLINE unsigned short byte_swap_fast( const char *data )       { return __builtin_bswap16( *(unsigned short*)data ); }
    template <> AINLINE int byte_swap_fast( const char *data )                  { return __builtin_bswap32( *(unsigned int*)data ); }
    template <> AINLINE unsigned int byte_swap_fast( const char *data )         { return __builtin_bswap32( *(unsigned int*)data ); }
    template <> AINLINE long byte_swap_fast( const char *data )                 { return __builtin_bswap32( *(unsigned long*)data ); }
    template <> AINLINE unsigned long byte_swap_fast( const char *data )        { return __builtin_bswap32( *(unsigned long*)data ); }
    template <> AINLINE long long byte_swap_fast( const char *data )            { return __builtin_bswap64( *(unsigned long long*)data ); }
    template <> AINLINE unsigned long long byte_swap_fast( const char *data )   { return __builtin_bswap64( *(unsigned long long*)data ); }
#endif

    template <typename numberType>
    struct big_endian
    {
        inline big_endian( void )
        {
            // Nothing really.
        }

        inline big_endian( const big_endian& right )
        {
            *(numberType*)this->data = *(numberType*)right.data;
        }

        inline big_endian( numberType right )
        {
            this->operator = ( right );
        }

        inline operator numberType( void ) const
        {
            if ( is_little_endian() )
            {
                return byte_swap_fast <numberType> ( this->data );
            }
            else
            {
                return *(numberType*)this->data;
            }
        }

        inline numberType operator = ( const numberType& right )
        {
            if ( is_little_endian() )
            {
                *(numberType*)data = byte_swap_fast <numberType> ( (const char*)&right );
            }
            else
            {
                *(numberType*)data = right;
            }

            return right;
        }

        inline big_endian& operator = ( const big_endian& right )
        {
            *(numberType*)this->data = *(numberType*)right.data;

            return *this;
        }

    private:
        char data[ sizeof(numberType) ];
    };

    template <typename numberType>
    struct little_endian
    {
        inline little_endian( void )
        {
            // Nothing really.
        }

        inline little_endian( const little_endian& right )
        {
            *(numberType*)this->data = *(numberType*)right.data;
        }

        inline little_endian( numberType right )
        {
            this->operator = ( right );
        }

        inline operator numberType( void ) const
        {
            if ( !is_little_endian() )
            {
                return byte_swap_fast <numberType> ( this->data );
            }
            else
            {
                return *(numberType*)this->data;
            }
        }

        inline numberType operator = ( const numberType& right )
        {
            if ( !is_little_endian() )
            {
                *(numberType*)data = byte_swap_fast <numberType> ( (const char*)&right );
            }
            else
            {
                *(numberType*)data = right;
            }

            return right;
        }

        inline little_endian& operator = ( const little_endian& right )
        {
            *(numberType*)this->data = *(numberType*)right.data;

            return *this;
        }

    private:
        char data[ sizeof(numberType) ];
    };
};

#endif //_ENDIAN_COMPAT_HEADER_