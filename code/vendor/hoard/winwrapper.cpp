#include <string.h>

extern "C" {

  void * xxmalloc (size_t);
  void   xxfree (void *);

  // Takes a pointer and returns how much space it holds.
  size_t xxmalloc_usable_size (void *);

}

extern "C" {
  __declspec(dllexport) void * x_malloc (size_t sz) 
  {
    return xxmalloc(sz);
  }
  
  __declspec(dllexport) void x_free(void * ptr)
  {
	xxfree(ptr);
  }

  __declspec(dllexport) void * x_realloc (void * ptr, size_t sz) 
  {
    // null ptr = act like a malloc.
    if (ptr == nullptr) {
      return xxmalloc(sz);
    }

    // 0 size = free. We return a small object.  This behavior is
    // apparently required under Mac OS X and optional under POSIX.
    if (sz == 0) {
      xxfree (ptr);
      return xxmalloc(1);
    }

    auto originalSize = xxmalloc_usable_size (ptr);
    auto minSize = (originalSize < sz) ? originalSize : sz;

    // Don't change size if the object is shrinking by less than half.
    if ((originalSize / 2 < sz) && (sz <= originalSize)) {
      // Do nothing.
      return ptr;
    }

    auto * buf = xxmalloc (sz);

    if (buf != nullptr) {
      // Successful malloc.
      // Copy the contents of the original object
      // up to the size of the new block.
      memcpy (buf, ptr, minSize);
      xxfree (ptr);
    }

    // Return a pointer to the new one.
    return buf;
  }
  
  
}

extern "C" void InitializeWinWrapper() {}
extern "C" void FinalizeWinWrapper() {}

extern "C" __declspec(dllexport) int ReferenceWinWrapperStub = 0;
