/*
* Memory Operations
* (C) 1999-2009,2012,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MEMORY_OPS_H_
#define BOTAN_MEMORY_OPS_H_

#include <botan/types.h>
#include <cstring>
#include <vector>

namespace Botan {

/**
* Allocate a memory buffer by some method. This should only be used for
* primitive types (uint8_t, uint32_t, etc).
*
* @param elems the number of elements
* @param elem_size the size of each element
* @return pointer to allocated and zeroed memory, or throw std::bad_alloc on failure
*/
BOTAN_PUBLIC_API(2,3) BOTAN_MALLOC_FN void* allocate_memory(size_t elems, size_t elem_size);

/**
* Free a pointer returned by allocate_memory
* @param p the pointer returned by allocate_memory
* @param elems the number of elements, as passed to allocate_memory
* @param elem_size the size of each element, as passed to allocate_memory
*/
BOTAN_PUBLIC_API(2,3) void deallocate_memory(void* p, size_t elems, size_t elem_size);

/**
* Ensure the allocator is initialized
*/
void initialize_allocator();

class Allocator_Initializer
   {
   public:
      Allocator_Initializer() { initialize_allocator(); }
   };

/**
* Scrub memory contents in a way that a compiler should not elide,
* using some system specific technique. Note that this function might
* not zero the memory (for example, in some hypothetical
* implementation it might combine the memory contents with the output
* of a system PRNG), but if you can detect any difference in behavior
* at runtime then the clearing is side-effecting and you can just
* use `clear_mem`.
*
* Use this function to scrub memory just before deallocating it, or on
* a stack buffer before returning from the function.
*
* @param ptr a pointer to memory to scrub
* @param n the number of bytes pointed to by ptr
*/
BOTAN_PUBLIC_API(2,0) void secure_scrub_memory(void* ptr, size_t n);

/**
* Memory comparison, input insensitive
* @param x a pointer to an array
* @param y a pointer to another array
* @param len the number of Ts in x and y
* @return 0xFF iff x[i] == y[i] forall i in [0...n) or 0x00 otherwise
*/
BOTAN_PUBLIC_API(2,9) uint8_t ct_compare_u8(const uint8_t x[],
                                            const uint8_t y[],
                                            size_t len);

/**
* Memory comparison, input insensitive
* @param x a pointer to an array
* @param y a pointer to another array
* @param len the number of Ts in x and y
* @return true iff x[i] == y[i] forall i in [0...n)
*/
inline bool constant_time_compare(const uint8_t x[],
                                  const uint8_t y[],
                                  size_t len)
   {
   return ct_compare_u8(x, y, len) == 0xFF;
   }

/**
* Zero out some bytes
* @param ptr a pointer to memory to zero
* @param bytes the number of bytes to zero in ptr
*/
inline void clear_bytes(void* ptr, size_t bytes)
   {
   if(bytes > 0)
      {
      std::memset(ptr, 0, bytes);
      }
   }

/**
* Zero memory before use. This simply calls memset and should not be
* used in cases where the compiler cannot see the call as a
* side-effecting operation (for example, if calling clear_mem before
* deallocating memory, the compiler would be allowed to omit the call
* to memset entirely under the as-if rule.)
*
* @param ptr a pointer to an array of Ts to zero
* @param n the number of Ts pointed to by ptr
*/
template<typename T> inline void clear_mem(T* ptr, size_t n)
   {
   clear_bytes(ptr, sizeof(T)*n);
   }

/**
* Copy memory
* @param out the destination array
* @param in the source array
* @param n the number of elements of in/out
*/
template<typename T> inline void copy_mem(T* out, const T* in, size_t n)
   {
   if(n > 0)
      {
      std::memmove(out, in, sizeof(T)*n);
      }
   }

template<typename T> inline void typecast_copy(uint8_t out[], T in)
   {
   std::memcpy(out, &in, sizeof(T));
   }

template<typename T> inline void typecast_copy(T& out, const uint8_t in[])
   {
   std::memcpy(&out, in, sizeof(T));
   }

template<typename T> inline void typecast_copy(T out[], const uint8_t in[], size_t N)
   {
   std::memcpy(out, in, sizeof(T)*N);
   }

