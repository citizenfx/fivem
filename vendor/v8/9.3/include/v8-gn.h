// AUTOMATICALLY GENERATED. DO NOT EDIT.

// The following definitions were used when V8 itself was built, but also appear
// in the externally-visible header files and so must be included by any
// embedder. This will be done automatically if V8_GN_HEADER is defined.
// Ready-compiled distributions of V8 will need to provide this generated header
// along with the other headers in include.

// This header must be stand-alone because it is used across targets without
// introducing dependencies. It should only be included via v8config.h.

#ifndef V8_COMPRESS_POINTERS
#define V8_COMPRESS_POINTERS 1
#else
#if V8_COMPRESS_POINTERS != 1
#error "V8_COMPRESS_POINTERS defined but not set to 1"
#endif
#endif  // V8_COMPRESS_POINTERS

#ifndef V8_COMPRESS_POINTERS_IN_SHARED_CAGE
#define V8_COMPRESS_POINTERS_IN_SHARED_CAGE 1
#else
#if V8_COMPRESS_POINTERS_IN_SHARED_CAGE != 1
#error "V8_COMPRESS_POINTERS_IN_SHARED_CAGE defined but not set to 1"
#endif
#endif  // V8_COMPRESS_POINTERS_IN_SHARED_CAGE

#ifndef V8_31BIT_SMIS_ON_64BIT_ARCH
#define V8_31BIT_SMIS_ON_64BIT_ARCH 1
#else
#if V8_31BIT_SMIS_ON_64BIT_ARCH != 1
#error "V8_31BIT_SMIS_ON_64BIT_ARCH defined but not set to 1"
#endif
#endif  // V8_31BIT_SMIS_ON_64BIT_ARCH

#ifndef V8_DEPRECATION_WARNINGS
#define V8_DEPRECATION_WARNINGS 1
#else
#if V8_DEPRECATION_WARNINGS != 1
#error "V8_DEPRECATION_WARNINGS defined but not set to 1"
#endif
#endif  // V8_DEPRECATION_WARNINGS

#ifndef V8_IMMINENT_DEPRECATION_WARNINGS
#define V8_IMMINENT_DEPRECATION_WARNINGS 1
#else
#if V8_IMMINENT_DEPRECATION_WARNINGS != 1
#error "V8_IMMINENT_DEPRECATION_WARNINGS defined but not set to 1"
#endif
#endif  // V8_IMMINENT_DEPRECATION_WARNINGS

#ifndef CPPGC_CAGED_HEAP
#define CPPGC_CAGED_HEAP 1
#else
#if CPPGC_CAGED_HEAP != 1
#error "CPPGC_CAGED_HEAP defined but not set to 1"
#endif
#endif  // CPPGC_CAGED_HEAP

#ifdef V8_ENABLE_CHECKS
#error "V8_ENABLE_CHECKS is defined but is disabled by V8's GN build arguments"
#endif  // V8_ENABLE_CHECKS

#ifdef V8_COMPRESS_POINTERS_IN_ISOLATE_CAGE
#error "V8_COMPRESS_POINTERS_IN_ISOLATE_CAGE is defined but is disabled by V8's GN build arguments"
#endif  // V8_COMPRESS_POINTERS_IN_ISOLATE_CAGE

#ifdef V8_COMPRESS_ZONES
#error "V8_COMPRESS_ZONES is defined but is disabled by V8's GN build arguments"
#endif  // V8_COMPRESS_ZONES

#ifdef V8_HEAP_SANDBOX
#error "V8_HEAP_SANDBOX is defined but is disabled by V8's GN build arguments"
#endif  // V8_HEAP_SANDBOX

#ifdef V8_NO_ARGUMENTS_ADAPTOR
#error "V8_NO_ARGUMENTS_ADAPTOR is defined but is disabled by V8's GN build arguments"
#endif  // V8_NO_ARGUMENTS_ADAPTOR

#ifdef V8_USE_PERFETTO
#error "V8_USE_PERFETTO is defined but is disabled by V8's GN build arguments"
#endif  // V8_USE_PERFETTO

#ifdef V8_MAP_PACKING
#error "V8_MAP_PACKING is defined but is disabled by V8's GN build arguments"
#endif  // V8_MAP_PACKING

#ifdef V8_IS_TSAN
#error "V8_IS_TSAN is defined but is disabled by V8's GN build arguments"
#endif  // V8_IS_TSAN

#ifdef CPPGC_SUPPORTS_OBJECT_NAMES
#error "CPPGC_SUPPORTS_OBJECT_NAMES is defined but is disabled by V8's GN build arguments"
#endif  // CPPGC_SUPPORTS_OBJECT_NAMES

#ifdef CPPGC_YOUNG_GENERATION
#error "CPPGC_YOUNG_GENERATION is defined but is disabled by V8's GN build arguments"
#endif  // CPPGC_YOUNG_GENERATION
