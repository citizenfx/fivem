/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/UniChar.h
*  PURPOSE:     Character environment to parse characters in strings properly
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _EIRREPO_CHARACTER_RESOLUTION_
#define _EIRREPO_CHARACTER_RESOLUTION_

#include <cstdint>
#include "Endian.h"

#include <locale>
#include <type_traits>

// UNICODE CHARACTER LIBRARY TESTING.
template <typename charType>
struct character_env
{
    //static_assert( false, "unknown character encoding" );
};

struct codepoint_exception
{
    inline codepoint_exception( const char *msg )
    {
        this->msg = msg;
    }

    inline const char* what( void ) const
    {
        return this->msg;
    }

private:
    const char *msg;
};

template <>
struct character_env <char>
{
    typedef char ucp_t; // UNICODE CODE POINT, can represent all characters.

private:
    // ANSI strings are very simple and fast. Each byte represents one unique character.
    // This means that the number of bytes equals the string length.
    template <typename charType>
    struct ansi_iterator
    {
        charType *iter;

        inline ansi_iterator( charType *str )
        {
            this->iter = str;
        }

        inline bool IsEnd( void ) const
        {
            return ( *iter == '\0' );
        }

        inline size_t GetIterateCount( void ) const
        {
            return 1;
        }

        inline void Increment( void )
        {
            iter++;
        }

        inline ucp_t Resolve( void ) const
        {
            return *iter;
        }

        inline charType* GetPointer( void ) const
        {
            return iter;
        }

        inline ucp_t ResolveAndIncrement( void )
        {
            ucp_t res = Resolve();

            Increment();

            return res;
        }
    };

public:
    typedef ansi_iterator <char> iterator;
    typedef ansi_iterator <const char> const_iterator;

    static constexpr size_t ucp_max = std::numeric_limits <std::uint8_t>::max();

    struct enc_result
    {
        char data[1];
        size_t numData;
    };

private:
    struct ansi_encoding_iterator
    {
        const char *iter;
        size_t count;
        size_t n;

        inline ansi_encoding_iterator( const char *src, size_t len )
        {
            this->iter = src;
            this->count = len;
            this->n = 0;
        }

        inline bool IsEnd( void ) const
        {
            return ( this->n == this->count ); 
        }

        inline void Increment( void )
        {
            this->n++;
            this->iter++;
        }

        inline void Resolve( enc_result& resOut ) const
        {
            resOut.data[0] = *iter;
            resOut.numData = 1;
        }
    };

public:
    typedef ansi_encoding_iterator encoding_iterator;
};

// UTF-16 encoding.
template <typename wideCharType>
struct utf16_character_env
{
    typedef char32_t ucp_t; // UNICODE CODE POINT, can represent all characters.

private:
    struct code_point_hi_surrogate
    {
        std::uint16_t hiorder_cp : 6;
        std::uint16_t plane_id : 4;
        std::uint16_t checksum : 6; // 54 in decimals
    };

    struct code_point_lo_surrogate
    {
        std::uint16_t loworder_cp : 10;
        std::uint16_t checksum : 6; // 55 in decimals
    };

    struct result_code_point
    {
        union
        {
            struct
            {
                ucp_t lo_order : 10;
                ucp_t hi_order : 6;
                ucp_t plane_id : 5;
                ucp_t pad : 11; // ZERO OUT.
            };
            ucp_t value;
        };
    };

    // Parsing UTF-16 character points is actually pretty complicated.
    // We support all characters of the world in 16bit code points, but many
    // characters require two 16bit code points because they are outside 0-0xFFFF (BMP).
    template <typename charType>
    struct utf16_iterator
    {
        charType *iter;
        bool isLittleEndian;

        inline utf16_iterator( charType *str, bool isLittleEndian = endian::is_little_endian() )
        {
            this->iter = str;
            this->isLittleEndian = isLittleEndian;
        }

        inline bool IsEnd( void ) const
        {
            return ( *iter == 0 );
        }

    private:
        struct code_point
        {
            union
            {
                std::uint16_t ucp;
                code_point_hi_surrogate hi_surrogate;
            };
        };

        template <typename structType>
        inline structType read_struct( const void *ptr ) const
        {
            if ( this->isLittleEndian )
            {
                return *(endian::little_endian <structType>*)ptr;
            }

            return *(endian::big_endian <structType>*)ptr;
        }

