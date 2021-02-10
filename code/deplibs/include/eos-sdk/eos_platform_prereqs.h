// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/******************************************************************************
 *
 * Please refer to https://dev.epicgames.com/docs/services for more details
 * on platform specific development.
 *
 * Some platforms need additional setup before `eos_base.h` is called.
 * The `eos_base.h` header is included by all EOS SDK headers.  This header
 * file provides one way of allowing cross platform development with minimal
 * setup.
 *
 * To use this add the EOS_BUILD_PLATFORM_NAME define when building platforms
 * which need to include a `<Platform>/eos_<Platform>_base.h`. The value of
 * EOS_BUILD_PLATFORM_NAME will be placed in the `<Platform>` spot.  Not all
 * platforms need one of these base header files. Blank files are sitll provided
 * for the platforms that do not require it.
 *
 * Alternatives to this header are to have the necessary file explicitly
 * included.  This is a good solution if only a single platform is needed.
 * For example:
 *   #include "ThePlatform/eos_ThePlatform_base.h"
 *   #include "eos_sdk.h"
 *   #include "eos_friends_types.h"
 *
 * Another option is to use the macros provided by the custom compiler to
 * determine which include to use.
 * For example:
 *   #if defined(__THECOMPILER__)
 *   #include "ThePlatform/eos_ThePlatform_base.h"
 *   #elif defined(__SOMECOMPILER__)
 *   #include "SomePlatform/eos_SomePlatform_base.h"
 *   #endif
 *   #include "eos_sdk.h"
 *   #include "eos_friends_types.h"
 *
 ******************************************************************************/

#if defined(EOS_BUILD_PLATFORM_NAME)

#if defined(EOS_USE_DLLEXPORT) || defined(USE_CALL) || defined(EOS_MEMORY_CALL)
#error \
The macros EOS_MEMORY_CALL, EOS_CALL, and EOS_USE_DLLEXPORT where unexpectedly partially defined. \
This can occur if `eos_platform_prereqs.h` is included after `eos_base.h` is included. \
Please refer to https://dev.epicgames.com/docs/services for more details.
#endif

#define EOS_PREPROCESSOR_TO_STRING(x) EOS_PREPROCESSOR_TO_STRING_INNER(x)
#define EOS_PREPROCESSOR_TO_STRING_INNER(x) #x
#define EOS_PREPROCESSOR_JOIN(x,y) EOS_PREPROCESSOR_JOIN_INNER(x,y)
#define EOS_PREPROCESSOR_JOIN_INNER(x,y) x##y
#define EOS_BUILD_PLATFORM_HEADER_BASE EOS_PREPROCESSOR_TO_STRING(EOS_PREPROCESSOR_JOIN(EOS_BUILD_PLATFORM_NAME/eos_,EOS_PREPROCESSOR_JOIN(EOS_BUILD_PLATFORM_NAME,_base.h)))

#include EOS_BUILD_PLATFORM_HEADER_BASE

#endif

