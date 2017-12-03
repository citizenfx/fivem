/*
Copyright 2015 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// ETW (Event Tracing for Windows) profiling helpers.
// This allows easy insertion of Generic Event markers into ETW/xperf tracing
// which then aids in analyzing the traces and finding performance problems.
// The usage patterns are to use ETWBegin and ETWEnd (typically through the
// convenience class CETWScope) to bracket time-consuming operations. In addition
// ETWFrameMark marks the beginning of each frame, and ETWMark can be used to
// mark other notable events. More event types and providers can be added as needed.

#ifndef ETWPROF_H
#define ETWPROF_H
#if defined(_MSC_VER)
#pragma once
#endif

typedef long long int64;

#ifdef	_WIN32
// ETW support should be compiled in for all Windows PC platforms. It isn't
// supported on Windows XP but that is determined at run-time. This #define
// is used to let the code compile (but do nothing) on other operating systems.
#ifndef DISABLE_ETW_MARKS
#define	ETW_MARKS_ENABLED
#endif
#endif

// Flag to indicate that a mouse-down actually corresponds to a double-click.
// Add this to the button number.
const int kFlagDoubleClick = 100;

#ifdef	ETW_MARKS_ENABLED

// Define this when compiling the ETW provider functions into your own
// executable, not for export.
//#define STATIC_LINKED_ETWPROVIDERS

#if defined(STATIC_LINKED_ETWPROVIDERS)
	#define PLATFORM_INTERFACE
#elif defined(ETWPROVIDERSDLL)
	#define PLATFORM_INTERFACE __declspec(dllexport)
#else
	#define PLATFORM_INTERFACE __declspec(dllimport)
#endif
#include <sal.h> // For _Printf_format_string_

#ifdef __cplusplus
extern "C" {
#endif

// Insert a single event to mark a point in an ETW trace.
PLATFORM_INTERFACE void __cdecl ETWMark(_In_z_ PCSTR pMessage);
// Wide character mark function.
PLATFORM_INTERFACE void __cdecl ETWMarkW(_In_z_ PCWSTR pMessage);
// ETWWorkerMark is identical to ETWMark but goes through a different provider,
// for different grouping.
PLATFORM_INTERFACE void __cdecl ETWWorkerMark(_In_z_ PCSTR pMessage);

// Insert events with one or more generic int or float data fields
PLATFORM_INTERFACE void __cdecl ETWMark1I(_In_z_ PCSTR pMessage, int data1);
PLATFORM_INTERFACE void __cdecl ETWMark2I(_In_z_ PCSTR pMessage, int data1, int data2);
PLATFORM_INTERFACE void __cdecl ETWMark3I(_In_z_ PCSTR pMessage, int data1, int data2, int data3);
PLATFORM_INTERFACE void __cdecl ETWMark4I(_In_z_ PCSTR pMessage, int data1, int data2, int data3, int data4);
PLATFORM_INTERFACE void __cdecl ETWMark1F(_In_z_ PCSTR pMessage, float data1);
PLATFORM_INTERFACE void __cdecl ETWMark2F(_In_z_ PCSTR pMessage, float data1, float data2);
PLATFORM_INTERFACE void __cdecl ETWMark3F(_In_z_ PCSTR pMessage, float data1, float data2, float data3);
PLATFORM_INTERFACE void __cdecl ETWMark4F(_In_z_ PCSTR pMessage, float data1, float data2, float data3, float data4);

// _Printf_format_string_ is used by /analyze
PLATFORM_INTERFACE void __cdecl ETWMarkPrintf(_Printf_format_string_ _In_z_ PCSTR pMessage, ...);
PLATFORM_INTERFACE void __cdecl ETWMarkWPrintf(_Printf_format_string_ _In_z_ PCWSTR pMessage, ...);
PLATFORM_INTERFACE void __cdecl ETWWorkerMarkPrintf(_Printf_format_string_ _In_z_ PCSTR pMessage, ...);

// Private Working Set, Proportional Set Size (shared memory charged proportionally, and total Working Set
// counter is just a number that allows grouping together all the working set samples collected at the same time.
PLATFORM_INTERFACE void __cdecl ETWMarkWorkingSet(_In_z_ PCWSTR pProcessName, _In_z_ PCWSTR pProcess, unsigned counter, unsigned privateWS, unsigned PSS, unsigned workingSet);

// Record powerState (charging/discharging/AC), batteryPercentage (of total capacity) and
// discharge rate from struct BATTERY_STATUS.
PLATFORM_INTERFACE void __cdecl ETWMarkBatteryStatus(_In_z_ PCSTR powerState, float batteryPercentage, _In_z_ PCSTR rate);

// Record CPU frequency data as measured occasionally by running code. This can
// detect thermal throttling of all types on all processors, however the data is
// slightly noisy.
PLATFORM_INTERFACE void __cdecl ETWMarkCPUThrottling(float initialMHz, float measuredMHz, float promisedMHz, float percentage, _In_z_ PCWSTR status);

PLATFORM_INTERFACE void __cdecl ETWMarkPerfCounter(unsigned sampleNumber, _In_z_ PCWSTR pCounterName, double value);

// Record CPU/package frequency, power usage, and temperature. Currently Intel only.
PLATFORM_INTERFACE void __cdecl ETWMarkCPUFrequency(_In_z_ PCWSTR MSRName, double frequencyMHz);
PLATFORM_INTERFACE void __cdecl ETWMarkCPUPower(_In_z_ PCWSTR MSRName, double powerW, double energymWh);
PLATFORM_INTERFACE void __cdecl ETWMarkCPUTemp(_In_z_ PCWSTR MSRName, double tempC, double maxTempC);
PLATFORM_INTERFACE void __cdecl ETWMarkTimerInterval(double intervalMs);

// Insert a begin event to mark the start of some work. The return value is a 64-bit
// time stamp which should be passed to the corresponding ETWEnd function.
PLATFORM_INTERFACE int64 __cdecl ETWBegin(_In_z_ PCSTR pMessage);
PLATFORM_INTERFACE int64 __cdecl ETWWorkerBegin(_In_z_ PCSTR pMessage);

// Insert a paired end event to mark the end of some work.
PLATFORM_INTERFACE int64 __cdecl ETWEnd(_In_z_ PCSTR pMessage, int64 nStartTime);
PLATFORM_INTERFACE int64 __cdecl ETWWorkerEnd(_In_z_ PCSTR pMessage, int64 nStartTime);

// Mark the start of the next render frame.
PLATFORM_INTERFACE void __cdecl ETWRenderFrameMark();
// Return the frame number recorded in the ETW trace -- useful for synchronizing
// other profile information to the ETW trace.
PLATFORM_INTERFACE int __cdecl ETWGetRenderFrameNumber();

// Button numbers are 0, 1, 2 for left, middle, right, with kFlagDoubleClick added
// in for double clicks.
PLATFORM_INTERFACE void __cdecl ETWMouseDown(int nWhichButton, unsigned flags, int nX, int nY);
PLATFORM_INTERFACE void __cdecl ETWMouseUp(int nWhichButton, unsigned flags, int nX, int nY);
PLATFORM_INTERFACE void __cdecl ETWMouseMove(unsigned flags, int nX, int nY);
PLATFORM_INTERFACE void __cdecl ETWMouseWheel(unsigned flags, int zDelta, int nX, int nY);
PLATFORM_INTERFACE void __cdecl ETWKeyDown(unsigned nChar, _In_opt_z_ PCSTR keyName, unsigned nRepCnt, unsigned flags);

#ifdef __cplusplus
} // end of extern "C"

// This class calls the ETW Begin and End functions in order to insert a
// pair of events to bracket some work.
class CETWScope
{
public:
	CETWScope(_In_z_ PCSTR pMessage)
		: m_pMessage(pMessage)
	{
		m_nStartTime = ETWBegin(pMessage);
	}
	~CETWScope()
	{
		ETWEnd(m_pMessage, m_nStartTime);
	}
private:
	// Disable copying. Don't use "= delete" because this header
	// should work with older compilers like VC++ 2010.
	CETWScope(const CETWScope& rhs);
	CETWScope& operator=(const CETWScope& rhs);

	PCSTR m_pMessage;
	int64 m_nStartTime;
};
#endif // __cplusplus

#pragma comment(lib, "etwproviders64.lib")

#else

// Portability macros to allow compiling on non-Windows platforms

#ifndef _WIN32
#define PCSTR const char*
#define PCWSTR const wchar_t*
#endif

inline void ETWMark(PCSTR) {}
inline void ETWMarkW(PCWSTR) {}
inline void ETWWorkerMark(PCSTR) {}
inline void ETWMark1I(PCSTR, int) {}
inline void ETWMark2I(PCSTR, int, int) {}
inline void ETWMark3I(PCSTR, int, int, int) {}
inline void ETWMark4I(PCSTR, int, int, int, int) {}
inline void ETWMark1F(PCSTR, float) {}
inline void ETWMark2F(PCSTR, float, float) {}
inline void ETWMark3F(PCSTR, float, float, float) {}
inline void ETWMark4F(PCSTR, float, float, float, float) {}
inline void ETWMarkPrintf(PCSTR, ...) {}
inline void ETWMarkWPrintf(PCWSTR, ...) {}
inline void ETWWorkerMarkPrintf(PCSTR, ...) {}
inline void ETWMarkWorkingSet(PCWSTR, PCWSTR, unsigned, unsigned, unsigned, unsigned) {}
inline void ETWMarkBatteryStatus(PCSTR, float, PCSTR) {}
inline void ETWMarkCPUThrottling(float, float, float, float, PCWSTR) {}
inline void ETWMarkPerfCounter(unsigned, PCWSTR, double) {}
inline void ETWMarkCPUFrequency(PCWSTR, double) {}
inline void ETWMarkCPUPower(PCWSTR, double, double) {}
inline void ETWMarkCPUTemp(PCWSTR, double, double) {}
inline void ETWMarkTimerInterval(double) {}
inline int64 ETWBegin(PCSTR) { return 0; }
inline int64 ETWWorkerBegin(PCSTR) { return 0; }
inline int64 ETWEnd(PCSTR, int64) { return 0; }
inline int64 ETWWorkerEnd(PCSTR, int64) { return 0; }
inline void ETWRenderFrameMark() {}
inline int ETWGetRenderFrameNumber() { return 0; }

inline void ETWMouseDown(int, unsigned int, int, int) {}
inline void ETWMouseUp(int, unsigned int, int, int) {}
inline void ETWMouseMove(unsigned int, int, int) {}
inline void ETWMouseWheel(unsigned int, int, int, int) {}
inline void ETWKeyDown(unsigned, PCSTR, unsigned, unsigned) {}

#ifdef __cplusplus
// This class calls the ETW Begin and End functions in order to insert a
// pair of events to bracket some work.
class CETWScope
{
public:
	CETWScope(PCSTR)
	{
	}
private:
	// disable copying.
	CETWScope(const CETWScope& rhs) = delete;
	CETWScope& operator=(const CETWScope& rhs) = delete;
};
#endif // __cplusplus

#endif

#endif // ETWPROF_H