        // Now comes the catch: detect how many code points we require to the next
        // code point! Thankfully UTF-16 will _never_ change.... rite?
        inline size_t codepoint_size( const charType *strpos ) const
        {
            // TODO: take into account endianness.

            code_point cp = read_struct <code_point> ( strpos );

            if ( cp.hi_surrogate.checksum == 54 )
            {
                code_point_lo_surrogate ls = read_struct <code_point_lo_surrogate> ( strpos + 1 );

                if ( ls.checksum == 55 )
                {
                    // We are two char16_t in size.
                    return 2;
                }

                // Kinda erroneous, you know?
                throw codepoint_exception( "UTF-16 stream error: invalid surrogate sequence detected" );
            }

            // We are simply char16_t in size.
            return 1;
        }

    public:
        inline size_t GetIterateCount( void ) const
        {
            return codepoint_size( iter );
        }

        // Note that we are allowed to throw exceptions if the character stream is ill-formed, so be careful.
        inline void Increment( void )
        {
            size_t cp_size = codepoint_size( iter );

            iter += cp_size;
        }

        inline ucp_t Resolve( void ) const
        {
            code_point cp = read_struct <code_point> ( iter );

            if ( cp.hi_surrogate.checksum == 54 )
            {
                code_point_lo_surrogate ls = read_struct <code_point_lo_surrogate> ( iter + 1 );

                if ( ls.checksum == 55 )
                {
                    // Decode a value outside BMP!
                    // It is a 21bit int, the top five bits are the plane, then 16bits of actual plane index.
                    result_code_point res;
                    res.lo_order = ls.loworder_cp;
                    res.hi_order = cp.hi_surrogate.hiorder_cp;
                    res.plane_id = ( 1 + cp.hi_surrogate.plane_id );
                    res.pad = 0;

                    return res.value;
                }

                // Kinda erroneous, you know?
                throw codepoint_exception( "UTF-16 stream error: invalid surrogate sequence detected" );
            }

            // In BMP.
            return cp.ucp;
        }

        inline charType* GetPointer( void ) const
        {
            return this->iter;
        }

        inline ucp_t ResolveAndIncrement( void )
        {
            ucp_t res = Resolve();

            Increment();

            return res;
        }
    };
    
public:
    typedef utf16_iterator <wideCharType> iterator;
    typedef utf16_iterator <const wideCharType> const_iterator;

    static constexpr size_t ucp_max = std::numeric_limits <std::uint32_t>::max();

    struct enc_result
    {
        wideCharType data[2];
        size_t numData;
    };

private:
    struct utf16_encoding_iterator
    {
        const ucp_t *iter;
        size_t count;
        size_t n;

        inline utf16_encoding_iterator( const ucp_t *ucp, size_t count )
        {
            this->iter = ucp;
            this->count = count;
            this->n = 0;
        }

        inline bool IsEnd( void ) const
        {
            return ( this->n == this->count );
        }

        inline void Increment( void )
        {
            this->iter++;
            this->n++;
        }

        inline void Resolve( enc_result& resOut ) const
        {
            ucp_t curVal = *iter;

            if ( curVal >= 0x10000 )
            {
                // We encode to two code points, basically a surrogate pair.
                code_point_hi_surrogate& hi_surrogate = *(code_point_hi_surrogate*)&resOut.data[0];
                code_point_lo_surrogate& lo_surrogate = *(code_point_lo_surrogate*)&resOut.data[1];

                const result_code_point& codepoint = (const result_code_point&)curVal;

                // Make sure the codepoint is even valid.
                if ( codepoint.pad != 0 )
                {
                    throw codepoint_exception( "UTF-16 encoding exception: invalid UTF-32 codepoint" );
                }

                hi_surrogate.hiorder_cp = codepoint.hi_order;
                hi_surrogate.plane_id = ( codepoint.plane_id - 1 );
                hi_surrogate.checksum = 54;

                lo_surrogate.loworder_cp = codepoint.lo_order;
                lo_surrogate.checksum = 55;

                resOut.numData = 2;
            }
            else
            {
                // We just encode to one code point.
                resOut.data[0] = (wideCharType)curVal;
                resOut.numData = 1;
            }
        }
    };

public:
    typedef utf16_encoding_iterator encoding_iterator;
};

template <>
struct character_env <char16_t> : public utf16_character_env <char16_t>
{};

template <>
struct character_env <char32_t>
{
    typedef char32_t ucp_t; // UNICODE CODE POINT, can represent all characters.

private:
    // We represent UTF-32 strings here. They are the fastest Unicode strings available.
    // They are fixed length and decode 1:1, very similar to ANSI, even if not perfect.
    template <typename charType>
    struct utf32_iterator
    {
        charType *iter;

        inline utf32_iterator( charType *str )
        {
            this->iter = str;
        }

