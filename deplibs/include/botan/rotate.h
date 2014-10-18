/*
* Word Rotation Operations
* (C) 1999-2008 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_WORD_ROTATE_H__
#define BOTAN_WORD_ROTATE_H__

#include <botan/types.h>

namespace Botan {

/**
* Bit rotation left
* @param input the input word
* @param rot the number of bits to rotate
* @return input rotated left by rot bits
*/
template<typename T> inline T rotate_left(T input, size_t rot)
   {
   if(rot == 0)
      return input;
   return static_cast<T>((input << rot) | (input >> (8*sizeof(T)-rot)));;
   }

/**
* Bit rotation right
* @param input the input word
* @param rot the number of bits to rotate
* @return input rotated right by rot bits
*/
template<typename T> inline T rotate_right(T input, size_t rot)
   {
   if(rot == 0)
      return input;
   return static_cast<T>((input >> rot) | (input << (8*sizeof(T)-rot)));
   }

}

#endif
