// -*- C++ -*-

/*

  The Hoard Multiprocessor Memory Allocator
  www.hoard.org

  Author: Emery Berger, http://www.emeryberger.com
  Copyright (c) 1998-2020 Emery Berger

  See the LICENSE file at the top-level directory of this
  distribution and at http://github.com/emeryberger/Hoard.

*/

/*
 * This file leverages compiler support for thread-local variables for
 * access to thread-local heaps, when available. It also intercepts
 * thread completions to flush these local heaps, returning any unused
 * memory to the global Hoard heap. On Windows, this happens in
 * DllMain. On Unix platforms, we interpose our own versions of
 * pthread_create and pthread_exit.
 */
#if defined(_WIN32)

#include <new>

#pragma warning(disable: 4447) // Disable weird warning about threading model.

// Windows TLS functions.

#include "VERSION.h"

#define versionMessage "Using the Hoard memory allocator (http://www.hoard.org), version " HOARD_VERSION_STRING "\n"

#include "hoardheap.h"
#include "hoardtlab.h"

using namespace Hoard;

#define USE_DECLSPEC_THREADLOCAL 1

#if USE_DECLSPEC_THREADLOCAL
__declspec(thread) TheCustomHeapType * threadLocalHeap = NULL;
#else
DWORD LocalTLABIndex;
#endif

extern HoardHeapType * getMainHoardHeap();

static TheCustomHeapType * initializeCustomHeap()
{
  // Allocate a per-thread heap.
  auto * mainHeap = getMainHoardHeap();
  auto * customHeapBuf = mainHeap->malloc(sizeof(TheCustomHeapType));
  auto * perThreadHeap = new (customHeapBuf) TheCustomHeapType (mainHeap);

  // Store it in the appropriate thread-local area.
#if USE_DECLSPEC_THREADLOCAL
  threadLocalHeap = perThreadHeap;
#else
  TlsSetValue (LocalTLABIndex, perThreadHeap);
#endif

  return perThreadHeap;
}

bool isCustomHeapInitialized() {
#if USE_DECLSPEC_THREADLOCAL
  return (threadLocalHeap != NULL);
#else
  return (TlsGetValue(LocalTLABIndex) != NULL);
#endif
}

TheCustomHeapType * getCustomHeap() {
#if USE_DECLSPEC_THREADLOCAL
  if (threadLocalHeap != NULL)
    return threadLocalHeap;
  initializeCustomHeap();
  return threadLocalHeap;
#else
  auto p = TlsGetValue(LocalTLABIndex);
  if (p == NULL) {
    initializeCustomHeap();
    p = TlsGetValue(LocalTLABIndex);
  }
  return (TheCustomHeapType *) p;
#endif
}

extern "C" void InitializeWinWrapper();
extern "C" void FinalizeWinWrapper();


//
// Intercept thread creation and destruction to flush the TLABs.
//

#include <iostream>
using namespace std;

// example usage: debugMessage(L"Hello, world.\n");
void debugMessage(const wchar_t * lpBuff) {
  static bool initialized = false;
  if (!initialized) {
    AllocConsole();
  }
  DWORD dwSize = 0;
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), lpBuff, lstrlen(lpBuff), &dwSize, NULL);
}

extern "C" void _CRT_INIT();

extern "C" {

  BOOL APIENTRY DllMain (HANDLE hinstDLL,
			 DWORD fdwReason,
			 LPVOID lpreserved)
  {
    static auto np = HL::CPUInfo::computeNumProcessors();
    switch (fdwReason) {
      
    case DLL_PROCESS_ATTACH:
      {
	// Before we do anything, force initialization of the C++
	// library. Without this pre-initialization, the Windows heap
	// and the Hoard heaps get mixed up, and then nothing
	// works. This is quite the hack but seems to do the trick.
	// -- Emery Berger, 24/1/2019
	cout << "";

	// Now we are good to go.
	InitializeWinWrapper();
	// Force creation of the heap.
	volatile auto * ch = getCustomHeap();
      }
      break;
      
    case DLL_THREAD_ATTACH:
      {
	if (np == 1) {
	  // We have exactly one processor - just assign the thread to
	  // heap 0.
	  getMainHoardHeap()->chooseZero();
	} else {
	  getMainHoardHeap()->findUnusedHeap();
	}
	// Force creation of the heap.
	volatile auto * ch = getCustomHeap();
      }
      break;
      
    case DLL_THREAD_DETACH:
      {
	// Dump the memory from the TLAB.
	getCustomHeap()->clear();
	
#if USE_DECLSPEC_THREADLOCAL
	auto * heap = threadLocalHeap;
#else
	auto * heap = (TheCustomHeapType *) TlsGetValue(LocalTLABIndex);
#endif
	
	if (np != 1) {
	  // If we're on a multiprocessor box, relinquish the heap
	  // assigned to this thread.
	  getMainHoardHeap()->releaseHeap();
	}

      }
      break;
      
    case DLL_PROCESS_DETACH:
      FinalizeWinWrapper();
      break;
      
    default:
      return TRUE;
    }

    return TRUE;
  }
}

#endif // _WIN32