        inline bool IsEnd( void ) const
        {
            return ( *iter == '\0' );
        }

        inline size_t GetIterateCount( void ) const
        {
            return 1;
        }

        inline void Increment( void )
        {
            iter++;
        }

        inline ucp_t Resolve( void ) const
        {
            return *iter;
        }

        inline charType* GetPointer( void ) const
        {
            return iter;
        }

        inline ucp_t ResolveAndIncrement( void )
        {
            ucp_t res = Resolve();

            Increment();

            return res;
        }
    };

public:
    typedef utf32_iterator <char32_t> iterator;
    typedef utf32_iterator <const char32_t> const_iterator;

    static constexpr size_t ucp_max = std::numeric_limits <std::uint32_t>::max();

    struct enc_result
    {
        char32_t data[1];
        size_t numData;
    };

private:
    struct utf32_encoding_iterator
    {
        const char32_t *iter;
        size_t count;
        size_t n;

        inline utf32_encoding_iterator( const char32_t *src, size_t len )
        {
            this->iter = src;
            this->count = len;
            this->n = 0;
        }

        inline bool IsEnd( void ) const
        {
            return ( this->n == this->count ); 
        }

        inline void Increment( void )
        {
            this->n++;
            this->iter++;
        }

        inline void Resolve( enc_result& resOut ) const
        {
            // TODO: check the UTF-32 character for validity.

            resOut.data[0] = *iter;
            resOut.numData = 1;
        }
    };

public:
    typedef utf32_encoding_iterator encoding_iterator;
};

// Declare the special wchar_t compatibility type.
using wchar_utf_char_env = std::conditional <sizeof(wchar_t) == sizeof(char32_t),  character_env <char32_t>, utf16_character_env <wchar_t>>::type;

template <>
struct character_env <wchar_t> : public wchar_utf_char_env
{};

template <typename charType>
AINLINE const charType GetDefaultConvFailureChar( void )
{
#ifdef _MSC_VER
    static_assert( std::is_same <some_type, void>::value, "invalid character type for default conv failure string" );
#endif // For Visual Studio only because it is a smarter compiler.
}

template <>
AINLINE const char GetDefaultConvFailureChar <char> ( void )
{
    return '_';
}

template <>
AINLINE const wchar_t GetDefaultConvFailureChar <wchar_t> ( void )
{
    return L'_';
}

template <>
AINLINE const char16_t GetDefaultConvFailureChar <char16_t> ( void )
{
    return u'_';
}

template <>
AINLINE const char32_t GetDefaultConvFailureChar <char32_t> ( void )
{
    return U'_';
}

template <typename src_env, typename dst_env>
AINLINE bool AcquireDirectUCP( typename src_env::ucp_t src_ucp, typename dst_env::ucp_t& dst_ucp )
{
    auto unsigned_ucp = (typename std::make_unsigned <typename src_env::ucp_t>::type)src_ucp;

    if ( unsigned_ucp < dst_env::ucp_max )
    {
        dst_ucp = (typename dst_env::ucp_t)unsigned_ucp;
        return true;
    }

    return false;
}

// Extended specialized cached character comparison.
template <typename leftCharType, typename rightCharType>
AINLINE bool IsCharacterEqualEx(
    leftCharType left, rightCharType right,
    const std::ctype <leftCharType>& leftFacet, const std::ctype <rightCharType>& rightFacet,
    bool caseSensitive
)
{
    bool isEqual = false;

    leftCharType real_left;
    rightCharType real_right;

    if ( caseSensitive )
    {
        real_left = left;
        real_right = right;
    }
    else
    {
        real_left = leftFacet.toupper( left );
        real_right = rightFacet.toupper( right );
    }

    auto unsigned_real_left = (typename std::make_unsigned <leftCharType>::type)real_left;
    auto unsigned_real_right = (typename std::make_unsigned <rightCharType>::type)real_right;

    isEqual = ( unsigned_real_left == unsigned_real_right );

    return isEqual;
}

// The main function of comparing characters.
template <typename leftCharType, typename rightCharType>
AINLINE bool IsCharacterEqual(
    leftCharType left, rightCharType right,
    bool caseSensitive
)
{
    const std::locale& classic_loc = std::locale::classic();
    
    const auto& leftFacet = std::use_facet <std::ctype <leftCharType>> ( classic_loc );
    const auto& rightFacet = std::use_facet <std::ctype <rightCharType>> ( classic_loc );

    return IsCharacterEqualEx( left, right, leftFacet, rightFacet, caseSensitive );
}

