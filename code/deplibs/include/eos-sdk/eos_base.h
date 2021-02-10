// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once


#if !defined(EOS_MEMORY_CALL) || !defined(EOS_CALL) || !defined(EOS_USE_DLLEXPORT)
#if !defined(_WIN32) && !defined(_WIN64) && !defined(__ANDROID__) && !defined(__linux__) && !defined(__APPLE__)
#error \
This platform expected a `eos_<platform>_base.h` include before this header. \
Please refer to https://dev.epicgames.com/docs/services or `eos_platform_prereqs.h` for details.
#endif
#endif

#ifndef EOS_USE_DLLEXPORT
#if defined(_WIN32) || defined(__CYGWIN__)
#define EOS_USE_DLLEXPORT 1
#else
#define EOS_USE_DLLEXPORT 0
#endif
#endif

#ifndef EOS_CALL
#if defined(_WIN32) && (defined(__i386) || defined(_M_IX86))
#define EOS_CALL __stdcall
#define EOS_MEMORY_CALL __stdcall
#else
#define EOS_CALL
#define EOS_MEMORY_CALL
#endif
#endif

#if !defined(EOS_MEMORY_CALL) || !defined(EOS_CALL) || !defined(EOS_USE_DLLEXPORT)
#error \
The expected macros EOS_MEMORY_CALL, EOS_CALL, and EOS_USE_DLLEXPORT where not all defined. \
Please refer to https://dev.epicgames.com/docs/services or `eos_platform_prereqs.h` for details.
#endif


#if defined(__cplusplus)
	#if defined(_MSC_VER) && _MSC_VER >= 1800
		/* Visual Studio 2013 or later */
		#include <cstdint>
	#else
		#include <stdint.h>
		#include <stddef.h>
	#endif

	#if __cplusplus >= 201103L
		#define EOS_HAS_ENUM_CLASS
	#elif defined(_MSC_VER) && defined(_MSVC_LANG) && _MSVC_LANG >= 201103L
		#define EOS_HAS_ENUM_CLASS
	#endif
#else
	/* C Compiler */
	#include <stdint.h>
	#include <stddef.h>
#endif


typedef int32_t EOS_Bool;
#define EOS_TRUE 1
#define EOS_FALSE 0


#if defined(EOS_BUILDING_SDK) && EOS_BUILDING_SDK > 0
	#if EOS_USE_DLLEXPORT
		#ifdef __GNUC__
			#define EOS_API __attribute__ ((dllexport))
		#else
			#define EOS_API __declspec(dllexport)
		#endif
	#else
		#if __GNUC__ >= 4
			#define EOS_API __attribute__ ((visibility ("default")))
		#else
			#define EOS_API
		#endif
	#endif

#else

	#if EOS_USE_DLLEXPORT
		#if defined(EOS_MONOLITHIC) && EOS_MONOLITHIC > 0
			#define EOS_API
		#elif defined(EOS_BUILD_DLL) && EOS_BUILD_DLL > 0
			#ifdef __GNUC__
				#define EOS_API __attribute__ ((dllexport))
			#else
				#define EOS_API __declspec(dllexport)
			#endif
		#else
			#ifdef __GNUC__
				#define EOS_API __attribute__ ((dllimport))
			#else
				#define EOS_API __declspec(dllimport)
			#endif
		#endif
	#else
		#if __GNUC__ >= 4
			#define EOS_API __attribute__ ((visibility ("default")))
		#else
			#define EOS_API
		#endif
	#endif
#endif

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#define EOS_DECLARE_FUNC(return_type) EXTERN_C EOS_API return_type EOS_CALL
#define EOS_DECLARE_CALLBACK(CallbackName, ...) EXTERN_C typedef void (EOS_CALL * CallbackName)(__VA_ARGS__)
#define EOS_DECLARE_CALLBACK_RETVALUE(ReturnType, CallbackName, ...) EXTERN_C typedef ReturnType (EOS_CALL * CallbackName)(__VA_ARGS__)
#define EOS_PASTE(...) __VA_ARGS__
#define EOS_STRUCT(struct_name, struct_def)           \
	EXTERN_C typedef struct _tag ## struct_name {     \
		EOS_PASTE struct_def                          \
	} struct_name


#ifdef EOS_HAS_ENUM_CLASS
#define EOS_ENUM_START(name) enum class name : int32_t {
#define EOS_ENUM_END(name) }
#else
#define EOS_ENUM_START(name) typedef enum name {
#define EOS_ENUM_END(name) , __##name##_PAD_INT32__ = 0x7FFFFFFF } name
#endif
#define EOS_ENUM(name, ...) EOS_ENUM_START(name) __VA_ARGS__ EOS_ENUM_END(name)


#ifdef EOS_HAS_ENUM_CLASS
#define EOS_ENUM_BOOLEAN_OPERATORS(name) \
/** A set of bitwise operators provided when the enum is provided as an `enum class`. */ \
inline constexpr name operator|(name Left, name Right) { return static_cast<name>((__underlying_type(name))Left | (__underlying_type(name))Right); } \
inline constexpr name operator&(name Left, name Right) { return static_cast<name>((__underlying_type(name))Left & (__underlying_type(name))Right); } \
inline constexpr name operator^(name Left, name Right) { return static_cast<name>((__underlying_type(name))Left ^ (__underlying_type(name))Right); } \
inline constexpr name& operator|=(name& Left, name Right) { return Left = Left | Right; } \
inline constexpr name& operator&=(name& Left, name Right) { return Left = Left & Right; } \
inline constexpr name& operator^=(name& Left, name Right) { return Left = Left ^ Right; } \
/**/
#else
#define EOS_ENUM_BOOLEAN_OPERATORS(name)
#endif


#undef EOS_HAS_ENUM_CLASS
