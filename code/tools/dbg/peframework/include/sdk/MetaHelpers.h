/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        eirrepo/sdk/MetaHelpers.h
*  PURPOSE:     Memory management templates
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _COMMON_META_PROGRAMMING_HELPERS_
#define _COMMON_META_PROGRAMMING_HELPERS_

// Thanks to http://stackoverflow.com/questions/87372/check-if-a-class-has-a-member-function-of-a-given-signature
#include <type_traits>

#define INSTANCE_METHCHECKEX( checkName, methName ) \
    template<typename, typename T> \
    struct has_##checkName { \
        static_assert( \
            std::integral_constant<T, false>::value, \
            "Second template parameter needs to be of function type."); \
    }; \
    template<typename C, typename Ret, typename... Args> \
    struct has_##checkName##<C, Ret(Args...)> { \
    private: \
        template<typename T> \
        static constexpr auto check(T*) \
        -> typename \
            std::is_same< \
                decltype( std::declval<T>().##methName##( std::declval<Args>()... ) ), \
                Ret \
            >::type; \
        template<typename> \
        static constexpr std::false_type check(...); \
        typedef decltype(check<C>(0)) type; \
    public: \
        static constexpr bool value = type::value; \
    };

#define INSTANCE_METHCHECK( methName ) INSTANCE_METHCHECKEX( methName, methName )

#endif //_COMMON_META_PROGRAMMING_HELPERS_