// Comparison of encoded strings.
template <typename srcCharType, typename dstCharType>
inline bool UniversalCompareStrings( const srcCharType *srcStr, size_t srcLen, const dstCharType *dstStr, size_t dstLen, bool caseSensitive )
{
    try
    {
        typedef character_env <srcCharType> src_char_env;
        typedef character_env <dstCharType> dst_char_env;

        typedef typename src_char_env::ucp_t src_ucp_t;
        typedef typename dst_char_env::ucp_t dst_ucp_t;

        const std::locale& classic_loc = std::locale::classic();
    
        const auto& srcFacet = std::use_facet <std::ctype <src_ucp_t>> ( classic_loc );
        const auto& dstFacet = std::use_facet <std::ctype <dst_ucp_t>> ( classic_loc );

        // Do a real unicode comparison.
        typename src_char_env::const_iterator srcIter( srcStr );
        typename dst_char_env::const_iterator dstIter( dstStr );

        size_t src_n = 0;
        size_t dst_n = 0;

        bool isSame = true;

        while ( true )
        {
            bool isSrcEnd = srcIter.IsEnd();
            bool isDstEnd = dstIter.IsEnd();

            if ( !isSrcEnd )
            {
                if ( src_n >= srcLen )
                {
                    isSrcEnd = true;
                }
            }

            if ( !isDstEnd )
            {
                if ( dst_n >= dstLen )
                {
                    isDstEnd = true;
                }
            }

            if ( isSrcEnd && isDstEnd )
            {
                break;
            }

            if ( isSrcEnd || isDstEnd )
            {
                isSame = false;
                break;
            }

            src_n += srcIter.GetIterateCount();
            dst_n += dstIter.GetIterateCount();

            src_ucp_t srcChar = srcIter.Resolve();
            dst_ucp_t dstChar = dstIter.Resolve();

            srcIter.Increment();
            dstIter.Increment();

            bool isCharSame = IsCharacterEqualEx( srcChar, dstChar, srcFacet, dstFacet, caseSensitive );

            if ( !isCharSame )
            {
                isSame = false;
                break;
            }
        }

        return isSame;
    }
    catch( std::bad_cast& )
    {
        // Something horrible happened.
        return false;
    }
}

namespace CharacterUtil
{
    template <typename charType>
    AINLINE const charType* GetDefaultConfErrorString( void )
    {
#ifdef _MSC_VER
        static_assert( false, "invalid character type in default conversion error string routine" );
#endif // Only available for the smart Visual Studio MSBUILD compiler.
    }

    template <>
    AINLINE const char* GetDefaultConfErrorString <char> ( void )
    {
        return "<codepoint_error>";
    }

    template <>
    AINLINE const wchar_t* GetDefaultConfErrorString <wchar_t> ( void )
    {
        return L"<codepoint_error>";
    }

    template <>
    AINLINE const char32_t* GetDefaultConfErrorString <char32_t> ( void )
    {
        return U"<codepoint_error>";
    }

    template <typename inputCharType, typename outputCharType>
    inline std::basic_string <outputCharType> ConvertStrings( const inputCharType *inputChars )
    {
        typedef character_env <inputCharType> input_env;
        typedef character_env <outputCharType> output_env;

        try
        {
            std::basic_string <outputCharType> output_str;

            // Convert things into the output.
            {
                typename input_env::const_iterator iter( inputChars );

                while ( !iter.IsEnd() )
                {
                    typename input_env::ucp_t codepoint = iter.ResolveAndIncrement();

                    // Encode it into the output string, if possible.
                    typename output_env::ucp_t enc_codepoint;

                    if ( AcquireDirectUCP <input_env, output_env> ( codepoint, enc_codepoint ) )
                    {
                        typename output_env::encoding_iterator enc_iter( &enc_codepoint, 1 );
                        {
                            typename output_env::enc_result result;

                            enc_iter.Resolve( result );

                            for ( size_t n = 0; n < result.numData; n++ )
                            {
                                output_str += result.data[ n ];
                            }
                        }
                    }
                    else
                    {
                        // We encode a failure.
                        output_str += GetDefaultConvFailureChar <outputCharType> ();
                    }
                }
            }

            return output_str;
        }
        catch( codepoint_exception& )
        {
            // On error we just return an error string.
            return GetDefaultConfErrorString <outputCharType> ();
        }
    }

    template <typename inputCharType, typename outputCharType>
    inline std::basic_string <outputCharType> ConvertStrings( const std::basic_string <inputCharType>& inputStr )
    {
        return ConvertStrings <inputCharType, outputCharType> ( inputStr.c_str() );
    }
};

#endif //_EIRREPO_CHARACTER_RESOLUTION_