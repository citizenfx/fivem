// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// These numbers define the banner SDK version, and are the most significant numbers when ordering two engine versions (that is, a 4.12.* version is always 
// newer than a 4.11.* version, regardless of the changelist that it was built with)
#define EOS_MAJOR_VERSION	1
#define EOS_MINOR_VERSION	10
#define EOS_PATCH_VERSION	2

// Macros for encoding strings
#define EOS_VERSION_STRINGIFY_2(x) #x
#define EOS_VERSION_STRINGIFY(x) EOS_VERSION_STRINGIFY_2(x)

// Various strings used for engine resources
#define EOS_COMPANY_NAME  "Epic Games, Inc."
#define EOS_COPYRIGHT_STRING "Copyright Epic Games, Inc. All Rights Reserved."
#define EOS_PRODUCT_NAME "Epic Online Services SDK"
#define EOS_PRODUCT_IDENTIFIER "Epic Online Services SDK"

#if defined(BUILT_FROM_CHANGELIST)
#define EOS_VERSION_STRING                       \
	EOS_VERSION_STRINGIFY(EOS_MAJOR_VERSION) "." \
	EOS_VERSION_STRINGIFY(EOS_MINOR_VERSION) "." \
	EOS_VERSION_STRINGIFY(EOS_PATCH_VERSION) "-" \
	EOS_VERSION_STRINGIFY(BUILT_FROM_CHANGELIST)
#else
#define EOS_VERSION_STRING                       \
	EOS_VERSION_STRINGIFY(EOS_MAJOR_VERSION) "." \
	EOS_VERSION_STRINGIFY(EOS_MINOR_VERSION) "." \
	EOS_VERSION_STRINGIFY(EOS_PATCH_VERSION)
#endif

#ifndef RC_INVOKED

#include "eos_base.h"

/** Get the version of the EOSSDK binary */
EOS_DECLARE_FUNC(const char*) EOS_GetVersion(void);

#endif /* #ifndef RC_INVOKED */