/**
* Set memory to a fixed value
* @param ptr a pointer to an array
* @param n the number of Ts pointed to by ptr
* @param val the value to set each byte to
*/
template<typename T>
inline void set_mem(T* ptr, size_t n, uint8_t val)
   {
   if(n > 0)
      {
      std::memset(ptr, val, sizeof(T)*n);
      }
   }

inline const uint8_t* cast_char_ptr_to_uint8(const char* s)
   {
   return reinterpret_cast<const uint8_t*>(s);
   }

inline const char* cast_uint8_ptr_to_char(const uint8_t* b)
   {
   return reinterpret_cast<const char*>(b);
   }

inline uint8_t* cast_char_ptr_to_uint8(char* s)
   {
   return reinterpret_cast<uint8_t*>(s);
   }

inline char* cast_uint8_ptr_to_char(uint8_t* b)
   {
   return reinterpret_cast<char*>(b);
   }

/**
* Memory comparison, input insensitive
* @param p1 a pointer to an array
* @param p2 a pointer to another array
* @param n the number of Ts in p1 and p2
* @return true iff p1[i] == p2[i] forall i in [0...n)
*/
template<typename T> inline bool same_mem(const T* p1, const T* p2, size_t n)
   {
   volatile T difference = 0;

   for(size_t i = 0; i != n; ++i)
      difference |= (p1[i] ^ p2[i]);

   return difference == 0;
   }

/**
* XOR arrays. Postcondition out[i] = in[i] ^ out[i] forall i = 0...length
* @param out the input/output buffer
* @param in the read-only input buffer
* @param length the length of the buffers
*/
inline void xor_buf(uint8_t out[],
                    const uint8_t in[],
                    size_t length)
   {
   while(length >= 16)
      {
      uint64_t x0, x1, y0, y1;

      typecast_copy(x0, in);
      typecast_copy(x1, in + 8);
      typecast_copy(y0, out);
      typecast_copy(y1, out + 8);

      y0 ^= x0;
      y1 ^= x1;
      typecast_copy(out, y0);
      typecast_copy(out + 8, y1);
      out += 16; in += 16; length -= 16;
      }

   while(length > 0)
      {
      out[0] ^= in[0];
      out += 1;
      in += 1;
      length -= 1;
      }
   }

/**
* XOR arrays. Postcondition out[i] = in[i] ^ in2[i] forall i = 0...length
* @param out the output buffer
* @param in the first input buffer
* @param in2 the second output buffer
* @param length the length of the three buffers
*/
inline void xor_buf(uint8_t out[],
                    const uint8_t in[],
                    const uint8_t in2[],
                    size_t length)
   {
   while(length >= 16)
      {
      uint64_t x0, x1, y0, y1;
      typecast_copy(x0, in);
      typecast_copy(x1, in + 8);
      typecast_copy(y0, in2);
      typecast_copy(y1, in2 + 8);

      x0 ^= y0;
      x1 ^= y1;
      typecast_copy(out, x0);
      typecast_copy(out + 8, x1);
      out += 16; in += 16; in2 += 16; length -= 16;
      }

   for(size_t i = 0; i != length; ++i)
      out[i] = in[i] ^ in2[i];
   }

template<typename Alloc, typename Alloc2>
void xor_buf(std::vector<uint8_t, Alloc>& out,
             const std::vector<uint8_t, Alloc2>& in,
             size_t n)
   {
   xor_buf(out.data(), in.data(), n);
   }

template<typename Alloc>
void xor_buf(std::vector<uint8_t, Alloc>& out,
             const uint8_t* in,
             size_t n)
   {
   xor_buf(out.data(), in, n);
   }

template<typename Alloc, typename Alloc2>
void xor_buf(std::vector<uint8_t, Alloc>& out,
             const uint8_t* in,
             const std::vector<uint8_t, Alloc2>& in2,
             size_t n)
   {
   xor_buf(out.data(), in, in2.data(), n);
   }

template<typename Alloc, typename Alloc2>
std::vector<uint8_t, Alloc>&
operator^=(std::vector<uint8_t, Alloc>& out,
           const std::vector<uint8_t, Alloc2>& in)
   {
   if(out.size() < in.size())
      out.resize(in.size());

   xor_buf(out.data(), in.data(), in.size());
   return out;
   }

}

#endif
