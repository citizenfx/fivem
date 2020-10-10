/* -*- C++ -*- */

/*

  The Hoard Multiprocessor Memory Allocator
  www.hoard.org

  Author: Emery Berger, http://www.emeryberger.com
  Copyright (c) 1998-2020 Emery Berger

  See the LICENSE file at the top-level directory of this
  distribution and at http://github.com/emeryberger/Hoard.

*/

// Link this code with your executable to use winhoard.

#include <StdInc.h>

#if defined(_WIN32)

extern "C" {
DLL_IMPORT void* x_malloc(size_t sz);
DLL_IMPORT void x_free(void* ptr);
DLL_IMPORT void* x_realloc(void* ptr, size_t sz);
}

void* ::operator new(std::size_t count)
{
	return x_malloc(count);
}

void* ::operator new[](std::size_t count)
{
	return x_malloc(count);
}

void ::operator delete(void* ptr) noexcept
{
	return x_free(ptr);
}

void ::operator delete[](void* ptr) noexcept
{
	return x_free(ptr);
}

#include <windows.h>
//#include <iostream>

#if defined(_WIN64)
#pragma comment(linker, "/include:ReferenceHoard")
#else
#pragma comment(linker, "/include:_ReferenceHoard")
#endif

extern "C" {

  __declspec(dllimport) int ReferenceWinWrapperStub;
  
  void ReferenceHoard()
  {
    LoadLibraryA ("libhoard.dll");
#if 0
    if (lib == NULL) {
      std::cerr << "Startup error code = " << GetLastError() << std::endl;
      abort();
    }
#endif
    ReferenceWinWrapperStub = 1; 
  }

}

#endif
