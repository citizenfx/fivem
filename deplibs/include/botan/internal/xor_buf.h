/*
* XOR operations
* (C) 1999-2008 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_XOR_BUF_H__
#define BOTAN_XOR_BUF_H__

#include <botan/types.h>
#include <vector>

namespace Botan {

/**
* XOR arrays. Postcondition out[i] = in[i] ^ out[i] forall i = 0...length
* @param out the input/output buffer
* @param in the read-only input buffer
* @param length the length of the buffers
*/
template<typename T>
void xor_buf(T out[], const T in[], size_t length)
   {
   while(length >= 8)
      {
      out[0] ^= in[0]; out[1] ^= in[1];
      out[2] ^= in[2]; out[3] ^= in[3];
      out[4] ^= in[4]; out[5] ^= in[5];
      out[6] ^= in[6]; out[7] ^= in[7];

      out += 8; in += 8; length -= 8;
      }

   for(size_t i = 0; i != length; ++i)
      out[i] ^= in[i];
   }

/**
* XOR arrays. Postcondition out[i] = in[i] ^ in2[i] forall i = 0...length
* @param out the output buffer
* @param in the first input buffer
* @param in2 the second output buffer
* @param length the length of the three buffers
*/
template<typename T> void xor_buf(T out[],
                                  const T in[],
                                  const T in2[],
                                  size_t length)
   {
   while(length >= 8)
      {
      out[0] = in[0] ^ in2[0];
      out[1] = in[1] ^ in2[1];
      out[2] = in[2] ^ in2[2];
      out[3] = in[3] ^ in2[3];
      out[4] = in[4] ^ in2[4];
      out[5] = in[5] ^ in2[5];
      out[6] = in[6] ^ in2[6];
      out[7] = in[7] ^ in2[7];

      in += 8; in2 += 8; out += 8; length -= 8;
      }

   for(size_t i = 0; i != length; ++i)
      out[i] = in[i] ^ in2[i];
   }

#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK

template<>
inline void xor_buf<byte>(byte out[], const byte in[], size_t length)
   {
   while(length >= 8)
      {
      *reinterpret_cast<u64bit*>(out) ^= *reinterpret_cast<const u64bit*>(in);
      out += 8; in += 8; length -= 8;
      }

   for(size_t i = 0; i != length; ++i)
      out[i] ^= in[i];
   }

template<>
inline void xor_buf<byte>(byte out[],
                          const byte in[],
                          const byte in2[],
                          size_t length)
   {
   while(length >= 8)
      {
      *reinterpret_cast<u64bit*>(out) =
         *reinterpret_cast<const u64bit*>(in) ^
         *reinterpret_cast<const u64bit*>(in2);

      in += 8; in2 += 8; out += 8; length -= 8;
      }

   for(size_t i = 0; i != length; ++i)
      out[i] = in[i] ^ in2[i];
   }

#endif

template<typename Alloc, typename Alloc2>
void xor_buf(std::vector<byte, Alloc>& out,
             const std::vector<byte, Alloc2>& in,
             size_t n)
   {
   xor_buf(&out[0], &in[0], n);
   }

template<typename Alloc>
void xor_buf(std::vector<byte, Alloc>& out,
             const byte* in,
             size_t n)
   {
   xor_buf(&out[0], in, n);
   }

template<typename Alloc, typename Alloc2>
void xor_buf(std::vector<byte, Alloc>& out,
             const byte* in,
             const std::vector<byte, Alloc2>& in2,
             size_t n)
   {
   xor_buf(&out[0], &in[0], &in2[0], n);
   }

template<typename T, typename Alloc, typename Alloc2>
std::vector<T, Alloc>&
operator^=(std::vector<T, Alloc>& out,
           const std::vector<T, Alloc2>& in)
   {
   if(out.size() < in.size())
      out.resize(in.size());

   xor_buf(&out[0], &in[0], in.size());
   return out;
   }

}

#endif
