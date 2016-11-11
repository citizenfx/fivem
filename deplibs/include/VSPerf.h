//-----------------------------------------------------------------------------
//
//  File: VSPerf.h
//  Copyright (C) 2003 Microsoft Corporation
//  All rights reserved.
//
//  General header for profiling support.
//
//-----------------------------------------------------------------------------
#pragma once

#if !defined(VSPERF_NO_DEFAULT_LIB)
#pragma comment(lib, "VSPerf.lib")
#endif  VSPERF_NO_DEFAULT_LIB

#include <basetsd.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(PROFILERAPI)
#define PROFILERAPI _declspec(dllimport) _stdcall
#endif


typedef enum
{
    PROFILE_GLOBALLEVEL     = 1,
    PROFILE_PROCESSLEVEL    = 2,
    PROFILE_THREADLEVEL     = 3,
    PROFILE_RESERVED        = 4,

} PROFILE_CONTROL_LEVEL;

typedef enum
{
    //
    // Profile control return codes
    //

    // xxxProfile call successful
    PROFILE_OK                          = 0,

    // API or level,id combination not supported
    PROFILE_ERROR_NOT_YET_IMPLEMENTED   = 1,

    // Mode was 'never' when called
    PROFILE_ERROR_MODE_NEVER            = 2,

    // Level doesn't exist
    PROFILE_ERROR_LEVEL_NOEXIST         = 3,

    // ID doesn't exist
    PROFILE_ERROR_ID_NOEXIST            = 4,

    //
    // MarkProfile return codes
    //

    // Mark was taken successfully
    MARK_OK                             = 0,

    // Profiling was never when MarkProfile called
    MARK_ERROR_MODE_NEVER               = 1,

    // Profiling was off when MarkProfile called
    MARK_ERROR_MODE_OFF                 = 2,

    // Mark value passed is a reserved value
    MARK_ERROR_MARKER_RESERVED          = 3,

    // Comment text was truncated
    MARK_TEXTTOOLONG                    = 4,

    // No mark support in this context.
    MARK_ERROR_NO_SUPPORT               = 5,

    // no memory was available in which to record the event
    MARK_ERROR_OUTOFMEMORY              = 6,

    //
    // NameProfile return codes
    //

    // Name was registered sucessfullly
    NAME_OK                             = 0,

    // The name text was too long and was therefore truncated
    NAME_ERROR_TEXTTRUNCATED            = 1,

    // The given profile element has already been named
    NAME_ERROR_REDEFINITION             = 2,

    // level doesn't exist
    NAME_ERROR_LEVEL_NOEXIST            = 3,

    // id doesn't exist
    NAME_ERROR_ID_NOEXIST               = 4,

    // name does not meet the specification's requirements
    NAME_ERROR_INVALID_NAME             = 5,

    // no memory was available in which to record the event
    NAME_ERROR_OUTOFMEMORY              = 6,

    // the given operation is not supported
    NAME_ERROR_NO_SUPPORT               = 7,

    //
    // TokenName return codes
    //
    // Token was named sucessfullly
    TOKEN_NAME_OK                       = 0,

    // Profiling was never when NameToken called
    TOKEN_NAME_MODE_NEVER               = 1,
    
    // The token name was too long and was therefore truncated
    TOKEN_NAME_ERROR_TEXTTRUNCATED      = 2,

    // no memory was available in which to record the event
    TOKEN_NAME_ERROR_OUTOFMEMORY        = 3,

    // the given operation is not supported
    TOKEN_NAME_ERROR_NO_SUPPORT         = 4,

    //
    // SourceLine return codes
    //
    // Source line was reported sucessfullly
    SOURCE_LINE_OK                       = 0,

    // Profiling was never when SourceLine called
    SOURCE_LINE_MODE_NEVER               = 1,
    
    // no memory was available in which to record the event
    SOURCE_LINE_ERROR_OUTOFMEMORY        = 2,

    // the given operation is not supported
    SOURCE_LINE_ERROR_NO_SUPPORT         = 3,

} PROFILE_COMMAND_STATUS;

static const unsigned int PROFILE_CURRENTID = (unsigned int)-1;

#if defined(_WCHAR_T_DEFINED) || defined(_NATIVE_WCHAR_T_DEFINED)
#define VSPWCHAR wchar_t
#else
#define VSPWCHAR unsigned short
#endif


// Start/Stop Api's
PROFILE_COMMAND_STATUS PROFILERAPI StopProfile(PROFILE_CONTROL_LEVEL Level, unsigned int dwId);
PROFILE_COMMAND_STATUS PROFILERAPI StartProfile(PROFILE_CONTROL_LEVEL Level, unsigned int dwId);

// Suspend/Resume Api's
PROFILE_COMMAND_STATUS PROFILERAPI SuspendProfile(PROFILE_CONTROL_LEVEL  Level, unsigned int dwId);
PROFILE_COMMAND_STATUS PROFILERAPI ResumeProfile(PROFILE_CONTROL_LEVEL Level, unsigned int dwId);

// Mark Api's
PROFILE_COMMAND_STATUS PROFILERAPI MarkProfile(long lMarker);
PROFILE_COMMAND_STATUS PROFILERAPI CommentMarkProfileA(long lMarker, const char *szComment);
PROFILE_COMMAND_STATUS PROFILERAPI CommentMarkAtProfileA(__int64 dnTimestamp, long lMarker, const char *szComment);

PROFILE_COMMAND_STATUS PROFILERAPI CommentMarkProfileW(long lMarker, const VSPWCHAR *szComment);
PROFILE_COMMAND_STATUS PROFILERAPI CommentMarkAtProfileW(__int64 dnTimestamp, long lMarker, const VSPWCHAR *szComment);


// Named Profiling Elements Api's
PROFILE_COMMAND_STATUS PROFILERAPI NameProfileA(const char *pszName, PROFILE_CONTROL_LEVEL Level, unsigned int dwId);
PROFILE_COMMAND_STATUS PROFILERAPI NameProfileW(const VSPWCHAR *pszName, PROFILE_CONTROL_LEVEL Level, unsigned int dwId);

#ifdef UNICODE
#define CommentMarkAtProfile CommentMarkAtProfileW
#define CommentMarkProfile CommentMarkProfileW
#define NameProfile NameProfileW
#else
#define CommentMarkAtProfile CommentMarkAtProfileA
#define CommentMarkProfile CommentMarkProfileA
#define NameProfile NameProfileA
#endif // !UNICODE



#ifdef __cplusplus
}
#endif
